#pragma once

#include "../database/Hypergraph.hpp"
#include "../database/result.hpp"

#include <algorithm>
#include <cstdlib>
#include <list>
#include <math.h>
#include <vector>
using namespace std;

inline int computeGain(Hypergraph *graph, result *inst, int nodeIdx) {
  std::vector<int> &edges = graph->getEdgesOf(nodeIdx);
  int part = inst->getPartitionOf(nodeIdx), gain = 0;

  for (auto it = edges.begin(); it != edges.end(); it++) {
    std::vector<int> &nds = graph->getNodesOf(*it);
    bool inSamePart = false, inAnotherPart = false;
    for (auto itt = nds.begin(); itt != nds.end(); itt++) {
      if (inst->getPartitionOf(*itt) == part && *itt != nodeIdx)
        inSamePart = true;
      else
        inAnotherPart = true;

      if (inSamePart && inAnotherPart)
        break;
    }

    if (inSamePart && !inAnotherPart)
      gain--;
    else if (!inSamePart && inAnotherPart)
      gain++;
  }

  return gain;
}

inline void enableTerminalSize(Hypergraph *graph, result *inst,
                               std::vector<double> &spaceLimit) {
  // reduce total terminal size
  double totalTerminalSize = inst->getTotalTerminalSize(),
         terminalSize = graph->getTerminalSize();
  double size0 = inst->getp0size(), size1 = inst->getp1size();

  while (totalTerminalSize > spaceLimit[0]) {
    for (int i = 0; i < graph->getNodeNum(); i++) {
      double nodeSize0 = graph->getNodeSizeOf(i, 0),
             nodeSize1 = graph->getNodeSizeOf(i, 1);
      if (size1 + nodeSize1 > spaceLimit[1] ||
          size0 + nodeSize0 > spaceLimit[0])
        continue;

      int gain = computeGain(graph, inst, i);
      if (gain <= 0)
        continue;

      int part = inst->getPartitionOf(i), partTo = (part == 0) ? 1 : 0;
      if (part == 0) {
        size0 -= nodeSize0;
        size1 += nodeSize1;
      } else {
        size0 += nodeSize0;
        size1 -= nodeSize1;
      }

      inst->setPartitionOf(i, partTo);
      totalTerminalSize += ((-1) * gain * terminalSize);
      if (totalTerminalSize <= spaceLimit[0])
        break;
    }
  }

  inst->setp0size(size0);
  inst->setp1size(size1);
  inst->setTotalTerminalSize(totalTerminalSize);
}

inline int myrandom(int i) { return std::rand() % i; }

