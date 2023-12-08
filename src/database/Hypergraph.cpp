#include "Hypergraph.hpp"

#include <algorithm>
#include <float.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <time.h>
#include <unordered_set>
#include <utility>
#include <vector>

//////////////////////////////////////////////////////////////////////
//////////////////////// HyperNode////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// add an edge to the hypernode
void HyperNode::addEdge(int edge) { edges.push_back(edge); }

// return all edges of the hypernode
std::vector<int> &HyperNode::getEdges() { return edges; }

// return all neighbors of the hypernode
std::vector<int> &HyperNode::getNeighbors() { return neighbors; }

// return node size on layer 0
double HyperNode::getNodeSize0() { return size0; }

// return node size on layer 1
double HyperNode::getNodeSize1() { return size1; }

// set node size on layer 0
void HyperNode::setNodeSize0(double n) { size0 = n; }

// set node size on layer 1
void HyperNode::setNodeSize1(double n) { size1 = n; }

/////////////////////////////////////////////////////////////////////
///////////////////////// HyperEdge//////////////////////////////////
/////////////////////////////////////////////////////////////////////

// get all nodes in the hyperedge
std::vector<int> &HyperEdge::getNodes() { return nodes; }

double HyperEdge::getEdgeWeight() { return weight; }

//////////////////////////////////////////////////////////////////////
//////////////////////// Hypergraph///////////////////////////////////
//////////////////////////////////////////////////////////////////////

// add a new edge and the given node list
// which are connected to the new edge
void Hypergraph::addNodeList(int edge_id, std::vector<int> &vtxs) {
  for (int i = 0; i < vtxs.size(); i++) {
    nodes[vtxs[i]]->addEdge(edge_id);
  }

  HyperEdge *newEdge =
      new HyperEdge(vtxs, 1.0 / (static_cast<double>(vtxs.size()) - 1.0));
  edges.emplace_back(newEdge);
}

// add a hypernode to the hypergraph and increase partition size
void Hypergraph::addNode(HyperNode *n) { nodes.emplace_back(n); }

// return all hyperedges in the current graph
std::vector<HyperEdge *> &Hypergraph::getAllEdges() { return edges; }

// return all hypernodes in the current graph
std::vector<HyperNode *> &Hypergraph::getAllNodes() { return nodes; }

// return connected edges of node n
std::vector<int> &Hypergraph::getEdgesOf(int n) { return nodes[n]->getEdges(); }

// return connected nodes of edge e
std::vector<int> &Hypergraph::getNodesOf(int e) { return edges[e]->getNodes(); }

// return node number
int Hypergraph::getNodeNum() { return nodeNum; }

// return edge number
int Hypergraph::getEdgeNum() { return edges.size(); }

double Hypergraph::getNodeSizeOf(int n, int layer) {
  if (layer == 0)
    return nodes[n]->getNodeSize0();
  else
    return nodes[n]->getNodeSize1();
}

// return edge weight of the edge
double Hypergraph::getEdgeWeightOf(int e) { return edges[e]->getEdgeWeight(); }

// return neighbors of the given node index
std::vector<int> &Hypergraph::getNeighborsOf(int n) {
  return nodes[n]->getNeighbors();
}

// return terminal size
double Hypergraph::getTerminalSize() { return terminalSize; }

std::pair<int, std::vector<int> *>
Hypergraph::coarseNodes(std::vector<int> *nds) {
  int minIdx = *std::min_element(nds->begin(), nds->end());
  double size0 = 0, size1 = 0;

  for (auto it = nds->begin(); it != nds->end(); it++) {
    size0 += getNodeSizeOf(*it, 0);
    size1 += getNodeSizeOf(*it, 1);

    if (*it == minIdx)
      continue;
    setNodeExistOf(*it, false);

    std::vector<int> &edges_ = getEdgesOf(*it);
    for (auto itt = edges_.begin(); itt != edges_.end(); itt++) {
      std::vector<int> &nodes_ = getNodesOf(*itt);
      int c = getNodeNumOf(*itt);

      auto end = nodes_.begin() + c;
      auto n_idx = std::find(nodes_.begin(), end, *it);
      std::iter_swap(n_idx, end - 1);

      if (std::find(nodes_.begin(), end, minIdx) != end)
        setNodeNumOf(*itt, c - 1); // delete
      else {
        // relink
        nodes_[c - 1] = minIdx;
        nodes[minIdx]->addEdge(*itt);
      }
    }
  }

  setNodeSizeOf(minIdx, size0, 0);
  setNodeSizeOf(minIdx, size1, 1);

  return std::make_pair(minIdx, nds);
}

