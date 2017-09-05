#include "Constraint.h"
#include "Request.h"
#include "Node.h"
#include "Edge.h"

Constraint::Constraint(){
    this->type = ConstType::UNKNOWN;

    this->lb = -IloInfinity;
    this->ub = IloInfinity;
}

Constraint::ConstType Constraint::getType(){
    return this->type;
}

double Constraint::getLb(){
    return this->lb;
}

double Constraint::getUb(){
    return this->ub;
}

void Constraint::setUb(double value){
    this->ub = value;
}

void Constraint::setLb(double value){
    this->lb = value;
}

void Constraint::setReqAccConst(Request * request){
    this->type = ConstType::REQ_ACC;

    this->request = request;
}

void Constraint::setCpuCapacityConst(Node * physicalNode){
    this->type = ConstType::CPU_CAPACITY;
    
    this->physNode = physicalNode;
}

void Constraint::setNodeMapConst(Request * request, Node * virtualNode){
    this->type = ConstType::NODE_MAP;
    
    this->request = request;
    this->virtualNode = virtualNode;
}

bool Constraint::operator < (const Constraint& other) const
{
    if(this->type != other.type)
        return this->type < other.type;

    if(this->type == ConstType::REQ_ACC){
        return (this->request) < (other.request);
    }

    if(this->type == ConstType::CPU_CAPACITY){
        return (this->physNode->getId()) < (other.physNode->getId());
    }

    if(this->type == ConstType::NODE_MAP){
         if(this->request->getId() != other.request->getId())
             return this->request->getId() < other.request->getId();
        return (this->virtualNode->getId()) < (other.virtualNode->getId());
    }

}