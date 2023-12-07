#include "database.hpp"
#include <cctype>
#include <string>

Netlist Database::read(const std::string &filename) {
  Netlist netlist;
  std::ifstream input_file(filename);
  if (!input_file) {
    std::cout << "Can't open file!" << std::endl;
    return netlist;
  }
  std::string line;
  int line_num = 0;
  while (getline(input_file, line)) {
    std::stringstream ss(line);
    std::string word;
    std::vector<std::string> words;
    while (std::getline(ss, word, ' ')) {
      words.emplace_back(word);
    }
    if (!words.empty()) {
      switch (words.at(0).at(0)) {
      case '.':
        if (words.at(0) == ".SUBCKT") {
          netlist.add_cell(words);
        } else if (words.at(0) == ".ENDS") {
        }
        break;
      case 'M':
        line_num++;
        netlist.cells.back().add_cell_ref(words);
        break;
      case 'X':
        line_num++;
        netlist.cells.back().add_cell_ref(words);
      case '*':
        break;
      default:
        break;
      }
    }
  }
  netlist.update_top_cell();
  // std::cout <<"the size:"<< netlist.cells.size()<< std::endl;
  return netlist;
}

void Database::add_instance(std::vector<instance> &instances,
                            cell_ref &the_cell) {
  instances.emplace_back();
  instances.back().name = the_cell.index_name;
  instances.back().type = the_cell.type;
  if (magic_table.count(the_cell.type) == 0) {
    instances.back().magic_num = magic();
    magic_table.emplace(the_cell.type, instances.back().magic_num);
  } else {
    instances.back().magic_num = magic_table.at(the_cell.type);
  }
}

void Database::add_net(std::vector<net> &nets, std::vector<instance> &instances,
                       cell_ref &the_cell,
                       std::map<std::string, int> &net_table) {

  for (int i = 0; i < the_cell.pin_array.size(); i++) {

    auto net_name = the_cell.pin_array.at(i);
    if (net_table.count(net_name) == 0) {

      nets.emplace_back();
      nets.back().name = net_name;
      net_table.emplace(net_name, nets.size() - 1);
      instances.back().net_ptr.emplace_back(nets.size() - 1);
      nets.back().inst_ptr.emplace_back(instances.size() - 1);
    } else {
      instances.back().net_ptr.emplace_back(net_table.at(net_name));
      nets.at(net_table.at(net_name))
          .inst_ptr.emplace_back(instances.size() - 1);
    }
  }
}

void Database::transfer(Netlist &netlist, std::vector<instance> &instances,
                        std::vector<net> &nets) {
  std::map<std::string, int> net_table;
  for (auto the_cell : netlist.get_top_cell().cell_refs) {
    if (the_cell.cell_type != -1) {
      add_instance(instances, the_cell);
      add_net(nets, instances, the_cell, net_table);
    }
  }
  for (auto &net : nets) {
    net.magic_num = net.inst_ptr.size();
  }
}

void Database::get_num(Netlist &netlist) {
  int ptr_number = 0;
  int inst_num = 0;
  std::set<std::string> names;
  for (auto the_cell : netlist.get_top_cell().cell_refs) {
    if (the_cell.cell_type != -1) {
      inst_num++;
      ptr_number = ptr_number + the_cell.pin_array.size();
      for (int i = 0; i < the_cell.pin_array.size(); i++) {
        names.emplace(the_cell.pin_array.at(i));
      }
    }
  }
  std::cout << inst_num << std::endl;
  std::cout << names.size() << std::endl;
  std::cout << ptr_number << std::endl;
}

