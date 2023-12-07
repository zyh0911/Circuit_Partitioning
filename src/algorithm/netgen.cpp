
#include "netgen.hpp"
#include "check.hpp"

#include <algorithm>
#include <ctime>
#include <iostream>
#include <random>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

void lock_inst(Database &db) {
  std::unordered_map<int, std::pair<std::vector<int>, std::vector<int>>>
      indexMap;

  for (int i = 0; i < db.instances1.size(); ++i) {
    indexMap[db.instances1.at(i).magic_num].first.emplace_back(i);
  }

  for (int i = 0; i < db.instances2.size(); ++i) {
    indexMap[db.instances2.at(i).magic_num].second.emplace_back(i);
  }

  for (const auto &element : indexMap) {
    auto pair = element.second;
    if (pair.first.size() == 1 and pair.second.size() == 1) {
      auto &inst1 = db.instances1.at(pair.first.front());
      auto &inst2 = db.instances2.at(pair.second.front());
      if (inst1.type == inst2.type) {
        std::set<int> types;
        for (const auto &net : inst1.net_ptr) {
          types.insert(db.nets1.at(net).magic_num);
        }
        bool is_lock = false;
        for (const auto &net : inst2.net_ptr) {
          if (db.nets2.at(net).lock != -1 and
              types.count(db.nets2.at(net).magic_num) != 0) {
            is_lock = true;
          }
        }
        if (is_lock) {
          inst1.lock = pair.second.front();
          inst2.lock = pair.first.front();
        }
      }
    }
  }
}

void lock_net(Database &db) {
  std::unordered_map<int, std::pair<std::vector<int>, std::vector<int>>>
      indexMap;

  for (int i = 0; i < db.nets1.size(); ++i) {
    indexMap[db.nets1.at(i).magic_num].first.emplace_back(i);
  }

  for (int i = 0; i < db.nets2.size(); ++i) {
    indexMap[db.nets2.at(i).magic_num].second.emplace_back(i);
  }

  for (const auto &element : indexMap) {
    auto pair = element.second;
    if (pair.first.size() == 1 and pair.second.size() == 1) {
      std::set<int> types;
      for (const auto &inst : db.nets1.at(pair.first.front()).inst_ptr) {
        types.insert(db.instances1.at(inst).magic_num);
      }
      bool is_lock = false;
      for (const auto &inst : db.nets2.at(pair.second.front()).inst_ptr) {
        if (db.instances2.at(inst).lock != -1 and
            types.count(db.instances2.at(inst).magic_num) != 0) {
          is_lock = true;
        }
      }
      if (is_lock) {
        db.nets1.at(pair.first.front()).lock = pair.second.front();
        db.nets2.at(pair.second.front()).lock = pair.first.front();
      }
    }
  }
}

void update_net1(Database &db) {
  for (auto &net : db.nets1) {
    for (auto &ptr : net.inst_ptr) {
      if (net.lock == -1) {
        net.magic_num ^= db.instances1[ptr].magic_num;
      }
    }
  }
}

void update_net2(Database &db) {
  for (auto &net : db.nets2) {
    for (auto &ptr : net.inst_ptr) {
      if (net.lock == -1) {
        net.magic_num ^= db.instances2[ptr].magic_num;
      }
    }
  }
}

void update_inst1(Database &db) {
  for (auto &instance : db.instances1) {
    for (int i = 0; i < instance.net_ptr.size(); i++) {
      if (instance.lock == -1) {
        auto ptr = instance.net_ptr[i];
        instance.magic_num ^= db.nets1[ptr].magic_num;
      }
    }
  }
}

void update_inst2(Database &db) {
  for (auto &instance : db.instances2) {
    for (int i = 0; i < instance.net_ptr.size(); i++) {
      if (instance.lock == -1) {
        auto ptr = instance.net_ptr[i];
        instance.magic_num ^= db.nets2[ptr].magic_num;
      }
    }
  }
}

void diffusion(Database &db) {
  int nums = 25;
  int max_equal_num = 0;
  int best_num = nums;
  while (nums != 0) {
    update_net1(db);
    update_net2(db);
    lock_net(db);
    update_inst1(db);
    update_inst2(db);
    lock_inst(db);
    nums--;
  }
}

