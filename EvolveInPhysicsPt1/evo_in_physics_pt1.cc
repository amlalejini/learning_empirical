/*
  evo_in_physics_pt1.cc
    * First attempt at incorporating physics into evolving organisms.
    * Environment: 2D physics world, nutrients A, B
    * Organisms: Defined by bit string genome
     - A nutrient pull: sum(0) in genome
     - B nutrient pull: sum(1) in genome
    * Replicate after collecting 10 nutrients
  Basically, me learning to use Evoke code.
*/

#include <iostream>

#include "tools/Random.h"

#include "evo/World.h"

#include "organisms/ABPhysicsOrganism.h"
#include "nutrients/ABPhysicsNutrient.h"
#include "population_managers/PopulationManager_ABPhysics.h"

///////////////////////////////
// Developer notes:
//  * TODO:
//    - Implement population manager with physics
//    - Implement physics body organisms
//    - Put organisms in environment, test to make sure it works
//    - Visualize organisms in environment (just to make sure things are working) -- just use onemax for replication
//    - Implement physics body nutrients
//    - Implement nutrient x organism interactions in population manager
///////////////////////////////

int main() {
  // Initialize the random number generator
  emp::Random random;
  // Evolution parameters
  const int GENOME_LENGTH = 50;
  const float POINT_MUTATION_RATE = 0.01;
  const int UPDATES = 100;

  // Build the world
  emp::evo::World<ABPhysicsOrganism, PopulationManager_ABPhysics<ABPhysicsOrganism>> world(random, "AB_Physics_World");

  // Build a population
  for (int p = 0; p < 10; p++) {
    emp::Point<double> org_loc(1, 1);
    int org_radius = 1;
    ABPhysicsOrganism baby_org(emp::Circle<double>(org_loc, org_radius), GENOME_LENGTH);
  }
  std::cout << "DONE" << std::endl;
  return 0;
}