void Database::get_info(Netlist &netlist1, Netlist &netlist2) {
  std::map<std::string, std::pair<int, int>> instances_info;
  for (const auto &the_cell : netlist1.get_top_cell().cell_refs) {
    if (the_cell.cell_type != -1) {
      if (instances_info.count(the_cell.type) == 0) {
        instances_info.emplace(the_cell.type, std::make_pair(1, 0));
      } else {
        instances_info.at(the_cell.type).first++;
      }
    }
  }

  for (const auto &the_cell : netlist2.get_top_cell().cell_refs) {
    if (the_cell.cell_type != -1) {
      if (instances_info.count(the_cell.type) == 0) {
        instances_info.emplace(the_cell.type, std::make_pair(0, 1));
      } else {
        instances_info.at(the_cell.type).second++;
      }
    }
  }
  std::set<std::string> names;
  for (const auto &info : instances_info) {
    if (info.second.first != info.second.second) {
      std::cout << info.first << " " << info.second.first << " "
                << info.second.second << std::endl;
      names.emplace(info.first);
    }
  }

  Netlist stdcell =
      read("/home/oem/Documents/LVS_comparer/test/gcd45/stdcells.cdl");
  for (const auto &the_cell_ref : netlist1.get_top_cell().cell_refs) {
    // std::cout << the_cell_ref.type << std::endl;
    // std::cout << stdcell.cell_map.at(the_cell_ref.type) << std::endl;
    if (the_cell_ref.type != "NMOS_VTL" and the_cell_ref.type != "PMOS_VTL") {
      // std::cout <<the_cell_ref.type << std::endl;
      auto &the_stdcell = stdcell.get_cell(the_cell_ref.type);
      if (the_stdcell.pin_array.size() != the_cell_ref.pin_array.size()) {
        std::cout << the_stdcell.name << " " << the_stdcell.pin_array.size()
                  << " " << the_cell_ref.type << " "
                  << the_cell_ref.pin_array.size() << " "
                  << the_cell_ref.index_name << std::endl;
        // std::cout << "error" << std::endl;
      }
    }
  }
}

// void Database::flatten(Netlist &netlist1, Netlist &netlist2) {
//   // flatten all instance.
//   Netlist stdcell =
//       read("/home/oem/Documents/LVS_comparer/test/gcd45/stdcells.cdl");
//   stdcell.update_top_cell();
//   std::map<std::string, std::pair<int, int>> instances_info;
//   for (const auto &the_cell : netlist1.get_top_cell().cell_refs) {
//     if (instances_info.count(the_cell.type) == 0) {
//       instances_info.emplace(the_cell.type, std::make_pair(1, 0));
//     } else {
//       instances_info.at(the_cell.type).first++;
//     }
//   }
//   for (const auto &the_cell : netlist2.get_top_cell().cell_refs) {
//     if (instances_info.count(the_cell.type) == 0) {
//       instances_info.emplace(the_cell.type, std::make_pair(0, 1));
//     } else {
//       instances_info.at(the_cell.type).second++;
//     }
//   }
//   std::set<std::string> names;
//   for (const auto &info : instances_info) {
//     if (info.second.first != info.second.second) {
//       std::cout << info.first << " " << info.second.first << " "
//                 << info.second.second << std::endl;
//       names.emplace(info.first);
//     }
//   }
//   std::vector<int> inst_nums1;
//   for (int i = 0; i < netlist1.get_top_cell().cell_refs.size(); i++) {
//     auto type = netlist1.get_top_cell().cell_refs.at(i).type;
//     int not_mos = netlist1.get_top_cell().cell_refs.at(i).cell_type;
//     if (not_mos and names.count(type) != 0) {
//       inst_nums1.emplace_back(i);
//     }
//   }
//   std::vector<int> inst_nums2;
//   for (int i = 0; i < netlist2.get_top_cell().cell_refs.size(); i++) {
//     auto type = netlist2.get_top_cell().cell_refs.at(i).type;
//     int not_mos = netlist2.get_top_cell().cell_refs.at(i).cell_type;
//     if (not_mos and names.count(type) != 0) {
//       inst_nums2.emplace_back(i);
//     }
//   }
//   // std::cout<<inst_nums1.size()<<" "<<inst_nums2.size()<<std::endl;
//   int net_num = 0;
//   for (const auto &num : inst_nums2) {
//     auto &the_cell_ref = netlist2.get_top_cell().cell_refs.at(num);
//     the_cell_ref.cell_type = -1;

//     auto &the_stdcell = stdcell.get_cell(the_cell_ref.type);