void isomorphism(Database &db) {
  std::unordered_map<int, std::pair<std::vector<int>, std::vector<int>>>
      indexMap;
  for (int i = 0; i < db.instances1.size(); ++i) {
    if (db.instances1.at(i).lock == -1) {
      indexMap[db.instances1.at(i).magic_num].first.emplace_back(i);
    }
  }
  for (int i = 0; i < db.instances2.size(); ++i) {
    if (db.instances2.at(i).lock == -1) {
      indexMap[db.instances2.at(i).magic_num].second.emplace_back(i);
    }
  }
  for (const auto &element : indexMap) {
    auto pair = element.second;
    if (pair.first.size() == pair.second.size()) {
      for (int i = 0; i < pair.first.size(); i++) {
        int new_magic_num = -magic();
        db.instances1.at(pair.first.at(i)).magic_num = new_magic_num;
        db.instances2.at(pair.second.at(i)).magic_num = new_magic_num;
        db.instances1.at(pair.first.at(i)).lock = pair.second.at(i);
        db.instances2.at(pair.second.at(i)).lock = pair.first.at(i);
      }
    }
  }
}

void update_net1_new(Database &db) {
  for (int j = 0; j < db.nets1.size(); j++) {
    auto &net = db.nets1.at(j);
    for (auto &ptr : net.inst_ptr) {
      if (net.lock == -1 and db.instances1[ptr].lock != -1) {
        net.magic_num ^= db.instances1[ptr].magic_num;
      }
    }
  }
}

void update_net2_new(Database &db) {
  for (int j = 0; j < db.nets2.size(); j++) {
    auto &net = db.nets2.at(j);
    for (auto &ptr : net.inst_ptr) {
      if (net.lock == -1 and db.instances2[ptr].lock != -1) {
        net.magic_num ^= db.instances2[ptr].magic_num;
      }
    }
  }
}

void update_inst1_new(Database &db) {
  for (auto &instance : db.instances1) {
    for (int i = 0; i < instance.net_ptr.size(); i++) {
      if (instance.lock == -1 and db.nets1[i].lock != -1) {
        auto ptr = instance.net_ptr[i];
        instance.magic_num ^= db.nets1[ptr].magic_num;
      }
    }
  }
}

void update_inst2_new(Database &db) {
  for (auto &instance : db.instances2) {
    for (int i = 0; i < instance.net_ptr.size(); i++) {
      if (instance.lock == -1 and db.nets2[i].lock != -1) {
        auto ptr = instance.net_ptr[i];
        instance.magic_num ^= db.nets2[ptr].magic_num;
      }
    }
  }
}

void update(Database &db) {
  for (int i = 0; i < db.instances1.size(); i++) {
    if (db.instances1.at(i).lock == -1) {
      db.instances1.at(i).magic_num =
          db.magic_table.at(db.instances1.at(i).type);
    }
  }
  for (int i = 0; i < db.instances2.size(); i++) {
    if (db.instances2.at(i).lock == -1) {
      db.instances2.at(i).magic_num =
          db.magic_table.at(db.instances2.at(i).type);
    }
  }

  for (int i = 0; i < db.nets1.size(); i++) {
    if (db.nets1.at(i).lock == -1) {
      // db.nets1.at(i).magic_num = db.nets1.at(i).inst_ptr.size();
      db.nets1.at(i).magic_num = 0;
    }
  }

  for (int i = 0; i < db.nets2.size(); i++) {
    if (db.nets2.at(i).lock == -1) {
      // db.nets2.at(i).magic_num = db.nets2.at(i).inst_ptr.size();
      db.nets2.at(i).magic_num = 0;
    }
  }
}

void contracte(Database &db) {
  int nums = 100;
  int max_equal_num = 0;
  int best_num = nums;
  while (nums != 0) {
    update(db);
    update_net1_new(db);
    update_net2_new(db);
    lock_net(db);
    update_inst1_new(db);
    update_inst2_new(db);
    lock_inst(db);
    nums--;
  }
}

