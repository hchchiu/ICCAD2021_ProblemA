#include<iostream>
#include<fstream>
#include<sstream>
#include<time.h>
#include<string>
#include<cstring>
#include<vector>
#include<map>
#include<stack>
#include<set>
#include<algorithm>
using namespace std;

#define RANDOM_UNSIGNED ((((unsigned)rand()) << 24) ^ (((unsigned)rand()) << 12) ^ ((unsigned)rand()))
#define GetBit(p, i)  (((p)[(i)>>5]  & (1<<((i) & 31))) > 0)
#define SetBit(p, i)  ((p)[(i)>>5] |= (1<<((i) & 31)))
#define UnSetBit(p, i)  ((p)[(i)>>5] ^= (1<<((i) & 31)))
struct Node
{
	string name;
	vector<Node*> fanin;
	vector<Node*> fanout;
	set <Node*> piset;
	string graphName;//Graph name :R1,R2,G1
	unsigned* seeds;
	int type; //0:not 1:and 2:or 3:nand 4:nor 5:xor 6:xnor 7:buf 8:assign 9:PI 10:PO
	int realGate; // -1:default
};

struct Graph
{
	vector< Node* > netlist;
	vector< Node* > PI;
	vector< Node* > PO;
	map<string, Node*> PIMAP;//use PI's name to find its pointer
	string name;//Graph name :R1,R2,G1
};

struct MatchInfo
{
	map<Node*, Node*> matches; //success match pair (golden,origin)
	map<Node*, bool> originState; //(in this map represent it visited , true represent still in NodeSet)
	map<Node*, bool> goldenState; //(in this map represent it visited , true represent still in NodeSet)
	vector<Node*> originNode; //the remains is matched node need to save
	map<Node*, bool> originRemoveNode; //the content node is not match in origin and needed to remove from origin
	map<Node*, bool> goldenRemoveNode; //the content node is not match in golden and needed to create into origin
	vector<Node*> goldenNode; //the remains is matched
	set<Node*> originSupprotSet; //record support set
	set<Node*> goldenSupprotSet; //record support set
};

void loadFile(Graph& graph, char* argv);

// Convert verilog command to graph
// Also process node which fanin number larger than two,
// and split it into multi nodes each node only have two fanin
void verilog2graph(string& verilog_command, Graph& graph, vector<Node*>& assign_name);
// Process assign command type
void assignCommandTransform(string& verilog_command);
// Record PI/PO node info
void PiPoRecord(string str, Graph& graph);
// Initialize new node
Node* initialNewnode(string name, int type, string graphName);
// Select gate type
int selectGateType(string gate);

//run topological sort
void topologicalSort(Graph& graph);
//topological sort recursive
void topologicalSortUtil(Graph& graph, Node* node, map<Node*, bool>& visited, stack<Node*>& Stack);
//Set every node piset and bitwise operation seed
void setNodePIsetandSeed(Graph& graph);
//Set the random seed
void setRandomSeed(Graph& R1, Graph& R2, Graph& G1);
//return random seed
unsigned* getRandomSeed();
//BitWiseOperation
void BitWiseOperation(vector<unsigned*>& fainSeed, Node* currNode);


// Match pair by using structure compare
void structureCompareMain(Graph origin, Graph golden, MatchInfo& matchInfo);
// structure compare operation
void structureCompareOper(Node* origin, Node* golden, MatchInfo& matchInfo);
// check gate is equal
bool IsGateTypeEqual(Node* origin, Node* golden);
// check fanin is equal
bool IsFaninEqual(Node* origin, Node* golden, MatchInfo matchInfo);
// check this node fanin whether visited or not
bool IsFaninVisited(Node* ptr, map<Node*, bool> maps);
// check this node whether visited or not
bool IsVisited(Node* target, map<Node*, bool>maps);

//transfer graph to blif file and write blif file
void graph2Blif(Graph& path_original, Graph& path_golden);
//find the node's fanout and call node2Blif 
void netlist2Blif(ofstream& outfile, vector<Node*>& netlist, map<Node*, bool>& visited);
//write gate type ex: and gate -> 11 1
void node2Blif(ofstream& outfile, Node* currNode);
//let original POs and Golden POs connet to the XOR to make the miter
void buildMiter(ofstream& outfile, vector<Node*>& PO_original, vector<Node*>PO_golden);


// Output patch
void outFile(Graph graph, char* argv);
// Process same node name but multi Bracket(like op[0],op[1],op[2],.....) and count name times
void strExtractBracket(Graph graph, vector<Node>& pi_items, vector<Node>& po_items);
// Output declare type(input/output/wire)
void outputFront(ofstream& outfile, vector<Node> p, string str);
// Output gate
void gateOutput(ofstream& outfile, Graph graph);
// Sort by type and number count
bool typeAndNumberCompare(const Node& p1, const Node& p2);
// Sort by node name
bool strTitleCompare(const string& p1, const string& p2);


void createRectifyPair(Graph& R2, Graph& G1);
bool pisetIsDifferent(Node object, Node golden);
bool seedIsDifferent(Node object, Node golden);

