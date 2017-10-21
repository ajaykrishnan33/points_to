#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std;

class Constraint {
	public:
		string lhs;
		string rhs;
		int type;
		/* TYPES:
		 * 0 -- > p = q  // copy constraint
		 * 1 -- > *p = q // store constraint
		 * 2 -- > p = *q // load constraint
		 * 3 -- > p = &q // address-of constraint
		*/

		Constraint(string cons){
			int lhs_prefix;
			int rhs_prefix;

			char left[10];
			char right[10];

			sscanf(cons.c_str(), "%[^,],%[^,],%d,%d", left, right, &lhs_prefix, &rhs_prefix);
			
			this->lhs = left;
			this->rhs = right;

			if(lhs_prefix==0 && rhs_prefix==0)
				this->type = 0;
			else
			if(lhs_prefix==1 && rhs_prefix==0)
				this->type = 1;
			else
			if(lhs_prefix==0 && rhs_prefix==1)
				this->type = 2;
			else
			if(lhs_prefix==0 && rhs_prefix==2)
				this->type = 3;
		}
};

class Node {
	public:
		string var;
		unordered_set<string> pointsTo;
		// unordered_set<string> outNeighbours;

		Node(string var){
			this->var = var;
		}

};

class Edge {
	public:
		string src;
		string dst;

		Edge(string dst, string src){
			this->src = src;
			this->dst = dst;
		}

		// bool operator ==(const Edge& e) const{
		// 	return e.src == this->src && e.dst == this->dst;
		// }
};

bool operator==(const Edge& x, const Edge& y)
{
    return x.src == y.src && x.dst == y.dst;
}

struct hashEdge
{	
    size_t operator()(const Edge& e) const {
    	hash<string> fn;
        return fn(e.src+"-"+e.dst);
    }
};

class PointsToAnalysis {
	private:
		vector<Constraint> constraint_list;
		vector<float> float_list;
		unordered_set<string> var_list;
		unordered_map<string, Node> graph;
		unordered_set<Edge,hashEdge> edge_list;

		void getVariableList(){
			vector<Constraint> :: iterator i;
			for(i=this->constraint_list.begin();i!=this->constraint_list.end();i++){
				if(var_list.find(i->lhs)==this->var_list.end()){// not already present in list
					Node n(i->lhs);
					this->var_list.emplace(i->lhs);
					this->graph.emplace(i->lhs, n);
				}
				if(var_list.find(i->rhs)==this->var_list.end()){// not already present in list
					Node n(i->rhs);
					this->var_list.emplace(i->rhs);
					this->graph.emplace(i->rhs, n);
				}
			}
		}

		void processAddressOfConstraints(){
			vector<Constraint> :: iterator i;
			for(i=this->constraint_list.begin();i!=this->constraint_list.end();i++){
				if(i->type == 3){ // address-of constraint
					Node& n = this->graph.find(i->lhs)->second;
					n.pointsTo.emplace(i->rhs);
				}
			}
		}

		// void addEdgeIfNotExists(string dst, string src){
		// 	Node& n = this->graph.find(src)->second;
		// 	if(n.outNeighbours.find(dst)==n.outNeighbours.end()){
		// 		n.outNeighbours.emplace(dst);
		// 		Edge e(dst, src);
		// 		this->edge_list.emplace(e);
		// 	}
		// }

		void processCopyConstraints(){
			vector<Constraint> :: iterator i;
			for(i=this->constraint_list.begin();i!=this->constraint_list.end();i++){
				if(i->type==0) { // copy constraint
					Edge e(i->lhs, i->rhs);
					this->edge_list.emplace(e);
				}
			}
		}

		bool fixedPoint(unordered_map<string,Node>* oldGraph, unordered_set<Edge,hashEdge>* oldEdgeList){
			
			unordered_set<string> :: iterator i;
			for(i=this->var_list.begin();i!=this->var_list.end();i++){
				Node& curr = this->graph.find(*i)->second;
				Node& old = oldGraph->find(*i)->second;
				if(curr.pointsTo!=old.pointsTo){
					return false;
				}

			}

			if(this->edge_list.size() > oldEdgeList->size()){
				return false;
			}

			return true;
		}

