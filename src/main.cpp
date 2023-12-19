#include "database/database.hpp"
#include "database/info.hpp"

#include <iostream>
#include <iterator>
#include <map>
#include <string>

int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    std::string filename1 = "../../test/10000.txt";
    parser the_parser;
    the_parser.read(filename1);
    the_parser.write();

    std::cout << "---------" << std::endl;
    database db = database();
    std::cout << "---------" << std::endl;

    db.configureGraph(the_parser);
    std::cout << "---------" << std::endl;

    db.coarsen(false);
    std::cout << "---------" << std::endl;

    db.initialPartition();
    std::cout << "---------" << std::endl;

    db.uncoarsen();
    std::cout << "---------" << std::endl;

    db.refine();
    std::cout << "---------" << std::endl;

    db.outputGraph(the_parser);
    std::cout << "---------" << std::endl;
  }
  else if (argc > 2)
  {
  }
  return 0;
}
