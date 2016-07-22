/*
  evo_in_physics_pt1_web.cc
    * Web version of evo_in_physics_pt1.cc. See that file for description.
  Still me learning how the old Evoke code works.
*/

#include "population_managers/PopulationManager_ABPhysics.h"
#include "organisms/ABPhysicsOrganism.h"

#include "web/Document.h"
#include "web/Canvas.h"
#include "web/keypress.h"
#include "web/web.h"
#include "web/Animate.h"
#include "web/canvas_utils.h"

#include "evo/World.h"
#include "tools/Random.h"

namespace web = emp::web;

// For now, use these parameters.
const int RANDOM_SEED = 101;
const int WORLD_WIDTH = 500;
const int WORLD_HEIGHT = 500;
const int MAX_ORG_DIAM = 10;
const int MIN_POP_SIZE = 4;
const int ORG_DETACH_ON_BIRTH = true;

bool OtherKey(const emp::html5::KeyboardEvent & evt)
{
  std::cout << "other key" << std::endl;

  return true;
}

class EvoInPhysicsInterface {
  private:
    emp::evo::World<ABPhysicsOrganism, emp::evo::PopulationManager_ABPhysics<ABPhysicsOrganism>> world;
    emp::Random random;
    web::Document dashboard;
    web::Document world_view;
    web::Document stats_view;
    web::Animate anim;
    web::KeypressManager keypress_manager;

    enum class MapMode { BLANK, MAKE_BLANK, BASIC } map_mode;

    int current_update;

  public:
    EvoInPhysicsInterface()
      : random(RANDOM_SEED),
        world(random, "EvoInPhysicsPt1"),
        dashboard("dashboard-panel-body"),
        world_view("world-view"),
        stats_view("stats-view"),
        anim([this]() { EvoInPhysicsInterface::Animate(anim); }),
        map_mode(MapMode::BASIC),
        current_update(-1)
    {
      std::cout << "Interface constructor." << std::endl;
      // Link keypresses to the proper handlers
      keypress_manager.AddKeydownCallback(std::bind(&EvoInPhysicsInterface::OnKeydown, this, _1));

      // Setup dashboard
      // - start/stop button -
      dashboard << web::Button([this]() { DoToggleRun(); },  "<span class=\"glyphicon glyphicon-play\" aria-hidden=\"true\"></span>", "start_but");
      auto start_button = dashboard.Button("start_but");
      start_button.SetAttr("class", "btn btn-success");
      // - reset button -
      dashboard << web::Button([this]() { DoReset(); }, "<span class=\"glyphicon glyphicon-refresh\" aria-hidden=\"true\"></span>", "reset_but");
      auto reset_button = dashboard.Button("reset_but");
      reset_button.SetAttr("class", "btn btn-primary");
      // - step button -
      dashboard << web::Button([this]() { DoStep(); }, "<span class=\"glyphicon glyphicon-step-forward\" aria-hidden=\"true\"></span>", "step_but");
      auto step_button = dashboard.Button("step_but");
      step_button.SetAttr("class", "btn btn-default");

      // Setup stats view
      stats_view.SetAttr("class", "well");
      stats_view << "Update: " << web::Live([this]() { return current_update; }) << "<br>";
      stats_view << "Organism Count: " << web::Live([this]() { return world.GetSize(); }) << "<br>";

      // Setup canvas for world visualization
      // canvas(world width, world height, name)
      world_view << web::Canvas(WORLD_WIDTH, WORLD_HEIGHT, "evo-in-physics-pt1-world") << "<br>";

      // Initialize the run
      Initialize();

    }

    void Initialize() {
      /* Do everything necessary to initialize our run. */
      // Configure the population
      world.ConfigPop(WORLD_WIDTH, WORLD_HEIGHT, MAX_ORG_DIAM, ORG_DETACH_ON_BIRTH, MIN_POP_SIZE);
      // Reset evolution back to the beginning
      DoReset();
    }

    void ResetEvolution() {
      /* Do what is necessary to reset evolution. */
      // Reset the current update.
      current_update = 0;
      // Clear out the population.
      world.Clear();
      // Initialize the population.
      // - Get mid-point of world.
      const emp::Point<double> mid_point(WORLD_WIDTH / 2.0, WORLD_HEIGHT / 2.0);
      int org_radius = 10;
      // - Insert ancestor into population.
      world.Insert(ABPhysicsOrganism(emp::Circle<double>(mid_point, org_radius)));
    }

    bool DoReset() {
      /* Called on reset button press. */
      // Reset evolution
      ResetEvolution();
      // Redraw the world
      web::Draw(world_view.Canvas("evo-in-physics-pt1-world"), world.popM.GetPhysics().GetSurface(), emp::GetHueMap(360));
      // Redraw the stats-view
      stats_view.Redraw();
      return true;
    }

    bool DoToggleRun() {
      /* Called on start/stop button press. */
      // Toggle animation object (active <--> not active)
      anim.ToggleActive();
      // Grab the start/stop button.
      auto start_but = dashboard.Button("start_but");
      auto step_but = dashboard.Button("step_but");
      // Update the button.
      if (anim.GetActive()) {
        // If active, set button to show 'stop' option.
        start_but.Label("<span class=\"glyphicon glyphicon-pause\" aria-hidden=\"true\"></span>");
        start_but.SetAttr("class", "btn btn-danger");
        step_but.Disabled(true);
      } else {
        // If inactive, set button to show 'play' option.
        start_but.Label("<span class=\"glyphicon glyphicon-play\" aria-hidden=\"true\"></span>");
        start_but.SetAttr("class", "btn btn-success");
        step_but.Disabled(false);
      }
      return true;
    }

    void DoStep() {
      /* Called from step button. */
      emp_assert(anim.GetActive() == false);
      anim.Step();
    }

    void Animate(const web::Animate &anim) {
      /* One step of animated evolution. */
      // Time marches on.
      current_update++;
      // Update world
      world.Update();
      // Redraw world
      web::Draw(world_view.Canvas("evo-in-physics-pt1-world"), world.popM.GetPhysics().GetSurface(), emp::GetHueMap(360));
      // Redraw stats
      stats_view.Redraw();
      // // Print out population...
      // std::cout << "======Population" << std::endl;
      // for (int p = 0; p < world.GetSize(); p++) {
      //   std::cout << "Org " << p << ": (" << world[p].GetCenter().GetX() << ", " << world[p].GetCenter().GetY() << ")" << std::endl;
      // }
      // std::cout << "================" << std::endl;
    }

    bool OnKeydown(const emp::html5::KeyboardEvent &evt) {
      /* */
      std::cout << "On keydown!" << std::endl;
      return true;
    }
};

EvoInPhysicsInterface *evo_in_physics_interface;

int main() {
  evo_in_physics_interface = new EvoInPhysicsInterface();
  return 0;
}
