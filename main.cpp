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
// 556sdas...
struct Node
{
	string name;
	vector<Node*> fanin;
	vector<Node*> fanout;
	set <Node*> piset;
	set <string> piset_str;
	set <Node*> faninCone;
	string graphName;//Graph name :R1,R2,G1
	unsigned* seeds;
	int type; //0:not 1:and 2:or 3:nand 4:nor 5:xor 6:xnor 7:buf 8:assign 9:PI 10:PO 
	int realGate; // -1:default
	int id;//for blif file
};

struct Graph
{
	vector< Node* > netlist;
	vector< Node* > PI;
	vector< Node* > PO;
	vector< Node*> Constants; //record 1'b0 1'b1
	map<string, Node*> PIMAP;//use PI's name to find its pointer
	string name;//Graph name -> R1,R2,G1
	set<Node*> PIFanoutNode;//record all PI fanout Nodes
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
	map<Node*, Node*> backmatches; //record PO to PI structure match and fanout only one
};

struct NameCompare
{
	bool operator()(const Node* lhs, const Node* rhs) {
		return lhs->name == rhs->name;
	}
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
void topologicalSort(Graph& graph, int idStart);
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
// when a node not match, removing all fanout
void removeAllFanout(Node* node, map<Node*, bool>& states, map<Node*, bool>& removed);


//start Random Simulation
void randomSimulation(MatchInfo& matchInfo);
//create a path for SAT solver
void outputPIwithFaninCone(ofstream& outfile, Node* nextNode, vector<Node*>& internalNode, vector<bool>& faninConst);
//check if this node's fanin is PI
bool faninIsPI(Node* nextNode);
//remove fanin node 
void removeAllFanin(MatchInfo& matchInfo, Node* originalSameNode, Node* goldenSameNode);


//outputBlif
void outputBlif(ofstream& outfile, Node* originalNode, Node* goldenNode);
//transfer graph to blif file and write blif file
void graph2Blif(Node* originalNode, Node* goldenNode);
//find the node's fanout and call node2Blif 
void netlist2Blif(ofstream& outfile, vector<Node*>& netlist);
//output constant 0 or 1 in blif file
void outputConst(ofstream& outfile, vector<bool>& faninConst);
//output .names to BLIF File
void outputDotNames(ofstream& outfile, Node* currNode, string currGraphName);
//write gate type ex: and gate -> 11 1
void node2Blif(ofstream& outfile, Node* currNode, int type);
//let original POs and Golden POs connet to the XOR to make the miter
void buildMiter(ofstream& outfile, Node* PO_original, Node* PO_golden, int miterPos);
//call abc -> "turn blif into cnf" and minisat -> "check if this two netlist is equal"
bool SATsolver();
//check if output is UNSAT
bool readSATsolverResult();
//abc tool
void abcBlif2CNF();


//PO to PI search
void backStructureSearch(Graph origin, Graph golden, MatchInfo& matchInfo);
//
void backMatchProcessing(Graph origin, Graph golden, MatchInfo& matchInfo);
//
void checkPOStructureEqual(Node* origin, Node* golden, MatchInfo& matchInfo);
//
bool checkGateTypeEqual(Node* origin, Node* golden);


//------ start create patch -------
void patchVerify(MatchInfo& matchInfo, Graph& R2, Graph& G1);
//check whether R2 and G1 are same after finishing  patch
bool compareNetlist(MatchInfo& matchInfo, Graph& R2, Graph& G1);
//out .names to BLIF File for patch
void outputPatchDotNames(ofstream& outfile, Node* currNode, string currGraphName, map<Node*, Node*>& matches);
//check if this RemoveNode fanins exist in RemoveNode
void checkRemoveNodeFaninExist(MatchInfo& matchInfo, Node* currNode, vector<Node*>& patchNode);


// generate patch verilog
void generatePatchVerilog(MatchInfo& matchInfo, Graph& R2, Graph& G1, char* argv);
// generate verilog instruction
string generateInstruction(Node* node, vector<string> names, int eco, int& cost);
// get type string
string getTypeString(int type);
// outfile declare variable
void generateDeclare(map<Node*, string> maps, string types, ofstream& outfile, int& cost);

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


void patchOptimize(MatchInfo& matchInfo);

void createRectifyPair(Graph& R2, Graph& G1);
bool pisetIsDifferent(Node object, Node golden);
bool seedIsDifferent(Node* origin, Node* golden);

//tool function
string toString(int trans) {
	stringstream ss;
	ss << trans;
	return ss.str();
};
//for vetor PO sort
bool PONameCompare(Node* lhs, Node* rhs) { return lhs->name > rhs->name; };
/* Function Flow
	---------------------------------------------------------
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
   ------------------------------------------------------------------------------------------
	structureCompareMain  ->   structureCompareOper  ->  IsGateTypeEqual
										.		     ->  IsFaninEqual
										.		     ->  IsVisited
										.		     ->  IsFaninVisited	  ->  IsVisited
										.		     ->  removeAllFanout  ->  removeAllFanout
   ------------------------------------------------------------------------------------------
	   |
   -------------------------------------------------------------------------------------
	randomSimulation  ->  outputBlif  ->  graph2Blif  ->  outputPIwithFaninCone  ->  node2Blif
											   .	  ->  outputConst
											   .	  ->  netlist2Blif  ->  node2Blif
							  .		  ->  buildMiter
			.		  ->  SATsolver  ->  abcBlif2CNF
							  .	     ->  readSATsolverResult
			.		  ->  removeAllFanin
   -------------------------------------------------------------------------------------
	   |
   -------------------------------------------------------------------------------------
	NewpatchVerify  ->  checkRemoveNodeFaninExist
		.		 ->  faninIsPI
		.		 ->  compareNetlist  ->  outputPatchDotNames
						   .         ->  outputConst
						   .         ->  buildMiter
						   .         ->  SATsolver
   ------------------------------------------------------------------------------------
	   |
   -------------------------------------------------------------------------------------
	generatePatchVerilog  ->  generateInstruction  ->  getTypeString
			.			  ->  generateDeclare
   -------------------------------------------------------------------------------------
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
	topologicalSort(R1, 1);
	topologicalSort(R2, R1.netlist.size() + 1);
	topologicalSort(G1, R2.netlist.size() + R1.netlist.size() + 1);

	//Set all nodes Primary Input
	setNodePIsetandSeed(R1);
	setNodePIsetandSeed(R2);
	setNodePIsetandSeed(G1);

	MatchInfo matchInfo;
	structureCompareMain(G1, R2, matchInfo);
	int k = 0;
	/*
	map<Node*, bool>::iterator it = matchInfo.goldenRemoveNode.begin();
	int pos = 0;
	for (; it != matchInfo.goldenRemoveNode.end(); ++it) {
		pos++;
		if (it->first->name == "n_307")
			cout << pos << endl;
	}
	*/
	//check seed and do SAT solver
	/*randomSimulation(matchInfo);*/