bool compare_instance(Database &db, int inst1, int inst2) {
  if (db.instances1.at(inst1).type != db.instances2.at(inst2).type) {
    return false;
  }
  std::set<int> match_ptr1;
  std::set<int> match_ptr2;
  std::set<int> match_net_ptr1;
  std::set<int> match_net_ptr2;

  for (const auto &net : db.instances1.at(inst1).net_ptr) {

    if (db.nets1.at(net).name != "VDD" and db.nets1.at(net).name != "VSS") {
      for (const auto &instance : db.nets1.at(net).inst_ptr) {
        match_ptr1.emplace(db.instances1.at(instance).magic_num);
      }
      if (db.nets1.at(net).lock == -1) {
        continue;
      }
      // match_net_ptr1.emplace(db.nets1.at(net).magic_num);
    }
  }
  for (const auto &net : db.instances2.at(inst2).net_ptr) {

    if (db.nets2.at(net).name != "VDD" and db.nets2.at(net).name != "VSS") {
      for (const auto &instance : db.nets2.at(net).inst_ptr) {
        match_ptr2.emplace(db.instances2.at(instance).magic_num);
      }
      if (db.nets2.at(net).lock == -1) {
        continue;
      }
      // match_net_ptr2.emplace(db.nets2.at(net).magic_num);
    }
  }
  int commonCount_net = 0;
  for (const int num : match_net_ptr2) {
    if (match_net_ptr1.find(num) != match_net_ptr1.end()) {
      ++commonCount_net;
    }
  }

  int commonCount_inst = 0;
  for (const int num : match_ptr2) {
    if (match_ptr1.find(num) != match_ptr1.end()) {
      ++commonCount_inst;
    }
  }
  // std::cout<<match_ptr1.size()+match_ptr2.size()<<std::endl;
  bool result =
      commonCount_net > 0 or commonCount_inst * 2 >= match_ptr2.size();
  return result;
}

void match_inst(Database &database, std::set<int> &unmatch_instance1,
                std::set<int> &unmatch_instance2) {
  for (auto it1 = unmatch_instance1.begin(); it1 != unmatch_instance1.end();) {
    for (auto it2 = unmatch_instance2.begin();
         it2 != unmatch_instance2.end();) {
      if (compare_instance(database, *it1, *it2)) {
        database.instances1[*it1].lock = *it2;
        database.instances2[*it2].lock = *it1;
        int magic_num = -magic();
        database.instances2[*it2].magic_num = magic_num;
        database.instances1[*it1].magic_num = magic_num;
        it1 = unmatch_instance1.erase(it1);
        it2 = unmatch_instance2.erase(it2);
      } else {
        ++it2;
      }
    }
    ++it1;
  }
}


void refine(Database &database) {
  std::set<int> unmatch_instance1;
  std::set<int> unmatch_instance2;

  for (int i = 0; i < database.instances1.size(); i++) {
    if (database.instances1.at(i).lock == -1) {
      unmatch_instance1.emplace(i);
      // std::cout<<database.instances1.at(i).name<<std::endl;
    }
  }
  for (int i = 0; i < database.instances2.size(); i++) {
    if (database.instances2.at(i).lock == -1) {
      unmatch_instance2.emplace(i);
      // std::cout<<database.instances2.at(i).name<<std::endl;
    }
  }

  std::set<int> unmatch_nets1;
  std::set<int> unmatch_nets2;

  for (int i = 0; i < database.nets1.size(); i++) {
    if (database.nets1.at(i).lock == -1) {
      unmatch_nets1.emplace(i);
    }
  }

  for (int i = 0; i < database.nets2.size(); i++) {
    if (database.nets2.at(i).lock == -1) {
      unmatch_nets2.emplace(i);
    }
  }
  std::cout << "---------" << std::endl;
  std::cout << unmatch_instance1.size() + unmatch_instance2.size() << std::endl;
  std::cout << unmatch_nets1.size() + unmatch_nets2.size() << std::endl;
  std::cout << "---------" << std::endl;
  match_inst(database, unmatch_instance1, unmatch_instance2);
  // match_net(database, unmatch_nets1, unmatch_nets2);
  int nums = 100;
  while (nums != 0) {
    update(database);
    update_net1_new(database);
    update_net2_new(database);
    lock_net(database);
    nums--;
  }
}

void compare(Database &database) {
  // if instance and net number equal do pre_match

  // if (database.instances1.size() == database.instances2.size() and
  //     database.nets1.size() == database.nets2.size()) {
  //   net_info(database);
  //   pre_match(database);
  // }
  clock_t d1,d2;
  d1=clock();
  diffusion(database);
  isomorphism(database);
  d2=clock();
  clock_t c1,c2;
  output_corage_rate(database);
  c1=clock();
  contracte(database);
  isomorphism(database);
  c2=clock();
  clock_t r1,r2;
  output_corage_rate(database);
  r1=clock();
  refine(database);
  r2=clock();
  output_corage_rate(database);
  std::cout<<"time1 = "<<double(d2-d1)/CLOCKS_PER_SEC<<"s"<<std::endl;  
  std::cout<<"time2 = "<<double(c2-c1)/CLOCKS_PER_SEC<<"s"<<std::endl;  
  std::cout<<"time3 = "<<double(r2-r1)/CLOCKS_PER_SEC<<"s"<<std::endl;  
  // output_info(database);
  // output_instance(database);
  // output_net(database);
}
