#include <stdio.h>
#include <iostream>
#include <fstream>
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
			int l1, l2;
			int r1, r2;

			char left[10];
			char right[10];

			// int lhs_prefix, rhs_prefix;

			// sscanf(cons.c_str(), "%[^,],%[^,],%d,%d", left, right, &lhs_prefix, &rhs_prefix);
			
			// this->lhs = left;
			// this->rhs = right;

			// if(lhs_prefix==0 && rhs_prefix==0)
			// 	this->type = 0;
			// else
			// if(lhs_prefix==1 && rhs_prefix==0)
			// 	this->type = 1;
			// else
			// if(lhs_prefix==0 && rhs_prefix==1)
			// 	this->type = 2;
			// else
			// if(lhs_prefix==0 && rhs_prefix==2)
			// 	this->type = 3;

			int left_num, right_num;
			sscanf(cons.c_str(), "%d 0 %d %d %d 0 %d %d", &left_num, &l1, &l2, &right_num, &r1, &r2);

			sprintf(left, "%d", left_num);
			sprintf(right, "%d", right_num);

			this->lhs = left;
			this->rhs = right;

			if(l1==0 && l2==0 && r1==0 && r2==1)
				this->type = 3;
			else
			if(l1==0 && l2==0 && r1==0 && r2==0)
				this->type = 0;
			else
			if(l1==1 && l2==0 && r1==0 && r2==0)
				this->type = 1;
			else
			if(l1==0 && l2==0 && r1==1 && r2==0)
				this->type = 2; 
			else{
				cout << "Constraint type error.\nExiting..."<<endl;
				exit(0);
			}

		}
};

class Node;

class Edge {
	public:
		string src;
		string dst;
		Edge(string dst, string src, unordered_map<string, Node> *graph);
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

class Node {
	public:
		string var;
		unordered_set<string> pointsTo;
		// unordered_set<string> outNeighbours;
		
		unordered_set<Edge,hashEdge> incoming;
		unordered_set<Edge,hashEdge> outgoing;

		Node(string var){
			this->var = var;
		}
};

Edge::Edge(string dst, string src, unordered_map<string, Node> *graph){
	this->src = src;
	this->dst = dst;
	Node &u = graph->find(src)->second;
	Node &v = graph->find(dst)->second;
	u.outgoing.emplace(*this);
	v.incoming.emplace(*this);
}

class PointsToAnalysis {
	private:

		const bool DATA_CAPTURE = true;

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

