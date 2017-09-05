#ifndef PATHS_H
#define PATHS_H

#include <list>
#include <map>

#include "Edge.h"

class Paths
{
public:
    int addPath(std::list<Edge*> path);
    std::list<Edge*> getPath(int i);

    Paths();
    ~Paths();

private:
    int count;
    std::map<int, std::list<Edge*>> pathsMap;
};

#endif

