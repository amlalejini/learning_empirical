/*
  Description: This will run a one max EA in the browser.
   This is entirely an educational thing for me to learn how to use empirical's web tools.
  By: Alex Lalejini
*/

#include <iostream>
#include <sstream>
#include <functional>

#include "../../Empirical/tools/BitVector.h"
#include "../../Empirical/tools/Random.h"

#include "../../Empirical/evo/World.h"
//#include "../../Empirical/evo/StatsManager.h"

#include "../../Empirical/web/web.h"
#include "../../Empirical/web/Animate.h"

#include "Organisms/OneMaxOrganism.h"
#include "Visualizations/OneMaxVisualization.h"

////////////////////////
// Notes to self:
//  * TODO:
//    * Get things working...
////////////////////////

namespace web = emp::web;

using std::cout; using std::endl;
using std::ostringstream;
using std::string;

// NOTE: Once compiled into JS, things outside of main will go out of scope.
//  - Thus, we need to make things we want to stick around globals.

// For now, these params will be globals.
const int POPULATION_SIZE = 25;
const int GENOME_LENGTH = 50;
const float POINT_MUTATION_RATE = 0.01;
const int UPDATES = 150;
const int RANDOM_SEED = 101;
const int TOURNY_SIZE = 4;

class OneMaxInterface {
  private:
    emp::Random random;
    emp::evo::World<OneMaxOrganism, emp::evo::PopEA> world;
    web::Document dashboard;
    web::Document display;
    web::Document onemax_vis;
    web::Animate anim;
    OneMaxVisualization vis;

    int update;
    string best_genotype;

  public:
    OneMaxInterface()
      : random(RANDOM_SEED),
        world(random, "OneMaxWorld"),
        dashboard("dashboard-panel-body"),
        display("display"),
        onemax_vis("onemax_visualization"),
        anim([this]() { OneMaxInterface::Evolve(anim); }), // Create an animation object. Setup a callback function.
        vis(POPULATION_SIZE, GENOME_LENGTH),
        update(0),
        best_genotype("")
    {
      // Print out a welcoming message to let everyone know we're here!
      cout << "Constructing OneMaxInterface!" << endl;
      cout << "\tRandom Seed: " << RANDOM_SEED << endl;

      //////////////////////////////////
      //       EVOLUTION SETUP        //
      //////////////////////////////////
      // Define and set fitness function
      std::function<double(OneMaxOrganism *)> fitness_fun = [](OneMaxOrganism *org) {
        return (double) org->genome.CountOnes();
      };
      world.SetDefaultFitnessFun(fitness_fun);
      // Define and set mutation function
      std::function<bool(OneMaxOrganism *, emp::Random &)> mutation_fun = [](OneMaxOrganism *org, emp::Random &random) -> bool {
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
      world.SetDefaultMutateFun(mutation_fun);

      //////////////////////////////////
      //    LINK EVO AND INTERFACE    //
      //////////////////////////////////
      //anim.SetCallback([this]() { Evolve(); });
      //////////////////////////////////
      //       INTERFACE SETUP        //
      //////////////////////////////////
      // Setup dashboard
      // -- Make buttons! --
      // - start button -
      dashboard << web::Button([this]() { DoToggleRun(); }, "<span class=\"glyphicon glyphicon-play\" aria-hidden=\"true\"></span>", "start_but");
      auto start_button = dashboard.Button("start_but");
      start_button.SetAttr("class", "btn btn-success"); 
      // reset button
      dashboard << web::Button([this]() { DoReset(); }, "<span class=\"glyphicon glyphicon-refresh\" aria-hidden=\"true\"></span>", "reset_but");
      auto reset_button = dashboard.Button("reset_but");
      reset_button.SetAttr("class", "btn btn-primary");
      // Display
      display << "Current update: " << web::Live(update) << "<br>";
      display << "Current best genotype: " << web::Live(best_genotype) << "<br>";
      // D3 visualization
      onemax_vis << vis;

      Initialize();

      // Hack in a visualization:
      // EM_ASM({
      //   // Create a main function
      //   var main = function() {
      //     console.log("Entering main!" );
      //
      //   };
      //   // On ready, call js main
      //   $(document).ready(main());
      // }, );

    }

    ~OneMaxInterface() { ; }

    void Initialize() {
      ResetEvolution();
    }

    void ResetEvolution() {
      // Reset update
      update = 0;
      // Clear population
      world.Clear();
      // Initialize population
      for (int p = 0; p < POPULATION_SIZE; p++) {
        OneMaxOrganism baby_org(GENOME_LENGTH);
        world.Insert(baby_org);
      }
      ostringstream oss;
      world[0].genome.Print(oss);
      best_genotype = oss.str();
      cout << "Population successfully initialized!" << endl;
    }

    void DoToggleRun() {
      // Toggle animation object active
      anim.ToggleActive();
      auto start_but = dashboard.Button("start_but");
      // Update button
      if (anim.GetActive()) {
        // If active, set button to show 'pause' option
        start_but.Label("<span class=\"glyphicon glyphicon-pause\" aria-hidden=\"true\"></span>");
        start_but.SetAttr("class", "btn btn-danger");
      } else {
        // If inactive, set button to show 'play' option
        start_but.Label("<span class=\"glyphicon glyphicon-play\" aria-hidden=\"true\"></span>");
        start_but.SetAttr("class", "btn btn-success");
      }
    }

    void DoReset() {
      ResetEvolution();
      display.Redraw();
      vis.AnimateStep(world);
    }

    void Evolve(const web::Animate &anim) {
      // time marches on
      update++;
      // run selection on population
      world.TournamentSelect(TOURNY_SIZE, world.GetSize());
      // move to next generation
      world.Update();
      // mutate next generation
      world.MutatePop();
      // Max fitness?
      int most_fit = 0;
      for (int i = 0; i < world.GetSize(); i++) {
        if (world[i] >= world[most_fit]) most_fit = i;
      }
      // Update best genotype
      vis.AnimateStep(world);
      ostringstream oss;
      world[most_fit].genome.Print(oss);
      best_genotype = oss.str();
      // Handle redrawing
      display.Redraw();
    }

};

OneMaxInterface *onemax_interface;

int main() {
  onemax_interface = new OneMaxInterface();
  return 0;
}
