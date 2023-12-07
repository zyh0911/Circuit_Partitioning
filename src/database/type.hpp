#pragma once
#include <cstdint>
#include <iostream>
#include <vector>


struct net {
  std::string name;
  std::vector<int> inst_ptr;
  int lock = -1;
  int magic_num;
};
