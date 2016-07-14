
#include <iostream>
#include <sstream>
#include <functional>

#include "../../Empirical/tools/BitVector.h"
#include "../../Empirical/tools/Random.h"

#include "../../Empirical/evo/World.h"
//#include "../../Empirical/evo/StatsManager.h"

#include "Organisms/OneMaxOrganism.h"

///////////////////
// Notes: How do I setup mutate on birth?
// * To learn:
///////////////////


int main() {
  // Initialize random num generator
  emp::Random random;
  const int POPULATION_SIZE = 1000;
  const int GENOME_LENGTH = 50;
  const float POINT_MUTATION_RATE = 0.01;
  const int UPDATES = 150;

  // Build the world
  emp::evo::World<OneMaxOrganism, emp::evo::PopEA> world(random, "OneMaxWorld");

  std::function<bool(OneMaxOrganism *, emp::Random &)> mut_fun = [POINT_MUTATION_RATE](OneMaxOrganism *org, emp::Random &random) -> bool {
    /* With some probability (point mutation rate), flip bits. */
    bool mutated = false;
    for (int i = 0; i < org->genome.GetSize(); i++) {
      if (random.P(POINT_MUTATION_RATE)) {
        org->genome[i] = !org->genome[i];
        mutated = true;
      }
    }
    return mutated;
  };
  world.SetDefaultMutateFun(mut_fun);

  std::function<double(OneMaxOrganism *)> fit_fun = [](OneMaxOrganism *org) -> double {
    return (double) org->genome.CountOnes();
  };
  world.SetDefaultFitnessFun(fit_fun);

  emp::LinkSignal("OneMaxWorld::on-update", []() {
    std::cout << "OneMaxWorld : on update signal" << std::endl;
  });
  // Initialize the population
  for (int p = 0; p < POPULATION_SIZE; p++) {
    OneMaxOrganism baby_org(GENOME_LENGTH);
    world.Insert(baby_org);
  }
  // Test all operators
  // std::cout << (world[0] == world[1]) << std::endl;
  // std::cout << (world[0] > world[1]) << std::endl;
  // std::cout << (world[0] < world[1]) << std::endl;
  // std::cout << (world[0] >= world[1]) << std::endl;
  // std::cout << (world[0] != world[1]) << std::endl;
  // std::cout << (world[0] <= world[1]) << std::endl;
  // // Print the population
  // std::cout << "-=== Initial population: ===-" << std::endl;
  // for (int i = 0; i < world.GetSize(); i++) {
  //   std::cout << "ORG #" << i << ": " << std::endl;
  //   world[i].Print();
  // }
  // Mutate pop:
  // world.MutatePop();
  // std::cout << "-=== Post-mutated population: ===-" << std::endl;
  // for (int i = 0; i < world.GetSize(); i++) {
  //   std::cout << "ORG #" << i << ": " << std::endl;
  //   world[i].Print();
  // }
  // Test string stream stuff
  // std::ostringstream oss;
  // world[0].genome.Print(oss);
  // std::cout << "====== " << oss.str() << std::endl;
  // exit(0);
  //std::cout << ss;
  // Evolution!
  for (int ud = 1; ud <= UPDATES; ud++) {
    int tourny_size = 4;
    // Run a tournament for every slot in next population
    world.TournamentSelect(tourny_size, world.GetSize());
    // Trigger the next generation (call: world.Update())
    world.Update();
    // Mutate the new population
    world.MutatePop();
    // Look at the population
    int most_fit = 0;
    for (int i = 0; i < world.GetSize(); i++) {
      if (world[i] >= world[most_fit]) most_fit = i;
    }
    // Get max fitness from population
    std::cout << "Generation: " << ud << " Best org: ";
    world[most_fit].Print();
    //std::cout << "\tMost fit genome: "; world[most_fit].Print();
  }

  return 0;
}
