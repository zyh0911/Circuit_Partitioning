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
    std::string filename = "../../test/10000.txt";
    parser the_parser;
    the_parser.read(filename);
    // the_parser.write();

    std::cout << "---------" << std::endl;
    database db = database();
    std::cout << "---------" << std::endl;

    db.configureGraph(the_parser);
    std::cout << "---------" << std::endl;

    db.bipartition();
    std::cout << "---------" << std::endl;

    db.outputGraph(the_parser);
    std::cout << "---------" << std::endl;
  }
  else if (argc == 3)
  {
    database db = database();
    std::string filename = argv[1];
    db.config.mix_scheme = std::strcmp(argv[2], "true") ? true : false;
    if (std::strcmp(argv[2], "true"))
    {
      std::vector<std::string> schemes{"EC", "HEC", "MHEC"};
      for (auto &the_scheme : schemes)
      {
        double min = 999999;
        db.config.coarsenScheme = the_scheme;
        int loop_num = 10;
        double average = 0;
        int right_loop = 0;
        clock_t start, end;
        start = clock();

        while (loop_num > 0)
        {
          parser the_parser;
          the_parser.read(filename);
          db.configureGraph(the_parser);
          db.bipartition();
          if (db.check())
          {
            double new_result = db.results.at(db.idx)->getPartitionScore();
            min = std::min(min, new_result);
            average += new_result;
            right_loop++;
          }
          loop_num--;
        }
        end = clock();
        if (right_loop == 0)
        {
          std::cout << "wrong!" << std::endl;
        }
        else
        {
          std::cout << "time = " << double(end - start) / right_loop / CLOCKS_PER_SEC << "s" << endl;
          std::cout << "MIX: " << argv[2] << std::endl;
          std::cout << "right_loop: " << right_loop << std::endl;
          std::cout.precision(4);
          std::cout << "Min: " << min << " Average: " << average / right_loop << std::endl;
        }
      }
    }
    else
    {
      int loop_num = 30;
      double min = 999999;
      double average = 0;
      int right_loop = 0;
      clock_t start, end;
      start = clock();

      while (loop_num > 0)
      {
        parser the_parser;
        the_parser.read(filename);
        db.configureGraph(the_parser);
        db.bipartition();
        if (db.check())
        {
          double new_result = db.results.at(db.idx)->getPartitionScore();
          min = std::min(min, new_result);
          average += new_result;
          right_loop++;
        }
        loop_num--;
      }
      end = clock();
      if (right_loop == 0)
      {
        std::cout << "wrong!" << std::endl;
      }
      else
      {
        std::cout << "time = " << double(end - start) / right_loop / CLOCKS_PER_SEC << "s" << endl;
        std::cout << "MIX: " << argv[2] << std::endl;
        std::cout << "right_loop: " << right_loop << std::endl;
        std::cout.precision(4);
        std::cout << "Min: " << min << " Average: " << average / right_loop << std::endl;
      }
    }
  }

  return 0;
}