/* Function Flow
    ----------------------------------------------------------
	loadFile  ->   verilog2graph  ->   assignCommandTransform 
						.		  ->   initialNewnode
						.         ->   selectGateType
						.         ->   PiPoRecord
   ----------------------------------------------------------
	   |
   ----------------------------------
	setRandomSeed  ->   getRandomSeed
   ----------------------------------
	   |
   -----------------------------------------
	topologicalSort  ->  topologicalSortUtil
   -----------------------------------------
	   |
   ------------------------------------------
	setNodePIsetandSeed  ->  BitWiseOperation
   ------------------------------------------
	   |
   ------------------------------------------------------------------------------------
	structureCompareMain  ->   structureCompareOper	 ->  IsGateTypeEqual
										.		     ->  IsFaninEqual
										.		     ->  IsVisited
										.		     ->  IsFaninVisited	 ->  IsVisited
   ------------------------------------------------------------------------------------
	   |
   -------------------------------------------
   graph2Blif  ->  netlist2Blif  ->  node2Blif
		.	   ->  buildMiter
   -------------------------------------------
*/


int nWords = 1;
// inpurt format
//./eco R1.v R2.v G1.v patch.v 
int main(int argc, char* argv[])
{
	srand(time(NULL));
	//original netlist
	Graph R1;
	R1.name = "R1";
	//golden netlist
	Graph R2;
	R2.name = "R2";
	//optimize netlist
	Graph G1;
	G1.name = "G1";


	//Load R1 original netlist
	//assign value is fault, need to retrieve fan_in value in next step
	loadFile(R1, argv[1]);
	//Load R2 golden netlist
	//assign value is fault, need to retrieve fan_in value in next step
	loadFile(R2, argv[2]);
	//Load G1 optimize netlist
	//assign value is fault, need to retrieve fan_in value in next step
	loadFile(G1, argv[3]);

	//Set random seed to PI
	setRandomSeed(R1, R2, G1);

	////Start topological sort
	//In order to set ID and PI
	topologicalSort(R1);
	topologicalSort(R2);
	topologicalSort(G1);

	//Set all nodes Primary Input
	setNodePIsetandSeed(R1);
	setNodePIsetandSeed(R2);
	setNodePIsetandSeed(G1);



	MatchInfo matchInfo;
	structureCompareMain(G1, R2, matchInfo);
	int k = 0;

	graph2Blif(G1, R2);

}

