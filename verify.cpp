#include<iostream>
#include<fstream>
#include<sstream>
#include<time.h>
#include<string>
#include<cstring>	
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
	map<Node*, Node*> matches;
	map<Node*, bool> originMatch;
	map<Node*, bool> netlist;
};


void loadFile(Graph& graph, string argv);

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


// configure patch name
void configurePatch(Graph& graph);
// match name
void nodeMatch(Graph& patch, Graph& origin, MatchInfo& info);
// remove extra node
void removeExtraNode(Graph& patch, Graph& origin, MatchInfo& info);
// remove all fanin single fanout node
void removeSingleFanout(Node* node, MatchInfo& info);
// apply node
void applyNode(Graph& patch, MatchInfo& info);
// outfile
void outputApplyG1(Graph& origin, MatchInfo& info);
//
string generateInstruction(Node* node, int eco);
//
string getTypeString(int type);


//start verify with minisat
bool compareNetlist(Graph& R2, Graph& patchedG1);
//node to blif
void node2Blif(ofstream& outfile, Node* currNode, int type);
//
void outputDotNames(ofstream& outfile, Node* currNode, string currGraphName);
//
void buildMiter(ofstream& outfile, Node* PO_original, Node* PO_golden, int miterPos);
//
void outputConst(ofstream& outfile, vector<bool>& faninConst);
void abcBlif2CNF();
bool SATsolver();
bool readSATsolverResult();

//tool function
string toString(int trans) {
	stringstream ss;
	ss << trans;
	return ss.str();
};

int nWords = 1;
int main(int argc, char* argv[])
{
	srand(time(NULL));
	//Patch netlist
	Graph Patch;
	Patch.name = "Patch";
	//golden netlist
	Graph R2;
	R2.name = "R2";
	//optimize netlist
	Graph G1;
	G1.name = "G1";

	loadFile(Patch, "patch.v");
	loadFile(R2, "r2.v");
	loadFile(G1, "g1.v");

	MatchInfo info;
	configurePatch(Patch);
	nodeMatch(Patch, G1, info);
	removeExtraNode(Patch, G1, info);
	applyNode(Patch, info);
	outputApplyG1(G1, info);
	int i = 0;

	//start verify
	Graph patchedG1;
	loadFile(patchedG1, "patchG1.v");
	compareNetlist(R2, patchedG1);

}


