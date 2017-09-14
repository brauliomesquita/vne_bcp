#ifndef PATHS_H
#define PATHS_H

#include <list>
#include <map>
#include <utility>

#include "Request.h"
#include "Edge.h"

class Paths
{
public:
    int addPath(std::list<Edge*> path, int orig, int dest);
    std::list<Edge*> getPath(int i);

    int getOrig(int id);
    int getDest(int id);

    Paths();
    ~Paths();

private:
    int count;
    std::map<int, std::list<Edge*>> pathsMap;
    std::map<int, std::pair<int, int>> pathsOrigDest;
};

#endif

