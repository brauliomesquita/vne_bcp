#include "TreeNode.h"

TreeNode::TreeNode(IloEnv env, Graph *substrate, std::vector<Request *> requests, Paths * paths)
{
	this->env = env;

	this->substrate = substrate;
	this->requests = requests;

	this->paths = paths;

	vars = std::set<Variable*, Variable::VariableComp>();
	consts = std::set<Constraint*, Constraint::ConstraintComp>();
}

TreeNode::TreeNode(IloEnv env, Graph *substrate, std::vector<Request *> requests, std::set<Variable*, Variable::VariableComp> vars, std::set<Constraint*, Constraint::ConstraintComp> consts, Paths * paths)
{
	this->env = env;

	this->substrate = substrate;
	this->requests = requests;

	this->paths = paths;

	vars = std::set<Variable*, Variable::VariableComp>(vars);
	consts = std::set<Constraint*, Constraint::ConstraintComp>(consts);
}

void TreeNode::createModel(){
	model = IloModel(env);
	problem = new IloCplex(env);

	createVariables();
	buildModel();
	model.add(variables_);

	createObjFunction();

    createConstraints();
	model.add(constraints_);
}

void TreeNode::createObjFunction()
{
	IloObjective objective(env);
	objective = IloAdd(model, IloMinimize(env));
	
	IloExpr obj(env);

	for(auto var = vars.begin(); var != vars.end(); var++){
		auto varIndex = (*var)->getIndex();
		auto var_ = variables_[varIndex];

		obj += (*var)->getCoef() * var_;
	}

	obj += requests.size() * 10000;

	objective.setExpr(obj);
	obj.end();
}

void TreeNode::createVariables()
{
	variables_ = IloNumVarArray(env);
	
	for(int v = 0; v < requests.size(); v++){
		auto var = new Variable();
		var->setReqAccVar(requests[v]);
		var->setCoef(-10000);

		vars.insert(var);
	}

	for(int v = 0; v < requests.size(); v++){
		for(int k = 0; k < requests[v]->getGraph()->getN(); k++){

			auto vNode = requests[v]->getGraph()->getNodes()[k];

			for(int i=0; i<substrate->getN(); i++){

				auto physNode = substrate->getNodes()[i];
				
				auto var = new Variable();
				var->setNodeMapVar(requests[v], vNode, physNode);

				vars.insert(var);
			}

		}
	}
}

void TreeNode::createConstraints()
{
	constraints_ = IloRangeArray(env);
	auto constCount = 0;
	auto var = new Variable();

	// If a VN is accepted, then all of its virtual nodes must be mapped onto a physical node
	for(int v = 0; v < requests.size(); v++){
		var->setReqAccVar(requests[v]);

		auto vIt = vars.find(var);
		auto varIndex = (*vIt)->getIndex();
		auto yVar = variables_[varIndex];

		for(int k = 0; k < requests[v]->getGraph()->getN(); k++){
			IloExpr expr(env);
			auto vNode = requests[v]->getGraph()->getNodes()[k];

			for(int i=0; i<substrate->getN(); i++){
				auto physNode = substrate->getNodes()[i];
				
				var->setNodeMapVar(requests[v], vNode, physNode);

				vIt = vars.find(var);
				if(vIt == vars.end())
					continue;
				varIndex = (*vIt)->getIndex();
				expr += variables_[varIndex];
			}
			
			auto constraint_ = new Constraint();
			constraint_->setReqAccConst(requests[v], vNode);
			constraint_->setIndex(constCount++);
			consts.insert(constraint_);

			constraints_.add(expr - yVar == 0);
			expr.end();
		}
		
	}

	// No more than 1 virtual node of the same VN can be mapped onto the same physical node
	for(int v = 0; v < requests.size(); v++){
		for(int i=0; i<substrate->getN(); i++){
			IloExpr expr(env);
			auto physNode = substrate->getNodes()[i]; 

			for(int k = 0; k < requests[v]->getGraph()->getN(); k++){
				auto vNode = requests[v]->getGraph()->getNodes()[k];

				var->setNodeMapVar(requests[v], vNode, physNode);

				auto vIt = vars.find(var);
				if(vIt == vars.end())
					continue;
				auto varIndex = (*vIt)->getIndex();
				expr += variables_[varIndex];
			}
		
			auto constraint_ = new Constraint();
			constraint_->setNodeMapConst(requests[v], physNode);
			constraint_->setIndex(constCount++);
			consts.insert(constraint_);

			constraints_.add(expr <= 1);
			expr.end();
		}
	}

	// CPU Constraint
	for(int i=0; i<substrate->getN(); i++){
		IloExpr expr(env);
		auto physNode = substrate->getNodes()[i]; 

		for(int v = 0; v < requests.size(); v++){
			for(int k = 0; k < requests[v]->getGraph()->getN(); k++){
				auto vNode = requests[v]->getGraph()->getNodes()[k];

				var->setNodeMapVar(requests[v], vNode, physNode);

				auto vIt = vars.find(var);
				if(vIt == vars.end())
					continue;
				auto varIndex = (*vIt)->getIndex();
				expr += vNode->getCPU() * variables_[varIndex];
			}
		}

		auto constraint_ = new Constraint();
		constraint_->setCpuCapacityConst(physNode);
		constraint_->setIndex(constCount++);
		consts.insert(constraint_);

		constraints_.add(expr <= physNode->getCPU());
		expr.end();
	}

	// Bandwidth Capacity
	for(int e=0; e<substrate->getM(); e++){
		IloExpr expr(env);
		auto physEdge = substrate->getEdges()[e]; 

		auto constraint_ = new Constraint();
		constraint_->setBWCapacityConst(physEdge);
		constraint_->setIndex(constCount++);
		consts.insert(constraint_);

		constraints_.add(expr <= physEdge->getBW());
		expr.end();
	}

	// Path Assignment
	for(int v = 0; v < requests.size(); v++){
		var->setReqAccVar(requests[v]);
		
		auto vIt = vars.find(var);
		auto varIndex = (*vIt)->getIndex();
		auto yVar = variables_[varIndex];

		for(int k = 0; k < requests[v]->getGraph()->getM(); k++){
			IloExpr expr(env);
			auto vEdge = requests[v]->getGraph()->getEdges()[k];

			auto constraint_ = new Constraint();
			constraint_->setPathAssignConst(requests[v], vEdge);
			constraint_->setIndex(constCount++);
			consts.insert(constraint_);

			constraints_.add(expr - yVar == 0);
			expr.end();
		}
	}

	for(auto var = vars.begin(); var != vars.end(); var++){
		if((*var)->getType() != Variable::VarType::LAMBDA)
			continue;

		auto varIndex = (*var)->getIndex();

		//(*var)->getIndex() = variables_.getSize();
	}

}

