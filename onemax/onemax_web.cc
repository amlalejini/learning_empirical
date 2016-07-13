/*
  Description: This will run a one max EA in the browser.
   This is entirely an educational thing for me to learn how to use empirical's web tools.
  By: Alex Lalejini
*/

#include <iostream>

#include "../../Empirical/tools/BitVector.h"
#include "../../Empirical/tools/Random.h"

#include "../../Empirical/evo/World.h"
#include "../../Empirical/evo/StatsManager.h"

#include "../../Empirical/web/web.h"

#incude "Organisms/OneMaxOrganism.h"

////////////////////////
// Notes to self:
//  * TODO:
//    * Get things working...
////////////////////////

namespace UI = emp::web;

// NOTE: Once compiled into JS, things outside of main will go out of scope.
//  - Thus, we need to make things we want to stick around globals.
UI::Document doc("emp_base");

// For now, these params will be globals.
const int POPULATION_SIZE = 1000;
const int GENOME_LENGTH = 50;
const float POINT_MUTATION_RATE = 0.01;
const int UPDATES = 150;

// World of EVOLUTION!
emp::evo::World<OneMax

int main() {

  return 0;
}
