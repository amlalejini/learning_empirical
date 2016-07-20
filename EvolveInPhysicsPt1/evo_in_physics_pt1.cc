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
// ISSUES
//  * EMP_TRACK_MEMORY on bodies kills program

int main() {
  // Initialize the random number generator
  emp::Random random;
  // Evolution parameters
  const int GENOME_LENGTH = 50;
  const float POINT_MUTATION_RATE = 0.01;
  const int UPDATES = 100;
  const double WORLD_WIDTH = 100.0;
  const double WORLD_HEIGHT = 100.0;
  const double MAX_ORG_DIAM = 10.0;
  const bool ORG_DETACH_ON_BIRTH = true;

  // Build the world
  emp::evo::World<ABPhysicsOrganism, emp::evo::PopulationManager_ABPhysics<ABPhysicsOrganism>> world(random, "AB_Physics_World");
  // Configure the population manager
  world.ConfigPop(WORLD_WIDTH, WORLD_HEIGHT, MAX_ORG_DIAM, ORG_DETACH_ON_BIRTH);
  // Build a population
  for (int p = 0; p < 10; p++) {
    emp::Point<double> org_loc(1, 1);
    int org_radius = 1;
    world.Insert(ABPhysicsOrganism(emp::Circle<double>(org_loc, org_radius), GENOME_LENGTH));
  }
  for (int u = 1; u <= UPDATES; u++) {
    std::cout << "Current update: " << u << std::endl;
    world.Update();
  }
  std::cout << "DONE" << std::endl;
  return 0;
}
