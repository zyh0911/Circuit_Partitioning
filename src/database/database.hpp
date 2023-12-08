#pragma once

#include "Hypergraph.hpp"
#include "../algorithm/partition.hpp"
#include "../algorithm/Coarsening.hpp"



#include <string>
#include <stack>
#include <vector>
#include <string>
#include <list>
class Config {
public:
    int numNodeLeft = 20;   // number of node left in the last layer of coarsening
    int numInstances = 20;  // number of initial partition done in "initial partition" phase
    double dropRate = 0.3;    // drop partition this rate worse than the best partition in each uncoarsening layer
    int maxCoarsenIter = 30;   // maximum number of coarening layer (stop coarsening even if the number of node left is larger than "numNodeLeft")
    int maxRefineIter = 30;    // maximum number of coarening layer in refinement (stop coarsening even if the number of node left is larger than "numNodeLeft")
    string coarsenScheme = "MHEC";    // coarsening scheme when coarsening (HEC/MHEC)
    string coarsenScheme_r = "MHEC"; // coarsening scheme when doing refinement (HEC/MHEC)
    string initialParScheme = "FM";
    string uncoarsenScheme = "FM";
    string uncoarsenScheme_r = "FM";
};

class Hmetis
{
private:
    Hypergraph *graph;
    std::list<LayerInfo *> Instances;
    std::vector<double> spaceLimit;
    std::vector<std::vector<std::pair<int, std::vector<int> *>> *> coarsenInfo;

public:
    Hmetis() = default;

    // int uncoarCnt = 0;
    // multiset<pair<double, pair<int, int>>> sizeOrder;

    //build graph by the given box
    void configureGraph(parser& the_parser);

    // output partitioned graph to the given box
    void outputGraph();

    // hmetis coarsening
    // bool for if the coarsening is "restricted"
    void coarsen(int, bool, std::string, int);

    // hmetis initial partitioning
    // int for the number of Instances desired
    // void initialPartition(int, Hypergraph&);
    void initialPartition(int, std::string);

    // hmetis uncoarsening
    // double stands for 'dropRate'
    void uncoarsen(double, std::string);

    // new_partition uncoarsening
    void newUncoarsen();

    // hmetis refine
    void refine(int, double, std::string, std::string, int);

    // return the size of instances[]
    int getInstanceSize();

    // drop instances which are 'dropRate' worse than the best cut (0.1 stands for 10 percent)
    void dropWorstCut(double);

    void chooseBestInstance();

    // return sorted node idx in non-decreasing node size order
    std::vector<int> sortNode(Hypergraph *);

    // reduce total terminal size to fit the limit in initial partition
    void enableTerminalSizeLimit(Hypergraph *, std::vector<int> &);

    void preIPAdjustment();
};

struct partition_info
{
    Hypergraph *graph;
    LayerInfo *inst;
    std::vector<double> spaceLimit;
    std::string scheme;

    partition_info(Hypergraph *graph,
                   LayerInfo *inst,
                   std::vector<double> spaceLimit,
                   std::string scheme) : graph(graph), inst(inst), spaceLimit(spaceLimit), scheme(scheme) {}
};