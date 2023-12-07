#pragma once
#include "../database/database.hpp"
#include "../database/type.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

static bool check_net(Database &db) {
  std::map<std::string, int> results;
  for (auto &net : db.nets1) {
    std::string name = net.name + std::to_string(net.magic_num);
    if (results.count(name) == 0) {
      results.emplace(name, 1);
    } else {
      results.at(name)++;
    }
  }
  for (auto &net : db.nets2) {
    std::string name = net.name + std::to_string(net.magic_num);
    if (results.count(name) == 0) {
      results.emplace(name, 1);
    } else {
      results.at(name)++;
    }
  }
  int match_num = 0;
  int mismatch_num = 0;
  for (auto &result : results) {
    if (result.second != 2) {
      // std::cout << std::endl;
      // std::cout << result.first << " " << result.second << std::endl;
      // return false;
      mismatch_num++;
    } else {
      match_num++;
    }
  }
  // std::cout << "mismatch nets number:" << mismatch_num << std::endl;
  // std::cout << "match nets number:" << match_num << std::endl;
  return mismatch_num == 0;
}

static bool check_instance(Database &db) {
  std::map<std::string, int> results;
  int match_num = 0;
  int mismatch_num = 0;
  for (auto &instance : db.instances1) {
    std::string name = instance.type + std::to_string(instance.magic_num);
    if (results.count(name) == 0) {
      results.emplace(name, 1);
    } else {
      results.at(name)++;
    }
  }
  for (auto &instance : db.instances2) {
    std::string name = instance.type + std::to_string(instance.magic_num);
    if (results.count(name) == 0) {
      results.emplace(name, 1);
    } else {
      results.at(name)++;
    }
  }

  for (auto &result : results) {
    if (result.second != 2) {
      // std::cout << std::endl;
      // std::cout << result.first << " " << result.second << std::endl;
      // return false;
      mismatch_num += result.second;
    } else {
      match_num++;
    }
  }
  // std::cout << "mismatch instances number:" << mismatch_num << std::endl;
  // std::cout << "match instances number:" << match_num << std::endl;
  return mismatch_num == 0;
}

static bool check(Database &db) { return check_instance(db) and check_net(db); }

static void output_net(Database &db) {
  std::map<std::string, int> results;
  for (auto &net : db.nets1) {
    if (net.lock == -1) {
      continue;
    }
    std::string name = net.name + std::to_string(net.magic_num);
    if (results.count(name) == 0) {
      results.emplace(name, 1);
    } else {
      results.at(name)++;
    }
  }
  for (auto &net : db.nets2) {
    if (net.lock == -1) {
      continue;
    }
    std::string name = net.name + std::to_string(net.magic_num);
    if (results.count(name) == 0) {
      results.emplace(name, 1);
    } else {
      results.at(name)++;
    }
  }
  int match_num = 0;
  int mismatch_num = 0;
  for (auto &result : results) {
    if (result.second != 2) {
      std::cout << std::endl;
      std::cout << result.first << " " << result.second << std::endl;
      // return false;
      mismatch_num++;
    } else {
      match_num++;
    }
  }
  std::cout << "mismatch nets number:" << mismatch_num << std::endl;
  std::cout << "match nets number:" << match_num << std::endl;
}

static void output_instance(Database &db) {
  std::map<std::string, int> results;
  std::set<int> map_inst1;
  std::set<int> map_inst2;
  int match_num = 0;
  int mismatch_num = 0;
  std::vector<int> map_inst1_num;
  std::vector<int> map_inst2_num;
  int unlock_num1=0;
  int unlock_num2=0;
  for (auto &instance : db.instances1) {
    if (instance.lock == -1) {
      unlock_num1++;
      continue;
    }
    if (map_inst1.find(instance.lock) == map_inst1.end()) {
      map_inst1.emplace(instance.lock);
    } else {
      map_inst1_num.emplace_back(instance.lock);
    }
    std::string name = instance.type + std::to_string(instance.magic_num);
    if (results.count(name) == 0) {
      results.emplace(name, 1);
    } else {
      results.at(name)++;
    }
  }
  for (auto &instance : db.instances2) {
    if (instance.lock == -1) {
      unlock_num2++;
      continue;
    }

    if (map_inst2.find(instance.lock) == map_inst2.end()) {
      map_inst2.emplace(instance.lock);
    } else {
      map_inst2_num.emplace_back(instance.lock);
    }
    std::string name = instance.type + std::to_string(instance.magic_num);
    if (results.count(name) == 0) {
      results.emplace(name, 1);
    } else {
      results.at(name)++;
    }
  }

  for (auto &result : results) {
    if (result.second != 2) {
      std::cout << std::endl;
      std::cout << result.first << " " << result.second << std::endl;
      // return false;
      mismatch_num += result.second;
    } else {
      match_num++;
    }
  }
  std::cout << "mismatch instances number:" << mismatch_num << std::endl;
  std::cout << "match instances number:" << match_num << std::endl;
  std::cout <<1- (double) unlock_num1/(db.instances1.size()-(unlock_num2-unlock_num1)) << std::endl;
  if(map_inst1_num.size()!=0 or map_inst2_num.size()!=0){
    std::cout << "overmap!" << std::endl;
  }
}

static void output_info(Database db) {
  for (const auto &instance : db.instances1) {
    std::cout << instance.name << " " << instance.lock << " " << instance.type
              << " " << instance.magic_num << " ";
    for (const auto &ptr : instance.net_ptr) {
      // std::cout << ptr<<" "<<db.nets.size()<<std::endl;
      std::cout << db.nets1[ptr].name << " ";
    }
    std::cout << std::endl;
  }

  std::cout << std::endl;
  for (const auto &net : db.nets1) {
    std::cout << net.name << " " << net.lock << " " << net.magic_num << " ";
    for (const auto &ptr : net.inst_ptr) {
      std::cout << db.instances1[ptr].name << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;

  for (const auto &instance : db.instances2) {
    std::cout << instance.name << " " << instance.lock << " " << instance.type
              << " " << instance.magic_num << " ";
    for (const auto &ptr : instance.net_ptr) {
      // std::cout << ptr<<" "<<db.nets.size()<<std::endl;
      std::cout << db.nets2[ptr].name << " ";
    }
    std::cout << std::endl;
  }

  std::cout << std::endl;
  for (const auto &net : db.nets2) {
    std::cout << net.name << " " << net.lock << " " << net.magic_num << " ";
    for (const auto &ptr : net.inst_ptr) {
      std::cout << db.instances2[ptr].name << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

static void output_corage_rate(Database &db) {
  int unlock_num1=0;
  int unlock_num2=0;
  for (auto &instance : db.instances1) {
    if (instance.lock == -1) {
      unlock_num1++;
      continue;
    }
  }
  for (auto &instance : db.instances2) {
    if (instance.lock == -1) {
      unlock_num2++;
      continue;
    }
  }

  std::cout <<1- (double) unlock_num1/(db.instances1.size()-(unlock_num2-unlock_num1)) << std::endl;
}