void loadFile(Graph& graph, char* argv)
{
	/*
	ifstream infileR1("r1.v"); //replace by argv[1]
	ifstream infileR2("r2.v"); //replace by argv[2]
	ifstream infileG1("g1.v"); //replace by argv[3]
	*/

	ifstream infile(argv); //replace by argv[]
	vector<Node*> assign; //optimize assign name

	string verilog_command;
	//load R1.v
	while (1) {
		getline(infile, verilog_command, ';');

		//if this command include "endmodule" then break while loop
		if (verilog_command.find("endmodule") != string::npos)
			break;
		verilog2graph(verilog_command, graph, assign);
		getline(infile, verilog_command); //dicard \n
	}
}
void verilog2graph(string& verilog_command, Graph& graph, vector<Node*>& assign_name)
{
	//delete front blank
	while (verilog_command[0] == ' ')
		verilog_command = verilog_command.substr(1, verilog_command.size() - 1);

	//if this command include "module" then return
	if (verilog_command.find("module") != string::npos)
		return;
	//if this command include "wire" then return
	if (verilog_command.find("wire") != string::npos || verilog_command.find("WIRE") != string::npos)
		return;
	//if this command include "input" then record
	if (verilog_command.find("input") != string::npos || verilog_command.find("INPUT") != string::npos) {
		PiPoRecord(verilog_command, graph);
		return;
	}
	//if this command include "output" then record
	if (verilog_command.find("output") != string::npos || verilog_command.find("OUTPUT") != string::npos) {
		PiPoRecord(verilog_command, graph);
		return;
	}

	//solve "assign" type special conversion
	if (verilog_command.find("assign") != string::npos || verilog_command.find("ASSIGN") != string::npos)
		assignCommandTransform(verilog_command);

	int type, count = 1;
	string split_command;
	Node* currGate = NULL;
	stringstream ss;
	ss << verilog_command;

	//return corresponding gate type id
	getline(ss, split_command, ' ');
	type = selectGateType(split_command);


	getline(ss, split_command, '(');
	while (getline(ss, split_command, ',')) {
		bool isexist = false;
		Node* scanNode = NULL;
		while (split_command[0] == ' ') //delete front blank
			split_command = split_command.substr(1, split_command.size() - 1);
		if (split_command[split_command.size() - 1] == ')') //delete right parentheses
			split_command = split_command.substr(0, split_command.size() - 1);

		// search this whether exist
		for (int i = 0; i < graph.netlist.size(); i++)
			if (graph.netlist[i]->name == split_command)
			{
				isexist = true;
				scanNode = graph.netlist[i];
				if (count)
					currGate = graph.netlist[i];
				break;
			}


		if (!isexist) {  // not exist
			Node* req = initialNewnode(split_command, -1, graph.name);
			graph.netlist.push_back(req);
			if (count) {  //first node
				currGate = req;
				req->type = type;
				count = 0;
			}
			else {
				//fanin node
				// >2 fanin split to 2 fanin gate
				req->fanout.push_back(currGate);
				if (currGate->fanin.size() == 2) {
					Node* n1 = currGate->fanin[0];
					Node* n2 = currGate->fanin[1];
					Node* newnode = initialNewnode(n1->name + "_" + n2->name, currGate->type, graph.name);
					for (int i = 0; i < n1->fanout.size(); i++)
						if (n1->fanout[i]->name == currGate->name)
							n1->fanout[i] = newnode;
					for (int i = 0; i < n2->fanout.size(); i++)
						if (n2->fanout[i]->name == currGate->name)
							n2->fanout[i] = newnode;
					newnode->fanin.push_back(n1);
					newnode->fanin.push_back(n2);
					newnode->fanout.push_back(currGate);
					currGate->fanin.clear();
					currGate->fanin.push_back(newnode);
					graph.netlist.push_back(newnode);
				}
				currGate->fanin.push_back(req);
			}
		}
		else { //is exist
			if (count) { //first node
				if (currGate->type == 9 || currGate->type == 10)
					currGate->realGate = type;
				else if (currGate->type == -1)
					currGate->type = type;
				count = 0;
			}
			else {
				// its fanin node
				// > 2 fanin split to 2 fanin gate
				scanNode->fanout.push_back(currGate);
				if (currGate->fanin.size() == 2) {
					Node* n1 = currGate->fanin[0];
					Node* n2 = currGate->fanin[1];
					Node* newnode = initialNewnode(n1->name + "_" + n2->name, currGate->type, graph.name);
					for (int i = 0; i < n1->fanout.size(); i++)
						if (n1->fanout[i]->name == currGate->name)
							n1->fanout[i] = newnode;
					for (int i = 0; i < n2->fanout.size(); i++)
						if (n2->fanout[i]->name == currGate->name)
							n2->fanout[i] = newnode;
					newnode->fanin.push_back(n1);
					newnode->fanin.push_back(n2);
					newnode->fanout.push_back(currGate);
					currGate->fanin.clear();
					currGate->fanin.push_back(newnode);
					graph.netlist.push_back(newnode);
				}
				currGate->fanin.push_back(scanNode);
			}
		}
	}
}
void assignCommandTransform(string& verilog_command) {
	verilog_command.insert(7, "(");
	verilog_command.insert(verilog_command.size(), ")");
	if (verilog_command.find(" = ") != string::npos) {
		int pos = verilog_command.find(" = ");
		verilog_command.replace(pos, 3, ","); // (position,offset,replace_str)
	}
}
void PiPoRecord(string str, Graph& graph)
{
	string type = str.substr(0, 6);
	str = str.substr(6, str.size() - 1);
	stringstream ss;
	string split_command;
	if (str.find(':') != string::npos) { //have multi input
		stringstream kk;
		int max_index, min_index;
		ss << str;
		getline(ss, split_command, '[');
		getline(ss, split_command, ':');
		kk << split_command;
		kk >> max_index;
		getline(ss, split_command, ']');
		stringstream cc;
		cc << split_command;
		cc >> min_index;

		while (getline(ss, split_command, ',')) {
			if (split_command[0] == ' ') //delete front blank
				split_command = split_command.substr(1, split_command.size() - 1);
			for (int i = min_index; i <= max_index; i++) {
				stringstream ss;
				ss << i;
				Node* req = new Node();
				req->name = split_command + "[" + ss.str() + "]";
				req->realGate = -1;
				if (type == "input ") {
					graph.PIMAP[req->name] = req;
					req->type = 9;
					req->seeds = new unsigned[nWords];
					;					graph.netlist.push_back(req);
					graph.PI.push_back(req);
					//req->piset.push_back(req);
					req->piset.insert(req);
				}
				else if (type == "output") {
					req->type = 10;
					req->seeds = new unsigned[nWords];
					req->graphName = graph.name;
					graph.netlist.push_back(req);
					graph.PO.push_back(req);
				}
			}
		}
	}
	else {
		ss << str;
		while (getline(ss, split_command, ',')) {
			while (split_command[0] == ' ') //delete front blank
				split_command = split_command.substr(1, split_command.size() - 1);
			Node* req = new Node();
			req->name = split_command;
			req->realGate = -1;

			if (type == "input ") {
				graph.PIMAP[req->name] = req;
				req->type = 9;
				req->seeds = new unsigned[nWords];
				graph.netlist.push_back(req);
				graph.PI.push_back(req);
				//req->piset.push_back(req);
				req->piset.insert(req);
			}
			else if (type == "output") {
				req->type = 10;
				req->seeds = new unsigned[nWords];
				req->graphName = graph.name;
				graph.netlist.push_back(req);
				graph.PO.push_back(req);
			}
		}
	}
}
int selectGateType(string gate)
{
	if (gate == "NOT" || gate == "not") return 0;
	else if (gate == "AND" || gate == "and") return 1;
	else if (gate == "OR" || gate == "or") return 2;
	else if (gate == "NAND" || gate == "nand") return 3;
	else if (gate == "NOR" || gate == "nor") return 4;
	else if (gate == "XOR" || gate == "xor") return 5;
	else if (gate == "XNOR" || gate == "xnor") return 6;
	else if (gate == "BUF" || gate == "buf") return 7;
	else if (gate == "ASSIGN" || gate == "assign") return 8;
	else if (gate == "input")return 9;
	else if (gate == "output")return 10;
}
Node* initialNewnode(string name, int type, string graphName) {
	Node* newnode = new Node();
	newnode->name = name;
	newnode->realGate = -1;
	newnode->type = type;
	newnode->seeds = new unsigned[nWords];
	newnode->graphName = graphName;
	return newnode;
}

