#include "algorithm/netgen.hpp"
#include "algorithm/check.hpp"
#include "database/function.hpp"
#include "database/type.hpp"

#include <chrono>
#include <iostream>
#include <map>
#include <string>

int main(int argc, char *argv[]) {
  if (argc == 1) {
    std::string filename1 =
        "/home/oem/Documents/LVS_comparer/test/jpeg/jpeg_verilog5.sp"; // layoput
    std::string filename2 =
        "/home/oem/Documents/LVS_comparer/test/jpeg/jpeg_verilog.sp"; // source
    Database db(filename1, filename2);
    compare_gpu(db);
  } else if (argc > 2) {
    Database db(argv[1], argv[2]);
    std::cout << "read two sp file" << std::endl;
       compare(db);
    output_info(db);

    std::cout << "compare two sp file" << std::endl;
    if (check(db)) {
      std::cout << "two netlist is equal" << std::endl;
    } else {
      std::cout << "two netlist is not equal" << std::endl;
    }
  }
  return 0;
}
