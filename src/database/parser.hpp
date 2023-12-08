#pragma once


#include <vector>
#include <set>
#include <map>

#include <iostream>
#include <fstream>
#include <sstream>
class parser
{
public:
    parser() {}

    void read(const std::string &filename);
    void update();
    void write();
    std::vector<std::vector<int>> edges;
    std::set<int> nodes;
    std::map<int,int> mapping;
private:
};
