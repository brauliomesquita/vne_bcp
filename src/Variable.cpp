#include "Variable.h"

Variable::Variable(){
    this->type = VarType::UNKNOWN;

    request = nullptr;
    
    virtualNode = nullptr;
    physNode = nullptr;

    virtualEdge = nullptr;
    this->pathId = -1;

    this->coef = 0.0;

    this->lb = 0.0;
    this->ub = 1.0;

    this->index = -1;
}

void Variable::setLambdaVar(Request * request, Edge * vEdge, int pathId){
    this->type = VarType::LAMBDA;

    this->request = request;
    this->virtualEdge = vEdge;

    this->pathId = pathId;
}

void Variable::setReqAccVar(Request * request){
    this->type = VarType::REQ_ACC;

    this->request = request;
}

void Variable::setNodeMapVar(Request * request, Node * virtualNode, Node * physicalNode){
    this->type = VarType::NODE_MAP;

    this->request = request;
        
    this->virtualNode = virtualNode;
    this->physNode = physicalNode;
}

int Variable::getIndex(){
    return index;
}

void Variable::setIndex(int value){
    this->index = value;
}


Node * Variable::getVirtualNode(){
    return this->virtualNode;
}

Node * Variable::getPhysNode(){
    return this->physNode;
}

void Variable::setCoef(double value){
    this->coef = value;
}

double Variable::getCoef(){
    return this->coef;
}

void Variable::setUb(double value) {
    this->ub = value;
}

void Variable::setLb(double value) {
    this->lb = value;
}    

void Variable::setValue(double value) {
    this->value = value;
}    

double Variable::getUb() {
    return this->ub;
}

double Variable::getLb() {
    return this->lb;
}

double Variable::getValue() {
    return this->value;
}

Variable::VarType Variable::getType(){
    return this->type;
}

char const* Variable::getName(){
    char *varName = new char[32];

    if(this->type == VarType::REQ_ACC){
        sprintf(varName, "y_%d", this->request->getId());
    } else if(this->type == VarType::NODE_MAP){
        sprintf(varName, "z_%d_%d_%d", this->request->getId(), this->virtualNode->getId(), this->physNode->getId());
    }
    return varName;
}

bool Variable::operator < (const Variable& other) const
{
    if(this->type != other.type)
        return this->type < other.type;

    if(this->type == VarType::REQ_ACC){
        return this->request->getId() < other.request->getId();
    }

    if(this->type == VarType::NODE_MAP){
        if(this->request->getId() != other.request->getId())
            return this->request->getId() < other.request->getId();

        if(this->virtualNode->getId() != other.virtualNode->getId())
            return this->virtualNode->getId() < other.virtualNode->getId();

        return this->physNode->getId() < other.physNode->getId();
    }
    
    if(this->type == VarType::LAMBDA){
        if(this->request->getId() != other.request->getId())
            return this->request->getId() < other.request->getId();

        if(this->virtualEdge->getId() != other.virtualEdge->getId())
            return this->virtualEdge->getId() < other.virtualEdge->getId();
        
        return this->pathId < other.pathId;
    }
}

Request* Variable::getRequest(){
    return this->request;
}