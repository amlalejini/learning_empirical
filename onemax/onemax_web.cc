/*
  Description: This will run a one max EA in the browser.
   This is entirely an educational thing for me to learn how to use empirical's web tools.
  By: Alex Lalejini
*/

#include <iostream>
#include <functional>

#include "../../Empirical/tools/BitVector.h"
#include "../../Empirical/tools/Random.h"

#include "../../Empirical/evo/World.h"
//#include "../../Empirical/evo/StatsManager.h"

#include "../../Empirical/web/web.h"
#include "../../Empirical/web/Animate.h"

#include "Organisms/OneMaxOrganism.h"

////////////////////////
// Notes to self:
//  * TODO:
//    * Get things working...
////////////////////////

namespace web = emp::web;

using std::cout; using std::endl;

// NOTE: Once compiled into JS, things outside of main will go out of scope.
//  - Thus, we need to make things we want to stick around globals.

// For now, these params will be globals.
const int POPULATION_SIZE = 1000;
const int GENOME_LENGTH = 50;
const float POINT_MUTATION_RATE = 0.01;
const int UPDATES = 150;
const int RANDOM_SEED = 101;

class OneMaxInterface {
  private:
    emp::Random random;
    emp::evo::World<OneMaxOrganism, emp::evo::PopEA> world;
    web::Document doc;
    web::Animate anim;

    int update;

  public:
    OneMaxInterface()
      : random(RANDOM_SEED),
        world(random, "OneMaxWorld"),
        doc("emp_base"),
        anim([this]() { OneMaxInterface::Evolve(anim); }), // Create an animation object. Setup a callback function.
        update(0)
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

      // Initialize evolution
      Initialize();
      //////////////////////////////////
      //    LINK EVO AND INTERFACE    //
      //////////////////////////////////
      //anim.SetCallback([this]() { Evolve(); });
      //////////////////////////////////
      //       INTERFACE SETUP        //
      //////////////////////////////////
      // Add a beautiful JUMBOTRON!
      doc <<
        "<div id=\"banner\" class = \"jumbotron\">"
          "<h1>Visualization: One Max -- Learning Empircal</h1>"
        "</div>";

      // Build the HTML backbone/layout of the page
      doc <<
        "<div class=row>"
        " <div class=col-md-12>"
        "   <div class=\"panel panel-default\">"
        "     <div class=\"panel-heading\">"
        "       <h2>Dashboard</h2>"
        "     </div>"
        "     <div class=\"panel-body\" id=\"dashboard-panel-body\">"
        "     </div>"
        "   </div>"
        " </div>"
        "</div>";
      // -- Make buttons! --
      // start button
      doc << web::Button([this]() { DoToggleRun(); }, "<span class=\"glyphicon glyphicon-play\" aria-hidden=\"true\"></span>", "start_but");
      auto start_button = doc.Button("start_but");
      start_button.Title("\" class=\"btn btn-default"); // HTML injection! -- pretty naughty
      // reset button
      doc << web::Button([this]() { DoReset(); }, "<span class=\"glyphicon glyphicon-refresh\" aria-hidden=\"true\"></span>", "reset_but");
      auto reset_button = doc.Button("reset_but");
      reset_button.Title("\" class=\"btn btn-default");
      // Use javascript to organize elements on the page
      EM_ASM({
        $(document).ready(function() {
          $('#dashboard-panel-body').append($('#start_but'));
          $('#dashboard-panel-body').append($('#reset_but'));
        });
      });
      //anim.ToggleActive();

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
      cout << "Population successfully initialized!" << endl;
    }

    void DoToggleRun() {
      // Toggle animation object active
      cout << "Do Toggle Run!" << endl;
      anim.ToggleActive();
      auto start_but = doc.Button("start_but");
      // Update button
      if (anim.GetActive()) {
        // If active, set button to show 'pause' option
        start_but.Label("<span class=\"glyphicon glyphicon-pause\" aria-hidden=\"true\"></span>");
      } else {
        // If inactive, set button to show 'play' option
        start_but.Label("<span class=\"glyphicon glyphicon-play\" aria-hidden=\"true\"></span>");
      }
    }

    void DoReset() {
      cout << "RESET!" << endl;
      ResetEvolution();
    }

    void Evolve(const web::Animate &anim) {
      cout << "Evolution is happening? " << update << endl;
      update++;
    }

};

OneMaxInterface *onemax_interface;

int main() {
  onemax_interface = new OneMaxInterface();
  return 0;
}
