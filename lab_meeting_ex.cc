#include <iostream>
#include <string>

#include "../Empirical/evo/World.h"       // Gives ability to generate simple world
#include "../Empirical/tools/Random.h"

using namespace emp::evo;

////////////////////////////////////////////////
// NOTES
//  * random.P(0.5); // <-- returns true half the time and false half the time
//  * If organism type (World<organism_type>) has a fitness function, it will use it
////////////////////////////////////////////////


double fitnessFunction(int *i) {
  return (double) *i - 17.0;
}

class Org {
private:
  int value;

public:
  Org(int in_v) : value(in_v) { ; }

  double Fitness() const { return (double) value; }
  bool Mutate(emp::Random & r) { value += r.GetInt(-50, 51); return true; }

  void Print() { std::cout << value; }
};

int old_main() {
  // everything in empirical is in namespace 'emp'
  emp::Random random;
  World<int, PopEA> world(random);

  world.SetDefaultFitnessFun([](int * i) { return (double) *i - 17.0; });
  world.SetDefaultMutateFun([](int * i, emp::Random& r) {
    if (r.P(0.5)) {
      if (*i % 2) *i = *i * 3 + 1;
      else *i /= 2;
      return true;
    } else {
      return false;
    }
  });

  // Build world population
  for (int i = 0; i < 36; i++) {
    world.Insert(random.GetInt(10000));
  }

  // What does the population look like?
  for (int i = 0; i < world.GetSize(); i++) {
    std::cout << world[i] << " ";
  } std::cout << std::endl;

  for (int gen = 0; gen < 10; gen++) {
    // Elite selection -- get top 3 orgs, make 3 copies of each; add to next population
    world.EliteSelect(3, 3);
    // This will run 36 tournaments. For each tournament, pick 5 randos to run tournament. (default: no discrete generations)
    world.TournamentSelect(5, 36);
    // if using PopEA, to trigger next generation: call world.Update
    world.Update();
    // Mutate!
    world.MutatePop();

    // What does the population look like?
    std::cout << "Generation " << gen << std::endl;
    for (int i = 0; i < world.GetSize(); i++) {
      std::cout << world[i] << " ";
    } std::cout << std::endl;

  }
}

int main() {
  // everything in empirical is in namespace 'emp'
  emp::Random random;
  World<Org, PopEA> world(random, "loud");
  //world.pop.Config(1, 20);
  // Build world population
  for (int i = 0; i < 36; i++) {
    world.Insert(random.GetInt(10, 100));
  }

  //world.SetDefaultFitnessFun( [](Org * i) { return -(i->Fitness()); } );
  emp::LinkSignal("loud::offspring-ready", [&random](Org * org) { org->Print(); std::cout << "  sadness" << std::endl; } );
  //world.OffspringReady( [&random](Org * org) { org->Print(); std::cout << "  sadness" << std::endl; } );

  // What does the population look like?
  for (int i = 0; i < world.GetSize(); i++) {
    world[i].Print(); std::cout << " ";
  } std::cout << std::endl;

  for (int gen = 0; gen < 100; gen++) {
    // Elite selection -- get top 3 orgs, make 3 copies of each; add to next population
    world.EliteSelect(3, 3);
    // This will run 36 tournaments. For each tournament, pick 5 randos to run tournament. (default: no discrete generations)
    world.TournamentSelect(5, 36);
    // if using PopEA, to trigger next generation: call world.Update
    world.Update();
    // Mutate!
    //world.MutatePop();
    // What does the population look like?
    std::cout << "Generation " << gen << std::endl;
    for (int i = 0; i < world.GetSize(); i++) {
      world[i].Print(); std::cout << " ";
    } std::cout << std::endl;
  }
}
