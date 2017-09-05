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

public:
    enum ConstType {
        UNKNOWN = 0,
        REQ_ACC,
        CPU_CAPACITY,
        NODE_MAP,
        BANDWIDTH_CAPACITY,
        FLOW
    };

    ConstType type;

    // Constructor
    Constraint();

    // Getters
    double getLb();
    double getUb();
    IloRange getConstraint();
    ConstType getType();
    
    // Setters
    void setUb(double value);
    void setLb(double value);

    // Operators
    bool operator < (const Constraint& other) const;

    void setReqAccConst(Request * request);
    void setCpuCapacityConst(Node * physicalNode);
    void setNodeMapConst(Request * request, Node * virtualNode);

    struct ConstraintComp {
        bool operator() (const Constraint* lhs, const Constraint* rhs) const
        {
            return *lhs < *rhs;
        }
    };

};

#endif