void setRandomSeed(Graph& R1, Graph& R2, Graph& G1)
{
	for (int i = 0; i < R1.PI.size(); ++i) {
		//get the PI name
		string PIname = R1.PI[i]->name;

		//get the random seed memory location
		unsigned* seed = getRandomSeed();

		//set the random seed
		R1.PIMAP[PIname]->seeds = seed;
		R2.PIMAP[PIname]->seeds = seed;
		G1.PIMAP[PIname]->seeds = seed;
	}
}

void topologicalSort(Graph& graph)
{
	stack<Node*> Stack;
	map<Node*, bool> visited;

	vector<Node*>::iterator it1 = graph.netlist.begin();
	for (; it1 != graph.netlist.end(); ++it1)
		visited[*it1] = false;

	/*
	map<Node*, bool>::iterator it2 = visited.begin();
	for (; it2 != visited.end(); ++it2) {
		if (!it2->second)
			topologicalSortUtil(graph, it2->first, visited, Stack);
	}
	*/

	for (int i = 0; i < graph.PI.size(); ++i) {
		if (!visited[graph.PI[i]])
			topologicalSortUtil(graph, graph.PI[i], visited, Stack);
	}

	int pos = 0;

	vector<Node*> sortNode;
	sortNode.resize(Stack.size());
	while (Stack.empty() == false) {
		//cout << Stack.top() << " ";
		sortNode[pos++] = Stack.top();
		Stack.pop();
	}
	graph.netlist = sortNode;
}
void topologicalSortUtil(Graph& graph, Node* node, map<Node*, bool>& visited, stack<Node*>& Stack)
{
	//Mark the current node as visited
	visited[node] = true;
	for (int i = 0; i < node->fanout.size(); ++i) {
		Node* nextNode = node->fanout[i];
		if (!visited[nextNode])
			topologicalSortUtil(graph, nextNode, visited, Stack);
	}
	Stack.push(node);
}
void setNodePIsetandSeed(Graph& graph)
{
	for (int i = 0; i < graph.netlist.size(); ++i) {
		Node* currNode = graph.netlist[i];
		if (currNode->fanin.size() > 0) {
			vector<unsigned*> faninSeed;
			for (int j = 0; j < currNode->fanin.size(); ++j) {
				//set the PI to the PISET
				Node* faninNode = currNode->fanin[j];
				currNode->piset.insert(faninNode->piset.begin(), faninNode->piset.end());

				//record fanin seed
				faninSeed.push_back(faninNode->seeds);
			}
			BitWiseOperation(faninSeed, currNode);
		}
	}
}

void BitWiseOperation(vector<unsigned*>& faninSeed, Node* currNode)
{
	//0:not 1:and 2:or 3:nand 4:nor 5:xor 6:xnor 7:buf 8:assign 9:PI 10:PO
	int type;
	if (currNode->type == 10 || currNode->type == 9)
		type = currNode->realGate;
	else
		type = currNode->type;

	if (faninSeed.size() > 1) {
		//and gate
		if (type == 1)
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = faninSeed[0][i] & faninSeed[1][i];
		//or gate
		else if (type == 2)
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = faninSeed[0][i] | faninSeed[1][i];
		//nand gate
		else if (type == 3)
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = ~(faninSeed[0][i] & faninSeed[1][i]);
		//nor gate
		else if (type == 4)
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = ~(faninSeed[0][i] | faninSeed[1][i]);
		//xor gate
		else if (type == 5)
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = faninSeed[0][i] ^ faninSeed[1][i];
		//xnor gate
		else if (type == 6)
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = ~(faninSeed[0][i] ^ faninSeed[1][i]);
	}
	else {
		//not gate
		if (type == 0)
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = ~faninSeed[0][i];
		//buffer or assign
		else if (type == 7 || type == 8)
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = faninSeed[0][i];

	}
}

unsigned* getRandomSeed()
{
	//32bits
	unsigned* bw = new unsigned[nWords];
	for (int i = 0; i < nWords; i++) bw[i] = RANDOM_UNSIGNED;
	/*
	for (int i = 0; i < nWords; i++)
		for (int k = 0; k < 32; k++)
			cout << GetBit(bw, k + (i * 32));
	*/
	return bw;
}