//     std::map<std::string, std::string> name_pair;
//     if (the_stdcell.pin_array.size() == the_cell_ref.pin_array.size()) {
//       for (int i = 0; i < the_cell_ref.pin_array.size(); i++) {
//         name_pair.emplace(the_stdcell.pin_array.at(i),
//                           the_cell_ref.pin_array.at(i));
//       }
//     } else {
//       std::cout << the_stdcell.name << " " << the_cell_ref.type << std::endl;
//       std::cout << "error" << std::endl;
//     }

//     for (int i = 0; i < the_stdcell.cell_refs.size(); i++) {
//       netlist2.get_top_cell().cell_refs.emplace_back();
//       netlist2.get_top_cell().cell_refs.back().cell_type =
//           the_stdcell.cell_refs.at(i).cell_type;
//       netlist2.get_top_cell().cell_refs.back().type =
//           the_stdcell.cell_refs.at(i).type;
//       netlist2.get_top_cell().cell_refs.back().index_name =
//           the_cell_ref.index_name + std::to_string(i);
//       for (int j = 0; j < the_stdcell.cell_refs.at(i).pin_array.size(); j++) {
//         auto &pin = the_stdcell.cell_refs.at(i).pin_array.at(j);
//         if (name_pair.count(pin) == 0) {
//           name_pair.emplace(pin, "X01_" + std::to_string(net_num));
//           net_num++;
//           netlist2.get_top_cell().cell_refs.back().pin_array.emplace_back(
//               "X01_" + std::to_string(net_num));
//         } else {
//           netlist2.get_top_cell().cell_refs.back().pin_array.emplace_back(
//               name_pair.at(pin));
//         }
//       }
//     }
//   }
//   std::cout << "net_num:" << net_num << std::endl;
// }

void Database::floating_pin(Netlist &netlist) {
  // std::cout << "------------------" << std::endl;
  // int net_num1 = 0;
  // for (const auto &net : nets1) {
  //   if (net.inst_ptr.size() == 1) {
  //     net_num1++;
  //     // std::cout << net.name << instances1.at(net.inst_ptr.front()).name
  //     //           << std::endl;
  //    // std::cout << net.name << std::endl;
  //   }
  // }
  // std::cout << "------------------" << std::endl;
  std::set<std::string> pin_set;
  for (const auto &pin : netlist.get_top_cell().pin_array) {
    pin_set.emplace(pin);
  }

  int net_num2 = 0;
  for (const auto &net : nets2) {
    if (net.inst_ptr.size() == 1) {
      if (pin_set.count(net.name) == 0) {
        net_num2++;
        std::cout << net.name << std::endl;
      }
    }
  }
  std::cout << net_num2 << std::endl;
}

void Database::output_match_info() {
  std::set<int> insts1;
  std::set<int> insts2;
  int seed = 1000;
  insts1.emplace(seed);
  insts2.emplace(seed);
  int num = 1000;
  while (insts1.size() < num) {
    std::set<int> new_insts;
    for (auto inst = insts1.begin(); inst != insts1.end(); ++inst) {
      int inst_id = *inst;
      for (int net : instances1[inst_id].net_ptr) {
        for (int the_inst : nets1[net].inst_ptr) {
          if (insts1.count(the_inst) == 0) {
            new_insts.emplace(the_inst);
            insts2.emplace(the_inst);
          }
        }
      }
    }
    for (int new_inst : new_insts) {
      if (insts1.size() <= num) {
        insts1.emplace(new_inst);
      }
    }
    for (auto inst = insts1.begin(); inst != insts1.end();) {
      if (insts2.count(*inst) == 0) {
        inst = insts1.erase(inst);
      } else {
        ++inst;
      }
    }
  }
  std::cout << insts1.size() << std::endl;
  for (auto &inst : insts1) {
    std::cout << instances1.at(inst).name << " ";
    for (auto &net : instances1.at(inst).net_ptr) {
      std::cout << net << " ";
    }
    std::cout << instances1.at(inst).type << std::endl;
  }
  std::cout << "-------------" << std::endl;
  for (auto &inst : insts1) {
    auto the_inst = instances1.at(inst).lock;
    std::cout << instances2.at(the_inst).name << " ";
    for (auto &net : instances2.at(the_inst).net_ptr) {
      std::cout << net << " ";
    }
    std::cout << instances2.at(the_inst).type << std::endl;
  }
  // std::cout << "-------------" << std::endl;
  // for (auto &inst : insts1) {
  //   auto the_inst = instances1.at(inst).lock;
  //   std::cout << instances1.at(inst).name << " " << instances1.at(inst).type
  //             << " ~ " << instances2.at(the_inst).name << " "
  //             << instances2.at(the_inst).type << std::endl;
  // }
}

