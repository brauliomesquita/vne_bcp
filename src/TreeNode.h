#ifndef TREENODE_H_
#define TREENODE_H_

#include <ilcplex/ilocplex.h>
#include <vector>
#include <set>
#include <fstream>
#include <ctime>

#include "Paths.h"
#include "Request.h"
#include "Variable.h"
#include "Constraint.h"

using namespace std;

#define M 10000

class TreeNode {
	IloEnv env;
	Paths * paths;
	Graph *substrate;
	std::vector<Request *> requests;

	std::set<Variable*, Variable::VariableComp> vars;
	std::set<Constraint*, Constraint::ConstraintComp> consts;

	IloModel model;
	IloCplex * problem;

	IloNumVarArray variables_;
	IloRangeArray constraints_;

public:
	TreeNode(IloEnv env, Graph *substrato, std::vector<Request*> requisicoes, Paths * paths);
	TreeNode(IloEnv env, Graph *substrate, std::vector<Request *> requests, std::set<Variable*, Variable::VariableComp> vars,
		std::set<Constraint*, Constraint::ConstraintComp> consts, Paths * paths);
	~TreeNode();

	float Solve();

	void createModel();
	void buildModel();
	void createVariables();
	void createConstraints();
	void createObjFunction();
	void getDuals();
	void addToModel();
};

#endif
