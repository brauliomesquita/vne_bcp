#include "BranchAndPrice.h"
#include "Request.h"

#include <stdio.h>
#include <iostream>


using namespace std;

Graph * substrate;
std::vector<Request*> requests;

void readSubstrate(char * subGraph) {

	FILE * arquivo = fopen(subGraph, "r");

	if (!arquivo){
		cout << "Erro ao abrir arquivo!" << endl;
		return;
	}

	int n, m, k, l;
	int x, y;
	double cpu, banda, atraso;

	fscanf(arquivo, "%d %d", &n, &m);

	substrate = new Graph(n, m);

	for (int i = 0; i < substrate->getN(); i++) {
		fscanf(arquivo, "%d %d %lf", &x, &y, &cpu);

		substrate->addNode(new Node(i, x, y, cpu));
    }

    for (int i = 0; i < substrate->getM(); i++) {
        fscanf(arquivo, "%d %d %lf %lf", &k, &l, &banda, &atraso);

        substrate->addEdge(new Edge(i, k, l, banda, atraso));
    }

	return;
}

void readVNsFolder(char * folder, int numberVNs) {

	char file[512];

	requests = std::vector<Request*>();

	int n, m, split, chegada, duracao, topologia, raio;

	for (int v = 0; v < numberVNs; v++) {
		sprintf(file, "%sreq%d.txt", folder, v);

		FILE * arquivo = fopen(file, "r");

		if(!arquivo){
			cout << "Erro ao abrir arquivo '" << file << "'!" << endl;
			return;
		}

		fscanf(arquivo, "%d %d %d %d %d %d %d", &n, &m, &split, &chegada,
			&duracao, &topologia, &raio);

		Request * r = new Request(v, chegada, duracao, raio);

		Graph * g = new Graph(n, m);

		int k, l;
		int x, y;
		double cpu;
		double banda, atraso;
		double profit = 0;

		for (int i = 0; i < g->getN(); i++) {
			fscanf(arquivo, "%d %d %lf", &x, &y, &cpu);

			g->addNode(new Node(i, x, y, cpu));
			profit += cpu;
		}

		for (int i = 0; i < g->getM(); i++) {
			fscanf(arquivo, "%d %d %lf %lf", &k, &l, &banda, &atraso);

			g->addEdge(new Edge(i, k, l, banda, atraso));
			profit += banda;
		}

		r->setGraph(g);
		r->setProfit(profit);

		requests.push_back(r);

		fclose(arquivo);

	}

}

int main(int argc, char *argv[])
{
	readSubstrate(argv[1]);
	readVNsFolder(argv[2], atoi(argv[3]));

	substrate->setDist(substrate);
	for(int v=0; v<requests.size(); v++){
		requests[v]->getGraph()->setDist(substrate);
	}

	auto ilp = new BranchAndPrice(substrate, requests);
	ilp->Solve();

delete ilp;

	delete substrate;
	for(int v=0; v<requests.size(); v++){
		delete requests[v];
	}
	requests.clear();
	return 0;
}
