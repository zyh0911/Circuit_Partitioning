#include "database/database.hpp"

#include <iostream>
#include <map>
#include <string>

int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    std::string filename1 = "../../test/100.txt";
    parser the_parser;
    the_parser.read(filename1);
    the_parser.write();
  }
  else if (argc > 2)
  {
  }
  return 0;
}
