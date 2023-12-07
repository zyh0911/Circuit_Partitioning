#pragma once

#include "type.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>


class Database {
public:
  explicit Database(const std::string &filename1) {
  };


private:

  void read(const std::string &filename);
};
