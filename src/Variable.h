#ifndef VARIABLE_H
#define VARIABLE_H

#include <ilcplex/ilocplex.h>
#include "Request.h"
#include "Node.h"
#include "Edge.h"
#include <list>

class Variable
{
    double value;
    double lb, ub;
    double coef;
    
    Request * request;

    Node * virtualNode;
    Node * physNode;

    Edge * virtualEdge;
    int pathId;
    int index;

public:
    enum VarType {
        UNKNOWN = 0,
        REQ_ACC,
        NODE_MAP,
        LAMBDA
    };

    VarType type;

    // Constructor
    Variable();

    // Getters
    double getCoef();
    double getUb();
    double getLb();
    double getValue();
    VarType getType();
    int getPathId();
    
    Request * getRequest();
    Node * getPhysNode();
    Node * getVirtualNode();

    Edge * getVirtualEdge();

    int getIndex();
    void setIndex(int value);

    // Setters
    void setCoef(double value);
    void setUb(double value);
    void setLb(double value);
    void setValue(double value);

    char const* getName();

    // Operators
    bool operator < (const Variable& other) const;

    void setReqAccVar(Request * request);
    void setNodeMapVar(Request * request, Node * virtualNode, Node * physicalNode);
    void setLambdaVar(Request * request, Edge * vEdge, int pathId);

    struct VariableComp {
        bool operator() (const Variable* lhs, const Variable* rhs) const
        {
            return *lhs < *rhs;
        }
    };

};

#endif