int Hypergraph::getNodeNumOf(int e_idx) { return edges[e_idx]->getNodeNum(); }

int HyperEdge::getNodeNum() { return count; }

bool Hypergraph::getEdgeExistOf(int idx) { return edgeExist[idx]; }

bool Hypergraph::getNodeExistOf(int idx) { return nodeExist[idx]; }

void HyperEdge::setNodeNum(int num) { count = num; }

void Hypergraph::setEdgeExistOf(int e_id, bool flag) { edgeExist[e_id] = flag; }

void Hypergraph::setNodeExistOf(int n_id, bool flag) { nodeExist[n_id] = flag; }

void Hypergraph::setNodeNumOf(int e_idx, int num) {
  edges[e_idx]->setNodeNum(num);
}

void Hypergraph::setNodeNum(int num) { nodeNum = num; }

// set node size on layer 0
void Hypergraph::setNodeSizeOf(int idx, double size, int layer) {
  if (layer == 0)
    nodes[idx]->setNodeSize0(size);
  else
    nodes[idx]->setNodeSize1(size);
}

// set terminal size
void Hypergraph::setTerminalSize(double size) { terminalSize = size; }

// build and merge neighbors of all nodes after coarsening
void Hypergraph::buildNeighbors() {
  for (int i = 0; i < nodes.size(); i++) {
    if (!getNodeExistOf(i))
      continue;

    std::vector<int> &edges_ = getEdgesOf(i);
    std::vector<int> &neigs = getNeighborsOf(i);
    neigs.clear();

    for (int j = 0; j < edges_.size(); j++) {
      std::vector<int> &nds = getNodesOf(edges_[j]);
      int count = getNodeNumOf(edges_[j]);
      neigs.insert(neigs.end(), nds.begin(), nds.begin() + count);
    }

    sort(neigs.begin(), neigs.end());
    neigs.erase(std::unique(neigs.begin(), neigs.end()), neigs.end());
  }
}

void Hypergraph::revertGraph(
    std::vector<std::pair<int, std::vector<int> *>> *coarsenInfo) {
  for (auto it = coarsenInfo->rbegin(); it != coarsenInfo->rend(); it++) {
    int base = it->first;
    double size0 = getNodeSizeOf(base, 0), size1 = getNodeSizeOf(base, 1);
    std::vector<bool> edgeConnected(edges.size(), false);
    std::vector<int> &eds = getEdgesOf(base);
    std::vector<int> *nds = it->second;

    for (auto itt = eds.begin(); itt != eds.end(); itt++)
      edgeConnected[*itt] = true;

    for (auto itt = nds->rbegin(); itt != nds->rend(); itt++) {
      if (*itt == base)
        continue;
      nodeExist[*itt] = true;

      std::vector<int> &eds_ = getEdgesOf(*itt);
      for (auto itt_ = eds_.rbegin(); itt_ != eds_.rend(); itt_++) {
        if (edgeConnected[*itt_]) {
          std::vector<int> &nds_ = getNodesOf(*itt_);
          int count = getNodeNumOf(*itt_);

          if (count < nds_.size() && nds_[count] == *itt) {
            setNodeNumOf(*itt_, count + 1);
          } else {
            auto idx = std::find(nds_.begin(), nds_.begin() + count, base);
            *idx = *itt;
            eds.pop_back();
          }

          if (!edgeExist[*itt_])
            edgeExist[*itt_] = true;
        }
      }

      size0 -= getNodeSizeOf(*itt, 0);
      size1 -= getNodeSizeOf(*itt, 1);
    }

    setNodeSizeOf(base, size0, 0);
    setNodeSizeOf(base, size1, 1);
    nodeNum = nodeNum + nds->size() - 1;
  }
}

// check if the size of the given node vector is under space limit
bool Hypergraph::sizeUnderLimit(std::vector<int> &nodesConnected, int count,
                                int part, double totalSize) {
  for (auto it = nodesConnected.begin(); it != nodesConnected.begin() + count;
       it++) {
    totalSize -= getNodeSizeOf(*it, part);
    if (totalSize < 0)
      return false;
  }

  return true;
}

// display current graph in hmetis format
void Hypergraph::displayGraph() {
  std::vector<HyperEdge *> &edges = getAllEdges();

  for (int i = 0; i < edges.size(); i++) {
    std::vector<int> &vtxs = edges[i]->getNodes();
    for (int j = 0; j < vtxs.size(); j++) {
      std::cout << vtxs[j] << ", ";
    }
    std::cout << std::endl;
  }
}

