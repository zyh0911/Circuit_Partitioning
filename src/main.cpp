#include "database/database.hpp"
#include "database/info.hpp"

#include <iostream>
#include <iterator>
#include <map>
#include <cstring>
#include <iomanip>
#include <ctime>
#include <algorithm>

int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    std::string filename = "../../test/100.txt";
    parser the_parser;
    the_parser.read(filename);
    // the_parser.write();

    std::cout << "---------" << std::endl;
    database db = database();
    std::cout << "---------" << std::endl;

    db.configureGraph(the_parser);
    std::cout << "---------" << std::endl;

    db.bipartition0();
    std::cout << "---------" << std::endl;

    db.outputGraph(the_parser);
    std::cout << "---------" << std::endl;
  }
  else if (argc == 2)
  {
    database db = database();
    std::string filename = argv[1];

    std::vector<std::string> schemes{"EC", "HEC", "MHEC"};
    for (auto &the_scheme : schemes)
    {
      double min = 999999;
      db.config.coarsenScheme = the_scheme;
      parser the_parser;
      the_parser.read(filename);
      clock_t start, end;
      start = clock();
      db.configureGraph(the_parser);
      db.bipartition1();
      min = db.results.at(db.idx)->getPartitionScore();
      end = clock();
      std::cout << "time = " << double(end - start) /100/ CLOCKS_PER_SEC << "s" << endl;
      std::cout.precision(4);
      std::cout << "Min: " << min << std::endl;
    }
  }

  return 0;
}
