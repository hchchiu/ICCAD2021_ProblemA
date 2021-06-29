#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<cstring>
#include<vector>
#include<map>
#include<stack>
#include<set>
#include<algorithm>
using namespace std;

typedef pair<string, int> Pair;

#define RANDOM_UNSIGNED ((((unsigned)rand()) << 24) ^ (((unsigned)rand()) << 12) ^ ((unsigned)rand()))
#define GetBit(p, i)  (((p)[(i)>>5]  & (1<<((i) & 31))) > 0)
#define SetBit(p, i)  ((p)[(i)>>5] |= (1<<((i) & 31)))
#define UnSetBit(p, i)  ((p)[(i)>>5] ^= (1<<((i) & 31)))

struct Node
{
	string name;
	vector<Node*> fanin;
	vector<Node*> fanout;
	set<Node*> piset;
	unsigned* seeds;
	int type; //0:not 1:and 2:or 3:nand 4:nor 5:xor 6:xnor 7:buf 8:assign 9:PI 10:PO
	int realGate; // -1:default
};

struct Graph
{
	vector< Node* > netlist;
	vector< Node* > PI;
	vector< Node* > PO;
	map<string, Node*> PIMAP;
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
Node* initialNewnode(string name, int type);
// Select gate type
int selectGateType(string gate);

void topologicalSort(Graph& graph, vector<Node*>& sortNode);
void topologicalSortUtil(Graph& graph, Node* node, map<Node*, bool>& visited, stack<Node*>& Stack);
void setNodePIandSeed (Graph& graph, vector<Node*>& sortNode);
bool existPiSet(vector<Node*>& fanoutgate, Node* pi);

//Set the random seed
void setRandomSeed(Graph& R1, Graph& R2, Graph& G1);
// Give random seed
unsigned* getRandomSeed();
//BitWiseOperation
void BitWiseOperation(vector<unsigned*>& fainSeed, Node* currNode);

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

int nWords = 1;
// inpurt format
//./eco R1.v R2.v G1.v patch.v 
int main(int argc, char* argv[])
{
	//original netlist
	Graph R1;
	//golden netlist
	Graph R2;
	//optimize netlist
	Graph G1;


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
	//Save netlist in topological sort oder
	vector<Node*> sortR1;
	vector<Node*> sortR2;
	vector<Node*> sortG1;

	////Start topological sort
	//In order to set ID and PI
	topologicalSort(R1, sortR1);
	topologicalSort(R2, sortR2);
	topologicalSort(G1, sortG1);

	//Set all nodes Primary Input
	setNodePIandSeed (R1, sortR1);
	setNodePIandSeed (R2, sortR2);
	setNodePIandSeed (G1, sortG1);
	//outFile(R2, argv[4]);
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
			if (graph.netlist[i]->name == split_command) { isexist = true; scanNode = graph.netlist[i]; if (count)currGate = graph.netlist[i]; break; }


		if (!isexist) {  // not exist
			Node* req = initialNewnode(split_command, -1);
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
					Node* newnode = initialNewnode(n1->name + "_" + n2->name, currGate->type);
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
					Node* newnode = initialNewnode(n1->name + "_" + n2->name, currGate->type);
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
Node* initialNewnode(string name, int type) {
	Node* newnode = new Node();
	newnode->name = name;
	newnode->realGate = -1;
	newnode->type = type;
	newnode->seeds = new unsigned[nWords];
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

void topologicalSort(Graph& graph, vector<Node*>& sortNode)
{
	stack<Node*> Stack;
	map<Node*, bool> visited;

	vector<Node*>::iterator it1 = graph.netlist.begin();
	for (; it1 != graph.netlist.end(); ++it1)
		visited[*it1] = false;

	map<Node*, bool>::iterator it2 = visited.begin();
	for (; it2 != visited.end(); ++it2) {
		if (!it2->second)
			topologicalSortUtil(graph, it2->first, visited, Stack);
	}

	int pos = 0;
	sortNode.resize(Stack.size());
	while (Stack.empty() == false) {
		//cout << Stack.top() << " ";
		sortNode[pos++] = Stack.top();
		Stack.pop();
	}
}
void topologicalSortUtil(Graph& graph, Node* node, map<Node*, bool>& visited, stack<Node*>& Stack)
{
	// Mark the current node as visited
	visited[node] = true;
	for (int i = 0; i < node->fanout.size(); ++i) {
		Node* nextNode = node->fanout[i];
		if (!visited[nextNode])
			topologicalSortUtil(graph, nextNode, visited, Stack);
	}
	Stack.push(node);
}
void setNodePIandSeed (Graph& graph, vector<Node*>& sortNode)
{
	for (int i = 0; i < sortNode.size(); ++i) {
		Node* currNode = sortNode[i];
		if (currNode->fanin.size() > 0) {
			vector<unsigned*> faninSeed;
			for (int j = 0; j < currNode->fanin.size(); ++j) {
				//set the PI to the PISET
				Node* faninNode = currNode->fanin[j];
				currNode->piset.insert(faninNode->piset.begin(), faninNode->piset.end());
				//record fanin seed
				faninSeed.push_back(faninNode->seeds);
			}
			BitWiseOperation(faninSeed,currNode);
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
		if (type == 1) {
			for (int i = 0; i < nWords; ++i) currNode-> seeds[i] = faninSeed[0][i] & faninSeed[1][i];
		}
		//or gate
		else if (type == 2) {
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = faninSeed[0][i] | faninSeed[1][i];
		}
		//nand gate
		else if (type == 3) {
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = ~(faninSeed[0][i] & faninSeed[1][i]);
		}
		//nor gate
		else if (type == 4) {
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = ~(faninSeed[0][i] | faninSeed[1][i]);
		}
		//xor gate
		else if (type == 5) {
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = faninSeed[0][i] ^ faninSeed[1][i];
		}
		//xnor gate
		else if (type == 6) {
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = ~(faninSeed[0][i] ^ faninSeed[1][i]);
		}
	}
	else{
		//not gate
		if (type == 0) {
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = ~faninSeed[0][i];
		}
		//buffer
		else if (type == 7) {
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = faninSeed[0][i] ;
		}
		//assign
		else if (type == 8) {
			for (int i = 0; i < nWords; ++i) currNode->seeds[i] = faninSeed[0][i];
		}
	}
}
bool existPiSet(vector<Node*>& fanoutgate, Node* pi)
{
	for (int i = 0; i < fanoutgate.size(); ++i) {
		if (fanoutgate[i] == pi)
			return true;
	}
	return false;
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