inline void HER(Hypergraph *graph, result *inst, vector<double> &spaceLimit) {
  double terminalSize = graph->getTerminalSize(),
         totalTerminalSize = inst->getTotalTerminalSize();
  double p1size = inst->getp0size(), p2size = inst->getp1size();
  vector<int> he;
  for (int i = 0; i < graph->getAllEdges().size(); i++)
    if (graph->getEdgeExistOf(i) && graph->getNodeNumOf(i) != 1)
      he.push_back(i);

  random_shuffle(he.begin(), he.end(), myrandom);
  // std::random_device rd;
  // std::mt19937 g(rd());
  // std::shuffle(he.begin(), he.end(), g);

  for (auto it = he.begin(); it != he.end(); it++) {
    vector<int> p1v, p2v;
    vector<int> &nodes = graph->getNodesOf(*it);
    vector<int> edgesToUpdate;
    vector<bool> nodesRelated(graph->getAllNodes().size(), false),
        edgesRelated(graph->getAllEdges().size(), false);
    double size1in1 = 0, size1in2 = 0, size2in1 = 0, size2in2 = 0;
    int gain1to2 = 0, gain2to1 = 0;
    int count = graph->getNodeNumOf(*it);

    for (auto itt = nodes.begin(); itt != nodes.begin() + count; itt++) {
      if (inst->getPartitionOf(*itt) == 0) {
        p1v.push_back(*itt);
        size1in1 += graph->getNodeSizeOf(*itt, 0);
        size1in2 += graph->getNodeSizeOf(*itt, 1);
      } else {
        p2v.push_back(*itt);
        size2in1 += graph->getNodeSizeOf(*itt, 0);
        size2in2 += graph->getNodeSizeOf(*itt, 1);
      }

      vector<int> &edges_ = graph->getEdgesOf(*itt);
      for (auto itt_ = edges_.begin(); itt_ != edges_.end(); itt_++) {
        if (!edgesRelated[*itt_] && graph->getNodeNumOf(*itt_) != 1) {
          edgesRelated[*itt_] = true;

          edgesToUpdate.push_back(*itt_);
        }
      }
      nodesRelated[*itt] = true;
    }

    if (p1v.empty() || p2v.empty())
      continue;

    // calculate terminal size change and gain
    for (auto itt = edgesToUpdate.begin(); itt != edgesToUpdate.end(); itt++) {
      vector<int> &nds = graph->getNodesOf(*itt);
      int c = graph->getNodeNumOf(*itt), part = 2;
      bool inBothLayer = false, inPart1 = false, inPart2 = false;

      for (auto itt_ = nds.begin(); itt_ != nds.begin() + c; itt_++) {
        if (!nodesRelated[*itt_]) {
          if (part == 2)
            part = inst->getPartitionOf(*itt_);
          else if (inst->getPartitionOf(*itt_) !=
                   part) { // check if the remaining nodes are in both layers
            inBothLayer = true; // 'remaining nodes': nodes in this edge(*itt)
                                // but not in the shifted edge(*it)
            break;
          }
        } else {
          if (inst->getPartitionOf(*itt_) == 0)
            inPart1 = true; // check if the nodes also in the shifted edge are
                            // in both layers
          else
            inPart2 = true;
        }
      }

      if (inBothLayer)
        continue;
      if (part == 0) { // all remaining nodes are in part 0
        if (inPart2)
          gain2to1++;
        else
          gain1to2--;
      } else if (part == 1) { // all remaining nodes are in part 1
        if (inPart1)
          gain1to2++;
        else
          gain2to1--;
      } else { // the whole edge is inside the shifted edge
        if (inPart1 && inPart2) {
          gain2to1++;
          gain1to2++;
        }
      }
    }

    // check if the shift fit the spacing limitations
    double terminalSizeTo1 = gain2to1 * (-1) * terminalSize,
           terminalSizeTo2 = gain1to2 * (-1) * terminalSize;
    if (gain1to2 > gain2to1 && gain1to2 > 0 &&
        abs(0.5 - (p1size - size1in1) / (p1size + p2size)) <= 0.25 &&
        abs(0.5 - (p2size + size1in2) / (p1size + p2size)) <= 0.25 &&
        (p2size + size1in2) <= spaceLimit[1] &&
        (totalTerminalSize + terminalSizeTo2) <= spaceLimit[0]) {
      for (auto itt = p1v.begin(); itt != p1v.end(); itt++) {
        inst->setPartitionOf((*itt), 1);
      }
      p1size -= size1in1;
      p2size += size1in2;
      totalTerminalSize += terminalSizeTo2;
    } else if (gain1to2 < gain2to1 && gain2to1 > 0 &&
               abs(0.5 - (p1size + size2in1) / (p1size + p2size)) <= 0.25 &&
               abs(0.5 - (p2size - size2in2) / (p1size + p2size)) <= 0.25 &&
               (p1size + size2in1) <= spaceLimit[0] &&
               (totalTerminalSize + terminalSizeTo1) <= spaceLimit[0]) {
      for (auto itt = p2v.begin(); itt != p2v.end(); itt++) {
        inst->setPartitionOf((*itt), 0);
      }
      p1size += size2in1;
      p2size -= size2in2;
      totalTerminalSize += terminalSizeTo1;
    }
  }

  inst->setp0size(p1size);
  inst->setp1size(p2size);
  inst->computePartitionScore(graph);
  inst->setTotalTerminalSize(totalTerminalSize);
}

class FM {
public:
  Hypergraph *graph;
  result *inst;
  vector<vector<int>> pv;
  vector<pair<int, list<int>::iterator>> nodePointers;
  vector<list<int>> gainBucket;
  vector<double> &spaceLimit;
  vector<double> partSize;
  vector<int> revertBuf;
  vector<bool> locked;
  int maxGain, tmpMaxGain;
  int noImprove, k;
  int debt;
  int pass;

  void getEdgePartitionStatus();
  void buildGainBucketDS();
  void updateGain(int, int);
  void updateDataStructure(int);
  void revertMoves();
  void reset();
  int getMaxGain();
  int selectVictimNode();

  void Partition();

  FM(Hypergraph *graph, result *inst, vector<double> &spaceLimit)
      : graph(graph), inst(inst), maxGain(getMaxGain()), tmpMaxGain(maxGain),
        pv(vector<vector<int>>(2, vector<int>(graph->getAllEdges().size(), 0))),
        nodePointers(vector<pair<int, list<int>::iterator>>(
            graph->getAllNodes().size())),
        spaceLimit(spaceLimit),
        partSize({inst->getp0size(), inst->getp1size()}), revertBuf({}),
        locked(vector<bool>(graph->getAllNodes().size(), false)), noImprove(0),
        k(graph->getNodeNum() * 0.01), debt(0), pass(2) {
    gainBucket = vector<list<int>>((2 * maxGain + 1), list<int>());
  };
};

inline void biPart(Hypergraph *graph, result *inst, vector<double> &spaceLimit,
                   string &scheme) {
  if (scheme == "FM") {
    FM Partitioner(graph, inst, spaceLimit);
    Partitioner.Partition();
  } else
    HER(graph, inst, spaceLimit);
}