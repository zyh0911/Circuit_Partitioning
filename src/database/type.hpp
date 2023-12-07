#pragma once
#include <cstdint>
#include <iostream>
#include <vector>

typedef struct net;
typedef struct instance;

// const int N1 = 47000;

struct net {
  std::string name;
  std::vector<int> inst_ptr;
  int lock = -1;
  int magic_num;
};

struct instance {
  std::string name;
  std::string type;
  std::vector<int> net_ptr;
  int lock = -1;
  int magic_num;
};

struct lock_info {
  int ptr1 = -1;
  int ptr2 = -1;
  bool state1 = false;
  bool state2 = false;
};

struct wire {
  int net;
  int instance;
};

struct error_type {
  int type; // 1 missing instance;2 extra instance;3 short; 4 open
  int net=-1;
  int instance=-1;
};