		unordered_map<string,Node>* deepCopyGraph(){
			unordered_set<string>::iterator i;
			unordered_map<string,Node> *newGraph = new unordered_map<string,Node>();
			for(i=this->var_list.begin();i!=this->var_list.end();i++){
				Node n1(*i);
				Node& n2 = this->graph.find(*i)->second;
				n1.pointsTo = unordered_set<string>(n2.pointsTo);
				newGraph->emplace(*i, n1);
			}
			return newGraph;
		}

		unordered_set<Edge,hashEdge>* deepCopyEdgeList(){
			unordered_set<Edge,hashEdge>::iterator i;
			unordered_set<Edge,hashEdge> *newEdgeList = new unordered_set<Edge,hashEdge>();
			for(i=this->edge_list.begin();i!=this->edge_list.end();i++){
				Edge e(i->dst, i->src);
				newEdgeList->emplace(e);
			}
			return newEdgeList;
		}

		void freeGraph(unordered_map<string,Node>* currGraph){
			free(currGraph);
		}

		void freeEdgeList(unordered_set<Edge,hashEdge>* currEdgeList){
			free(currEdgeList);
		}

		void propagatePointsToInfo(){
			unordered_set<Edge,hashEdge> :: iterator i;
			for(i=this->edge_list.begin();i!=this->edge_list.end();i++){
				Node& src = this->graph.find(i->src)->second;
				Node& dst = this->graph.find(i->dst)->second;
				dst.pointsTo.insert(src.pointsTo.begin(), src.pointsTo.end());
			}
		}

		void addLS_Constraints(){
			vector<Constraint> :: iterator i;
			unordered_set<string> :: iterator s;

			for(i=this->constraint_list.begin();i!=this->constraint_list.end();i++){
				if(i->type==1){ //store : *p = q
					Node& dst = this->graph.find(i->lhs)->second;
					for(s=dst.pointsTo.begin();s!=dst.pointsTo.end();s++){
						Edge e(*s, i->rhs);
						this->edge_list.emplace(e);
					}
				}
				if(i->type==2){ //load  : p = *q
					Node& src = this->graph.find(i->rhs)->second;
					for(s=src.pointsTo.begin();s!=src.pointsTo.end();s++){
						Edge e(i->lhs, *s);
						this->edge_list.emplace(e);
					}
				}
			}
		}

		void iterativeProcess(){

			unordered_map<string,Node>* oldGraph;
			unordered_set<Edge,hashEdge>* oldEdgeList;
			bool fp;
			int ct = 0;
			do {
				oldGraph = this->deepCopyGraph();
				oldEdgeList = this->deepCopyEdgeList();

				this->propagatePointsToInfo();

				this->addLS_Constraints();

				fp = fixedPoint(oldGraph, oldEdgeList);

				cout << "Fixed point computed" << endl << flush;

				this->freeEdgeList(oldEdgeList);
				this->freeGraph(oldGraph);
			} 
			while(!fp);
		}

	public:
		PointsToAnalysis(const char* filename){
			FILE *file;
			file = fopen(filename, "r");

			char input[10];
			float x;
			cout << "Reading the data from the file " << filename << " :" << endl;

			while(!feof(file)) {
				fscanf(file, "%s\n", input);
				Constraint *cts = new Constraint(input);
				this->constraint_list.push_back(*cts);
			}
		}

		void run(){
			cout << "Starting to run..." << endl << flush;
			this->getVariableList();
			cout << "Extracted variable list..." << endl << flush;
			this->processAddressOfConstraints();
			cout << "Processed address-of constraints..." << endl << flush;
			this->processCopyConstraints();
			cout << "Processed copy constraints..." << endl << flush;
			this->iterativeProcess();
		}

};

int main(int argc, char* argv[]){

	string filename;

	cout << "Input filename:";

	cin >> filename;

	PointsToAnalysis x(filename.c_str());

	x.run();

	cout << "Done" << endl;

}