void structureCompareMain(Graph origin, Graph golden, MatchInfo& matchInfo)
{
	for (int i = 0; i < golden.PI.size(); i++) {
		// put (golden , origin) PI in matches 
		matchInfo.matches[golden.PI[i]] = origin.PI[i];
		matchInfo.originState[origin.PI[i]] = true;
		matchInfo.goldenState[golden.PI[i]] = true;
		matchInfo.originSupprotSet.insert(origin.PI[i]);
		matchInfo.goldenSupprotSet.insert(golden.PI[i]);
	}

	for (int i = 0; i < origin.netlist.size(); i++)
		matchInfo.originNode.push_back(origin.netlist[i]);
	for (int i = 0; i < golden.netlist.size(); i++)
		matchInfo.goldenNode.push_back(golden.netlist[i]);

	int originIndex = 0, goldenIndex = 0;
	while (true) {
		bool ending = false;
		Node* originPtr = matchInfo.originNode[originIndex];
		Node* goldenPtr = matchInfo.goldenNode[goldenIndex];
		while (matchInfo.originState.find(originPtr) != matchInfo.originState.end() && matchInfo.originState[originPtr] == false) {
			matchInfo.originNode.erase(matchInfo.originNode.begin() + originIndex);
			if (originIndex < matchInfo.originNode.size())
				originPtr = matchInfo.originNode[originIndex];
			else {
				ending = true;
				break;
			}
		}
		while (matchInfo.goldenState.find(goldenPtr) != matchInfo.goldenState.end() && matchInfo.goldenState[goldenPtr] == false) {
			matchInfo.goldenNode.erase(matchInfo.goldenNode.begin() + goldenIndex);
			if (goldenIndex < matchInfo.goldenNode.size())
				goldenPtr = matchInfo.goldenNode[goldenIndex];
			else {
				ending = true;
				break;
			}
		}
		if (!ending) {
			structureCompareOper(originPtr, goldenPtr, matchInfo);
			originIndex++; goldenIndex++;
		}
		if (originIndex >= matchInfo.originNode.size() || goldenIndex >= matchInfo.goldenNode.size()) {
			int minisize = matchInfo.originNode.size();
			if (matchInfo.originNode.size() > matchInfo.goldenNode.size())
				minisize = matchInfo.goldenNode.size();
			while (matchInfo.originNode.size() > minisize) {
				Node* ptr = matchInfo.originNode[matchInfo.originNode.size() - 1];
				if (matchInfo.originState.find(ptr) != matchInfo.originState.end() && matchInfo.originState[ptr] == false)
					matchInfo.originNode.pop_back();
				else {
					matchInfo.originState[ptr] = false;
					matchInfo.originRemoveNode[ptr] = true;
					matchInfo.originNode.pop_back();
				}
			}
			while (matchInfo.goldenNode.size() > minisize) {
				Node* ptr = matchInfo.goldenNode[matchInfo.goldenNode.size() - 1];
				if (matchInfo.goldenState.find(ptr) != matchInfo.goldenState.end() && matchInfo.goldenState[ptr] == false)
					matchInfo.goldenNode.pop_back();
				else {
					matchInfo.goldenState[ptr] = false;
					matchInfo.goldenRemoveNode[ptr] = true;
					matchInfo.goldenNode.pop_back();
				}
			}
			break;
		}
	}
	set<Node*>::iterator req = matchInfo.originSupprotSet.begin();
	for (set<Node*>::iterator it = matchInfo.originSupprotSet.begin(); it != matchInfo.originSupprotSet.end(); it = req) {
		++req;
		bool updates = true;
		Node* accesses = *it;
		for (int m = 0; m < accesses->fanout.size(); m++)
			if (matchInfo.originRemoveNode.find(accesses->fanout[m]) != matchInfo.originRemoveNode.end()) {
				updates = false;
				break;
			}
		if (updates) {
			for (int m = 0; m < accesses->fanout.size(); m++)
				matchInfo.originSupprotSet.insert(accesses->fanout[m]);
			matchInfo.originSupprotSet.erase(it);
		}
	}
	req = matchInfo.goldenSupprotSet.begin();
	for (set<Node*>::iterator it = matchInfo.goldenSupprotSet.begin(); it != matchInfo.goldenSupprotSet.end(); it = req) {
		++req;
		bool updates = true;
		Node* accesses = *it;
		for (int m = 0; m < accesses->fanout.size(); m++)
			if (matchInfo.matches.find(accesses->fanout[m]) == matchInfo.matches.end()) {
				updates = false;
				break;
			}
		if (updates) {
			for (int m = 0; m < accesses->fanout.size(); m++)
				matchInfo.goldenSupprotSet.insert(accesses->fanout[m]);
			matchInfo.goldenSupprotSet.erase(it);
		}
	}
	/*for (int i = 0; i < matchInfo.originSupprotSet.size(); i++) {
		Node* ptr = matchInfo.originSupprotSet[i];
		bool updates = true;
		for (int m = 0; m < ptr->fanout.size(); m++)
			if (matchInfo.originRemoveNode.find(ptr->fanout[m]) != matchInfo.originRemoveNode.end()) {
				updates = false;
				break;
			}
		if (updates) {
			matchInfo.originSupprotSet.erase(ptr);
			for (int m = 0; m < ptr->fanout.size(); m++)
				matchInfo.originSupprotSet.insert(ptr->fanout[m]);
		}
	}
	for (int i = 0; i < matchInfo.goldenNode.size(); i++) {
		Node* ptr = matchInfo.goldenNode[i];
		bool updates = true;
		for (int m = 0; m < ptr->fanout.size(); m++)
			if (matchInfo.matches.find(ptr->fanout[m]) == matchInfo.matches.end()) {
				updates = false;
				break;
			}
		if (updates) {
			matchInfo.goldenSupprotSet.erase(ptr);
			for (int m = 0; m < ptr->fanout.size(); m++)
				matchInfo.goldenSupprotSet.insert(ptr->fanout[m]);
		}
	}*/
	int k = 0;
}