	backStructureSearch(G1, R2, matchInfo);

	//start create and verify patch
	//patchVerify(matchInfo, R2, G1);

	//start optimize patch with abc tool
	patchOptimize(matchInfo);

	//output the patch.v
	generatePatchVerilog(matchInfo, R2, G1, argv[4]);
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
	graph.Constants.resize(2);
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
			if (split_command == "1'b0")
				graph.Constants[0] = req;
			else if (split_command == "1'b1")
				graph.Constants[1] = req;
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
					int currtype = currGate->type;
					if (currtype == 10)
						currtype = currGate->realGate;
					Node* newnode = initialNewnode(n1->name + "_" + n2->name, currtype, graph.name);
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

				//if currGate fanin is Pi then add to PIfanoutNode
				if (req->type == 9)
					graph.PIFanoutNode.insert(currGate);
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
					int currtype = currGate->type;
					if (currtype == 10)
						currtype = currGate->realGate;
					Node* newnode = initialNewnode(n1->name + "_" + n2->name, currtype, graph.name);

					//modify
					if (newnode->type == 9 || newnode->type == 10)
						newnode->realGate = currGate->realGate;
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

				//if currGate fanin is Pi then add to PIfanoutNode
				if (scanNode->type == 9)
					graph.PIFanoutNode.insert(currGate);
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
					graph.netlist.push_back(req);
					graph.PI.push_back(req);
					//req->piset.push_back(req);
					req->piset.insert(req);
					//add piset_str with string not Node*
					req->piset_str.insert(req->name);
					req->graphName = graph.name;
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
				//add piset_str with string not Node*
				req->piset_str.insert(req->name);
				req->graphName = graph.name;
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

void topologicalSort(Graph& graph, int idStart)
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

	for (int i = 0; i < graph.Constants.size(); ++i) {
		if (graph.Constants[i] != NULL && !visited[graph.Constants[i]])
			topologicalSortUtil(graph, graph.Constants[i], visited, Stack);
	}

	for (int i = 0; i < graph.PI.size(); ++i) {
		if (!visited[graph.PI[i]])
			topologicalSortUtil(graph, graph.PI[i], visited, Stack);
	}

	int pos = 0;