void loadFile(Graph& graph, string argv)
{
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

void nodeMatch(Graph& patch, Graph& origin, MatchInfo& info)
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

void removeExtraNode(Graph& patch, Graph& origin, MatchInfo& info)
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

void removeSingleFanout(Node* node, MatchInfo& info)
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

void applyNode(Graph& patch, MatchInfo& info)
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

void outputApplyG1(Graph& origin, MatchInfo& info)
{
	ofstream outfile("patchG1.v");
	int eco = 1;
	outfile << "module top(";
	for (int i = 0; i < origin.PI.size(); i++)
		outfile << origin.PI[i]->name << ", ";
	for (int i = 0; i < origin.PO.size(); i++) {
		if (i != origin.PO.size() - 1)
			outfile << origin.PO[i]->name << ", ";
		else
			outfile << origin.PO[i]->name << ");\n";
	}
	for (int i = 0, record = 0; i < origin.PI.size(); i++, record++) {
		if (record == 0)
			outfile << "  input ";
		if (i != origin.PI.size() - 1 && record != 5)
			outfile << origin.PI[i]->name << ", ";
		else
			outfile << origin.PI[i]->name << ";\n";
		if (record == 5)
			record = -1;
	}
	for (int i = 0, record = 0; i < origin.PO.size(); i++, record++) {
		if (record == 0)
			outfile << "  output ";
		if (i != origin.PO.size() - 1 && record != 5)
			outfile << origin.PO[i]->name << ", ";
		else
			outfile << origin.PO[i]->name << ";\n";
		if (record == 5)
			record = -1;
	}

	for (map<Node*, bool>::iterator it=info.netlist.begin(); it != info.netlist.end(); ++it) {
		if (it->first->name == "1'b0" || it->first->name == "1'b1")
			continue;
		if (it->first->type != 9) {
			outfile << generateInstruction(it->first, eco++) << "\n";
		}
	}
	outfile << "endmodule";
}

string generateInstruction(Node* node, int eco)
{
	if (node->type == -1)
		system("pause");
	string gate = getTypeString(node->type);
	string res = "  ";
	stringstream ss;
	ss << eco;
	if (gate == "PO") {
		if (node->realGate == -1)
			system("pause");
		gate = getTypeString(node->realGate);
	}

	

	if (gate == "not" || gate == "buf") {
		res += gate + " eco" + ss.str() + " (" + node->name + ", " + node->fanin[0]->name + ");";
	}
	else if (gate == "assign")
		res += gate + " " + node->name + " = " + node->fanin[0]->name + ";";
	else
		res += gate + " eco" + ss.str() + " (" + node->name + ", " + node->fanin[0]->name + ", " + node->fanin[1]->name + ");";
	return res;
}

string getTypeString(int type)
{
	vector<string> gates = { "not","and","or","nand","nor","xor","xnor","buf","assign","PI","PO" };
	return gates[type];
}

bool compareNetlist(Graph& R2, Graph& patchedG1)
{
	ofstream outfile("./blif/check.blif");

	//write -> ".model check"
	outfile << ".model check" << endl;

	//write -> ".inputs ..."
	outfile << ".inputs";
	for (int i = 0; i < R2.PI.size(); ++i) {
		outfile << " " << R2.PI[i]->name;
	}
	outfile << endl;

	//write -> ".outputs ..."
	outfile << ".outputs " << "output";
	outfile << endl;

	map<Node*, bool> isVisitedG1;
	map<Node*, bool> isVisitedR2;
	for (int i = 0; i < patchedG1.netlist.size(); ++i)
		isVisitedG1[patchedG1.netlist[i]] = false;

	for (int i = 0; i < R2.netlist.size(); ++i)
		isVisitedR2[R2.netlist[i]] = false;

	//Original Netlist PI
	set<Node*>::iterator it = patchedG1.PIFanoutNode.begin();
	for (; it != patchedG1.PIFanoutNode.end(); ++it) {
		Node* fanoutNode = *it;
		outputDotNames(outfile, fanoutNode, "G1");
		isVisitedG1[fanoutNode] = true;
	}

	//Golden Netlist PI
	it = R2.PIFanoutNode.begin();
	for (; it != R2.PIFanoutNode.end(); ++it) {
		Node* fanoutNode = *it;
		outputDotNames(outfile, fanoutNode, "R2");
		isVisitedR2[fanoutNode] = true;
	}

	//output Constant
	vector<bool> existConst(2, false);
	for (int i = 0; i < R2.Constants.size(); ++i) {
		if (R2.Constants[i] != NULL || patchedG1.Constants[i] != NULL)
			existConst[i] = true;
	}
	outputConst(outfile, existConst);

	//output Golden Remove Node connect to Original Netlist 
	for (int i = 0; i < patchedG1.netlist.size(); ++i) {
		if (!isVisitedG1[patchedG1.netlist[i]] && patchedG1.netlist[i]->type != 9)
			outputDotNames(outfile, patchedG1.netlist[i], "G1");
	}

	//output Golden Netlist internal node
	for (int i = 0; i < R2.netlist.size(); ++i) {
		if (!isVisitedR2[R2.netlist[i]] && R2.netlist[i]->type != 9)
			outputDotNames(outfile, R2.netlist[i], "R2");
	}

	//build the miter
	for (int i = 0; i < R2.PO.size(); ++i) {
		buildMiter(outfile, patchedG1.PO[i], R2.PO[i], i);
	}

	outfile << ".names";
	for (int i = 0; i < R2.PO.size(); ++i) {
		outfile << " miter_" << toString(i);
	}

	if (R2.PO.size() > 1) {
		outfile << " outputTmp" << endl;
		for (int i = 0; i < R2.PO.size(); ++i) {
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

	//call minisat and return result
	return SATsolver();
}

void outputDotNames(ofstream& outfile, Node* currNode, string currGraphName)
{
	int type;
	if (currNode->type == 10 || currNode->type == 9)
		type = currNode->realGate;
	else
		type = currNode->type;

	outfile << ".names";
	for (int i = 0; i < currNode->fanin.size(); ++i) {
		outfile << " " << currNode->fanin[i]->name;
		if (currNode->fanin[i]->type != 9)
			outfile << "_" + currGraphName;
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

bool SATsolver()
{
	abcBlif2CNF();
	system("./minisat ./cnf/check.cnf out.txt > minisatScreen.txt ");
	if (readSATsolverResult())
		return true;
	return false;
}

void abcBlif2CNF()
{
	system("./blif2cnf.out ./blif/check.blif");
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

void buildMiter(ofstream& outfile, Node* PO_original, Node* PO_golden, int miterPos)
{
	outfile << ".names";
	outfile << " " << PO_original->name + "_" + "G1";
	outfile << " " << PO_golden->name + "_" + "R2";

	outfile << " " << "miter_" << toString(miterPos) << endl;
	//outfile xor gate
	outfile << "01 1" << endl
		<< "10 1" << endl;
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