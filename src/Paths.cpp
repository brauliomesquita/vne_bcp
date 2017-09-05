#include "Paths.h"

int Paths::addPath(std::list<Edge*> path){
    pathsMap[count] = std::list<Edge*>(path);
    count++;
    return count;
}

std::list<Edge*> Paths::getPath(int i){
    return pathsMap[i];
}

Paths::Paths(){
    this->pathsMap = std::map<int, std::list<Edge*> >();
    this->count = 0;
}

Paths::~Paths(){
    this->pathsMap.clear();
}