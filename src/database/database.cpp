#include "database.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <list>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <algorithm>

void Hmetis::configureGraph(parser &the_parser)
{
    graph = new Hypergraph(the_parser.nodes.size(), the_parser.edges.size());
    // std::cout << "1111" << std::endl;
    //  std::cout << the_parser.nodes.size() << " " << the_parser.edges.size() << std::endl;
    //  for (const auto& node:the_parser.nodes )
    //  {
    //      std::cout << node << std::endl;
    //  }
    for (int i = 0; i < the_parser.edges.size() - 1; i++)
    {
        std::vector<int> hEdge;
        for (const auto &node : the_parser.edges.at(i))
        {
            hEdge.emplace_back(the_parser.mapping.at(node));
        }
        graph->addNodeList(i, hEdge);
    }
    for (int i = 1; i < the_parser.nodes.size(); i++)
    {
        graph->setNodeSizeOf(i - 1, 1, 0);
        graph->setNodeSizeOf(i - 1, 1, 1);
    }
    spaceLimit.push_back(0.01);
    spaceLimit.push_back(0.01);
    graph->setTerminalSize(0);
}

// output partitioned graph to the given box
void Hmetis::outputGraph()
{
    LayerInfo *inst = Instances.front();

    for (int i = 0; i < graph->getNodeNum(); i++)
    {
        std::cout << i << " " << inst->getPartitionOf(i) << std::endl;
    }

    delete inst;
}

// hmetis coarsening
// 'num' is the number of nodes left after coarsening
// 'restricted' for if the coarsen is restricted
// 'maxIter' for maximum coarsening iteration
void Hmetis::coarsen(int num, bool restricted, std::string scheme, int maxIter)
{
    Coarsen C;
    LayerInfo *base;
    int prevNodeNum = INT_MAX, tmpNodeNum;

    if (Instances.empty())
        base = new LayerInfo(graph);
    else
        base = Instances.front();

    while (maxIter > 0 && graph->getNodeNum() > num)
    {
        auto info = C.Coarsening(graph, base, spaceLimit, restricted, scheme);
        coarsenInfo.push_back(info);

        // early stopping
        tmpNodeNum = graph->getNodeNum();
        if (double(prevNodeNum - tmpNodeNum) / double(prevNodeNum) < 0.01)
            break;
        prevNodeNum = tmpNodeNum;

        maxIter--;
    }
}

// check if the coarsening pattern can be placed under constraint
void Hmetis::preIPAdjustment()
{
    LayerInfo *inst = new LayerInfo(graph);

    while (!inst->bfsPlacing(graph, spaceLimit))
    {
        auto cf = coarsenInfo.back();
        coarsenInfo.pop_back();
        graph->revertGraph(cf);

        for (auto it = cf->begin(); it != cf->end(); it++)
            delete it->second;
        delete cf;

        inst->~LayerInfo();
        new (inst) LayerInfo(graph);
    }

    delete inst;
}

// wrapper of layer information
void IPwrapper(partition_info info)
{
    while (!info.inst->bfsPlacing(info.graph, info.spaceLimit))
    {
        info.inst->~LayerInfo();
        new (info.inst) LayerInfo(info.graph);
    }

    biPart(info.graph, info.inst, info.spaceLimit, info.scheme);
}

// hmetis initial partitioning
// 'num' is the number of instances
void Hmetis::initialPartition(int num, std::string scheme)
{
    // make sure the hypergraph can be partitioned under space limit
    preIPAdjustment();
    std::cout << "1111" << std::endl;
    // graph->buildNeighbors();

    std::vector<partition_info> infos;
    for (int i = 0; i < num; i++)
    {
        LayerInfo *inst = new LayerInfo(graph);
        infos.push_back(partition_info(graph, inst, spaceLimit, scheme));
    }
    std::cout << "1111" << std::endl;
    std::vector<std::thread> threads;
    for (int i = 0; i < num; i++)
    {
        threads.push_back(std::thread(IPwrapper, infos[i]));
    }
    std::cout << "1111" << std::endl;
    for (int i = 0; i < num; i++)
    {
        threads[i].join();
        Instances.push_back(infos[i].inst);
    }
}

void biPartWrapper(partition_info info)
{
    biPart(info.graph, info.inst, info.spaceLimit, info.scheme);
}

// hmetis uncoarsening
void Hmetis::uncoarsen(double dropRate, string scheme)
{
    while (!coarsenInfo.empty())
    {
        auto cf = coarsenInfo.back();
        coarsenInfo.pop_back();
        graph->revertGraph(cf);
        // graph->buildNeighbors();

        std::vector<partition_info> infos;
        for (auto inst = Instances.begin(); inst != Instances.end(); inst++)
        {
            infos.push_back(partition_info(graph, *inst, spaceLimit, scheme));
        }

        std::vector<std::thread> threads;
        for (auto info = infos.begin(); info != infos.end(); info++)
        {
            (*info).inst->mapPartition(cf);

            threads.push_back(std::thread(biPartWrapper, *info));
        }

        for (int i = 0; i < threads.size(); i++)
        {
            threads[i].join();
        }

        dropWorstCut(dropRate);

        for (auto it = cf->begin(); it != cf->end(); it++)
            delete it->second;
        delete cf;
    }

    chooseBestInstance();
}

// hmetis refine
void Hmetis::refine(int num, double dropRate, std::string scheme_c, std::string scheme_u, int maxIter)
{
    coarsen(num, true, scheme_c, maxIter);

    uncoarsen(dropRate, scheme_u);
}

// return the size of instances[]
int Hmetis::getInstanceSize()
{
    return Instances.size();
}

// drop instances which are 'dropRate' worse than the best cut (0.1 stands for 10 percent)
void Hmetis::dropWorstCut(double dropRate)
{
    double minScore = DBL_MAX;

    for (auto it = Instances.begin(); it != Instances.end(); it++)
        minScore = std::min((*it)->getPartitionScore(), minScore);

    minScore *= (1.0 + dropRate);

    for (auto it = Instances.begin(); it != Instances.end();)
    {
        if ((*it)->getPartitionScore() > minScore)
        {
            delete *it;
            Instances.erase(it++);
        }
        else
            it++;
    }
}

// return the best instance in instances[]
void Hmetis::chooseBestInstance()
{
    auto bestIdx = Instances.begin();
    double tmpScore, bestScore = (*bestIdx)->getPartitionScore();

    for (auto it = ++(Instances.begin()); it != Instances.end();)
    {
        tmpScore = (*it)->getPartitionScore();
        if (tmpScore < bestScore)
        {
            bestScore = tmpScore;
            delete *bestIdx;
            Instances.erase(bestIdx);
            bestIdx = it++;
        }
        else
        {
            delete *it;
            Instances.erase(it++);
        }
    }
}