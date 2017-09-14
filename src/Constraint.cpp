#include "Constraint.h"
#include "Request.h"
#include "Node.h"
#include "Edge.h"

Constraint::Constraint(){
    this->type = ConstType::UNKNOWN;

    this->lb = -IloInfinity;
    this->ub = IloInfinity;

    this->index = -1;
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

int Constraint::getIndex(){
    return this->index;
}

void Constraint::setIndex(int value){
    this->index = value;
}

void Constraint::setLb(double value){
    this->lb = value;
}

void Constraint::setReqAccConst(Request * request, Node * pNode){
    this->type = ConstType::REQ_ACC;

    this->request = request;
    this->physNode = pNode;
}

void Constraint::setCpuCapacityConst(Node * physicalNode){
    this->type = ConstType::CPU_CAPACITY;
    
    this->physNode = physicalNode;
}

void Constraint::setBWCapacityConst(Edge * physicalEdge){
    this->type = ConstType::BANDWIDTH_CAPACITY;

    this->physEdge = physicalEdge;
}

void Constraint::setNodeMapConst(Request * request, Node * virtualNode){
    this->type = ConstType::NODE_MAP;
    
    this->request = request;
    this->virtualNode = virtualNode;
}

void Constraint::setPathAssignConst(Request * request, Edge * virtualEdge){
    this->type = ConstType::PATH_ASSIGN;
    
    this->request = request;
    this->virtualEdge = virtualEdge;
}

bool Constraint::operator < (const Constraint& other) const
{
    if(this->type != other.type)
        return this->type < other.type;

    if(this->type == ConstType::REQ_ACC){
        return this->request->getId() < other.request->getId();
    }

    if(this->type == ConstType::CPU_CAPACITY){
        return (this->physNode->getId()) < (other.physNode->getId());
    }

    if(this->type == ConstType::BANDWIDTH_CAPACITY){
        return (this->physEdge->getId()) < (other.physEdge->getId());
    }

    if(this->type == ConstType::NODE_MAP){
         if(this->request->getId() != other.request->getId())
             return this->request->getId() < other.request->getId();
        return (this->virtualNode->getId()) < (other.virtualNode->getId());
    }

    if(this->type == ConstType::PATH_ASSIGN){
        if(this->request->getId() != other.request->getId())
            return this->request->getId() < other.request->getId();
       return (this->virtualEdge->getId()) < (other.virtualEdge->getId());
   }

}