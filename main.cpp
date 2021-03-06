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
// if one node's fanin have two same nodes could have error...
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
	map<Node*, Node*> backMatches; //record PO to PI structure match and fanout only one
};

struct PatchInfo
{
	map<Node*, Node*> matches;
	map<Node*, bool> originMatch;
	map<Node*, bool> netlist;
	int cost;
};

struct PatchGraph
{
	Graph graph;
	PatchInfo info;
};

struct NodeName
{
	Node* node;
	string graphName;
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
void topologicalSort(Graph& graph, int& idStart);
//topological sort recursive
void topologicalSortUtil(Graph& graph, Node* node, map<Node*, bool>& visited, stack<Node*>& Stack);
//Set every node piset and bitwise operation seed
void setNodePIsetandSeed(Graph& graph);
//Set the random seed
void setRandomSeed(Graph& R2, Graph& G1);
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
void outputPIwithFaninCone(ofstream& outfile, Node* nextNode, vector<NodeName>& internalNode, vector<bool>& faninConst);
//check if this node's fanin is PI
bool faninIsPI(Node* nextNode);
//remove fanin node 
void removeAllFanin(MatchInfo& matchInfo, Node* originalSameNode, Node* goldenSameNode);


//outputBlif
void outputBlif(ofstream& outfile, Node* originalNode, Node* goldenNode);
//transfer graph to blif file and write blif file
void graph2Blif(Node* originalNode, Node* goldenNode);
//find the node's fanout and call node2Blif 
void netlist2Blif(ofstream& outfile, vector<NodeName>& netlist);
//output constant 0 or 1 in blif file
void outputConst(ofstream& outfile, vector<bool>& faninConst);
//output .names to BLIF File
void outputDotNames(ofstream& outfile, Node* currNode, string currGraphName);
//write gate type ex: and gate -> 11 1
void node2Blif(ofstream& outfile, Node* currNode, int type);
//let original POs and Golden POs connet to the XOR to make the miter
void buildMiter(ofstream& outfile, Node* PO_original, Node* PO_golden, int miterPos, string originalGraphName, string goldenGraphName);
//call abc -> "turn blif into cnf" and minisat -> "check if this two netlist is equal"
bool SATsolver();
//check if output is UNSAT
bool readSATsolverResult();
//abc tool
void abcBlif2CNF();


// From PO to PI Comapre
void backStructureComapre(Graph origin, Graph golden, MatchInfo& matchInfo);
// Check PO to PI all path gate type equal 
void backCheckStructureEqual(Node* origin, Node* golden, MatchInfo& matchInfo);
// Check gate type equal
bool checkGateTypeEqual(Node* origin, Node* golden);




// generate patch verilog
PatchGraph generatePatchVerilog(MatchInfo& matchInfo, Graph& R2, Graph& G1, char* argv);
// generate verilog instruction
string generateInstruction(Node* node, vector<string> names, int eco, int& cost);
// get type string
string getTypeString(int type);
// outfile declare variable
void generateDeclare(map<Node*, string> maps, string types, ofstream& outfile, int& cost);
// patch format
string generatePatchFormat(Node* node);
// is constant
void isConstantCondition(string& name, map<string, bool>& useConstant, Node* node, int& totalCost);
// is in match
void isInMatchCondition(string& name, map<Node*, Node*> matches, Node* node, map<Node*, string>& inputDeclareMap);
// is in goldenremoveNode
void isInGoldenRemoveNode(string& name, map<Node*, string>& newGateMap, Node* node);
// is randomsimulation remove from goldenremovenode and need to declare this leaking gate
void isLeakingNode(string& name, vector<Node*>& leakingNodeVec, Node* node, map<Node*, string>& newGateMap);
// accounting
PatchGraph costAccounting(MatchInfo& matchInfo, Graph& R2, Graph& G1);


// generate patch PI/PO node and put them in graph
void generatePatchPIPO(string type, string name, Graph& graph);
// configure patch name
void configurePatch(Graph& graph);
// match name
void nodeMatch(Graph& patch, Graph& origin, PatchInfo& info);
// remove extra node
void removeExtraNode(Graph& patch, Graph& origin, PatchInfo& info);
// remove all fanin single fanout node
void removeSingleFanout(Node* node, PatchInfo& info);
// apply node
void applyNode(Graph& patch, PatchInfo& info);
// generate patchG1 graph
void generatePatchG1(Graph& origin, PatchInfo& info, Graph& patchG1);

//// Output patch
//void outFile(Graph graph, char* argv);
//// Process same node name but multi Bracket(like op[0],op[1],op[2],.....) and count name times
//void strExtractBracket(Graph graph, vector<Node>& pi_items, vector<Node>& po_items);
//// Output declare type(input/output/wire)
//void outputFront(ofstream& outfile, vector<Node> p, string str);
//// Output gate
//void gateOutput(ofstream& outfile, Graph graph);
//// Sort by type and number count
//bool typeAndNumberCompare(const Node& p1, const Node& p2);
//// Sort by node name
//bool strTitleCompare(const string& p1, const string& p2);


void patchOptimize(MatchInfo& matchInfo);


//void createRectifyPair(Graph& R2, Graph& G1);
//bool pisetIsDifferent(Node object, Node golden);

//out patch blif to optimize
void outputPatchBlif(Graph& currPatchGraph, map<Node*, bool>& isVisitedPatch);
//read optimized patch blif file
void readOptPatchBlif(Graph& currPatchGraph, map<Node*, bool>& newGoldenRemoveNode);
//read the patch blif PI
void readOptPatchPI(ifstream& infile, Graph& currPatchGraph, map<string, string>& patchPI);
//read the patch blif PO
void readOptPatchPO(ifstream& infile, Graph& currPatchGraph, map<string, string>& patchPI);

//transfer blif command to graph
void blif2Graph(ifstream& infile, string& line, Graph& currPatchGraph, map<Node*, bool>& newGoldenRemoveNode,
	map<string, Node*>& checkExist, int& notGatePos, map<string, string>& patchPI, map<string, string>& patchPO);
//return blif file gate type
int selectBlifGateType(ifstream& infile);
//solve the proble of blif file (ex:10 1,01 1)
Node* connectNewNotGate(Node* faninNode, string& notGateName, int& notGatePos, map<Node*, bool>& newGoldenRemoveNode);
//find two fanin node
bool faninNodeisLegal(Node* currNode, map<Node*, bool>& newGoldenRemoveNode, int& gatePos);
//combine into XRO gate or XNOR gate
void removeRedundantNode(map<Node*, bool>& newGoldenRemoveNode);
//start rebuild new node
void startRebuildNode(Node* fanoutNode, set<Node*>& redundantFaninNode, set<Node*>& redundantNode,
	map<Node*, bool>& newGoldenRemoveNode, int type, int& gatePos);
//remove fanout node's redundant fanin
void removeRedundantFanin(Node* redundantNode, Node* newNode);
//remove fanin node's redundant fanout
void removeRedundantFanout(Node* faninNode, set<Node*>& redundantNode);
//solve the problem of old node in the fanin node's fanout
void removeOldNode(Graph currPatchGraph, map<Node*, bool>& newGoldenRemoveNode, map<Node*, bool>& oldGoldenRemoveNode);
//call the abc to optimize patch
void optimizePatch();


/*start verify patch*/
//start verify with minisat
bool patchSelfVerify();


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
	patchVerify  ->  checkRemoveNodeFaninExist
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


int nWords = 10;
int nodeID = 1;
//for debug pointer
Node* debug;
// inpurt format
//./eco R1.v R2.v G1.v patch.v 
int main(int argc, char* argv[])
{
	srand(time(NULL));
	//golden netlist
	Graph R2;
	vector<MatchInfo> eachMatchInfoVersion;
	R2.name = "R2";
	//optimize netlist
	Graph G1;
	G1.name = "G1";


	//Load R2 golden netlist
	//assign value is fault, need to retrieve fan_in value in next step
	loadFile(R2, argv[2]);
	//Load G1 optimize netlist
	//assign value is fault, need to retrieve fan_in value in next step
	loadFile(G1, argv[3]);

	//Set random seed to PI
	setRandomSeed(R2, G1);

	////Start topological sort
	//In order to set ID and PI
	topologicalSort(R2, nodeID);
	topologicalSort(G1, nodeID);

	//Set all nodes Primary Input
	setNodePIsetandSeed(R2);
	setNodePIsetandSeed(G1);

	MatchInfo matchInfo;
	PatchGraph test;
	int currcost;
	structureCompareMain(G1, R2, matchInfo);
	//cout << "structureCompareMain\n";
	test = costAccounting(matchInfo, R2, G1);
	currcost = test.info.cost;
	int k = 0;
	//record matchInfo w/ struct compare
	eachMatchInfoVersion.push_back(matchInfo);
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
	/*for (map<Node*, bool>::iterator it = matchInfo.goldenRemoveNode.begin(); it != matchInfo.goldenRemoveNode.end(); ++it) {
		if (it->first->name == "n_505")
			cout << "505---\n";
		if (it->first->name == "n_264")
			cout << "264---\n";
	}*/

	MatchInfo m1 = matchInfo;
	randomSimulation(m1);
	//cout << "randomSimulation\n";
	test = costAccounting(m1, R2, G1);
	//cout << test.info.cost << " : " << currcost << "\n";
	if (test.info.cost <= currcost) {
		currcost = test.info.cost;
		matchInfo = m1;
		eachMatchInfoVersion.push_back(m1);
	}
	/*for (map<Node*, bool>::iterator it = matchInfo.goldenRemoveNode.begin(); it != matchInfo.goldenRemoveNode.end(); ++it) {
		if (it->first->name == "n_505")
			cout << "505***\n";
		if (it->first->name == "n_264")
			cout << "264***\n";
	}*/
	//cout << "after random\n";

	MatchInfo m2 = matchInfo;
	/*start optimize*/
	backStructureComapre(G1, R2, m2);
	//cout << "backStructureComapre\n";
	test = costAccounting(m2, R2, G1);
	//cout << test.info.cost << " : " << currcost << "\n";
	if (test.info.cost <= currcost) {
		currcost = test.info.cost;
		matchInfo = m2;
		eachMatchInfoVersion.push_back(m2);
	}

	MatchInfo m3 = matchInfo;
	//start optimize patch with abc tool
	patchOptimize(m3);
	//cout << "patchOptimize\n";
	test = costAccounting(m3, R2, G1);
	//cout << test.info.cost << " : " << currcost << "\n";
	if (test.info.cost <= currcost) {
		currcost = test.info.cost;
		matchInfo = m3;
		eachMatchInfoVersion.push_back(m3);
	}
	//cout << "eachVersionMatchInfo size : " << eachMatchInfoVersion.size() << endl;


	//output the patch.v
	for (int m = eachMatchInfoVersion.size() - 1; m >= 0; --m) {
		PatchGraph patch;
		Graph patchG1;
		MatchInfo currMatchInfo = eachMatchInfoVersion[m];
		patch = generatePatchVerilog(currMatchInfo, R2, G1, argv[4]);
		configurePatch(patch.graph);
		nodeMatch(patch.graph, G1, patch.info);
		removeExtraNode(patch.graph, G1, patch.info);
		applyNode(patch.graph, patch.info);
		generatePatchG1(G1, patch.info, patchG1);
		int i = 0;
		//make sure the patch is correct
		if (patchSelfVerify() || m == 0) {
			//cout << "Success output patch version:" << m+1 << endl;
			break;
		}
	}
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
	/*if (verilog_command.find("buf") != string::npos)
		int i = 0;*/
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
		while (split_command[0] == ' ' || split_command[0] == '\n') //delete front blank
			split_command = split_command.substr(1, split_command.size() - 1);
		if (split_command[split_command.size() - 1] == ')') //delete right parentheses
			split_command = split_command.substr(0, split_command.size() - 1);
		while (split_command[split_command.size() - 1] == ' ' || split_command[split_command.size() - 1] == '\n') //delete front blank
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
					Node* newnode = initialNewnode(n1->name + "_" + n2->name + "_" + toString(nodeID++), currtype, graph.name);
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
					Node* newnode = initialNewnode(n1->name + "_" + n2->name + "_" + toString(nodeID++), currtype, graph.name);

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
			while (split_command[0] == ' ') //delete front blank
				split_command = split_command.substr(1, split_command.size() - 1);
			while (split_command[split_command.size() - 1] == ' ') //delete front blank
				split_command = split_command.substr(0, split_command.size() - 1);
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
			while (split_command[split_command.size() - 1] == ' ') //delete front blank
				split_command = split_command.substr(0, split_command.size() - 1);
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

void setRandomSeed(Graph& R2, Graph& G1)
{
	for (int i = 0; i < R2.PI.size(); ++i) {
		//get the PI name
		string PIname = R2.PI[i]->name;

		//get the random seed memory location
		unsigned* seed = getRandomSeed();

		R2.PIMAP[PIname]->seeds = seed;
		G1.PIMAP[PIname]->seeds = seed;
	}
}

void topologicalSort(Graph& graph, int& idStart)
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
		//Stack.top()->id = pos + idStart;
		Stack.top()->id = idStart++;
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
		/*if (goldenPtr->name == "overflow")
			cout << "point";*/
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
	//set<Node*>::iterator req = matchInfo.originSupprotSet.begin();
	//for (set<Node*>::iterator it = matchInfo.originSupprotSet.begin(); it != matchInfo.originSupprotSet.end(); it = req) {
	//	++req;
	//	bool updates = true;
	//	Node* accesses = *it;
	//	for (int m = 0; m < accesses->fanout.size(); m++)
	//		if (matchInfo.originRemoveNode.find(accesses->fanout[m]) != matchInfo.originRemoveNode.end()) {
	//			updates = false;
	//			break;
	//		}
	//	if (updates) {
	//		for (int m = 0; m < accesses->fanout.size(); m++)
	//			matchInfo.originSupprotSet.insert(accesses->fanout[m]);
	//		matchInfo.originSupprotSet.erase(it);
	//	}
	//}
	//req = matchInfo.goldenSupprotSet.begin();
	//for (set<Node*>::iterator it = matchInfo.goldenSupprotSet.begin(); it != matchInfo.goldenSupprotSet.end(); it = req) {
	//	++req;
	//	bool updates = true;
	//	Node* accesses = *it;
	//	for (int m = 0; m < accesses->fanout.size(); m++)
	//		if (matchInfo.matches.find(accesses->fanout[m]) == matchInfo.matches.end()) {
	//			updates = false;
	//			break;
	//		}
	//	if (updates) {
	//		for (int m = 0; m < accesses->fanout.size(); m++)
	//			matchInfo.goldenSupprotSet.insert(accesses->fanout[m]);
	//		matchInfo.goldenSupprotSet.erase(it);
	//	}
	//}
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
	map<Node*, Node*> removeMAP;
	bool findSameNode = false;
	for (; og_it != matchInfo.originRemoveNode.end();) {
		findSameNode = false;
		gd_it = matchInfo.goldenRemoveNode.begin();
		for (; gd_it != matchInfo.goldenRemoveNode.end(); ++gd_it) {
			if (!seedIsDifferent(og_it->first, gd_it->first)) {
				//turn this two gate fanin cone into blif file
				if (og_it->first->piset.size() == gd_it->first->piset.size()) {
					if (og_it->first->piset_str == gd_it->first->piset_str) {//add this if condition
						graph2Blif(og_it->first, gd_it->first);
						//call SAT solver
						if (SATsolver()) {
							//cout << "Success golden: " << gd_it->first->name << " <-equal-> original: " << og_it->first->name << endl;
							matchInfo.matches[gd_it->first] = og_it->first;
							removeMAP[gd_it->first] = og_it->first;
						}
					}
				}
			}
		}
		//if not find the same node
		if (!findSameNode)
			++og_it;
		if (matchInfo.originRemoveNode.size() == 0 || matchInfo.goldenRemoveNode.size() == 0)
			break;
	}

	map<Node*, Node*>::iterator it = removeMAP.begin();
	for (; it != removeMAP.end(); ++it) {
		removeAllFanin(matchInfo, it->second, it->first);
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
	vector<NodeName> internalNode;
	vector<bool> faninConst;
	faninConst.resize(2, false);

	outputBlif(outfile, originalNode, goldenNode);
	outputPIwithFaninCone(outfile, originalNode, internalNode, faninConst);
	outputPIwithFaninCone(outfile, goldenNode, internalNode, faninConst);
	outputConst(outfile, faninConst);
	netlist2Blif(outfile, internalNode);
	buildMiter(outfile, originalNode, goldenNode, 0, originalNode->graphName, goldenNode->graphName);

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

void outputPIwithFaninCone(ofstream& outfile, Node* nextNode, vector<NodeName>& internalNode, vector<bool>& faninConst)
{
	//!!!!!
	//need to modify because of faninCone 
	//so here can use piset to output pi fanout into blif file
	//!!!!!
	set<Node*>::iterator it = nextNode->faninCone.begin();
	for (; it != nextNode->faninCone.end(); ++it) {
		if (faninIsPI(*it))
			outputDotNames(outfile, *it, nextNode->graphName);
		else if ((*it)->type != 9) {
			if ((*it)->name == "1'b0" && !faninConst[0])
				faninConst[0] = true;
			else if ((*it)->name == "1'b1" && !faninConst[1])
				faninConst[1] = true;

			NodeName newNodewithName;
			newNodewithName.node = *it;
			newNodewithName.graphName = nextNode->graphName;
			internalNode.push_back(newNodewithName);
		}
	}

}

void netlist2Blif(ofstream& outfile, vector<NodeName>& netlist)
{
	for (int i = 0; i < netlist.size(); ++i) {
		outputDotNames(outfile, netlist[i].node, netlist[i].graphName);
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
	if (currNode->name == "n1241_n1242")
		int k = 0;
		//cout << "!!!";

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
		//if (currNode->fanin[i]->graphName == currGraphName || currGraphName == "patch" || currGraphName == "patchG1") {
		outfile << " " << currNode->fanin[i]->name;
		if (currNode->fanin[i]->type != 9 && currNode->fanin[i]->name != "1'b0" && currNode->fanin[i]->name != "1'b1")
			outfile << "_" + currGraphName;
		//}
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

void buildMiter(ofstream& outfile, Node* PO_original, Node* PO_golden, int miterPos, string originalGraphName, string goldenGraphName)
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

	//originalGraphName equal to G1 or patchG1
	outfile << ".names";
	outfile << " " << PO_original->name + "_" + originalGraphName;
	outfile << " " << PO_golden->name + "_" + goldenGraphName;

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
	//system("./blif2cnf.out ./blif/check.blif");
}

void backStructureComapre(Graph origin, Graph golden, MatchInfo& matchInfo)
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
		Node* goldenNode = 0;
		if (matchInfo.originRemoveNode.find(originNode) == matchInfo.originRemoveNode.end())
			continue;
		if (goldenPoMap.find(it->first) != goldenPoMap.end())
			goldenNode = goldenPoMap.find(it->first)->second;
		if (matchInfo.originRemoveNode.find(originNode) == matchInfo.originRemoveNode.end())
			continue;
		if (goldenNode == 0 || matchInfo.goldenRemoveNode.find(goldenNode) == matchInfo.goldenRemoveNode.end())
			continue;
		if (originNode->fanout.size() != 0 || goldenNode->fanout.size() != 0)
			continue;
		if (getTypeString(originNode->realGate) == "assign" || getTypeString(goldenNode->realGate) == "assign")
			continue;
		if (checkGateTypeEqual(originNode, goldenNode))
			backCheckStructureEqual(originNode, goldenNode, matchInfo);
	}

	int k = 0;
}


void backCheckStructureEqual(Node* origin, Node* golden, MatchInfo& matchInfo)
{
	map<Node*, bool> originFaninMap;
	map<Node*, bool> goldenFaninMap;
	map<Node*, Node*> currentMatch;

	// check in removeOriginNode and put into Map 
	for (int i = 0; i < origin->fanin.size(); i++) {
		if (origin->fanin[i]->name == "1'b1" || origin->fanin[i]->name == "1'b0")
			originFaninMap[origin->fanin[i]] = true;
		else if (matchInfo.originRemoveNode.find(origin->fanin[i]) == matchInfo.originRemoveNode.end()) //match or have been recognize in randomSimulation
			continue;
		else if (origin->fanin[i]->fanout.size() < 2)
			originFaninMap[origin->fanin[i]] = true;
	}

	// check in removeGoldenNode and put into Map 
	for (int i = 0; i < golden->fanin.size(); i++) {
		if (golden->fanin[i]->name == "1'b1" || golden->fanin[i]->name == "1'b0")
			goldenFaninMap[golden->fanin[i]] = true;
		else if (matchInfo.goldenRemoveNode.find(golden->fanin[i]) == matchInfo.goldenRemoveNode.end()) //match or have been recognize in randomSimulations
			continue;
		else if (golden->fanin[i]->fanout.size() < 2)
			goldenFaninMap[golden->fanin[i]] = true;
	}

	// check fanin match
	for (map<Node*, bool>::iterator goldenptr = goldenFaninMap.begin(); goldenptr != goldenFaninMap.end(); ++goldenptr) {
		Node* originNode = 0;
		Node* goldenNode = goldenptr->first;
		for (map<Node*, bool>::iterator originptr = originFaninMap.begin(); originptr != originFaninMap.end(); ++originptr) {
			originNode = originptr->first;
			if (checkGateTypeEqual(originNode, goldenNode)) {
				currentMatch[goldenNode] = originNode;
				originFaninMap.erase(originNode);
				break;
			}
		}
	}

	// chaeck all fanin is matching
	if (currentMatch.size() == origin->fanin.size()) {
		matchInfo.matches[golden] = origin;
		for (map<Node*, Node*>::iterator it = currentMatch.begin(); it != currentMatch.end(); ++it) {
			Node* originNode = it->second;
			Node* goldenNode = it->first;
			if (originNode->name == "1'b1" || originNode->name == "1'b0")
				continue;
			if (goldenNode->name == "1'b1" || goldenNode->name == "1'b0")
				continue;
			backCheckStructureEqual(originNode, goldenNode, matchInfo);
		}
	}
	else
		matchInfo.backMatches[golden] = origin;

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
	if (origin->type != golden->type || origin->realGate != golden->realGate)
		return false;
	if (origin->realGate == 8 || golden->realGate == 8)
		return false;
	return true;
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



PatchGraph generatePatchVerilog(MatchInfo& matchInfo, Graph& R2, Graph& G1, char* argv)
{
	ofstream outfile(argv);
	map<Node*, string> inputDeclareMap; //module(....) input....
	map<Node*, string> outputDeclareMap; //module(....) output....
	map<Node*, string> newGateMap; //wire...
	map<Node*, Node*> matches = matchInfo.matches;
	map<Node*, Node*> backMatches = matchInfo.backMatches;
	map<Node*, bool> originRemoveNode = matchInfo.originRemoveNode; //inherit
	map<Node*, bool> goldenRemoveNode = matchInfo.goldenRemoveNode; //inherit
	vector<Node*> leakingNodeVec;
	vector<string> instructionSet; //and(X,X,X)....
	map<string, bool> useConstant{ {"1'b0",false},{"1'b1",false} }; //check whether using constant
	int ecoNumber = 1;
	int totalCost = 0;

	// PO-PI's match handler
	for (map<Node*, Node*>::iterator it = backMatches.begin(); it != backMatches.end(); ++it) {
		vector<string> names = { "","","" }; // [0]:fanin1 [1]:fanin2 [2]:node.name
		Node* originNode = it->second;
		Node* goldenNode = it->first;
		//originNode->fanin.clear();
		//originNode->fanin.resize(goldenNode->fanin.size());

		// fanin node
		for (int i = 0; i < goldenNode->fanin.size(); i++) {
			Node* faninNode = goldenNode->fanin[i];
			if (faninNode->name == "1'b0" || faninNode->name == "1'b1") // is constant
				isConstantCondition(names[i], useConstant, faninNode, totalCost);
			else if (matches.find(faninNode) != matches.end())   // in match
				isInMatchCondition(names[i], matches, faninNode, inputDeclareMap);
			else if (goldenRemoveNode.find(faninNode) != goldenRemoveNode.end()) // in goldenRemoveNode
				isInGoldenRemoveNode(names[i], newGateMap, faninNode);
			else
				isLeakingNode(names[i], leakingNodeVec, faninNode, newGateMap);

			if (names[i] == "") { //have error
				//cout << "PO to PI generate patch Error!\n";
			}
		}
		names[2] = generatePatchFormat(originNode);
		outputDeclareMap[originNode] = names[2];
		string instrucs = generateInstruction(originNode, names, ecoNumber++, totalCost);
		if (instrucs == "error") {
			//cout << "generatePatchVerilog backMatches error!\n";
			continue;
		}
		instructionSet.push_back(instrucs);
	}
	//cout << "after po-pi\n";
	// goldenRemoveNode handler
	for (map<Node*, bool>::iterator it = goldenRemoveNode.begin(); it != goldenRemoveNode.end(); ++it) {
		Node* currentNode = it->first;
		vector<string> names = { "","","" }; // [0]:fanin1 [1]:fanin2 [2]:node.name
		// fanin node
		for (int i = 0; i < currentNode->fanin.size(); i++) {
			Node* faninNode = currentNode->fanin[i];
			if (faninNode->name == "1'b0" || faninNode->name == "1'b1") // is constant
				isConstantCondition(names[i], useConstant, faninNode, totalCost);
			else if (matches.find(faninNode) != matches.end())  // in match
				isInMatchCondition(names[i], matches, faninNode, inputDeclareMap);
			else if (goldenRemoveNode.find(faninNode) != goldenRemoveNode.end()) // in goldenRemoveNode
				isInGoldenRemoveNode(names[i], newGateMap, faninNode);
			else
				isLeakingNode(names[i], leakingNodeVec, faninNode, newGateMap);

			if (names[i] == "") { //have error
				//cout << "goldenRemoveNode generate patch Error!\n";
			}
		}
		//names[2] = generatePatchFormat(currentNode);
		isInGoldenRemoveNode(names[2], newGateMap, currentNode);
		if (currentNode->type == 10)
			outputDeclareMap[currentNode] = names[2];
		string instrucs = generateInstruction(currentNode, names, ecoNumber++, totalCost);
		if (instrucs == "error") {
			//cout << "generatePatchVerilog goldenRemoveNode error!\n";
			continue;
		}
		instructionSet.push_back(instrucs);
	}
	//cout << "after goldenRemoveNode\n";

	//leakingNode handler
	for (int i = 0; i < leakingNodeVec.size(); i++) {
		Node* currentNode = leakingNodeVec[i];
		vector<string> names = { "","","" }; // [0]:fanin1 [1]:fanin2 [2]:node.name
		// fanin node
		for (int i = 0; i < currentNode->fanin.size(); i++) {
			Node* faninNode = currentNode->fanin[i];
			if (faninNode->name == "1'b0" || faninNode->name == "1'b1") // is constant
				isConstantCondition(names[i], useConstant, faninNode, totalCost);
			else if (matches.find(faninNode) != matches.end())  // in match
				isInMatchCondition(names[i], matches, faninNode, inputDeclareMap);
			else if (goldenRemoveNode.find(faninNode) != goldenRemoveNode.end()) // in goldenRemoveNode
				isInGoldenRemoveNode(names[i], newGateMap, faninNode);
			else
				isLeakingNode(names[i], leakingNodeVec, faninNode, newGateMap);

			if (names[i] == "") { //have error
				//cout << "leakingNode generate patch Error!\n";
			}
		}
		names[2] = newGateMap[currentNode];
		instructionSet.push_back(generateInstruction(currentNode, names, ecoNumber++, totalCost));

	}
	//cout << "after leakingNode\n";

	PatchGraph patch;
	patch.graph.name = "patch";
	patch.graph.Constants.resize(2);
	// output file
	outfile << "module top_eco(";
	// module(input.....)
	int count = 0;
	for (map<Node*, string>::iterator it = inputDeclareMap.begin(); it != inputDeclareMap.end(); ++it) {
		if (count == 0) {
			generatePatchPIPO("input", it->second, patch.graph);
			outfile << it->second;
			count++;
			continue;
		}
		if (outputDeclareMap.find(it->first) != outputDeclareMap.end()) //output prior to input
			continue;
		if (it->second.find("\\") == string::npos)
			outfile << ", " << it->second;
		else
			outfile << "," << it->second;
		generatePatchPIPO("input", it->second, patch.graph);
	}
	//cout << "after input\n";

	// module(output.....)
	for (map<Node*, string>::iterator it = outputDeclareMap.begin(); it != outputDeclareMap.end(); ++it) {
		generatePatchPIPO("output", it->second, patch.graph);
		if (inputDeclareMap.find(it->first) != inputDeclareMap.end())
			inputDeclareMap.erase(it->first);
		if (count == 0) {
			outfile << it->second;
			count++;
			continue;
		}
		else if (it->second.find("\\") == string::npos)
			outfile << ", " << it->second;
		else
			outfile << "," << it->second;

	}
	outfile << ");\n";
	//cout << "after output\n";
	// input.... output....  wire....
	//cout << totalCost << "\n";
	generateDeclare(inputDeclareMap, "input", outfile, totalCost);
	//cout << totalCost << "\n";
	generateDeclare(outputDeclareMap, "output", outfile, totalCost);
	//cout << totalCost << "\n";
	generateDeclare(newGateMap, "wire", outfile, totalCost);
	//cout << totalCost << "\n";

	// and or not....
	vector<Node*> regists;
	for (int i = 0; i < instructionSet.size(); i++) {
		outfile << instructionSet[i] << "\n";
		string command = instructionSet[i].substr(0, instructionSet[i].size() - 1);
		verilog2graph(command, patch.graph, regists);
	}
	outfile << "endmodule\n";
	//cout << "Success cost:" << totalCost << "\n";
	patch.info.cost = totalCost;
	return patch;

}

string generateInstruction(Node* node, vector<string> names, int eco, int& cost)
{
	string gate = getTypeString(node->type);
	string res = "  ";
	stringstream ss;
	ss << eco;
	if (gate == "PO")
		gate = getTypeString(node->realGate);
	if (gate == "error") {
		//cout << names[2] << "--->" << "generateInstruction Error !" << endl;
		return "error";
	}

	if (gate == "not" || gate == "buf" || gate == "assign") {
		if (gate == "assign")
			gate = "buf";
		if (names[0].find("\\") == string::npos)
			res += gate + " eco" + ss.str() + " (" + names[2] + ", " + names[0] + ");";
		else
			res += gate + " eco" + ss.str() + " (" + names[2] + "," + names[0] + ");";
		cost += -1;
	}
	else {
		string loc1 = ", ";
		string loc2 = ", ";
		if (names[0].find("\\") != string::npos)
			loc1 = ",";
		if (names[1].find("\\") != string::npos)
			loc2 = ",";
		res += gate + " eco" + ss.str() + " (" + names[2] + loc1 + names[0] + loc2 + names[1] + ");";
	}
	//else if (gate == "assign")
	//	res += gate + " " + names[2] + " = " + names[0] + ";";
	return res;
}

string getTypeString(int type)
{
	if (type == -1) {
		//cout << "getTypeString function error!! (type==-1)" << endl;
		return "error";
	}
	vector<string> gates = { "not","and","or","nand","nor","xor","xnor","buf","assign","PI","PO" };
	return gates[type];
}

void generateDeclare(map<Node*, string> maps, string types, ofstream& outfile, int& cost)
{
	int last = 1;
	int count = 1;
	bool first = true;
	for (map<Node*, string>::iterator it = maps.begin(); it != maps.end(); ++it) {
		if (types != "wire") {
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
		}
		count++;
		last++;
		cost += 1;
		if (count == 5)
			count = 0;
	}
}

string generatePatchFormat(Node* node)
{
	string name = node->name;
	if (name == "1'b1" || name == "1'b0")
		return name;
	else if (node->graphName == "G1") {
		if (name.find("[") != string::npos)
			name = "\\" + name + " ";
	}
	else if (node->graphName == "R2" || node->graphName == "patch") {
		if (node->type == 10) { //is PO reprent it need to be using G1's PO
			if (name.find("[") != string::npos)
				name = "\\" + name + " ";
		}
		else {
			if (name.find("[") != string::npos) {  // in R2 represent it need to be add new gate
				string req = name.substr(name.find("[") + 1, name.find("]") - name.find("[") - 1);
				name = name.substr(0, name.find("[")) + req;
				name = "patchNew_" + name + "_" + toString(node->id);
			}
			else
				name = "patchNew_" + name + "_" + toString(node->id);
		}

	}
	return name;
}

void isConstantCondition(string& name, map<string, bool>& useConstant, Node* node, int& totalCost)
{
	name = generatePatchFormat(node);
	if (useConstant[node->name] == false) {
		useConstant[node->name] = true;
		totalCost++;
	}
}

void isInMatchCondition(string& name, map<Node*, Node*> matches, Node* node, map<Node*, string>& inputDeclareMap)
{
	Node* faninMatchNode = matches.find(node)->second;
	name = generatePatchFormat(faninMatchNode);
	inputDeclareMap[faninMatchNode] = name; //have risk need to solve in behind cout condition
}

void isInGoldenRemoveNode(string& name, map<Node*, string>& newGateMap, Node* node)
{
	name = generatePatchFormat(node);
	if (node->type != 10) {
		if (newGateMap.find(node) != newGateMap.end())
			name = newGateMap.find(node)->second;
		else {
			newGateMap[node] = name;
		}
	}
}

void isLeakingNode(string& name, vector<Node*>& leakingNodeVec, Node* node, map<Node*, string>& newGateMap)
{
	name = generatePatchFormat(node);
	if (newGateMap.find(node) == newGateMap.end()) {
		leakingNodeVec.push_back(node);
		newGateMap[node] = name;
	}
	else
		name = newGateMap.find(node)->second;

}

PatchGraph costAccounting(MatchInfo& matchInfo, Graph& R2, Graph& G1)
{
	//cout << "---------costAccounting--------------\n";
	map<Node*, string> inputDeclareMap; //module(....) input....
	map<Node*, string> outputDeclareMap; //module(....) output....
	map<Node*, string> newGateMap; //wire...
	map<Node*, Node*> matches = matchInfo.matches;
	map<Node*, Node*> backMatches = matchInfo.backMatches;
	map<Node*, bool> originRemoveNode = matchInfo.originRemoveNode; //inherit
	map<Node*, bool> goldenRemoveNode = matchInfo.goldenRemoveNode; //inherit
	vector<Node*> leakingNodeVec;
	vector<string> instructionSet; //and(X,X,X)....
	map<string, bool> useConstant{ {"1'b0",false},{"1'b1",false} }; //check whether using constant
	int ecoNumber = 1;
	int totalCost = 0;

	// PO-PI's match handler
	for (map<Node*, Node*>::iterator it = backMatches.begin(); it != backMatches.end(); ++it) {
		vector<string> names = { "","","" }; // [0]:fanin1 [1]:fanin2 [2]:node.name
		Node* originNode = it->second;
		Node* goldenNode = it->first;
		//originNode->fanin.clear();
		//originNode->fanin.resize(goldenNode->fanin.size());

		// fanin node
		for (int i = 0; i < goldenNode->fanin.size(); i++) {
			Node* faninNode = goldenNode->fanin[i];
			if (faninNode->name == "1'b0" || faninNode->name == "1'b1") // is constant
				isConstantCondition(names[i], useConstant, faninNode, totalCost);
			else if (matches.find(faninNode) != matches.end())   // in match
				isInMatchCondition(names[i], matches, faninNode, inputDeclareMap);
			else if (goldenRemoveNode.find(faninNode) != goldenRemoveNode.end()) // in goldenRemoveNode
				isInGoldenRemoveNode(names[i], newGateMap, faninNode);
			else
				isLeakingNode(names[i], leakingNodeVec, faninNode, newGateMap);

			if (names[i] == "") { //have error
				//cout << "PO to PI generate patch Error!\n";
			}
		}
		names[2] = generatePatchFormat(originNode);
		outputDeclareMap[originNode] = names[2];
		string instrucs = generateInstruction(originNode, names, ecoNumber++, totalCost);
		if (instrucs == "error") {
			//cout << "generatePatchVerilog backMatches error!\n";
			continue;
		}
		instructionSet.push_back(instrucs);
	}
	//cout << "after po-pi\n";
	// goldenRemoveNode handler
	for (map<Node*, bool>::iterator it = goldenRemoveNode.begin(); it != goldenRemoveNode.end(); ++it) {
		Node* currentNode = it->first;
		vector<string> names = { "","","" }; // [0]:fanin1 [1]:fanin2 [2]:node.name
		// fanin node
		for (int i = 0; i < currentNode->fanin.size(); i++) {
			Node* faninNode = currentNode->fanin[i];
			if (faninNode->name == "1'b0" || faninNode->name == "1'b1") // is constant
				isConstantCondition(names[i], useConstant, faninNode, totalCost);
			else if (matches.find(faninNode) != matches.end())  // in match
				isInMatchCondition(names[i], matches, faninNode, inputDeclareMap);
			else if (goldenRemoveNode.find(faninNode) != goldenRemoveNode.end()) // in goldenRemoveNode
				isInGoldenRemoveNode(names[i], newGateMap, faninNode);
			else
				isLeakingNode(names[i], leakingNodeVec, faninNode, newGateMap);

			if (names[i] == "") { //have error
				//cout << "goldenRemoveNode generate patch Error!\n";
			}
		}
		//names[2] = generatePatchFormat(currentNode);
		isInGoldenRemoveNode(names[2], newGateMap, currentNode);
		if (currentNode->type == 10)
			outputDeclareMap[currentNode] = names[2];
		string instrucs = generateInstruction(currentNode, names, ecoNumber++, totalCost);
		if (instrucs == "error") {
			//cout << "generatePatchVerilog goldenRemoveNode error!\n";
			continue;
		}
		instructionSet.push_back(instrucs);
	}
	//cout << "after goldenRemoveNode\n";

	//leakingNode handler
	for (int i = 0; i < leakingNodeVec.size(); i++) {
		Node* currentNode = leakingNodeVec[i];
		vector<string> names = { "","","" }; // [0]:fanin1 [1]:fanin2 [2]:node.name
		// fanin node
		for (int i = 0; i < currentNode->fanin.size(); i++) {
			Node* faninNode = currentNode->fanin[i];
			if (faninNode->name == "1'b0" || faninNode->name == "1'b1") // is constant
				isConstantCondition(names[i], useConstant, faninNode, totalCost);
			else if (matches.find(faninNode) != matches.end())  // in match
				isInMatchCondition(names[i], matches, faninNode, inputDeclareMap);
			else if (goldenRemoveNode.find(faninNode) != goldenRemoveNode.end()) // in goldenRemoveNode
				isInGoldenRemoveNode(names[i], newGateMap, faninNode);
			else
				isLeakingNode(names[i], leakingNodeVec, faninNode, newGateMap);

			if (names[i] == "") { //have error
				//cout << "leakingNode generate patch Error!\n";
			}
		}
		names[2] = newGateMap[currentNode];
		instructionSet.push_back(generateInstruction(currentNode, names, ecoNumber++, totalCost));

	}
	//cout << "after leakingNode\n";

	PatchGraph patch;
	patch.graph.name = "patch";
	patch.graph.Constants.resize(2);
	for (map<Node*, string>::iterator it = inputDeclareMap.begin(); it != inputDeclareMap.end(); ++it) {
		if (outputDeclareMap.find(it->first) != outputDeclareMap.end()) //output prior to input
			continue;
		totalCost++;
	}

	for (map<Node*, string>::iterator it = outputDeclareMap.begin(); it != outputDeclareMap.end(); ++it) {
		if (inputDeclareMap.find(it->first) != inputDeclareMap.end())
			inputDeclareMap.erase(it->first);
		totalCost++;
	}
	for (map<Node*, string>::iterator it = newGateMap.begin(); it != newGateMap.end(); ++it) {
		totalCost++;
	}
	//cout << "Success cost:" << totalCost << "\n";
	patch.info.cost = totalCost;
	//cout << "---------costEND--------------\n";
	return patch;
}

void generatePatchPIPO(string type, string name, Graph& graph)
{
	//if (name.find("\\") != string::npos)
	//	name = name.substr(1, name.size());
	while (name[0] == ' ') //delete front blank
		name = name.substr(1, name.size() - 1);
	while (name[name.size() - 1] == ' ') //delete back blank
		name = name.substr(0, name.size() - 1);
	Node* node;
	if (type == "input") {
		node = initialNewnode(name, 9, graph.name);
		graph.PIMAP[node->name] = node;
		graph.netlist.push_back(node);
		graph.PI.push_back(node);
		node->piset.insert(node);
		node->piset_str.insert(node->name);
	}
	else {
		node = initialNewnode(name, 10, graph.name);
		graph.netlist.push_back(node);
		graph.PO.push_back(node);
	}
}

void configurePatch(Graph& graph)
{
	for (int i = 0; i < graph.PI.size(); i++) {
		if (graph.PI[i]->name.find("\\") != string::npos)
			graph.PI[i]->name = graph.PI[i]->name.substr(1, graph.PI[i]->name.size());
		/*if(graph.PI[i]->name.find("_in") != string::npos)
			graph.PI[i]->name = graph.PI[i]->name.substr(0, graph.PI[i]->name.find("_in"));*/
	}
	for (int i = 0; i < graph.PO.size(); i++) {
		if (graph.PO[i]->name.find("\\") != string::npos)
			graph.PO[i]->name = graph.PO[i]->name.substr(1, graph.PO[i]->name.size());
	}
}
void nodeMatch(Graph& patch, Graph& origin, PatchInfo& info)
{
	map<string, Node*> patchMap;
	map<string, Node*> originMap;
	for (int i = 0; i < patch.netlist.size(); i++) {
		patchMap[patch.netlist[i]->name] = patch.netlist[i];
	}
	for (int i = 0; i < origin.netlist.size(); i++) {
		originMap[origin.netlist[i]->name] = origin.netlist[i];
	}
	for (map<string, Node*>::iterator it = patchMap.begin(); it != patchMap.end(); ++it) {
		if (originMap.find(it->first) != originMap.end()) {
			info.matches[it->second] = originMap.find(it->first)->second;
			info.originMatch[originMap.find(it->first)->second] = true;
		}
		else if (it->first.find("_in") != string::npos) {
			string req = it->first.substr(0, it->first.find("_in"));
			if (originMap.find(req) != originMap.end()) {
				info.matches[it->second] = originMap.find(req)->second;
				info.originMatch[originMap.find(req)->second] = true;
			}
		}
	}
}
void removeExtraNode(Graph& patch, Graph& origin, PatchInfo& info)
{
	for (int i = 0; i < origin.netlist.size(); i++)
		info.netlist[origin.netlist[i]] = true;

	for (int i = 0; i < patch.PO.size(); i++) {
		if (info.matches.find(patch.PO[i]) != info.matches.end()) {
			Node* req = info.matches.find(patch.PO[i])->second;
			removeSingleFanout(req, info);
			req->fanin.clear();
		}
	}

}
void removeSingleFanout(Node* node, PatchInfo& info)
{
	for (int i = 0; i < node->fanin.size(); i++) {
		if (node->fanin[i]->type == 9 || node->fanin[i]->type == 10)
			continue;
		if (info.originMatch.find(node->fanin[i]) != info.originMatch.end())
			continue;
		if (node->fanin[i]->fanout.size() == 1) {
			removeSingleFanout(node->fanin[i], info);
			info.netlist.erase(node->fanin[i]);
		}
		else {
			int record = 0;
			for (int j = 0; j < node->fanin[i]->fanout.size(); j++) {
				if (node->fanin[i]->fanout[j] == node) {
					record = j;
					break;
				}
			}
			node->fanin[i]->fanout.erase(node->fanin[i]->fanout.begin() + record);
		}
	}
}

void applyNode(Graph& patch, PatchInfo& info)
{
	for (int i = 0; i < patch.netlist.size(); i++) {
		if (patch.netlist[i]->type == 9)
			continue;
		else if (patch.netlist[i]->type == 10) {
			Node* origin = info.matches.find(patch.netlist[i])->second;
			origin->fanin.resize(patch.netlist[i]->fanin.size());
			for (int j = 0; j < patch.netlist[i]->fanin.size(); j++) {
				Node* patchNode = patch.netlist[i]->fanin[j];
				if (info.matches.find(patchNode) != info.matches.end()) {
					origin->fanin[j] = info.matches.find(patchNode)->second;
					info.matches.find(patchNode)->second->fanout.push_back(origin);
				}
				else {
					Node* newnode = initialNewnode(patchNode->name, patchNode->type, "");
					info.netlist[newnode] = true;
					info.matches[patchNode] = newnode;
					info.originMatch[newnode] = true;
					origin->fanin[j] = newnode;
					newnode->fanout.push_back(origin);
				}
			}
			origin->realGate = patch.netlist[i]->realGate;
		}
		else {
			Node* patchNode = patch.netlist[i];
			Node* originNode;
			if (info.matches.find(patch.netlist[i]) != info.matches.end())
				originNode = info.matches.find(patch.netlist[i])->second;
			else {
				originNode = initialNewnode(patchNode->name, patchNode->type, "");
				info.netlist[originNode] = true;
				info.matches[patchNode] = originNode;
				info.originMatch[originNode] = true;
			}
			originNode->fanin.resize(patchNode->fanin.size());
			originNode->type = patchNode->type;
			originNode->realGate = patchNode->realGate;
			for (int j = 0; j < patchNode->fanin.size(); j++) {
				if (info.matches.find(patchNode->fanin[j]) != info.matches.end()) {
					originNode->fanin[j] = info.matches.find(patchNode->fanin[j])->second;
					info.matches.find(patchNode->fanin[j])->second->fanout.push_back(originNode);
				}
				else {
					Node* newnode = initialNewnode(patchNode->fanin[j]->name, patchNode->fanin[j]->type, "");
					info.netlist[newnode] = true;
					info.matches[patchNode->fanin[j]] = newnode;
					info.originMatch[newnode] = true;
					originNode->fanin[j] = newnode;
					newnode->fanout.push_back(originNode);
				}
			}
		}
	}
	int k = 0;
}
void generatePatchG1(Graph& origin, PatchInfo& info, Graph& patchG1)
{
	int cost = 0;
	vector<Node*> regists;
	patchG1.name = "patchG1";
	patchG1.Constants.resize(2);
	for (int i = 0; i < origin.PI.size(); i++)
		generatePatchPIPO("input", origin.PI[i]->name, patchG1);
	for (int i = 0; i < origin.PO.size(); i++)
		generatePatchPIPO("output", origin.PO[i]->name, patchG1);
	for (map<Node*, bool>::iterator it = info.netlist.begin(); it != info.netlist.end(); ++it) {
		if (it->first->name == "1'b0" || it->first->name == "1'b1" || it->first->type == 9)
			continue;
		vector<string> names(3);
		for (int i = 0; i < it->first->fanin.size(); i++)
			names[i] = it->first->fanin[i]->name;
		names[2] = it->first->name;
		string command = generateInstruction(it->first, names, 1, cost);
		if (command == "error") {
			//cout << "generatePatchG1 error!!\n";
			continue;
		}
		command = command.substr(0, command.size() - 1);
		verilog2graph(command, patchG1, regists);
	}
}
//void outFile(Graph graph, char* argv)
//{
//	ofstream outfile(argv);
//	vector<Node> pi_items;
//	vector<Node> po_items;
//	// ExtractBracket PI/PO
//	strExtractBracket(graph, pi_items, po_items);
//	outfile << "module top_eco(";
//	for (int i = 0; i < pi_items.size(); i++)
//		outfile << pi_items[i].name << ", ";
//	for (int i = 0; i < po_items.size(); i++) {
//		if (i != po_items.size() - 1)
//			outfile << po_items[i].name << ", ";
//		else
//			outfile << po_items[i].name << ");\n";
//	}
//	sort(pi_items.begin(), pi_items.end(), typeAndNumberCompare);
//	sort(po_items.begin(), po_items.end(), typeAndNumberCompare);
//	outputFront(outfile, pi_items, "  input ");
//	outputFront(outfile, po_items, "  output ");
//	outputFront(outfile, pi_items, "  wire ");
//	outputFront(outfile, po_items, "  wire ");
//
//	vector<string> wireOutput;
//	for (int i = 0; i < graph.netlist.size(); ++i)
//		if (graph.netlist[i]->type < 9 && graph.netlist[i]->name.find("'b") == string::npos)
//			wireOutput.push_back(graph.netlist[i]->name);
//	sort(wireOutput.begin(), wireOutput.end(), strTitleCompare);
//	for (int i = 0, counter = 1; i < wireOutput.size(); i++) {
//		if (counter == 1 && i != wireOutput.size() - 1)
//			outfile << "  wire " << wireOutput[i] << ", ";
//		else if (counter == 1)
//			outfile << "  wire " << wireOutput[i] << ";\n";
//		else if (counter < 8 && i != wireOutput.size() - 1)
//			outfile << wireOutput[i] << ", ";
//		else if (counter < 8)
//			outfile << wireOutput[i] << ";\n";
//		else {
//			outfile << wireOutput[i] << ";\n";
//			counter = 0;
//		}
//		counter++;
//	}
//	gateOutput(outfile, graph);
//	outfile << "endmodule\n";
//	outfile.close();
//}
//void strExtractBracket(Graph graph, vector<Node>& pi_items, vector<Node>& po_items)
//{
//	for (int i = 0; i < graph.PI.size(); i++) {
//		bool is_exist = false;
//		Node req;
//		req.type = graph.PI[i]->type;
//		req.seeds = new unsigned[1];
//		req.seeds[0] = 1;
//		if (graph.PI[i]->name.find("[") != string::npos)
//			req.name = graph.PI[i]->name.substr(0, graph.PI[i]->name.find('['));
//		else
//			req.name = graph.PI[i]->name;
//		for (int j = pi_items.size() - 1; j >= 0; j--)
//			if (pi_items[j].name == req.name) { pi_items[j].seeds[0]++; is_exist = true; break; }
//		if (!is_exist)
//			pi_items.push_back(req);
//	}
//	for (int i = 0; i < graph.PO.size(); i++) {
//		bool is_exist = false;
//		Node req;
//		req.type = graph.PO[i]->type;
//		req.seeds = new unsigned[1];
//		req.seeds[0] = 1;
//		if (graph.PO[i]->name.find("[") != string::npos)
//			req.name = graph.PO[i]->name.substr(0, graph.PO[i]->name.find('['));
//		else
//			req.name = graph.PO[i]->name;
//		for (int j = po_items.size() - 1; j >= 0; j--)
//			if (po_items[j].name == req.name) { po_items[j].seeds[0]++; is_exist = true; break; }
//		if (!is_exist)
//			po_items.push_back(req);
//	}
//}
//void outputFront(ofstream& outfile, vector<Node> p, string str)
//{
//	for (int i = 0; i < p.size(); i++) {
//		unsigned int* current_num = p[i].seeds;
//		vector<string> vec;
//		vec.push_back(p[i].name);
//		for (int j = i + 1; j < p.size(); j++) {
//			if (*current_num == *p[j].seeds)
//				vec.push_back(p[j].name);
//			else {
//				i = j - 1;
//				break;
//			}
//			if (j == p.size() - 1)
//				i = j;
//		}
//		outfile << str;
//		if (*current_num > 1) {
//			stringstream ss;
//			ss << (*current_num - 1);
//			string req = ss.str();
//			outfile << "[" << req << ":0] ";
//		}
//		for (int j = 0; j < vec.size(); j++) {
//			if (j != vec.size() - 1)
//				outfile << vec[j] << ", ";
//			else
//				outfile << vec[j] << ";\n";
//		}
//	}
//}
//void gateOutput(ofstream& outfile, Graph graph)
//{
//	for (int gatenum = 0; gatenum < graph.netlist.size(); ++gatenum) {
//		int type = graph.netlist[gatenum]->type;
//		int realGate = graph.netlist[gatenum]->realGate;
//		stringstream ss;
//		ss << gatenum;
//		if (type != -1 && type != 9) {
//			if (type == 0 || realGate == 0)
//				outfile << "  not eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
//			else if (type == 1 || realGate == 1)
//				outfile << "  and eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
//			else if (type == 2 || realGate == 2)
//				outfile << "  or eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
//			else if (type == 3 || realGate == 3)
//				outfile << "  nand eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
//			else if (type == 4 || realGate == 4)
//				outfile << "  nor eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
//			else if (type == 5 || realGate == 5)
//				outfile << "  xor eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
//			else if (type == 6 || realGate == 6)
//				outfile << "  xnor eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
//			else if (type == 7 || realGate == 7)
//				outfile << "  buf eco_g" << ss.str() << " (" << graph.netlist[gatenum]->name << ", ";
//			if (type == 8 || realGate == 8)
//				outfile << "  assign " << graph.netlist[gatenum]->name << " = " << graph.netlist[gatenum]->fanin[0]->name << ";\n";
//			else {
//				for (int i = 0; i < graph.netlist[gatenum]->fanin.size(); i++) {
//					if (i == graph.netlist[gatenum]->fanin.size() - 1)
//						outfile << graph.netlist[gatenum]->fanin[i]->name << ");\n";
//					else
//						outfile << graph.netlist[gatenum]->fanin[i]->name << ", ";
//				}
//			}
//		}
//	}
//}
//bool typeAndNumberCompare(const Node& p1, const Node& p2)
//{
//	if (*p1.seeds == *p2.seeds)
//		return p1.name < p2.name;
//	return *p1.seeds < *p2.seeds;
//}
//bool strTitleCompare(const string& p1, const string& p2)
//{
//	if (p1.size() == p2.size())
//		return p1 < p2;
//	else
//		return p1.size() < p2.size();
//}


void patchOptimize(MatchInfo& matchInfo)
{
	Graph currPatchGraph;
	map<Node*, bool> isVisitedPatch;
	map<Node*, bool>::iterator it = matchInfo.goldenRemoveNode.begin();

	currPatchGraph.Constants.resize(2);
	for (; it != matchInfo.goldenRemoveNode.end(); ++it) {
		//push all golden remove node to currPatchGraph and isVisitedPatch
		currPatchGraph.netlist.push_back(it->first);
		isVisitedPatch[it->first] = false;
		//find PI
		for (int i = 0; i < it->first->fanin.size(); ++i) {
			Node* faninNode = it->first->fanin[i];
			if (matchInfo.goldenRemoveNode.find(faninNode) == matchInfo.goldenRemoveNode.end()) {
				if (faninNode->name == "1'b0")
					currPatchGraph.Constants[0] = faninNode;
				else if (faninNode->name == "1'b1")
					currPatchGraph.Constants[1] = faninNode;
				else {
					currPatchGraph.PI.push_back(faninNode);
					currPatchGraph.PIMAP[faninNode->name] = faninNode;
					currPatchGraph.PIFanoutNode.insert(it->first);
				}
			}
		}
		//find PO
		if (it->first->fanout.size() == 0 || it->first->type == 10)
			currPatchGraph.PO.push_back(it->first);
		else {
			for (int i = 0; i < it->first->fanout.size(); ++i) {
				Node* fanoutNode = it->first->fanout[i];
				if (matchInfo.goldenRemoveNode.find(fanoutNode) == matchInfo.goldenRemoveNode.end()) {
					currPatchGraph.PO.push_back(it->first);
					break;
				}
			}
		}
	}
	//output blif
	outputPatchBlif(currPatchGraph, isVisitedPatch);
	//cout << "Success outputPatchBlif!" << endl;

	//optimize the patch
	optimizePatch();
	//cout << "Success optimize the patch!" << endl;

	//read optimized patch blif and transfer into goldenRemoveNode
	map<Node*, bool> newGoldenRemoveNode;
	readOptPatchBlif(currPatchGraph, newGoldenRemoveNode);
	//cout << "Success read the New patch!" << endl;

	//combine split gate into xor or xnor gate
	/**/
	removeRedundantNode(newGoldenRemoveNode);
	//cout << "Success removeRedundantNode!" << endl;


	//remove old R2 node
	removeOldNode(currPatchGraph, newGoldenRemoveNode, matchInfo.goldenRemoveNode);
	//cout << "Success removeOldNode!" << endl;

	//assign new goldenRemoveNode
	matchInfo.goldenRemoveNode = newGoldenRemoveNode;
}

void outputPatchBlif(Graph& currPatchGraph, map<Node*, bool>& isVisitedPatch)
{
	ofstream outfile("./blif/check.blif");
	//write -> ".model check"
	outfile << ".model check" << endl;

	//write -> ".inputs ..."
	outfile << ".inputs";
	map<string, Node*>::iterator it1 = currPatchGraph.PIMAP.begin();
	for (; it1 != currPatchGraph.PIMAP.end(); ++it1) {
		outfile << " " << it1->first;
		if (it1->second->type != 9)
			outfile << "_patch";
	}
	outfile << endl;

	//write -> ".outputs ..."
	outfile << ".outputs";
	for (int i = 0; i < currPatchGraph.PO.size(); ++i) {
		outfile << " " << currPatchGraph.PO[i]->name << "_patch";
	}
	outfile << endl;

	//output PI nodes
	set<Node*>::iterator it = currPatchGraph.PIFanoutNode.begin();
	for (; it != currPatchGraph.PIFanoutNode.end(); ++it) {
		Node* fanoutNode = *it;
		if (!isVisitedPatch[fanoutNode]) {
			outputDotNames(outfile, fanoutNode, "patch");
			isVisitedPatch[fanoutNode] = true;
		}
	}

	//output Constants
	vector<bool> existConst(2, false);
	for (int i = 0; i < currPatchGraph.Constants.size(); ++i) {
		if (currPatchGraph.Constants[i] != NULL)
			existConst[i] = true;
	}
	outputConst(outfile, existConst);

	//output netlist
	for (int i = 0; i < currPatchGraph.netlist.size(); ++i) {
		Node* currNode = currPatchGraph.netlist[i];
		if (!isVisitedPatch[currPatchGraph.netlist[i]])
			outputDotNames(outfile, currNode, "patch");
	}

	outfile << ".end";
}

void readOptPatchBlif(Graph& currPatchGraph, map<Node*, bool>& newGoldenRemoveNode)
{
	ifstream infile("opt_patch.blif");
	string line;
	map<string, Node*> checkExist;
	map<string, string> patchPI;
	map<string, string> patchPO;
	int notGatePos = 0;
	int state = 0;// 0:comment 1:inputs 2:outputs 3:names
	readOptPatchPI(infile, currPatchGraph, patchPI);
	readOptPatchPO(infile, currPatchGraph, patchPO);

	while (1) {
		size_t _namesPos;
		getline(infile, line);
		if (line == ".end")
			break;
		if ((_namesPos = line.find(".names")) != string::npos || state) {
			//remove .names in line
			line = line.substr(_namesPos + 7, line.size() - 1);
			//transfer blif into node
			blif2Graph(infile, line, currPatchGraph, newGoldenRemoveNode, checkExist, notGatePos, patchPI, patchPO);
			state = 1;
		}
	}
	infile.close();
}
void readOptPatchPI(ifstream& infile, Graph& currPatchGraph, map<string, string>& patchPI)
{
	bool finish = false;
	bool findInput = false;
	stringstream ss;
	map<string, Node*>::iterator it = currPatchGraph.PIMAP.begin();
	while (1) {
		string line;
		getline(infile, line);
		if (line.find(".inputs") == string::npos && findInput == false)
			continue;
		else {
			if (findInput)
				line.erase(0, 1);
			findInput = true;
		}

		if (line[line.size() - 1] != '\\')
			finish = true;
		ss << line;
		while (1) {
			string eachNode;
			getline(ss, eachNode, ' ');
			//continue when meet .inputs
			if (eachNode == ".inputs")
				continue;
			//break when meet \\ and back blank
			if (eachNode == "\\" || eachNode.size() == 0)
				break;
			patchPI[eachNode] = it->first;
			it++;
		}
		ss.str("");
		ss.clear();
		if (finish)
			break;
	}
}

void readOptPatchPO(ifstream& infile, Graph& currPatchGraph, map<string, string>& patchPO)
{
	bool finish = false;
	bool findOutput = false;
	stringstream ss;
	int i = 0;
	while (1) {
		string line;
		getline(infile, line);
		if (line.find(".outputs") == string::npos && findOutput == false)
			continue;
		else {
			if (findOutput)
				line.erase(0, 1);
			findOutput = true;
		}

		if (line[line.size() - 1] != '\\')
			finish = true;
		ss << line;
		while (1) {
			string eachNode;
			getline(ss, eachNode, ' ');
			//continue when meet .inputs
			if (eachNode == ".outputs")
				continue;
			//break when meet \\ and back blank
			if (eachNode == "\\" || eachNode.size() == 0)
				break;
			patchPO[eachNode] = currPatchGraph.PO[i]->name;
			i++;
		}
		ss.str("");
		ss.clear();
		if (finish)
			break;
	}
}

void blif2Graph(ifstream& infile, string& line, Graph& currPatchGraph, map<Node*, bool>& newGoldenRemoveNode,
	map<string, Node*>& checkExist, int& notGatePos, map<string, string>& patchPI, map<string, string>& patchPO)
{
	stringstream ss;
	vector<Node*> NodeList;
	vector<string> NodeListString;
	bool existPO; //check if this Node is PO
	//record whether this node is fanin or fanout node
	ss << line;
	//split blif file command and save into vector
	while (1) {
		string new_line;
		size_t _patchPos = 0;
		getline(ss, new_line, ' ');
		if (new_line.size() == 0)
			break;
		if ((_patchPos = new_line.find("_patch")) != string::npos)
			new_line = new_line.substr(0, _patchPos);

		NodeListString.push_back(new_line);
	}

	for (int gatePos = 0; gatePos < NodeListString.size(); ++gatePos) {
		string new_line = NodeListString[gatePos];
		//replace with original PI and PO names
		if (patchPI.find(new_line) != patchPI.end())
			new_line = patchPI[new_line];
		else if (patchPO.find(new_line) != patchPO.end())
			new_line = patchPO[new_line];
		//node name
		if (currPatchGraph.PIMAP.find(new_line) != currPatchGraph.PIMAP.end()) {
			NodeList.push_back(currPatchGraph.PIMAP[new_line]);
			continue;
		}
		else if (checkExist.find(new_line) != checkExist.end()) {
			if (checkExist[new_line]->type == -1 && gatePos == NodeListString.size() - 1)
				checkExist[new_line]->type = selectBlifGateType(infile);
			NodeList.push_back(checkExist[new_line]);
			continue;
		}
		else {
			existPO = false;
			for (int i = 0; i < currPatchGraph.PO.size(); ++i) {
				if (currPatchGraph.PO[i]->name == new_line && gatePos == NodeListString.size() - 1) {
					existPO = true;
					//get the new type

					if (currPatchGraph.PO[i]->type == 10)
						currPatchGraph.PO[i]->realGate = selectBlifGateType(infile);
					else
						currPatchGraph.PO[i]->type = selectBlifGateType(infile);
					//push into new GoldenRemoveNode
					newGoldenRemoveNode[currPatchGraph.PO[i]] = false;
					checkExist[new_line] = currPatchGraph.PO[i];
					currPatchGraph.PO[i]->fanin.resize(0);
					NodeList.push_back(currPatchGraph.PO[i]);
					break;
				}
			}
			//this node is PO
			if (existPO)
				continue;
		}

		//select blif file gate type by 11 1 or 00 0 ... 
		int GataType;
		//check if this node is fanout node 
		//if this node is fanin node give this node gate type -1
		if (gatePos == NodeListString.size() - 1)
			GataType = selectBlifGateType(infile);
		else
			GataType = -1;
		//create new node and push into NodeList
		Node* newNode = initialNewnode(new_line, GataType, "patch");
		//give this node specific id number
		newNode->id = nodeID++;
		NodeList.push_back(newNode);
		//push into new GoldenRemoveNode
		newGoldenRemoveNode[newNode] = false;
		//check if this node exist
		checkExist[new_line] = newNode;
	}
	//start push fanin node and fanout node
	Node* fanoutNode = NodeList[NodeList.size() - 1];
	//not gate name for 10 1 or 01 1 situation
	string notGateName = "_not_GATE_";
	for (int i = 0; i < NodeList.size() - 1; ++i) {
		//currGate fanin Node
		Node* faninNode = NodeList[i];


		//fix the problem of blif file case(ex:10 1,01 1,10 0,01 0)
		if ((fanoutNode->type == 210 || fanoutNode->realGate == 210 || fanoutNode->type == 101 || fanoutNode->realGate == 101) && i == 0) //10 1
			faninNode = connectNewNotGate(faninNode, notGateName, notGatePos, newGoldenRemoveNode);
		else if ((fanoutNode->type == 201 || fanoutNode->realGate == 201 || fanoutNode->type == 110 || fanoutNode->realGate == 110) && i == 1)//01 1
			faninNode = connectNewNotGate(faninNode, notGateName, notGatePos, newGoldenRemoveNode);

		//push into fanin and fanout
		faninNode->fanout.push_back(fanoutNode);
		fanoutNode->fanin.push_back(faninNode);
	}
	//transfer special gate type into primitive gate type
	if (fanoutNode->type == 101 || fanoutNode->type == 110 ||
		fanoutNode->realGate == 101 || fanoutNode->realGate == 110) {//and gate

		if (fanoutNode->type == 10)
			fanoutNode->realGate = 1;
		else
			fanoutNode->type = 1;
	}
	else if (fanoutNode->type == 201 || fanoutNode->type == 210 ||
		fanoutNode->realGate == 201 || fanoutNode->realGate == 210) {//or gate
		if (fanoutNode->type == 10)
			fanoutNode->realGate = 2;
		else
			fanoutNode->type = 2;
	}
	else if (fanoutNode->type == 80 || fanoutNode->type == 81 ||
		fanoutNode->realGate == 80 || fanoutNode->realGate == 81) { //assign constant 0 or 1
		Node* newConstNode;
		if (fanoutNode->type == 80 || fanoutNode->realGate == 80)
			newConstNode = initialNewnode("1'b0", -1, "patch");
		else
			newConstNode = initialNewnode("1'b1", -1, "patch");

		if (fanoutNode->type == 10)
			fanoutNode->realGate = 8;
		else
			fanoutNode->type = 8;

		newConstNode->fanout.push_back(fanoutNode);
		fanoutNode->fanin.push_back(newConstNode);
	}

}
Node* connectNewNotGate(Node* faninNode, string& notGateName, int& notGatePos, map<Node*, bool>& newGoldenRemoveNode)
{
	//new a not gate
	Node* notGate = initialNewnode(notGateName + toString(notGatePos++), 0, "patch");
	//not gate fanin is currNode
	notGate->fanin.push_back(faninNode);
	//currNode fanin is not gate
	faninNode->fanout.push_back(notGate);
	//push not gate into newGoldenRemoveNode
	newGoldenRemoveNode[notGate] = false;

	return notGate;
}

void removeRedundantNode(map<Node*, bool>& newGoldenRemoveNode)
{
	map<Node*, bool>::iterator it = newGoldenRemoveNode.begin();
	int gatePos = 0;
	bool addNewGate = false;
	for (; it != newGoldenRemoveNode.end();) {
		addNewGate = false;
		Node* currNode = it->first;

		//if this node is OR or NOR gate
		if (currNode->type == 2 || currNode->realGate == 2 ||
			currNode->type == 4 || currNode->realGate == 4) {
			if (faninNodeisLegal(currNode, newGoldenRemoveNode, gatePos)) {
				it = newGoldenRemoveNode.begin();
				addNewGate = true;
			}
		}
		if (addNewGate == false)
			++it;
	}
}
bool faninNodeisLegal(Node* currNode, map<Node*, bool>& newGoldenRemoveNode, int& gatePos)
{
	//return 0:not the xor gate 5:xor 6:xnor

	//check fanin are same by its name
	set<Node*> redundantFaninNode;
	//record redundant name
	set<Node*> redundantNode;

	for (int i = 0; i < currNode->fanin.size(); ++i) {
		Node* faninNode = currNode->fanin[i];
		int notGateNumber = 0;
		//check node type is and gate
		if (faninNode->type != 1 || faninNode->fanout.size() != 1)
			return 0;

		//insert into redundant node
		redundantNode.insert(faninNode);

		//check not gate number 
		for (int j = 0; j < faninNode->fanin.size(); ++j) {
			//if this gate is NOT gate
			if (faninNode->fanin[j]->type == 0) {
				//fanout must be 1
				if (faninNode->fanin[j]->fanout.size() != 1)
					return 0;

				redundantFaninNode.insert(faninNode->fanin[j]->fanin[0]);
				redundantNode.insert(faninNode->fanin[j]);
				//number of not gate
				notGateNumber++;
				continue;
			}
			redundantFaninNode.insert(faninNode->fanin[j]);
			//redundantNode.insert(faninNode->fanin[j]);
		}
		if (notGateNumber != 1)
			return 0;
	}
	if (redundantFaninNode.size() != 2)
		return 0;

	if (currNode->type == 2 || currNode->realGate == 2)
		startRebuildNode(currNode, redundantFaninNode, redundantNode, newGoldenRemoveNode, 5, gatePos);
	else if (currNode->type == 4 || currNode->realGate == 4)
		startRebuildNode(currNode, redundantFaninNode, redundantNode, newGoldenRemoveNode, 6, gatePos);

	return true;
}
void startRebuildNode(Node* fanoutNode, set<Node*>& redundantFaninNode, set<Node*>& redundantNode,
	map<Node*, bool>& newGoldenRemoveNode, int type, int& gatePos)
{
	string gateName = "_xor_GATE_";
	if (type == 6)
		gateName = "_xnor_GATE_";

	Node* newNode = initialNewnode(gateName + toString(gatePos++), type, "patch");
	removeRedundantFanin(fanoutNode, newNode);
	set<Node*>::iterator it = redundantFaninNode.begin();
	for (; it != redundantFaninNode.end(); ++it) {
		newNode->fanin.push_back(*it);
		(*it)->fanout.push_back(newNode);
		removeRedundantFanout(*it, redundantNode);
	}
	//erase redundant node in newGoldenRemoveNode 
	it = redundantNode.begin();
	for (; it != redundantNode.end(); ++it)
		if (newGoldenRemoveNode.find(*it) != newGoldenRemoveNode.end())
			newGoldenRemoveNode.erase(*it);
	//erase fanout node(OR gate or AND gate)
	newGoldenRemoveNode.erase(fanoutNode);
	//add new XOR or XNOR gate into newGoldenRemoveNode
	newGoldenRemoveNode[newNode] = false;
}
void removeRedundantFanin(Node* redundantNode, Node* newNode)
{
	for (int i = 0; i < redundantNode->fanout.size(); ++i) {
		Node* fanoutNode = redundantNode->fanout[i];
		//newNode fanout push into fanoutNode
		newNode->fanout.push_back(fanoutNode);
		//fanoutNode fanin push into newNode
		fanoutNode->fanin.push_back(newNode);
		//remove redundant faninNode
		for (int j = 0; j < fanoutNode->fanin.size(); ++j) {
			if (fanoutNode->fanin[j] == redundantNode) {
				fanoutNode->fanin.erase(fanoutNode->fanin.begin() + j);
				break;
			}
		}
	}
}
void removeRedundantFanout(Node* faninNode, set<Node*>& redundantNode)
{
	for (int i = 0; i < faninNode->fanout.size(); ++i) {
		Node* fanoutNode = faninNode->fanout[i];
		//remove redundant faninNode
		if (redundantNode.find(fanoutNode) != redundantNode.end()) {
			faninNode->fanout.erase(faninNode->fanout.begin() + i);
			i--;
		}
	}
}
void removeOldNode(Graph currPatchGraph, map<Node*, bool>& newGoldenRemoveNode, map<Node*, bool>& oldGoldenRemoveNode)
{
	/*remove PI fanout Node*/
	for (int i = 0; i < currPatchGraph.PI.size(); ++i) {
		Node* PInode = currPatchGraph.PI[i];
		for (int j = 0; j < PInode->fanout.size(); ++j) {
			Node* fanoutNode = PInode->fanout[j];
			if ((newGoldenRemoveNode.find(fanoutNode) == newGoldenRemoveNode.end()) &&
				oldGoldenRemoveNode.find(fanoutNode) != oldGoldenRemoveNode.end()) {
				PInode->fanout.erase(PInode->fanout.begin() + j);
				j--;
			}
		}
	}

	/*remove PO fanin Node*/
	for (int i = 0; i < currPatchGraph.PO.size(); ++i) {
		Node* POnode = currPatchGraph.PO[i];
		for (int j = 0; j < POnode->fanin.size(); ++j) {
			Node* faninNode = POnode->fanin[j];
			if ((newGoldenRemoveNode.find(faninNode) == newGoldenRemoveNode.end()) &&
				oldGoldenRemoveNode.find(faninNode) != oldGoldenRemoveNode.end()) {
				POnode->fanin.erase(POnode->fanin.begin() + j);
				j--;
			}
		}
	}
}
void optimizePatch()
{
	system("./optimize2.out ./blif/check.blif > optScreen.txt");
}

int selectBlifGateType(ifstream& infile)
{
	string gateTypeLine;
	//get the next line to know the gate type
	getline(infile, gateTypeLine);
	if (gateTypeLine == "0 1") return 0;// not
	else if (gateTypeLine == "11 1") return 1; // and 
	else if (gateTypeLine == "00 0") return 2;// or
	else if (gateTypeLine == "11 0") return 3;// nand
	else if (gateTypeLine == "00 1") return 4;// nor
	else if (gateTypeLine == "1 1") return 8;// assign and buffer
	else if (gateTypeLine == " 0") return 80;// assgin constant 0
	else if (gateTypeLine == " 1") return 81;// assgin constant 1
	else if (gateTypeLine == "10 1") return 110; // and (ab')
	else if (gateTypeLine == "01 1") return 101; // and (a'b)
	else if (gateTypeLine == "10 0") return 210; // and (a'+b)
	else if (gateTypeLine == "01 0") return 201; // and (a+b')

	//error gate type
	//cout << "Patch Ocurr Error Gate Type!" << endl;
	return -1;

}
bool patchSelfVerify()
{
	system("./verify");

	ifstream infile("verify_out.txt");
	string result;
	infile >> result;
	infile.close();
	if (result == "UNSAT") {
		ofstream outfile("verify_out.txt");
		outfile << "SAT" << endl;
		outfile.close();
		return true;
	}

	return false;
}

