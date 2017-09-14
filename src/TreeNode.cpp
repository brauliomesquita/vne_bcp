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

void TreeNode::addLambdaVariables(std::list<Variable*> newVariables){
	cout << "Adding Lambda Variables to the Model" << endl;

	auto countVars = variables_.getSize();
	auto constraint_ = new Constraint();

	for (auto var = newVariables.begin(); var != newVariables.end(); var++){
		auto virtualEdge = (*var)->getVirtualEdge();
		auto var_ = IloNumVar(env, (*var)->getLb(), (*var)->getUb(), (*var)->getName());
		(*var)->setIndex(countVars++);

		model.add(var_);

		// Path Assignment Constraint
		constraint_->setPathAssignConst((*var)->getRequest(), virtualEdge);
		auto cIt = consts.find(constraint_);

		if(cIt != consts.end()){
			int index = (*cIt)->getIndex();
			constraints_[index].setLinearCoef(var_, 1);
		}

		// Bandwidth Capacity Constraint
		int pathId = (*var)->getPathId();
		auto path = paths->getPath(pathId);

		cout << "Path " << pathId << "\t" << path.size() << " physical edges." << endl;

		auto objCoef = 0.0;

		for (auto physEdge = path.begin(); physEdge != path.end(); physEdge++){
			objCoef += (*physEdge)->getBW();

			constraint_->setBWCapacityConst(*physEdge);
			cIt = consts.find(constraint_);

			if(cIt != consts.end()){
				int index = (*cIt)->getIndex();
				constraints_[index].setLinearCoef(var_, virtualEdge->getBW());
			}
		}

		objective.setLinearCoef(var_, objCoef);

	}		
}

float TreeNode::Solve()
{
	auto newVariables = std::list<Variable*>();
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
	
		Pricing(duals_, &newVariables);
		cout << "Number of Variables to add to the model:\t" << newVariables.size() << endl;

		addLambdaVariables(newVariables);

		break;
	}

	//problem->extract(model);
	problem->exportModel("completeModel.lp");

	if(!problem->solve())
		return 0.0;

	cout << "Objective Function Value: " << problem->getObjValue() << endl;

	/*for(int i=0; i<variables_.getSize(); i++){
		if(problem->getValue(variables_[i]) > 0.001)
			cout << variables_[i].getName() << "\t" << problem->getValue(variables_[i]) << endl;
	}*/

	return 1.0;
}