		void processCopyConstraints(){
			vector<Constraint> :: iterator i;
			for(i=this->constraint_list.begin();i!=this->constraint_list.end();i++){
				if(i->type==0) { // copy constraint
					if(i->lhs!=i->rhs){
						Edge e(i->lhs, i->rhs, &(this->graph));
						this->edge_list.emplace(e);
					}
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
				Edge e(i->dst, i->src, &(this->graph));
				newEdgeList->emplace(e);
			}
			return newEdgeList;
		}

		void freeGraph(unordered_map<string,Node>* currGraph){
			// free(currGraph);
			delete currGraph;
		}

		void freeEdgeList(unordered_set<Edge,hashEdge>* currEdgeList){
			// free(currEdgeList);
			delete currEdgeList;
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
						if(*s!=i->rhs){
							Edge e(*s, i->rhs, &(this->graph));
							this->edge_list.emplace(e);	
						}
					}
				}
				if(i->type==2){ //load  : p = *q
					Node& src = this->graph.find(i->rhs)->second;
					for(s=src.pointsTo.begin();s!=src.pointsTo.end();s++){
						if(*s!=i->lhs){
							Edge e(i->lhs, *s, &(this->graph));
							this->edge_list.emplace(e);
						}
					}
				}
			}
		}

		void captureEdgeData(string v, char x[100]){
			//number of incoming and outgoing edges for a node v
			unordered_set<Edge,hashEdge>::iterator i;
			int incoming = 0, outgoing = 0;

			Node& n = this->graph.find(v)->second;

			incoming = n.incoming.size();
			outgoing = n.outgoing.size();

			// for(i=this->edge_list.begin();i!=this->edge_list.end();i++){
			// 	if(i->src==v){
			// 		outgoing++;
			// 	}
			// 	else
			// 	if(i->dst==v){
			// 		incoming++;
			// 	}
			// }
			sprintf(x, "%d,%d", incoming, outgoing);
		}

		void captureConstraintData(string v, char x[100]){
			//number of constraints of different types a node v is part of
			vector<Constraint> :: iterator i;
			int type_counts[4];
			for(int j=0;j<4;j++){
				type_counts[j]=0;
			}
			for(i=this->constraint_list.begin();i!=this->constraint_list.end();i++){
				if(i->lhs==v || i->rhs==v){
					type_counts[i->type]++;
				}
			}
			sprintf(x, "%d,%d,%d,%d", type_counts[0], type_counts[1], type_counts[2], type_counts[3]);
		}

		void pathBegin(string curr, string v, int length, int level, int* count){
			Node& n = this->graph.find(curr)->second;
			unordered_set<Edge,hashEdge>::iterator i;
			for(i=n.outgoing.begin();i!=n.outgoing.end();i++){
				// if(i->src==curr){
					if(i->dst!=v){
						if(level==length)
							*count += 1;
						else
							pathBegin(i->dst, v, length, level+1, count);
					}
				// }
			}
		}

		void pathEnd(string curr, string v, int length, int level, int* count){
			Node& n = this->graph.find(curr)->second;
			unordered_set<Edge,hashEdge>::iterator i;
			for(i=n.incoming.begin();i!=n.incoming.end();i++){
				// if(i->dst==curr){
					if(i->src!=v){
						if(level==length)
							*count += 1;
						else
							pathEnd(i->dst, v, length, level+1, count);
					}
				// }
			}
		}

		void capturePathData(string v, char x[100]){
			//number of 3,4 length paths a node v is part of
			int begin3 = 0, begin4 = 0, end3 = 0, end4 = 0;
			pathBegin(v, v, 3, 1, &begin3);
			pathBegin(v, v, 4, 1, &begin4);
			pathEnd(v, v, 3, 1, &end3);
			pathEnd(v, v, 4, 1, &end4);
			sprintf(x, "%d,%d,%d,%d", begin3, begin4, end3, end4);
		}

		void cycleSearch(string curr, string v, int length, int level, int* count){
			Node& n = this->graph.find(curr)->second;
			unordered_set<Edge,hashEdge>::iterator i;
			for(i=n.outgoing.begin();i!=n.outgoing.end();i++){
				// if(i->src==curr){
					if(i->dst==v){
						if(level==length)
							*count += 1;
					}
					else{
						if(level<length)
							cycleSearch(i->dst, v, length, level+1, count);
					}
				// }
			}
		}
		
		void captureCycleData(string v, char x[100]){
			//number of 3,4 length cycles a node v is part of
			int count3 = 0, count4 = 0;
			cycleSearch(v, v, 3, 1, &count3);
			cycleSearch(v, v, 4, 1, &count4);
			sprintf(x, "%d,%d", count3, count4);
		}

		float nodeClusteringCoefficient(string v){
			unordered_set<Edge,hashEdge>::iterator i;
			Node &n = this->graph.find(v)->second;
			int degree = n.incoming.size()+n.outgoing.size();

			unordered_set<string> neighbours;

			for(i=n.incoming.begin();i!=n.incoming.end();i++){
				neighbours.emplace(i->src);
			}

			for(i=n.outgoing.begin();i!=n.outgoing.end();i++){
				neighbours.emplace(i->dst);
			}

			int count = 0;
			unordered_set<string>::iterator j;
			for(j=neighbours.begin();j!=neighbours.end();j++){
				Node& curr = this->graph.find(*j)->second;
				for(i=curr.outgoing.begin();i!=curr.outgoing.end();i++){
					if(neighbours.find(i->dst)!=neighbours.end()){
						count++;
					}
				}
			}

			if(degree<=1)
				return 0.0;

			return (float)count/(degree*(degree-1));

		}

		void captureIterationData(int iteration){
			if(!DATA_CAPTURE)
				return;

			FILE* fp;

			fp = fopen("data.txt", "a");

			unordered_set<string> :: iterator i;
			for(i=this->var_list.begin();i!=this->var_list.end();i++){
				char x1[100], x2[100], x3[100], x4[100];
				this->captureEdgeData(*i, x1);
				this->captureConstraintData(*i, x2);
				this->capturePathData(*i, x3);
				this->captureCycleData(*i, x4);
				float val = this->nodeClusteringCoefficient(*i);
				fprintf(fp, "%s:%d:%s:%s:%s:%s:%f\n", i->c_str(), iteration, x1, x2, x3, x4, val);
			}
			fclose(fp);
		}

		void captureFinalGraphData(){
			if(!DATA_CAPTURE)
				return;

			FILE* fp;
			fp = fopen("graph.txt", "w");
			unordered_set<string>::iterator i;
			unordered_set<Edge,hashEdge>::iterator j;

			for(i=this->var_list.begin();i!=this->var_list.end();i++){
				Node& curr = this->graph.find(*i)->second;
				fprintf(fp, "%s:", curr.var.c_str());
				for(j=curr.outgoing.begin();j!=curr.outgoing.end();j++){
					fprintf(fp, "%s,", j->dst.c_str());
				}
				fprintf(fp, "\n");
			}
			fclose(fp);
		}

		void iterativeProcess(){

			unordered_map<string,Node>* oldGraph;
			unordered_set<Edge,hashEdge>* oldEdgeList;
			bool fp;
			int iteration = 0;

			FILE* F;
			F = fopen("data.txt", "w");
			fclose(F);

			do {
				
				this->captureIterationData(iteration);

				oldGraph = this->deepCopyGraph();
				oldEdgeList = this->deepCopyEdgeList();

				this->propagatePointsToInfo();

				this->addLS_Constraints();

				fp = fixedPoint(oldGraph, oldEdgeList);

				cout << "Computing Fixed point" << endl << flush;

				this->freeEdgeList(oldEdgeList);
				this->freeGraph(oldGraph);

				iteration++;
			} 
			while(!fp);

			this->captureFinalGraphData();
		}

		void printPointsTo(){
			unordered_set<string> :: iterator i;
			for(i=this->var_list.begin();i!=this->var_list.end();i++){
				Node& curr = this->graph.find(*i)->second;
				cout << curr.var << " : ";
				for(auto j=curr.pointsTo.begin();j!=curr.pointsTo.end();j++){
					cout << *j << ",";
				}
				cout << endl << flush;
			}
		}

	public:
		PointsToAnalysis(const char* filename){
			ifstream file;
			file.open(filename);

			// char input[10];
			string input;
			float x;
			cout << "Reading the data from the file " << filename << " :" << endl;

			int m, n;
			string temp;

			getline(file, temp);
			sscanf(temp.c_str(), "%d", &n);
			getline(file, temp);
			getline(file, temp);
			getline(file, temp);
			sscanf(temp.c_str(), "%d", &m);

			cout << "Number of variables: " << n << endl;
			cout << "Number of constraints: " << m << endl;

			int i = 0;
			while(i<m && i<70000) {
				getline(file, input);
				Constraint *cts = new Constraint(input);
				this->constraint_list.push_back(*cts);
				i++;
			}
			file.close();
		}

		void run(){
			cout << "constraint_list size: " << this->constraint_list.size() << endl;
			cout << "Starting to run..." << endl << flush;
			this->getVariableList();
			cout << "Extracted variable list... : " << this->var_list.size() << endl << flush;
			this->processAddressOfConstraints();
			cout << "Processed address-of constraints..." << endl << flush;
			this->processCopyConstraints();
			cout << "Processed copy constraints..." << endl << flush;
			this->iterativeProcess();
			// cout << "Printing points to information:" << endl << flush;
			// this->printPointsTo();
		}

};

int main(int argc, char* argv[]){

	string filename;

	// cout << "Input filename:";

	// cin >> filename;

	PointsToAnalysis x(argv[1]);

	x.run();

	cout << "Done" << endl;

}