// hierarchy comparer
// compare_list and flatten
void Database::hierarchy_compare(Netlist &netlist1, Netlist &netlist2) {
  std::set<std::string> set1;
  std::set<std::string> set2;
  std::set<std::string> difference1;
  std::set<std::string> difference2;
  std::vector<std::string> compare_list;
  if (netlist1.cells.size() == 1 and netlist2.cells.size() == 1) {
    return;
  }
  for (const auto &the_cell : netlist1.cells) {
    set1.emplace(the_cell.name);
  }
  for (const auto &the_cell : netlist2.cells) {
    set2.emplace(the_cell.name);
  }
  for (const auto cell : set1) {
    if (set2.find(cell) == set2.end()) {
      difference1.emplace(cell);
    }else {
      compare_list.emplace_back(cell);
      //std::cout<<compare_list.size()<<std::endl;
    }
  }
  for (const auto cell : set2) {
    if (set1.find(cell) == set1.end()) {
      difference2.emplace(cell);
    }
  }
  flatten_cell(netlist1,difference1);
  flatten_cell(netlist2, difference2);
}

void Database::flatten_cell(Netlist & netlist,std::set<std::string>& difference){
  std::map<std::string, std::pair<int, int>> instances_info;
  std::vector<int> inst_nums1;
  for (int i = 0; i < netlist.get_top_cell().cell_refs.size(); i++) {
    auto type = netlist.get_top_cell().cell_refs.at(i).type;
    if (difference.count(type) != 0) {
      inst_nums1.emplace_back(i);
    }
  }
  int net_num = 0;
  for (const auto &num : inst_nums1) {
    auto &the_cell_ref = netlist.get_top_cell().cell_refs.at(num);
    the_cell_ref.cell_type = -1;

    auto &the_cell = netlist.get_cell(the_cell_ref.type);

    std::map<std::string, std::string> name_pair;
    if (the_cell.pin_array.size() == the_cell_ref.pin_array.size()) {
      for (int i = 0; i < the_cell_ref.pin_array.size(); i++) {
        name_pair.emplace(the_cell.pin_array.at(i),
                          the_cell_ref.pin_array.at(i));
      }
    } else {
      std::cout << the_cell.name << " " << the_cell_ref.type << std::endl;
      std::cout << "error" << std::endl;
    }

    for (int i = 0; i < the_cell.cell_refs.size(); i++) {
      netlist.get_top_cell().cell_refs.emplace_back();
      netlist.get_top_cell().cell_refs.back().cell_type =
          the_cell.cell_refs.at(i).cell_type;
      netlist.get_top_cell().cell_refs.back().type =
          the_cell.cell_refs.at(i).type;
      netlist.get_top_cell().cell_refs.back().index_name =
          the_cell_ref.index_name + std::to_string(i);
      for (int j = 0; j < the_cell.cell_refs.at(i).pin_array.size(); j++) {
        auto &pin = the_cell.cell_refs.at(i).pin_array.at(j);
        if (name_pair.count(pin) == 0) {
          name_pair.emplace(pin, "X01_" + std::to_string(net_num));
          net_num++;
          netlist.get_top_cell().cell_refs.back().pin_array.emplace_back(
              "X01_" + std::to_string(net_num));
        } else {
          netlist.get_top_cell().cell_refs.back().pin_array.emplace_back(
              name_pair.at(pin));
        }
      }
    }
  }
  std::cout << "net_num:" << net_num << std::endl;
}