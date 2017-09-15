#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <list>
#include <utility>
#include "Request.h"
#include "Variable.h"
#include "Node.h"
#include "Edge.h"

class Constraint
{
    double ub;
    double lb;
    
    Request * request;

    Node * virtualNode;
    Node * physNode;

    Edge * virtualEdge;
    Edge * physEdge;

    int index;

public:
    enum ConstType {
        UNKNOWN = 0,
        REQ_ACC,
        NODE_MAP,
        CPU_CAPACITY,
        BANDWIDTH_CAPACITY,
        PATH_ASSIGN,
        PATH_INITNODE,
        PATH_ENDNODE
    };

    ConstType type;

    // Constructor
    Constraint();

    // Getters
    double getLb();
    double getUb();
    IloRange getConstraint();
    ConstType getType();
    
    int getIndex();    
    void setIndex(int value);

    // Setters
    void setUb(double value);
    void setLb(double value);

    // Operators
    bool operator < (const Constraint& other) const;

    void setReqAccConst(Request * request, Node * pNode);
    void setCpuCapacityConst(Node * physicalNode);
    void setBWCapacityConst(Edge * physicalEdge);
    void setNodeMapConst(Request * request, Node * virtualNode);
    void setPathAssignConst(Request * request, Edge * virtualEdge);

    void setInitialNodeConst(Request * request, Edge * virtualEdge, Node * physNode);
    void setEndNodeConst(Request * request, Edge * virtualEdge, Node * physNode);

    struct ConstraintComp {
        bool operator() (const Constraint* lhs, const Constraint* rhs) const
        {
            return *lhs < *rhs;
        }
    };

};

#endif