	vector<Node*> sortNode;
	sortNode.resize(Stack.size());
	while (Stack.empty() == false) {
		//cout << Stack.top() << " ";
		Stack.top()->id = pos + idStart;
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
	if (node->name != "1'b0" && node->name != "1'b1")
		Stack.push(node);
}

void setNodePIsetandSeed(Graph& graph)
{
	for (int i = 0; i < graph.netlist.size(); ++i) {
		Node* currNode = graph.netlist[i];
		//add itself to fanincone
		currNode->faninCone.insert(currNode);
		if (currNode->fanin.size() > 0) {
			vector<unsigned*> faninSeed;
			for (int j = 0; j < currNode->fanin.size(); ++j) {
				//set the PI to the PISET
				Node* faninNode = currNode->fanin[j];
				currNode->piset.insert(faninNode->piset.begin(), faninNode->piset.end());

				//add piset_str with string not Node*
				currNode->piset_str.insert(faninNode->piset_str.begin(), faninNode->piset_str.end());

				//set fanin cone
				currNode->faninCone.insert(faninNode->faninCone.begin(), faninNode->faninCone.end());

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
		if (matchInfo.originState[it->first])
			removeAllFanout(it->first, matchInfo.originState, matchInfo.originRemoveNode);
	}
	for (map<Node*, bool>::iterator it = usingGolden.begin(); it != usingGolden.end(); ++it) {
		if (matchInfo.goldenState[it->first])
			removeAllFanout(it->first, matchInfo.goldenState, matchInfo.goldenRemoveNode);
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
			if (origin->fanin[i]->name == "1'b1" && golden->fanin[i]->name == "1'b1")
				break;
			if (origin->fanin[i]->name == "1'b0" && golden->fanin[i]->name == "1'b0")
				break;
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
	if (maps.find(target) != maps.end() || target->name == "1'b0" || target->name == "1'b1")
		return true;
	return false;
}

void removeAllFanout(Node* node, map<Node*, bool>& states, map<Node*, bool>& removed)
{
	states[node] = false;
	removed[node] = true;
	for (int i = 0; i < node->fanout.size(); i++)
		if (states.find(node->fanout[i]) == states.end() || states[node->fanout[i]] == true)
			removeAllFanout(node->fanout[i], states, removed);
}

void randomSimulation(MatchInfo& matchInfo)
{
	map<Node*, bool>::iterator gd_it = matchInfo.goldenRemoveNode.begin();
	map<Node*, bool>::iterator og_it = matchInfo.originRemoveNode.begin();

	for (; og_it != matchInfo.originRemoveNode.end(); ++og_it) {
		gd_it = matchInfo.goldenRemoveNode.begin();
		for (; gd_it != matchInfo.goldenRemoveNode.end(); ++gd_it) {
			if (!seedIsDifferent(og_it->first, gd_it->first)) {
				//turn this two gate fanin cone into blif file
				if (og_it->first->piset.size() == gd_it->first->piset.size()) {
					if (og_it->first->piset_str == gd_it->first->piset_str) {//add this if condition
						graph2Blif(og_it->first, gd_it->first);
						//call SAT solver
						if (SATsolver()) {
							//cout << "golden: " << gd_it->first->name << " <-equal-> original: " << og_it->first->name << endl;
							matchInfo.matches[gd_it->first] = og_it->first;
							removeAllFanin(matchInfo, og_it->first, gd_it->first);
							og_it = matchInfo.originRemoveNode.begin();
							break;
						}
					}
				}
			}
		}
		if (matchInfo.originRemoveNode.size() == 0 || matchInfo.goldenRemoveNode.size() == 0)
			break;
	}
}

void removeAllFanin(MatchInfo& matchInfo, Node* originalSameNode, Node* goldenSameNode)
{
	set<Node*>::iterator it = originalSameNode->faninCone.begin();
	for (; it != originalSameNode->faninCone.end(); ++it) {
		if (matchInfo.originRemoveNode.find(*it) != matchInfo.originRemoveNode.end()) {
			matchInfo.originRemoveNode.erase(*it);
		}
	}

	it = goldenSameNode->faninCone.begin();
	for (; it != goldenSameNode->faninCone.end(); ++it) {
		if (matchInfo.goldenRemoveNode.find(*it) != matchInfo.goldenRemoveNode.end()) {
			matchInfo.goldenRemoveNode.erase(*it);
		}
	}
}

bool seedIsDifferent(Node* origin, Node* golden)
{
	for (int i = 0; i < nWords; ++i) {
		if (origin->seeds[i] != golden->seeds[i])
			return true;
	}
	return false;
}

bool faninIsPI(Node* nextNode)
{
	for (int i = 0; i < nextNode->fanin.size(); ++i) {
		if (nextNode->fanin[i]->type == 9) {
			return true;
		}
	}
	return false;
}

void graph2Blif(Node* originalNode, Node* goldenNode)
{
	ofstream outfile("./blif/check.blif");
	vector<Node*> internalNode;
	vector<bool> faninConst;
	faninConst.resize(2, false);

	outputBlif(outfile, originalNode, goldenNode);
	outputPIwithFaninCone(outfile, originalNode, internalNode, faninConst);
	outputPIwithFaninCone(outfile, goldenNode, internalNode, faninConst);
	outputConst(outfile, faninConst);
	netlist2Blif(outfile, internalNode);
	buildMiter(outfile, originalNode, goldenNode, 0);

	outfile << ".names " << "miter_0" << " output" << endl;
	outfile << "1 1" << endl;
	outfile << ".end";

	outfile.close();
}

void outputBlif(ofstream& outfile, Node* originalNode, Node* goldenNode)
{
	//write -> ".model check"
	outfile << ".model check" << endl;

	//write -> ".inputs ..."
	outfile << ".inputs";
	/*set<Node*> tmp = originalNode->piset;
	for (const auto& it : tmp) {
		outfile << " " << it->name;
		//std::cout << it->name << " ";
	}*/

	set<Node*>::iterator it = originalNode->piset.begin();
	for (; it != originalNode->piset.end(); ++it) {
		outfile << " " << (*it)->name;
		//cout << " " << (*it)->name;
	}

	outfile << endl;
	//write -> ".outputs ..."
	outfile << ".outputs " << "output";
	outfile << endl;
}

void outputPIwithFaninCone(ofstream& outfile, Node* nextNode, vector<Node*>& internalNode, vector<bool>& faninConst)
{
	//check whether this node's fanin is PI or not
	/*
	if (faninIsPI(nextNode))
		node2Blif(outfile, nextNode);
	else
		internalNode.push_back(nextNode);

	for (int i = 0; i < nextNode->fanin.size(); ++i) {
		if (nextNode->fanin[i]->type != 9)
			outputPIwithFaninCone(outfile, nextNode->fanin[i], internalNode);
	}
	*/

	//!!!!!
	//need to modify because of faninCone 
	//so here can use piset to output pi fanout into blif file
	//!!!!!
	set<Node*>::iterator it = nextNode->faninCone.begin();
	for (; it != nextNode->faninCone.end(); ++it) {
		if (faninIsPI(*it))
			outputDotNames(outfile, *it, (*it)->graphName);
		else if ((*it)->type != 9) {
			if ((*it)->name == "1'b0" && !faninConst[0])
				faninConst[0] = true;
			else if ((*it)->name == "1'b1" && !faninConst[1])
				faninConst[1] = true;

			internalNode.push_back(*it);
		}
	}

}

void netlist2Blif(ofstream& outfile, vector<Node*>& netlist)
{
	for (int i = 0; i < netlist.size(); ++i) {
		outputDotNames(outfile, netlist[i], netlist[i]->graphName);
	}
}

void outputConst(ofstream& outfile, vector<bool>& faninConst)
{
	if (faninConst[0]) {
		outfile << ".names 1'b0" << endl
			<< "0" << endl;
	}

	if (faninConst[1]) {
		outfile << ".names 1'b1" << endl
			<< "1" << endl;
	}

}

void outputDotNames(ofstream& outfile, Node* currNode, string currGraphName)
{
	int type;
	if (currNode->type == 10 || currNode->type == 9)
		type = currNode->realGate;
	else
		type = currNode->type;

	//write -> ".names ..."
	/*cout << ".names"
		<< " " << currNode->fanin[0]->name + "_" + currNode->graphName
		<< " " << currNode->fanin[1]->name + "_" + currNode->graphName
		<< " " << currNode->name + "_" + currNode->graphName << endl;*/


	outfile << ".names";
	for (int i = 0; i < currNode->fanin.size(); ++i) {
		if (currNode->fanin[i]->graphName == currGraphName) {
			outfile << " " << currNode->fanin[i]->name;
			if (currNode->fanin[i]->type != 9)
				outfile << "_" + currGraphName;
		}
	}

	outfile << " " << currNode->name;
	if (currNode->type != 9)
		outfile << "_" + currGraphName;

	outfile << endl;
	node2Blif(outfile, currNode, type);
}


void node2Blif(ofstream& outfile, Node* currNode, int type)
{
	//0:not 1:and 2:or 3:nand 4:nor 5:xor 6:xnor 7:buf 8:assign 9:PI 10:PO

	if (currNode->fanin.size() > 1) {
		//and gate
		if (type == 1)
			outfile << "11 1" << endl;
		//or gate
		else if (type == 2) {
			outfile << "-1 1" << endl
				<< "1- 1" << endl;
		}
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

void buildMiter(ofstream& outfile, Node* PO_original, Node* PO_golden, int miterPos)
{
	/*vector<Node*> PO_goldenTemp = PO_golden;
	vector<string> miter;
	for (int i = 0; i < PO_original.size(); ++i) {
		for (int j = 0; j < PO_goldenTemp.size(); ++j) {
			if (PO_original[i]->name == PO_goldenTemp[j]->name) {
				outfile << ".names";
				//outfile .names ...
				outfile << " " << PO_original[i]->name + "_" + PO_original[i]->graphName;
				outfile << " " << PO_goldenTemp[j]->name + "_" + PO_goldenTemp[j]->graphName;
				outfile << " " << "miter_" + to_string(i) << endl;
				//outfile xor gate
				outfile << "01 1" << endl
					<< "10 1" << endl;
				PO_goldenTemp.erase(PO_goldenTemp.begin() + j);

				miter.push_back("miter_" + to_string(i));
			}
		}
	}*/
	//need to add

	outfile << ".names";
	outfile << " " << PO_original->name + "_" + "G1";
	outfile << " " << PO_golden->name + "_" + "R2";

	outfile << " " << "miter_" << toString(miterPos) << endl;
	//outfile xor gate
	outfile << "01 1" << endl
		<< "10 1" << endl;

	/*
	outfile << ".names " << "miter_"<< to_string(miterPos) << " output" << endl;
	outfile << "1 1" << endl;
	*/
}

bool SATsolver()
{
	abcBlif2CNF();
	system("./minisat ./cnf/check.cnf out.txt > minisatScreen.txt ");
	if (readSATsolverResult())
		return true;
	return false;
}

bool readSATsolverResult()
{
	ifstream infile("out.txt");
	string result;
	infile >> result;
	infile.close();
	if (result == "UNSAT")
		return true;

	return false;
}

void abcBlif2CNF()
{
	system("./blif2cnf.out ./blif/check.blif > abcScreen.txt");
}

void backStructureSearch(Graph origin, Graph golden, MatchInfo& matchInfo)
{
	map<string, Node*> originPoMap;
	map<string, Node*> goldenPoMap;
	for (int i = 0; i < origin.PO.size(); i++) {
		Node* node = origin.PO[i];
		if (matchInfo.originRemoveNode.find(node) != matchInfo.originRemoveNode.end())
			originPoMap[node->name] = node;
	}
	for (int i = 0; i < golden.PO.size(); i++) {
		Node* node = golden.PO[i];
		if (matchInfo.goldenRemoveNode.find(node) != matchInfo.goldenRemoveNode.end())
			goldenPoMap[node->name] = node;
	}

	for (map<string, Node*>::iterator it = originPoMap.begin(); it != originPoMap.end(); ++it) {
		Node* originNode = it->second;
		Node* goldenNode = nullptr;
		if (matchInfo.originRemoveNode.find(originNode) == matchInfo.originRemoveNode.end())
			continue;
		if (goldenPoMap.find(it->first) != goldenPoMap.end())
			goldenNode = goldenPoMap.find(it->first)->second;
		if (goldenNode == nullptr || matchInfo.goldenRemoveNode.find(goldenNode) == matchInfo.goldenRemoveNode.end())
			continue;
		if(checkGateTypeEqual(originNode,goldenNode))
			checkPOStructureEqual(originNode, goldenNode, matchInfo);
	}

	int k = 0;
}


void checkPOStructureEqual(Node* origin, Node* golden, MatchInfo& matchInfo)
{
	map<Node*, bool> originFaninMap;
	map<Node*, bool> goldenFaninMap;
	map<Node*, Node*> matchNow;

	for (int i = 0; i < origin->fanin.size(); i++) {
		if (origin->fanin[i]->name == "1'b1" || origin->fanin[i]->name == "1'b0")
			originFaninMap[origin->fanin[i]] = true;
		else if (origin->fanin[i]->fanout.size() < 2 && matchInfo.originRemoveNode.find(origin->fanin[i])!=matchInfo.originRemoveNode.end())
			originFaninMap[origin->fanin[i]] = true;
	}

	for (int i = 0; i < golden->fanin.size(); i++) {
		if (golden->fanin[i]->name == "1'b1" || golden->fanin[i]->name == "1'b0")
			goldenFaninMap[golden->fanin[i]] = true;
		else if (golden->fanin[i]->fanout.size() < 2 && matchInfo.goldenRemoveNode.find(golden->fanin[i]) != matchInfo.goldenRemoveNode.end())
			goldenFaninMap[golden->fanin[i]] = true;
	}
		

	for (map<Node*, bool>::iterator goldenptr = goldenFaninMap.begin(); goldenptr != goldenFaninMap.end(); ++goldenptr) {
		Node* originNode = nullptr;
		Node* goldenNode = goldenptr->first;
		for (map<Node*, bool>::iterator originptr = originFaninMap.begin(); originptr != originFaninMap.end(); ++originptr) {
			originNode = originptr->first;
			if (checkGateTypeEqual(originNode, goldenNode)) {
				matchNow[goldenNode] = originNode;
				originFaninMap.erase(originNode);
				break;
			}
		}
	}

	if (matchNow.size() == origin->fanin.size()) {
		matchInfo.matches[golden] = origin;
		for (map<Node*, Node*>::iterator it = matchNow.begin(); it != matchNow.end(); ++it) {
			Node* originNode = it->second;
			Node* goldenNode = it->first;
			if (originNode->name == "1'b1" || originNode->name == "1'b0")
				continue;
			checkPOStructureEqual(originNode, goldenNode, matchInfo);
		}
	}
	else
		matchInfo.backmatches[golden] = origin;

	if (matchInfo.originRemoveNode.find(origin) != matchInfo.originRemoveNode.end())
		matchInfo.originRemoveNode.erase(origin);
	if (matchInfo.goldenRemoveNode.find(golden) != matchInfo.goldenRemoveNode.end())
		matchInfo.goldenRemoveNode.erase(golden);

}

bool checkGateTypeEqual(Node* origin, Node* golden)
{
	if (origin->name == "1'b1" || origin->name == "1'b0")
		if (origin->name != golden->name)
			return false;
	if (golden->name == "1'b1" || golden->name == "1'b0")
		if (origin->name != golden->name)
			return false;
	if(origin->type!=golden->type || origin->realGate!=golden->realGate)
		return false;
	return true;
}

void patchVerify(MatchInfo& matchInfo, Graph& R2, Graph& G1)
{
	map<Node*, bool>::iterator it = matchInfo.goldenRemoveNode.begin();
	vector<Node*> patchNode;
	Graph R2backup = R2;
	Graph G1backup = G1;
	//solve problem of Random Simulation
	for (; it != matchInfo.goldenRemoveNode.end();) {
		Node* currNode = it->first;
		int ogSize = matchInfo.goldenRemoveNode.size();
		checkRemoveNodeFaninExist(matchInfo, currNode, patchNode);
		if (ogSize != matchInfo.goldenRemoveNode.size())
			it = matchInfo.goldenRemoveNode.begin();
		else
			++it;
	}

	it = matchInfo.goldenRemoveNode.begin();
	for (; it != matchInfo.goldenRemoveNode.end(); ++it) {
		Node* currNode = it->first;
		if (faninIsPI(currNode))
			G1backup.PIFanoutNode.insert(currNode);
		G1backup.netlist.push_back(currNode);
	}

	if (compareNetlist(matchInfo, R2backup, G1backup))
		cout << "Patched G1 Success!" << endl;
	else
		cout << "Patched G1 Error!" << endl;
}

bool compareNetlist(MatchInfo& matchInfo, Graph& R2backup, Graph& G1backup)
{
	ofstream outfile("./blif/check.blif");

	//write -> ".model check"
	outfile << ".model check" << endl;

	//write -> ".inputs ..."
	outfile << ".inputs";
	for (int i = 0; i < R2backup.PI.size(); ++i) {
		outfile << " " << R2backup.PI[i]->name;
	}
	outfile << endl;

	//write -> ".outputs ..."
	outfile << ".outputs " << "output";
	outfile << endl;
	map<Node*, bool> isVisitedG1;
	map<Node*, bool> isVisitedR2;

	for (int i = 0; i < G1backup.netlist.size(); ++i)
		isVisitedG1[G1backup.netlist[i]] = false;

	for (int i = 0; i < R2backup.netlist.size(); ++i)
		isVisitedR2[R2backup.netlist[i]] = false;

	//original netlist PI
	/*
	for (int i = 0; i < G1backup.PI.size(); ++i) {
		for (int j = 0; j < G1backup.PI[i]->fanout.size(); ++j) {
			Node* fanoutNode = G1backup.PI[i]->fanout[j];
			if (matchInfo.originRemoveNode.find(fanoutNode) == matchInfo.originRemoveNode.end() && !isVisitedG1[fanoutNode]) {
				outputPatchDotNames(outfile, fanoutNode, "G1", matchInfo.matches);
				isVisitedG1[fanoutNode] = true;
			}
		}
	}*/

	//Original Netlist PI
	set<Node*>::iterator it = G1backup.PIFanoutNode.begin();
	for (; it != G1backup.PIFanoutNode.end(); ++it) {
		Node* fanoutNode = *it;
		if (matchInfo.originRemoveNode.find(fanoutNode) == matchInfo.originRemoveNode.end() && !isVisitedG1[fanoutNode]) {
			outputPatchDotNames(outfile, fanoutNode, "G1", matchInfo.matches);
			isVisitedG1[fanoutNode] = true;
		}
	}

	//Golden Netlist PI
	//!!!!!
	//here can optimize with PIFanoutNode
	for (int i = 0; i < R2backup.PI.size(); ++i) {
		for (int j = 0; j < R2backup.PI[i]->fanout.size(); ++j) {
			Node* fanoutNode = R2backup.PI[i]->fanout[j];
			if (!isVisitedR2[fanoutNode]) {
				outputPatchDotNames(outfile, fanoutNode, "R2", matchInfo.matches);
				isVisitedR2[fanoutNode] = true;
			}
		}
	}

	//output Constant
	vector<bool> existConst(2, false);
	for (int i = 0; i < R2backup.Constants.size(); ++i) {
		if (R2backup.Constants[i] != NULL)
			existConst[i] = true;
		else if (G1backup.Constants[i] != NULL) {
			//check if this const's fanout is not in the OriginalRemoveNode
			//then we can output .name const
			for (int j = 0; j < G1backup.Constants[i]->fanout.size(); ++j) {
				Node* constFanoutNode = G1backup.Constants[i]->fanout[j];
				if (matchInfo.originRemoveNode.find(constFanoutNode) == matchInfo.originRemoveNode.end()) {
					existConst[i] = true;
					break;
				}
			}
		}
	}
	outputConst(outfile, existConst);

	//output Golden Remove Node connect to Original Netlist 
	for (int i = 0; i < G1backup.netlist.size(); ++i) {
		if (!isVisitedG1[G1backup.netlist[i]] && G1backup.netlist[i]->type != 9 && matchInfo.originRemoveNode.find(G1backup.netlist[i]) == matchInfo.originRemoveNode.end())
			outputPatchDotNames(outfile, G1backup.netlist[i], "G1", matchInfo.matches);
	}

	//output Golden Netlist internal node
	for (int i = 0; i < R2backup.netlist.size(); ++i) {
		if (!isVisitedR2[R2backup.netlist[i]] && R2backup.netlist[i]->type != 9)
			outputPatchDotNames(outfile, R2backup.netlist[i], "R2", matchInfo.matches);
	}

	sort(R2backup.PO.begin(), R2backup.PO.end(), PONameCompare);
	sort(G1backup.PO.begin(), G1backup.PO.end(), PONameCompare);
	for (int i = 0; i < R2backup.PO.size(); ++i) {
		buildMiter(outfile, G1backup.PO[i], R2backup.PO[i], i);
	}

	outfile << ".names";
	for (int i = 0; i < R2backup.PO.size(); ++i) {
		outfile << " miter_" << toString(i);
	}

	if (R2backup.PO.size() > 1) {
		outfile << " outputTmp" << endl;
		for (int i = 0; i < R2backup.PO.size(); ++i) {
			outfile << "0";
		}
		outfile << " 0" << endl;
		outfile << ".names" << " outputTmp output" << endl << "1 1" << endl;
	}
	else {
		outfile << " output" << endl;
		outfile << "1 1" << endl;
	}
	outfile << ".end";
	outfile.close();
	return SATsolver();
}

void checkRemoveNodeFaninExist(MatchInfo& matchInfo, Node* currNode, vector<Node*>& patchNode)
{
	for (int i = 0; i < currNode->fanin.size(); ++i) {
		if (matchInfo.goldenRemoveNode.find(currNode->fanin[i]) == matchInfo.goldenRemoveNode.end() &&
			matchInfo.matches.find(currNode->fanin[i]) == matchInfo.matches.end())
			matchInfo.goldenRemoveNode[currNode->fanin[i]] = false;
	}
}

void outputPatchDotNames(ofstream& outfile, Node* currNode, string currGraphName, map<Node*, Node*>& matches)
{
	int type;
	stringstream ss;

	if (currNode->type == 10 || currNode->type == 9)
		type = currNode->realGate;
	else
		type = currNode->type;

	//write -> ".names ..."
	/*cout << ".names"
		<< " " << currNode->fanin[0]->name + "_" + currNode->graphName
		<< " " << currNode->fanin[1]->name + "_" + currNode->graphName
		<< " " << currNode->name + "_" + currNode->graphName << endl;*/

	outfile << ".names";

	for (int i = 0; i < currNode->fanin.size(); ++i) {
		//check if currNode fanin is same in G1
		if (matches.find(currNode->fanin[i]) != matches.end() && currGraphName == "G1") {
			outfile << " " << matches[currNode->fanin[i]]->name;
			ss << matches[currNode->fanin[i]]->id;
		}
		else {
			outfile << " " << currNode->fanin[i]->name;
			ss << currNode->fanin[i]->id;
		}

		//if currNode fanin is not the PI
		if (currNode->fanin[i]->type != 9 && currNode->fanin[i]->name != "1'b0" && currNode->fanin[i]->name != "1'b1") {
			outfile << "_" + currGraphName;
			if (currNode->fanin[i]->type != 10)
				outfile << "_" << ss.str();
		}

		ss.clear();
		ss.str("");
	}

	//output current node with its id
	outfile << " " << currNode->name;
	if (currNode->type != 9) {
		outfile << "_" + currGraphName;
		if (currNode->type != 10)
			outfile << "_" << toString(currNode->id);
	}

	outfile << endl;
	node2Blif(outfile, currNode, type);
}

/*
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
}*/



void generatePatchVerilog(MatchInfo& matchInfo, Graph& R2, Graph& G1, char* argv)
{
	ofstream outfile(argv);
	set<Node*> input;
	set<Node*> output;
	map<Node*, string> inputDeclare;
	map<Node*, string> outputDeclare;
	map<Node*, string> newGate;
	map<Node*, bool> goldenRemoveNode = matchInfo.goldenRemoveNode;
	vector<string> instructions;
	map<string, bool> useConst;
	useConst["1'b0"] = false;
	useConst["1'b1"] = false;
	int eco = 1;
	int cost = 0;
	for (map<Node*, bool>::iterator it = goldenRemoveNode.begin(); it != goldenRemoveNode.end(); ++it) {
		Node* curr = it->first;
		vector<string> names = { "","","" }; // [0]:fanin1 [1]:fanin2 [2]:node.name
		// fanin
		for (int i = 0; i < curr->fanin.size(); i++) {
			Node* fanin = curr->fanin[i];
			if (fanin->name == "1'b0" || fanin->name == "1'b1") {
				names[i] = fanin->name;
				if (!useConst[fanin->name]) {
					cost += 1;
					useConst[fanin->name] = true;
				}
			}
			else if (matchInfo.matches.find(fanin) != matchInfo.matches.end()) { //have match ((PI and match gate
				Node* realfanin = matchInfo.matches.find(fanin)->second;
				if (realfanin->name.find('[') != string::npos)
					names[i] = "\\" + realfanin->name + " ";
				else
					names[i] = realfanin->name;
				inputDeclare[realfanin] = names[i];
			}
			else if (matchInfo.backmatches.find(fanin) != matchInfo.backmatches.end()) {
				Node* realfanin = matchInfo.backmatches.find(fanin)->second;
				if (realfanin->name.find('[') != string::npos)
					names[i] = "\\" + realfanin->name + " ";
				else
					names[i] = realfanin->name;
				inputDeclare[realfanin] = names[i];
			}
			else {
				string faninName = "n" + fanin->name;
				if (fanin->type == 10) { //PO
					if (fanin->name.find("[") != string::npos)
						faninName = "\\" + fanin->name + " ";
					else
						faninName = fanin->name;
					if (matchInfo.goldenRemoveNode.find(fanin) != matchInfo.goldenRemoveNode.end()) {
						//faninName += "_in";
					}
					else
						inputDeclare[fanin] = faninName;
				}
				else if (faninName.find("[") != string::npos) {
					string req = faninName.substr(faninName.find("[") + 1, faninName.find("]") - faninName.find("[") - 1);
					faninName = faninName.substr(0, faninName.find("[")) + req;
				}
				names[i] = faninName;
			}
		}
		// node+
		if (curr->name == "1'b0" || curr->name == "1'b1")
			continue;
		else if (curr->type == 10) {
			string name = curr->name;
			if (name.find("[") != string::npos)
				name = "\\" + name + " ";
			names[2] = name;
			//newGate[curr] = name;
			outputDeclare[curr] = name;
		}
		else if (curr->type == 9) {
			cout << "ERROR IN GENERATE PATCH VERILOG!!";
		}
		else if (curr->name == "1'b0" || curr->name == "1'b1")
			continue;
		else {
			string name = "n" + curr->name;
			if (name.find("[") != string::npos) {
				string req = name.substr(name.find("[") + 1, name.find("]") - name.find("[") - 1);
				name = name.substr(0, name.find("[")) + req;
			}
			names[2] = name;
			newGate[curr] = name;
		}
		instructions.push_back(generateInstruction(curr, names, eco++, cost));
	}

	for (map<Node*, Node*>::iterator it = matchInfo.backmatches.begin(); it != matchInfo.backmatches.end(); ++it) {
		vector<string> names = { "","","" }; // [0]:fanin1 [1]:fanin2 [2]:node.name
		Node* originNode = it->second;
		Node* goldenNode = it->first;
		if (originNode->name.find("[") != string::npos)
			names[2] = "\\" + originNode->name + " ";
		else
			names[2] = originNode->name;
		outputDeclare[originNode] = names[2];
		for (int i = 0; i < goldenNode->fanin.size(); i++) {
			if (goldenNode->fanin[i]->name == "1'b1" || goldenNode->fanin[i]->name == "1'b0")
				names[i] = goldenNode->fanin[i]->name;
			else if (matchInfo.matches.find(goldenNode->fanin[i]) != matchInfo.matches.end()) {
				Node* node = matchInfo.matches.find(goldenNode->fanin[i])->second;
				string name = node->name;
				if (name.find('[') != string::npos)
					name = "\\" + name + " ";
				inputDeclare[node] = name;
				names[i] = name;
			}
			else if (newGate.find(goldenNode->fanin[i]) != newGate.end()) {
				names[i] = newGate.find(goldenNode->fanin[i])->second;
			}
			else if (matchInfo.backmatches.find(goldenNode->fanin[i]) != matchInfo.backmatches.end()) {
				Node* node = matchInfo.backmatches.find(goldenNode->fanin[i])->second;
				string name = node->name;
				if (name.find('[') != string::npos)
					name = "\\" + name + " ";
				inputDeclare[node] = name;
				names[i] = name;
				cout << "ERROR\n";
			}
		}
		instructions.push_back(generateInstruction(originNode, names, eco++, cost));
	}

	outfile << "module top_eco(";
	map<Node*, string>::iterator it = inputDeclare.begin();
	outfile << it->second;
	++it;
	for (; it != inputDeclare.end(); ++it) {
		if (outputDeclare.find(it->first) != outputDeclare.end())
			continue;
		if (it->second.find("\\") == string::npos)
			outfile << ", " << it->second;
		else
			outfile << "," << it->second;
	}
	for (it = outputDeclare.begin(); it != outputDeclare.end(); ++it) {
		if (inputDeclare.find(it->first) != inputDeclare.end())
			inputDeclare.erase(it->first);
		if (it->second.find("\\") == string::npos)
			outfile << ", " << it->second;
		else
			outfile << "," << it->second;
	}
	outfile << ");\n";

	generateDeclare(inputDeclare, "input", outfile, cost);
	generateDeclare(outputDeclare, "output", outfile, cost);
	generateDeclare(newGate, "wire", outfile, cost);

	for (int i = 0; i < instructions.size(); i++)
		outfile << instructions[i] << "\n";
	outfile << "endmodule\n";
	//outfile << "// cost:" << cost;
	cout << "// cost:" << cost<<"\n";
	int i = 0;
}

string generateInstruction(Node* node, vector<string> names, int eco, int& cost)
{
	string gate = getTypeString(node->type);
	string res = "  ";
	stringstream ss;
	ss << eco;
	if (gate == "PO")
		gate = getTypeString(node->realGate);

	if (gate == "not" || gate == "buf") {
		if (names[0].find("\\") == string::npos)
			res += gate + " eco" + ss.str() + " (" + names[2] + ", " + names[0] + ");";
		else
			res += gate + " eco" + ss.str() + " (" + names[2] + "," + names[0] + ");";
		cost += -1;
	}
	else if (gate == "assign")
		res += gate + " " + names[2] + " = " + names[0] + ";";
	else {
		string loc1 = ", ";
		string loc2 = ", ";
		if (names[0].find("\\") != string::npos)
			loc1 = ",";
		if (names[1].find("\\") != string::npos)
			loc2 = ",";
		res += gate + " eco" + ss.str() + " (" + names[2] + loc1 + names[0] + loc2 + names[1] + ");";
	}
	return res;
}

string getTypeString(int type)
{
	vector<string> gates = { "not","and","or","nand","nor","xor","xnor","buf","assign","PI","PO" };
	return gates[type];
}

void generateDeclare(map<Node*, string> maps, string types, ofstream& outfile, int& cost)
{
	int last = 1;
	int count = 1;
	bool first = true;
	for (map<Node*, string>::iterator it = maps.begin(); it != maps.end(); ++it) {
		if (first) {
			outfile << "  " + types + " ";
			if (last == maps.size()) {
				outfile << it->second << ";\n";
			}
			else {
				outfile << it->second;
			}
			first = false;
		}
		else {
			if (count == 0)
				outfile << "  " + types + " ";
			if (last == maps.size() || count == 4) {
				if (count != 0) {
					if (it->second.find("\\") == string::npos)
						outfile << ", " << it->second << ";\n";
					else
						outfile << "," << it->second << ";\n";
				}
				else {
					outfile << it->second << ";\n";
				}
			}
			else {
				if (count != 0) {
					if (it->second.find("\\") == string::npos)
						outfile << ", " << it->second;
					else
						outfile << "," << it->second;
				}
				else
					outfile << it->second;
			}
		}

		count++;
		last++;
		cost += 1;
		if (count == 5)
			count = 0;
	}
}





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


void patchOptimize(MatchInfo& matchInfo)
{
	Graph currPatch;
	map<Node*, bool>::iterator it = matchInfo.goldenRemoveNode.begin();
	for (; it != matchInfo.goldenRemoveNode.end(); ++it) {
		for (int i = 0; i < it->first->fanin.size(); ++i) {
			Node* faninNode = it->first->fanin[i];
			if (matchInfo.goldenRemoveNode.find(faninNode) == matchInfo.goldenRemoveNode.end())
				currPatch.PI.push_back(faninNode);
		}
	}


}

