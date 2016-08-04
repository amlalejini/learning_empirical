/*
  evo_in_physics_pt1_web.cc


  TODO:
    * Add organism density parameter to define organism mass.
    * Add resource density parameter to define resource mass.
    * Push pop pressure variable down chain to where it needs to go.
*/

#include "population_managers/PopulationManager_ABPhysics.h"
#include "organisms/ABPhysicsOrganism.h"

#include "web/Document.h"
#include "web/Canvas.h"
#include "web/keypress.h"
#include "web/web.h"
#include "web/Animate.h"
#include "web/canvas_utils.h"

#include "evo/world.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "config/config.h"


namespace emp {
namespace web {
  // Draw a Surface2D, specifying the full colormap to be used.
  template <typename BODY_TYPE>
  void Draw(Canvas canvas,
            const emp::vector<Surface2D<BODY_TYPE> *> & surface_set,
            const emp::vector<std::string> & color_map)
  {
    emp_assert((int)surface_set.size() > 0);
    canvas.Clear();

    const double w = surface_set[0]->GetWidth();
    const double h = surface_set[0]->GetHeight();

    // Setup a black background for the surface
    canvas.Rect(0, 0, w, h, "black");

    for (auto *surface : surface_set) {
      const auto &body_set = surface->GetConstBodySet();
      for (auto *body : body_set) {
        canvas.Circle(body->GetPerimeter(), "", color_map[body->GetColorID()]);
      }
    }
  }
}
}

namespace web = emp::web;

EMP_BUILD_CONFIG(EvoInPhysicsPt1Config,
  GROUP(DEFAULT, "Default settings for Evo In Physics Pt 1 Experiments."),
  VALUE(RANDOM_SEED, int, 0, "Random number seed (0 for based on time)."),
  VALUE(WORLD_WIDTH, int, 500, "Width of the physics world->"),
  VALUE(WORLD_HEIGHT, int, 500, "Height of the physics world->"),
  VALUE(MAX_ORG_RADIUS, double, 15, "Maximum radius that an organism can grow to."),
  VALUE(POP_PRESSURE, double, 1.0, "Physics body popping pressure."),
  VALUE(MAX_POP_SIZE, int, 250, "Maximum population size allowed in the world->"),
  VALUE(ORG_DETACH_ON_BIRTH, bool, true, "Do organisms detach from parents at birth?"),
  VALUE(MAX_RESOURCE_AGE, int, 10000, "Maximum number of updates that resources may persist."),
  VALUE(MAX_RESOURCE_COUNT, int, 250, "Maximum number of resources allowed in the environment at any given time."),
  VALUE(COST_OF_REPRODUCTION, double, 5.0, "How much energy is necessary for an organism to reproduce?"),
  VALUE(RESOURCE_ENERGY_CONTENT, double, 1.0, "How much energy is each resource worth when consumed?"),
  VALUE(GENOME_LENGTH, int, 50, "Number of sites in each organism's genome."),
  VALUE(POINT_MUTATION_RATE, double, 0.01, "Organism per-site mutation rate.")
)

class EvoInPhysicsInterface {
  private:
    using PhysicsWorld = emp::evo::World<ABPhysicsOrganism, emp::evo::PopulationManager_ABPhysics<ABPhysicsOrganism>>;
    emp::Random *random;
    PhysicsWorld *world;
    web::Document dashboard;
    web::Document world_view;
    web::Document stats_view;
    web::Animate anim;
    web::KeypressManager keypress_manager;
    EvoInPhysicsPt1Config config;

    // Localized config values.
    int random_seed;
    int world_width;
    int world_height;
    int max_pop_size;
    int genome_length;
    double point_mutation_rate;
    double max_org_radius;
    bool org_detach_on_birth;
    int max_resource_age;
    int max_resource_count;
    double cost_of_reproduction;
    double resource_energy_content;
    double pop_pressure;

    int current_update;

    enum class MapMode { BLANK, MAKE_BLANK, BASIC } map_mode;

