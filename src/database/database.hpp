#pragma once
#include "random.hpp"
#include "type.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class cell_ref {
public:
  cell_ref(){};
  cell_ref(std::vector<std::string> &words) {
    index_name = words.at(0);
    if (words.at(0).at(0) == 'X') {
      cell_type = 1; // subckt
    } else if (words.at(0).at(0) == 'M') {
      cell_type = 0; // mosfet
    }
    // int num = words.size() - 1;
    int num = 0;
    for (int i = num; i < words.size(); i++) {
      if (i == words.size() - 1 and
          words.at(i).find('=') == std::string::npos) {
        num = words.size() - 1;
        break;
      }
      if (words.at(i).find('=') != std::string::npos) {
        num = i - 1;
        break;
      }
    }
    type = words.at(num);
    transform(type.begin(), type.end(), type.begin(), toupper);
    pin_array.insert(pin_array.end(), words.begin() + 1, words.begin() + num);
  };
  int cell_type = -1; // Ints or cell
  std::string index_name;
  std::string type;
  std::vector<std::string> pin_array;
};

// a cell has one or more instances
class cell {
public:
  explicit cell(std::vector<std::string> &words) {
    name = words.at(1);
    transform(name.begin(), name.end(), name.begin(), toupper);
    pin_array.insert(pin_array.end(), words.begin() + 2, words.end());
  };
  void add_cell_ref(std::vector<std::string> &words) {
    cell_refs.emplace_back(cell_ref(words));
  };
  std::vector<std::string> pin_array;
  std::string name;
  std::vector<cell_ref> cell_refs;
};

class Netlist {
public:
  void add_cell(std::vector<std::string> &words) {
    cells.emplace_back(cell(words));
    cell_map.emplace(cells.back().name, cells.size() - 1);
  };

  void update_top_cell() {
    for (int i = 0; i < cells.size(); i++) {
      if (cells.at(i).cell_refs.size() >= get_top_cell().cell_refs.size()) {
        top_cell_id = i;
      }
    }
  };

  cell &get_top_cell() {
    return (top_cell_id == -1) ? cells.at(0) : cells.at(top_cell_id);
  };
  cell &get_cell(std::string cell_name) {
    return cells.at(cell_map.at(cell_name));
  };
  std::map<std::string, int> cell_map;
  int top_cell_id = -1;
  std::vector<cell> cells;
};

class Database {
public:
  explicit Database(const std::string &filename1,
                    const std::string &filename2) {
    Netlist netlist1;
    Netlist netlist2;
    netlist1 = read(filename1);
    netlist2 = read(filename2);
    hierarchy_compare(netlist1, netlist2);
    std::cout << "----" << std::endl;
    // get_num(netlist1);
    // get_num(netlist2);
    transfer(netlist1, instances1, nets1);
    transfer(netlist2, instances2, nets2);
  };

  std::vector<instance> instances1;
  std::vector<instance> instances2;
  std::vector<net> nets1;
  std::vector<net> nets2;

  std::map<std::string, int> magic_table;
  // void flatten(Netlist &netlist1, Netlist &netlist2);
  void get_info(Netlist &netlist1, Netlist &netlist2);
  void get_num(Netlist &netlist);
  void out_map();
  void output_match_info();

private:
  void floating_pin(Netlist &netlist);
  void flatten_cell(Netlist &netlist, std::set<std::string> &difference);
  void hierarchy_compare(Netlist &netlist1, Netlist &netlist2);
  Netlist read(const std::string &filename);
  void transfer(Netlist &netlist, std::vector<instance> &instances,
                std::vector<net> &nets);
  void add_instance(std::vector<instance> &instances, cell_ref &the_cell);
  void add_net(std::vector<net> &nets, std::vector<instance> &instances,
               cell_ref &the_cell, std::map<std::string, int> &net_table);
};
