#include "BranchAndPrice.h"

BranchAndPrice::BranchAndPrice(Graph *substrato, std::vector<Request*> requisicoes){
    this->substrato = substrato;
    this->requisicoes = requisicoes;
}

void BranchAndPrice::Solve(){
    Paths * paths = new Paths();
    IloEnv env;

    auto ilp = new TreeNode(env, substrato, requisicoes, paths);

    ilp->Solve();

    delete ilp;
    env.end();

}