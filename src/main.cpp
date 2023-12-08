#include "database/database.hpp"

#include <iostream>
#include <map>
#include <string>

int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    Config config;
    std::string filename1 = "../../test/100.txt";
    parser the_parser;
    the_parser.read(filename1);
    // the_parser.write();
    std::cout << "---------" << std::endl;
    Hmetis hmetis = Hmetis();
    std::cout << "---------" << std::endl;

    hmetis.configureGraph(the_parser);
    std::cout << "---------" << std::endl;

    hmetis.coarsen(config.numNodeLeft, false, config.coarsenScheme, config.maxCoarsenIter);
    std::cout << "---------" << std::endl;
    
    hmetis.initialPartition(config.numInstances, config.initialParScheme);
    std::cout << "---------" << std::endl;

    hmetis.uncoarsen(config.dropRate, config.uncoarsenScheme);
    std::cout << "---------" << std::endl;

    hmetis.refine(config.numNodeLeft, config.dropRate, config.coarsenScheme_r, config.uncoarsenScheme_r, config.maxRefineIter);
    std::cout << "---------" << std::endl;
    hmetis.outputGraph();
    std::cout << "---------" << std::endl;
  }
  else if (argc > 2)
  {
  }
  return 0;
}
