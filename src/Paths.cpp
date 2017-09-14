#include "Paths.h"

int Paths::addPath(std::list<Edge*> path, int orig, int dest){
    pathsMap[count] = std::list<Edge*>(path);
    
    auto pair = std::make_pair(orig, dest);
    pathsOrigDest[count] = pair;

    count++;
    return count - 1;
}

std::list<Edge*> Paths::getPath(int i){
    return pathsMap[i];
}

Paths::Paths(){
    this->pathsMap = std::map<int, std::list<Edge*> >();
    this->pathsOrigDest = std::map<int, std::pair<int, int>>();
    this->count = 0;
}

Paths::~Paths(){
    this->pathsMap.clear();
}

int Paths::getOrig(int id){
    return pathsOrigDest[count].first;
}

int Paths::getDest(int id){
    return pathsOrigDest[count].second;
}