void structureCompareOper(Node* origin, Node* golden, MatchInfo& matchInfo)
{
	vector<Node*> originMatch;
	vector<Node*> goldenMatch;
	map<Node*, bool> usingOrigin;
	map<Node*, bool> usingGolden;
	for (int i = 0; i < origin->fanout.size(); i++) {
		if (IsVisited(origin->fanout[i], matchInfo.originState))
			continue;
		if (IsFaninVisited(origin->fanout[i], matchInfo.originState)) {
			originMatch.push_back(origin->fanout[i]);
			matchInfo.originState[origin->fanout[i]] = true;
			usingOrigin[origin->fanout[i]] = false;
		}
	}
	for (int i = 0; i < golden->fanout.size(); i++) {
		if (IsVisited(golden->fanout[i], matchInfo.goldenState))
			continue;
		if (IsFaninVisited(golden->fanout[i], matchInfo.goldenState)) {
			goldenMatch.push_back(golden->fanout[i]);
			matchInfo.goldenState[golden->fanout[i]] = true;
			usingGolden[golden->fanout[i]] = false;
		}
	}
	for (int i = 0; i < originMatch.size(); i++) {
		Node* ptr1 = originMatch[i];
		for (int m = 0; m < goldenMatch.size(); m++) {
			Node* ptr2 = goldenMatch[m];
			if (usingOrigin.find(ptr1) == usingOrigin.end() || usingGolden.find(ptr2) == usingGolden.end())
				continue;
			if (!IsGateTypeEqual(ptr1, ptr2))
				continue;
			if (!IsFaninEqual(ptr1, ptr2, matchInfo))
				continue;
			matchInfo.matches[ptr2] = ptr1;
			usingOrigin.erase(ptr1);
			usingGolden.erase(ptr2);
		}
	}
	//remove in maps content
	for (map<Node*, bool>::iterator it = usingOrigin.begin(); it != usingOrigin.end(); ++it) {
		if (matchInfo.originState[it->first]) {
			matchInfo.originState[it->first] = false;
			matchInfo.originRemoveNode[it->first] = true;
		}
	}
	for (map<Node*, bool>::iterator it = usingGolden.begin(); it != usingGolden.end(); ++it) {
		if (matchInfo.goldenState[it->first]) {
			matchInfo.goldenState[it->first] = false;
			matchInfo.goldenRemoveNode[it->first] = true;
		}
	}
}

bool IsGateTypeEqual(Node* origin, Node* golden)
{
	int type1 = origin->type;
	int type2 = golden->type;
	if (origin->type == 8)
		type1 = origin->realGate;
	if (golden->type == 8)
		type2 = golden->realGate;
	if (type1 == type2)
		return true;
	return false;
}


bool IsFaninEqual(Node* origin, Node* golden, MatchInfo matchInfo)
{
	for (int i = 0; i < origin->fanin.size(); i++) {
		for (int m = 0; m < golden->fanin.size(); m++) {
			if (matchInfo.matches.find(golden->fanin[m]) != matchInfo.matches.end())
				if (matchInfo.matches[golden->fanin[m]] == origin->fanin[i])
					break;
			if (m == golden->fanin.size() - 1)
				return false;
		}
	}
	return true;
}

bool IsFaninVisited(Node* ptr, map<Node*, bool> maps)
{
	for (int i = 0; i < ptr->fanin.size(); i++)
		if (!IsVisited(ptr->fanin[i], maps))
			return false;
	return true;
}

bool IsVisited(Node* target, map<Node*, bool> maps)
{
	if (maps.find(target) != maps.end())
		return true;
	return false;
}

void graph2Blif(Graph& path_original, Graph& path_golden)

