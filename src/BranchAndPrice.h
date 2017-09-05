#ifndef BRANCHANDPRICE_H
#define BRANCHANDPRICE_H

#include <ilcplex/ilocplex.h>
#include "TreeNode.h"
#include "Request.h"
#include "Paths.h"
#include "Graph.h"

class BranchAndPrice {
    Graph *substrato;
    std::vector<Request*> requisicoes;

public:
    BranchAndPrice(Graph *substrato, std::vector<Request*> requisicoes);
    void Solve();
};

#endif