void TreeNode::buildModel(){

	auto countVars = 0;

	for(auto var = vars.begin(); var != vars.end(); var++){
		auto var_ = IloNumVar(env, (*var)->getLb(), (*var)->getUb(), (*var)->getName());
		
		(*var)->setIndex(countVars++);
		variables_.add(var_);
	}

	problem->extract(model);
}

TreeNode::~TreeNode()
{
	model.end();
	delete problem;
}

void TreeNode::getDuals()
{
	for(auto cons = consts.begin(); cons != consts.end(); cons++){
		//(*cons)->dualValue = problem->getDual((*cons)->getConstraint());
	}
}

float TreeNode::Solve()
{
	const clock_t begin_time = clock();
	createModel();
	std::cout << "Model built in " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << " seconds." << endl;

	cout << "Num Vars: " << vars.size() << endl;
	cout << "Const Vars: " << consts.size() << endl;

	problem->exportModel("modeloMatematico.lp");

	while(1){
		if(!problem->solve())
			return 0.0;

		IloNumArray duals_(env, constraints_.getSize());
		problem->getDuals(duals_, constraints_);
	
		Pricing(duals_);

		break;
	}

	cout << "Objective Function Value: " << problem->getObjValue() << endl;

	/*for(int i=0; i<variables_.getSize(); i++){
		if(problem->getValue(variables_[i]) > 0.001)
			cout << variables_[i].getName() << "\t" << problem->getValue(variables_[i]) << endl;
	}*/

	return 1.0;
}

void TreeNode::Pricing(IloNumArray duals_){
	cout << "********************************************" << endl;
	cout << "Entering Pricing Function" << endl;
	IloEnv pEnv;
	IloModel pModel(pEnv);
	IloCplex pProblem(pEnv);

	IloObjective objective = IloAdd(pModel, IloMinimize(pEnv));
	
	auto constraint_ = new Constraint();
	char varName[32];

	IloIntVarArray O_(pEnv);
	IloIntVarArray D_(pEnv);

	for(int i = 0; i < substrate->getN(); i++){
		sprintf(varName, "O_%d", i);
		IloIntVar o(pEnv, 0, 1, varName);
		O_.add(o);

		sprintf(varName, "D_%d", i);
		IloIntVar d(pEnv, 0, 1, varName);
		D_.add(d);
	}
	
	pModel.add(O_);
	pModel.add(D_);

	IloExpr obj(pEnv);
	for(int e = 0; e < substrate->getM(); e++){
	}
	objective.setExpr(obj);

	IloExpr exprO(pEnv);
	for(int i = 0; i < substrate->getN(); i++){
		exprO += O_[i];
	}
	pModel.add(exprO == 1);

	IloExpr exprD(pEnv);
	for(int i = 0; i < substrate->getN(); i++){
		exprD += D_[i];
	}
	pModel.add(exprD == 1);

	

	pProblem.extract(pModel);

	pProblem.solve();

	int value;
	for(int i = 0; i < substrate->getN(); i++){
		value = pProblem.getIntValue(O_[i]);
		if(value == 1)
			cout << O_[i] << "\t" << value << endl;

		value = pProblem.getIntValue(D_[i]);
		if(value == 1)
			cout << D_[i] << "\t" << value << endl;
	}


	pProblem.exportModel("Pricing.lp");
	cout << "********************************************" << endl;
}