{
	//we need to make sure the structure of input data 
	//...

	ofstream outfile("check.blif");
	//write -> ".model check"
	outfile << ".model check" << endl;

	//write -> ".inputs ..."
	outfile << ".inputs";
	for (int i = 0; i < path_original.PI.size(); ++i) {
		outfile << " " << path_original.PI[i]->name + "_G1";
		outfile << " " << path_golden.PI[i]->name + "_R2";
	}
	outfile << endl;

	//write -> ".outputs ..."
	outfile << ".outputs " << "output";
	outfile << endl;

	//write -> ".names ..."
	//we can modify the topological sort pi oder
	//that we can code here easier
	map<Node*, bool> visited;
	for (int i = 0; i < path_original.netlist.size(); ++i)
		visited[path_original.netlist[i]] = false;

	for (int i = 0; i < path_golden.netlist.size(); ++i)
		visited[path_golden.netlist[i]] = false;

	//visit original pi fanout node
	//write blif
	//some order question ???
	netlist2Blif(outfile, path_original.PI, visited);
	//golden pi
	netlist2Blif(outfile, path_golden.PI, visited);
	//original netlist
	netlist2Blif(outfile, path_original.netlist, visited);
	//golden netlist
	netlist2Blif(outfile, path_golden.netlist, visited);

	buildMiter(outfile, path_original.PO, path_golden.PO);
	outfile << ".end";
	outfile.close();
}
void netlist2Blif(ofstream& outfile, vector<Node*>& netlist, map<Node*, bool>& visited)
{
	for (int i = 0; i < netlist.size(); ++i) {
		for (int j = 0; j < netlist[i]->fanout.size(); ++j) {
			if (!visited[netlist[i]->fanout[j]]) {
				node2Blif(outfile, netlist[i]->fanout[j]);
				visited[netlist[i]->fanout[j]] = true;
			}
		}
	}
}
void node2Blif(ofstream& outfile, Node* currNode)
{
	//0:not 1:and 2:or 3:nand 4:nor 5:xor 6:xnor 7:buf 8:assign 9:PI 10:PO
	int type;
	if (currNode->type == 10 || currNode->type == 9)
		type = currNode->realGate;
	else
		type = currNode->type;

	//write -> ".names ..."
	cout << ".names"
		<< " " << currNode->fanin[0]->name + "_" + currNode->graphName
		<< " " << currNode->fanin[1]->name + "_" + currNode->graphName
		<< " " << currNode->name + "_" + currNode->graphName << endl;

	outfile << ".names"
		<< " " << currNode->fanin[0]->name + "_" + currNode->graphName
		<< " " << currNode->fanin[1]->name + "_" + currNode->graphName
		<< " " << currNode->name + "_" + currNode->graphName << endl;

	if (currNode->fanin.size() > 1) {
		//and gate
		if (type == 1)
			outfile << "11 1" << endl;
		//or gate
		else if (type == 2)
			outfile << "-1 1" << endl
			<< "1- 1" << endl;
		//nand gate
		else if (type == 3) {
			outfile << "0- 1" << endl
				<< "10 1" << endl;
		}
		//nor gate
		else if (type == 4) {
			outfile << "00 1" << endl;
		}
		//xor gate
		else if (type == 5) {
			outfile << "01 1" << endl
				<< "10 1" << endl;
		}
		//xnor gate
		else if (type == 6) {
			outfile << "00 1" << endl
				<< "11 1" << endl;
		}
	}
	else {
		//not gate
		if (type == 0) {
			outfile << "0 1" << endl;
		}
		//buffer or assign
		else if (type == 7 || type == 8) {
			outfile << "1 1" << endl;
		}
	}
}
void buildMiter(ofstream& outfile, vector<Node*>& PO_original, vector<Node*>PO_golden)
{
	vector<Node*> PO_goldenTemp = PO_golden;
	vector<string> miter;
	for (int i = 0; i < PO_original.size(); ++i) {
		for (int j = 0; j < PO_goldenTemp.size(); ++j) {
			if (PO_original[i]->name == PO_goldenTemp[j]->name) {
				outfile << ".names";
				//outfile .names ...
				outfile << " " << PO_original[i]->name + "_" + PO_original[i]->graphName
					<< " " << PO_goldenTemp[i]->name + "_" + PO_goldenTemp[i]->graphName
					<< " " << "miter_" + to_string(i) << endl;
				//outfile xor gate
				outfile << "01 1" << endl
					<< "10 1" << endl;
				PO_goldenTemp.erase(PO_goldenTemp.begin() + j);

				miter.push_back("miter_" + to_string(i));
			}
		}
	}
	//need to add
	for (int i = 0; i < miter.size(); ++i) {
		outfile << ".names " << miter[i] << " output" << endl;
		outfile << "1 1" << endl;
	}
}


/*
void createRectifyPair(Graph& R2, Graph& G1)
{
	//for(int i=0;i<G1.PO.size();i++)
}
bool pisetIsDifferent(Node object, Node golden)
{
	map<string, bool> maps;
	if (golden.piset.size() != object.piset.size())
		return true;
	for (int i = 0; i < golden.piset.size(); i++)
		maps[golden.piset[i]->name] = false;
	for (int i = 0; i < object.piset.size(); i++) {
		if (maps.find(object.piset[i]->name) == maps.end())
			return true;
	}
	return false;
}
bool seedIsDifferent(Node object, Node golden)
{
	if (*object.seeds != *golden.seeds)
		return true;
	return false;
}
*/