  public:
    EvoInPhysicsInterface(int argc, char *argv[])
      : dashboard("dashboard-panel-body"),
        world_view("world-view"),
        stats_view("stats-view"),
        anim([this]() { EvoInPhysicsInterface::Animate(anim); }),
        current_update(-1),
        map_mode(MapMode::BASIC)
    {
      // Load config.
      std::string config_filename = "evo-in-physics-pt1.cfg";
      config.Read(config_filename);
      auto args = emp::cl::ArgManager(argc, argv);
      if (!args.ProcessConfigOptions(config, std::cout, config_filename)) exit(0);
      if (!args.TestUnknown()) exit(0);
      // Localize configs.
      random_seed = config.RANDOM_SEED();
      world_width = config.WORLD_WIDTH();
      world_height = config.WORLD_HEIGHT();
      max_pop_size = config.MAX_POP_SIZE();
      genome_length = config.GENOME_LENGTH();
      point_mutation_rate = config.POINT_MUTATION_RATE();
      max_org_radius = config.MAX_ORG_RADIUS();
      org_detach_on_birth = config.ORG_DETACH_ON_BIRTH();
      max_resource_age = config.MAX_RESOURCE_AGE();
      max_resource_count = config.MAX_RESOURCE_COUNT();
      cost_of_reproduction = config.COST_OF_REPRODUCTION();
      resource_energy_content = config.RESOURCE_ENERGY_CONTENT();
      pop_pressure = config.POP_PRESSURE();

      std::cout << "RANDOM SEED: " << random_seed << std::endl;

      // Some setup required.
      random = new emp::Random(random_seed);
      world = new PhysicsWorld(random, "EvoInPhysicsPt1");

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
      stats_view << "Organism Count: " << web::Live([this]() { return world->GetSize(); }) << "<br>";
      stats_view << "Resource Count: " << web::Live([this]() { return world->popM.GetNumResources(); }) << "<br>";
      stats_view << "Best 1-pull in population: " << web::Live([this]() { return world->popM.GetBestOnes(); }) << "<br>";
      stats_view << "Best 0-pull in population: " << web::Live([this]() { return world->popM.GetBestZeros(); }) << "<br>";
      // Setup canvas for world visualization
      world_view << web::Canvas(world_width, world_height, "evo-in-physics-pt1-world") << "<br>";

      // Initialize the run
      Initialize();

    }

    ~EvoInPhysicsInterface() {
      delete world;
    }

    void Initialize() {
      /* Do everything necessary to initialize our run. */
      // Configure the population
      world->ConfigPop(world_width, world_height, max_pop_size, max_org_radius, org_detach_on_birth, point_mutation_rate, cost_of_reproduction, max_resource_count, max_resource_age, resource_energy_content);
      // Reset evolution back to the beginning
      DoReset();
    }

    void ResetEvolution() {
      /* Do what is necessary to reset evolution. */
      // Reset the current update.
      current_update = 0;
      // Clear out the population.
      world->Clear();
      // Initialize the population.
      // - Get mid-point of world->
      const emp::Point<double> mid_point(world_width / 2.0, world_height / 2.0);
      int org_radius = max_org_radius;
      // - Insert ancestor seeds into population.
      ABPhysicsOrganism ancestor = ABPhysicsOrganism(emp::Circle<double>(mid_point, org_radius), genome_length, true);
      for (int i = 0; i < ancestor.genome.GetSize(); i++) {
        if (random->P(0.5)) ancestor.genome[i] = !ancestor.genome[i];
      }
      ancestor.SetColorID( (int)((ancestor.genome.CountOnes() / (double) ancestor.genome.GetSize()) * 200) );
      ancestor.SetMass(15.0);
      ancestor.SetPopPressureThreshold(pop_pressure);
      world->Insert(ancestor);
    }

    bool DoReset() {
      /* Called on reset button press. */
      // Reset evolution
      ResetEvolution();
      // Redraw the world
      switch(map_mode) {
        case MapMode::MAKE_BLANK:
          world_view.Canvas("evo-in-physics-pt1-world").Clear();
          map_mode = MapMode::BLANK;
        case MapMode::BLANK:
          break;
        case MapMode::BASIC:
          // Redraw world
          web::Draw(world_view.Canvas("evo-in-physics-pt1-world"), world->popM.GetPhysics().GetSurfaceSet(), emp::GetHueMap(360));
          break;
      }
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
      world->Update();
      switch (map_mode) {
        case MapMode::MAKE_BLANK:
          world_view.Canvas("evo-in-physics-pt1-world").Clear();
          map_mode = MapMode::BLANK;
        case MapMode::BLANK:
          break;
        case MapMode::BASIC:
          // Redraw world
          web::Draw(world_view.Canvas("evo-in-physics-pt1-world"), world->popM.GetPhysics().GetSurfaceSet(), emp::GetHueMap(360));
          break;
      }
      stats_view.Redraw();
    }

    bool OnKeydown(const emp::html5::KeyboardEvent &evt) {
      /* */
      // Reject most modified keypresses.
      if (evt.altKey || evt.ctrlKey || evt.metaKey) return false;
      const int key_code = evt.keyCode;
      bool return_value = true;

      switch (key_code) {
        case 'M':
          switch (map_mode) {
            case MapMode::BLANK:
            case MapMode::MAKE_BLANK:
              map_mode = MapMode::BASIC;
              web::Draw(world_view.Canvas("evo-in-physics-pt1-world"), world->popM.GetPhysics().GetSurfaceSet(), emp::GetHueMap(360));
              break;
            case MapMode::BASIC:
              map_mode = MapMode::MAKE_BLANK;
              world_view.Canvas("evo-in-physics-pt1-world").Clear();
              break;
          }
          break;
        default:
          return_value = false;
          break;
      }
      return return_value;
    }
};

EvoInPhysicsInterface *evo_in_physics_interface;

int main(int argc, char *argv[]) {
  evo_in_physics_interface = new EvoInPhysicsInterface(argc, argv);
  return 0;
}