void TreeNode::Pricing(IloNumArray duals_, std::list<Variable*> * newVars){
	cout << "********************************************" << endl;
	cout << "Entering Pricing Function" << endl;
	IloEnv pEnv;
	IloModel pModel(pEnv);
	IloCplex pProblem(pEnv);

	newVars->clear();

	IloObjective pObjective = IloAdd(pModel, IloMinimize(pEnv));
	
	auto constraint_ = new Constraint();
	char varName[32];

	// Assign Variables
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

	// Flow Variables
	IloArray<IloIntVarArray> X_(pEnv, substrate->getN());
	for(int i = 0; i < substrate->getN(); i++){
		X_[i] = IloIntVarArray(pEnv, substrate->getN());
	}

	for(int i = 0; i < substrate->getN(); i++){
		for(int j = 0; j < substrate->getN(); j++){

			int edgeId = substrate->getAdj(i, j);
			if(edgeId == -1)
				continue;

				auto edge = substrate->getEdges()[edgeId];
			
			sprintf(varName, "X_%d_%d", i, j);
			X_[i][j] = IloIntVar(pEnv, 0, 1, varName);

			sprintf(varName, "X_%d_%d", j, i);
			X_[j][i] = IloIntVar(pEnv, 0, 1, varName);
		}
	}

	for(int i = 0; i < substrate->getN(); i++){
		pModel.add(X_[i]);
	}

	// Assign Constraints
	IloExpr orig(pEnv);
	for(int i = 0; i < substrate->getN(); i++){
		orig += O_[i];
	}
	pModel.add(orig == 1);

	IloExpr dest(pEnv);
	for(int i = 0; i < substrate->getN(); i++){
		dest += D_[i];
	}
	pModel.add(dest == 1);

	for(int i = 0; i < substrate->getN(); i++){
		pModel.add(O_[i] + D_[i] <= 1);
	}
	

	// Flow Constraints
	for(int i = 0; i < substrate->getN(); i++){
		IloExpr flow(pEnv);
		
		for(int j = 0; j < substrate->getN(); j++){
			if(substrate->getAdj(i, j) != -1){
				flow +=  X_[i][j];
				flow += -X_[j][i];
			}
		}

		pModel.add(flow - O_[i] + D_[i] == 0);

	}

	IloExpr edgeObjExpr(pEnv);
	for(int e=0; e<substrate->getM(); e++){
		auto physEdge = substrate->getEdges()[e];
		int i = physEdge->getOrig();
		int j = physEdge->getDest();

		auto constraint_ = new Constraint();
		constraint_->setBWCapacityConst(physEdge);

		auto cIt = consts.find(constraint_);
		if(cIt == consts.end()){
			cout << "PUTA QUE PARIU!" << endl;
		}

		int cIndex = (*cIt)->getIndex();

		edgeObjExpr += - duals_[cIndex] * physEdge->getBW() * X_[i][j];
		edgeObjExpr += - duals_[cIndex] * physEdge->getBW() * X_[j][i];
	}


	// Foreach network
	// Foreach demand
	for(int v=0; v<requests.size(); v++){
		auto request = requests[v];

		for(int e=0; e<request->getGraph()->getM(); e++){
			auto edge = request->getGraph()->getEdges()[e];

			for(int i = 0; i < substrate->getN(); i++){
				O_[i].setUb(0);
				D_[i].setUb(0);
			}

			int i = edge->getOrig();
			int j = edge->getDest();

			auto orig = request->getGraph()->getNodes()[i];
			auto dest = request->getGraph()->getNodes()[j];

			for(int f = 0; f < substrate->getN(); f++){
				auto physNode = substrate->getNodes()[f];

				// if orig can be mapped into this physical node
				O_[f].setUb(1);

				// if dest can be mapped into this physical node
				D_[f].setUb(1);
			}

			pProblem.extract(pModel);

			pObjective.setExpr(edgeObjExpr);

			pProblem.solve();

			cout << "Pricing Solution: " << endl;

			// if cost > lambda
			//	continue;

			// List of Physical Edges 
			auto path = std::list<Edge*>();

			for(int i = 0; i < substrate->getN(); i++){
				for(int j = 0; j < substrate->getN(); j++){
					if(substrate->getAdj(i, j) != -1){
						if(pProblem.getIntValue(X_[i][j]) >= 0.999){
							cout << X_[i][j] << "\t" << pProblem.getIntValue(X_[i][j]) << endl;
							int physEdgeId = substrate->getAdj(i, j);
							auto physEdge = substrate->getEdges()[physEdgeId];
							path.push_back(physEdge);
						}
					}
				}
			}

			int orig_ = 1, dest_ = 1;
			for(int i = 0; i < substrate->getN(); i++){
				if(pProblem.getIntValue(O_[i]) >= 0.999){
					cout << O_[i] << "\t" << pProblem.getIntValue(O_[i]) << endl;
					orig_ = i;
				}
				
				if(pProblem.getIntValue(D_[i]) >= 0.999){
					cout << D_[i] << "\t" << pProblem.getIntValue(D_[i]) << endl;
					dest_ = i;
				}
			}

			for(int i = 0; i < substrate->getN(); i++){
				for(int j = 0; j < substrate->getN(); j++){
					if(substrate->getAdj(i, j) != -1){
						if(pProblem.getIntValue(X_[i][j]) >= 0.999){
							cout << X_[i][j] << "\t" << pProblem.getIntValue(X_[i][j]) << endl;
						}
					}
				}
			}
			
			auto pathId = paths->addPath(path, orig_, dest_);

			cout << "\n\n\n\n\n" << endl;
			cout << "New path added:\t" << orig_ << " " << dest_ << "\t" << pathId << endl;

			cout << path.size() << endl;

			cout << "\n\n\n\n\n" << endl;

			auto newVariable = new Variable();
			newVariable->setLambdaVar(request, edge, pathId);

			newVars->push_back(newVariable);
			cout << ">>>>>>>>> " << newVars->size() << endl;
		}
	}

	pProblem.exportModel("Pricing.lp");
	cout << "********************************************" << endl;
}