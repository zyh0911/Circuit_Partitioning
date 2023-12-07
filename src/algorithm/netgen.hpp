#pragma once
#include "../database/database.hpp"
#include "../database/type.hpp"
#include "pruning.hpp"
#include "check.hpp"

#include <time.h>
#include <iostream>
#include <map>
#include <vector>
#include <unordered_set>


void compare(Database &database);
void compare_gpu(Database &database);