void outFile(Graph graph, char* argv)
{
	ofstream outfile(argv);
	vector<Node> pi_items;
	vector<Node> po_items;
	// ExtractBracket PI/PO
	strExtractBracket(graph, pi_items, po_items);
	outfile << "module top_eco(";
	for (int i = 0; i < pi_items.size(); i++)
		outfile << pi_items[i].name << ", ";
	for (int i = 0; i < po_items.size(); i++) {
		if (i != po_items.size() - 1)
			outfile << po_items[i].name << ", ";
		else
			outfile << po_items[i].name << ");\n";
	}
	sort(pi_items.begin(), pi_items.end(), typeAndNumberCompare);
	sort(po_items.begin(), po_items.end(), typeAndNumberCompare);
	outputFront(outfile, pi_items, "  input ");
	outputFront(outfile, po_items, "  output ");
	outputFront(outfile, pi_items, "  wire ");
	outputFront(outfile, po_items, "  wire ");

	vector<string> wireOutput;
	for (int i = 0; i < graph.netlist.size(); ++i)
		if (graph.netlist[i]->type < 9 && graph.netlist[i]->name.find("'b") == string::npos)
			wireOutput.push_back(graph.netlist[i]->name);
	sort(wireOutput.begin(), wireOutput.end(), strTitleCompare);
	for (int i = 0, counter = 1; i < wireOutput.size(); i++) {
		if (counter == 1 && i != wireOutput.size() - 1)
			outfile << "  wire " << wireOutput[i] << ", ";
		else if (counter == 1)
			outfile << "  wire " << wireOutput[i] << ";\n";
		else if (counter < 8 && i != wireOutput.size() - 1)
			outfile << wireOutput[i] << ", ";
		else if (counter < 8)
			outfile << wireOutput[i] << ";\n";
		else {
			outfile << wireOutput[i] << ";\n";
			counter = 0;
		}
		counter++;
	}
	gateOutput(outfile, graph);
	outfile << "endmodule\n";
	outfile.close();
}
void strExtractBracket(Graph graph, vector<Node>& pi_items, vector<Node>& po_items)
{
	for (int i = 0; i < graph.PI.size(); i++) {
		bool is_exist = false;
		Node req;
		req.type = graph.PI[i]->type;
		req.seeds = new unsigned[1];
		req.seeds[0] = 1;
		if (graph.PI[i]->name.find("[") != string::npos)
			req.name = graph.PI[i]->name.substr(0, graph.PI[i]->name.find('['));
		else
			req.name = graph.PI[i]->name;
		for (int j = pi_items.size() - 1; j >= 0; j--)
			if (pi_items[j].name == req.name) { pi_items[j].seeds[0]++; is_exist = true; break; }
		if (!is_exist)
			pi_items.push_back(req);
	}
	for (int i = 0; i < graph.PO.size(); i++) {
		bool is_exist = false;
		Node req;
		req.type = graph.PO[i]->type;
		req.seeds = new unsigned[1];
		req.seeds[0] = 1;
		if (graph.PO[i]->name.find("[") != string::npos)
			req.name = graph.PO[i]->name.substr(0, graph.PO[i]->name.find('['));
		else
			req.name = graph.PO[i]->name;
		for (int j = po_items.size() - 1; j >= 0; j--)
			if (po_items[j].name == req.name) { po_items[j].seeds[0]++; is_exist = true; break; }
		if (!is_exist)
			po_items.push_back(req);
	}
}
void outputFront(ofstream& outfile, vector<Node> p, string str)
{
	for (int i = 0; i < p.size(); i++) {
		unsigned int* current_num = p[i].seeds;
		vector<string> vec;
		vec.push_back(p[i].name);
		for (int j = i + 1; j < p.size(); j++) {
			if (*current_num == *p[j].seeds)
				vec.push_back(p[j].name);
			else {
				i = j - 1;
				break;
			}
			if (j == p.size() - 1)
				i = j;
		}
		outfile << str;
		if (*current_num > 1) {
			stringstream ss;
			ss << (*current_num - 1);
			string req = ss.str();
			outfile << "[" << req << ":0] ";
		}
		for (int j = 0; j < vec.size(); j++) {
			if (j != vec.size() - 1)
				outfile << vec[j] << ", ";
			else
				outfile << vec[j] << ";\n";
		}
	}
}
void gateOutput(ofstream& outfile, Graph graph)
{
	for (int gatenum = 0; gatenum < graph.netlist.size(); ++gatenum) {
		int type = graph.netlist[gatenum]->type;
		int realGate = graph.netlist[gatenum]->realGate;
		stringstream ss;
		ss << gatenum;
		if (type != -1 && type != 9) {
			if (type == 0 || realGate == 0)
				outfile << "  not eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
			else if (type == 1 || realGate == 1)
				outfile << "  and eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
			else if (type == 2 || realGate == 2)
				outfile << "  or eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
			else if (type == 3 || realGate == 3)
				outfile << "  nand eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
			else if (type == 4 || realGate == 4)
				outfile << "  nor eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
			else if (type == 5 || realGate == 5)
				outfile << "  xor eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
			else if (type == 6 || realGate == 6)
				outfile << "  xnor eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
			else if (type == 7 || realGate == 7)
				outfile << "  buf eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
			if (type == 8 || realGate == 8)
				outfile << "  assign " << graph.netlist[gatenum]->name << " = " << graph.netlist[gatenum]->fanin[0]->name << ";\n";
			else {
				for (int i = 0; i < graph.netlist[gatenum]->fanin.size(); i++) {
					if (i == graph.netlist[gatenum]->fanin.size() - 1)
						outfile << graph.netlist[gatenum]->fanin[i]->name << ");\n";
					else
						outfile << graph.netlist[gatenum]->fanin[i]->name << ", ";
				}
			}
		}
	}
}
bool typeAndNumberCompare(const Node& p1, const Node& p2)
{
	if (*p1.seeds == *p2.seeds)
		return p1.name < p2.name;
	return *p1.seeds < *p2.seeds;
}
bool strTitleCompare(const string& p1, const string& p2)
{
	if (p1.size() == p2.size())
		return p1 < p2;
	else
		return p1.size() < p2.size();
}

