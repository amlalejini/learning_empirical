/*
  evo_in_physics_pt2_web.cc

  Second iteration of learning physics in Empirical.
  Goal:
    * Everything from EvolveInPhysicsPt1
    * Sensors
    * Actuators
    * Parameter configuration from webpage.
*/

#include <iostream>
#include <string>
#include <sstream>

#include "./geometry/Point2D.h"
#include "./organisms/SimpleOrganism.h"
#include "./population-managers/PopulationManager_SimplePhysics.h"
// TODO: Merge my geometry package into Empirical.
//       Having include conflicts because canvas utils includes a few geometry things.
#include "web/web.h"
#include "web/Document.h"
#include "web/Animate.h"
#include "web/Canvas.h"
#include "web/canvas_utils.h"

#include "emtools/emfunctions.h"

#include "tools/Random.h"
#include "tools/vector.h"
#include "tools/assert.h"

#include "evo/World.h"

namespace emp {
namespace web {
  // Draw a set of Surface2Ds, specifying the full colormap to be used.
  // TODO (fix): currently assumes body has a perimeter and will draw as a Circle..
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
        // TODO: Add draw body function (should have one for each type of physics body)
        canvas.Circle(body->GetPerimeter(), "", color_map[body->GetColorID()]);
      }
    }
  }
}
}

namespace web = emp::web;

// Some useful macros:
#define PLAY_GLYPH        "<span class=\"glyphicon glyphicon-play\" aria-hidden=\"true\"></span>"
#define PAUSE_GLYPH       "<span class=\"glyphicon glyphicon-pause\" aria-hidden=\"true\"></span>"
#define REFRESH_GLYPH     "<span class=\"glyphicon glyphicon-refresh\" aria-hidden=\"true\"></span>"
#define SINGLE_STEP_GLYPH "<span class=\"glyphicon glyphicon-step-forward\" aria-hidden=\"true\"></span>"
#define SETTINGS_GLYPH    "<span class=\"glyphicon glyphicon-cog\" aria-hidden=\"true\"></span>"
#define LOLLY_GLYPH       "<span class=\"glyphicon glyphicon-ice-lolly\" aria-hidden=\"true\"></span>"

// Default experiment settings.
// TODO: try to switch this to using config system (use config system to generate HTML stuff as well)
//  -- General Settings --
const int DEFAULT_RANDOM_SEED = 1;
const int DEFAULT_WORLD_WIDTH = 500;
const int DEFAULT_WORLD_HEIGHT = 500;
//  -- Population-specific --
const int DEFAULT_MAX_POP_SIZE = 250;
//  -- Organism-specific --
const int DEFAULT_GENOME_LENGTH = 10;
const double DEFAULT_POINT_MUTATION_RATE = 0.01;
const double DEFAULT_MAX_ORGANISM_RADIUS = 10;
const double DEFAULT_COST_OF_REPRO = 1;
const bool DEFAULT_DETACH_ON_BIRTH = true;
const double DEFAULT_ORGANISM_MEMBRANE_STRENGTH = 10.0;
//  -- Resource-specific --
const int DEFAULT_MAX_RESOURCE_AGE = 100;
const int DEFAULT_MAX_RESOURCE_COUNT = 100;
const double DEFAULT_RESOURCE_RADIUS = 5.0;
const double DEFAULT_RESOURCE_VALUE = 1.0;
//  -- Physics-specific --
const double DEFAULT_SURFACE_FRICTION = 0.0025;
const double DEFAULT_MOVEMENT_NOISE = 0.15;


class EvoInPhysicsInterface {
  private:
    // Aliases
    using Organism_t = SimpleOrganism;
    using SimplePhysicsWorld = emp::evo::World<Organism_t, emp::evo::PopulationManager_SimplePhysics<Organism_t> >;

    emp::Random *random;
    SimplePhysicsWorld *world;
    // Interface-specific objects.
    //  - Exp run views.
    web::Document dashboard;      // Visible during exp page mode.
    web::Document world_view;     // Visible during exp page mode.
    web::Document stats_view;     // Visible during exp page mode.
    web::Document param_view;     // Visible during config page mode.
    //  - Config views.
    web::Document exp_config;
    // Animation
    web::Animate anim;
    // Page mode.
    enum class PageMode { EXPERIMENT, CONFIG } page_mode;
    // Localized exp configuration variables.
    //  -- General Settings --
    int random_seed;
    int world_width;
    int world_height;
    //  -- Population-specific --
    int max_pop_size;
    //  -- Organism-specific --
    double point_mutation_rate;
    int genome_length;
    double max_organism_radius;
    double cost_of_repro;
    bool detach_on_birth;
    double organism_membrane_strength;
    //double organism_density;
    //  -- Resource-specific --
    int max_resource_age;
    int max_resource_count;
    double resource_radius;
    double resource_value;
    //  -- Physics-specific --
    double surface_friction;
    double movement_noise;

  public:
    EvoInPhysicsInterface(int argc, char *argv[]) :
      dashboard("dashboard-panel-body"),
      world_view("world-view"),
      stats_view("stats-view"),
      param_view("config-view"),
      exp_config("exp-config-panel-body"),
      anim([this]() { EvoInPhysicsInterface::Animate(anim); }),
      page_mode(PageMode::CONFIG)
    {
      /* Web interface constructor. */

      // Localize parameter values.
      // -- General --
      random_seed = DEFAULT_RANDOM_SEED;
      world_width = DEFAULT_WORLD_WIDTH;
      world_height = DEFAULT_WORLD_HEIGHT;
      //  -- Population-specific --
      max_pop_size = DEFAULT_MAX_POP_SIZE;
      //  -- Organism-specific --
      point_mutation_rate = DEFAULT_POINT_MUTATION_RATE;
      genome_length = DEFAULT_GENOME_LENGTH;
      max_organism_radius = DEFAULT_MAX_ORGANISM_RADIUS;
      cost_of_repro = DEFAULT_COST_OF_REPRO;
      detach_on_birth = DEFAULT_DETACH_ON_BIRTH;
      organism_membrane_strength = DEFAULT_ORGANISM_MEMBRANE_STRENGTH;
      //  -- Resource-specific --
      max_resource_age = DEFAULT_MAX_RESOURCE_AGE;
      max_resource_count = DEFAULT_MAX_RESOURCE_COUNT;
      resource_radius = DEFAULT_RESOURCE_RADIUS;
      resource_value = DEFAULT_RESOURCE_VALUE;
      //  -- Physics-specific --
      surface_friction = DEFAULT_SURFACE_FRICTION;
      movement_noise = DEFAULT_MOVEMENT_NOISE;

      // Setup page
      // - Setup EXPERIMENT RUN mode view. -
      // --- Setup Dashboard. ---
      // ----- start/stop button -----
      dashboard << web::Button([this]() { DoToggleRun(); }, PLAY_GLYPH, "start_but");
      auto start_button = dashboard.Button("start_but");
      start_button.SetAttr("class", "btn btn-success");
      // ----- refresh button -----
      dashboard << web::Button([this]() { DoReset(); }, REFRESH_GLYPH, "reset_but");
      auto reset_button = dashboard.Button("reset_but");
      reset_button.SetAttr("class", "btn btn-primary");
      // ----- step button ------
      dashboard << web::Button([this]() { DoStep(); }, SINGLE_STEP_GLYPH, "step_but");
      auto step_button = dashboard.Button("step_but");
      step_button.SetAttr("class", "btn btn-default");
      // ----- reconfigure button -----
      dashboard << web::Button([this]() { DoReconfigureExperiment(); }, SETTINGS_GLYPH, "settings_but");
      auto reconfigure_button = dashboard.Button("settings_but");
      reconfigure_button.SetAttr("class", "btn btn-default pull-right");

      // --- Setup Stats View. ---
      stats_view.SetAttr("class", "well");
      stats_view << "<h2>Stats View</h2><br>";
      stats_view << "Update: "
                 << web::Live([this]() {
                      if (world != nullptr) return world->update;
                      else return -1; }) << "<br>";
      stats_view << "Organism Count: "
                 << web::Live([this]() {
                      if (world != nullptr) return world->popM.GetSize();
                      else return -1; })
                 << "<br>";
      stats_view << "Resource Count: "
                 << web::Live([this]() {
                      if (world != nullptr) return world->popM.GetNumResources();
                      else return -1; })
                 << "<br>";
      // --- Setup Config View. ---
      param_view.SetAttr("class", "well");
      // -- General --
      param_view << "<h2>Parameter View</h2><br>";
      param_view << "Random Seed: " << web::Live([this]() { return random_seed; }) << "<br>";
      param_view << "World Width: " << web::Live([this]() { return world_width; }) << "<br>";
      param_view << "World Height: " << web::Live([this]() { return world_height; }) << "<br>";
      // -- Population-Specific --
      param_view << "Max Population Size: " << web::Live([this]() { return max_pop_size; }) << "<br>";
      // -- Organism-Specific --
      param_view << "Genome Length: " << web::Live([this]() { return genome_length; }) << "<br>";
      param_view << "Point Mutation Rate: " << web::Live([this]() { return point_mutation_rate; }) << "<br>";
      param_view << "Max Organism Radius: " << web::Live([this]() { return max_organism_radius; }) << "<br>";
      param_view << "Cost of Reproduction: " << web::Live([this]() { return cost_of_repro; }) << "<br>";
      param_view << "Detach on Birth: " << web::Live([this]() { return (int) detach_on_birth; }) << "<br>";
      param_view << "Organism Membrane Strength: " << web::Live([this]() { return organism_membrane_strength; }) << "<br>";
      // -- Resource-Specific --
      param_view << "Max Resource Age: " << web::Live([this]() { return max_resource_age; }) << "<br>";
      param_view << "Max Resource Count: " << web::Live([this]() { return max_resource_count; }) << "<br>";
      param_view << "Resource Radius: " << web::Live([this]() { return resource_radius; }) << "<br>";
      param_view << "Resource Value: " << web::Live([this]() { return resource_value; }) << "<br>";
      // -- Physics-Specific --
      param_view << "Surface Friction: " << web::Live([this]() { return surface_friction; }) << "<br>";
      param_view << "Movement Noise: " << web::Live([this]() { return movement_noise; }) << "<br>";

      // - Setup EXPERIMENT CONFIG mode view. -
      exp_config << web::Button([this]() { DoRunExperiment(); }, LOLLY_GLYPH, "start_exp_but");
      auto start_exp_but = exp_config.Button("start_exp_but");
      start_exp_but.SetAttr("class", "btn btn-danger");
      exp_config << "<hr>";
      //  - Experiment Settings -
      // TODO: Switch over to config system?
      // -- General --
      exp_config << "<h3>General Settings</h3>";  // TODO: Config system group name
      exp_config << GenerateParamNumberField("Random Seed", "random-seed", random_seed);
      exp_config << GenerateParamNumberField("World Width", "world-width", world_width);
      exp_config << GenerateParamNumberField("World Height", "world-height", world_height);
      // -- Population-specific --
      exp_config << "<h3>Population-Specific Settings</h3>";
      exp_config << GenerateParamNumberField("Max Population Size", "max-pop-size", max_pop_size);
      // -- Organism-specific --
      exp_config << "<h3>Organism-Specific Settings</h3>";
      exp_config << GenerateParamNumberField("Genome Length", "genome-length", genome_length);
      exp_config << GenerateParamNumberField("Point Mutation Rate", "point-mutation-rate", point_mutation_rate);
      exp_config << GenerateParamNumberField("Max Organism Radius", "max-organism-radius", max_organism_radius);
      exp_config << GenerateParamNumberField("Cost of Reproduction", "cost-of-repro", cost_of_repro);
      exp_config << GenerateParamCheckboxField("Detach on Birth", "detach-on-birth", detach_on_birth);
      exp_config << GenerateParamNumberField("Organism Membrane Strength", "organism-membrane-strength", organism_membrane_strength);
      //  -- Resource-specific --
      exp_config << "<h3>Resource-Specific Settings</h3>";
      exp_config << GenerateParamNumberField("Max Resource Age", "max-resource-age", max_resource_age);
      exp_config << GenerateParamNumberField("Max Resource Count", "max-resource-count", max_resource_count);
      exp_config << GenerateParamNumberField("Resource Radius", "resource-radius", resource_radius);
      exp_config << GenerateParamNumberField("Resource Value", "resource-value", resource_value);
      //  -- Physics-specific --
      exp_config << "<h3>Physics-Specific Settings</h3>";
      exp_config << GenerateParamNumberField("Surface Friction", "surface-friction", surface_friction);
      exp_config << GenerateParamNumberField("Movement Noise", "movement-noise", movement_noise);

      // Configure page view.
      ChangePageView(page_mode);

      if (page_mode == PageMode::EXPERIMENT) InitializeExperiment();

    }

    ~EvoInPhysicsInterface() {
      if (world != nullptr) delete world;
    }

    template<typename FieldType>
    std::string GenerateParamTextField(std::string field_name, std::string field_id, FieldType default_value) {
      return GenerateParamField(field_name, field_id, default_value, "Text");
    }

    template<typename FieldType>
    std::string GenerateParamNumberField(std::string field_name, std::string field_id, FieldType default_value) {
      return GenerateParamField(field_name, field_id, default_value, "Number");
    }

    std::string GenerateParamCheckboxField(std::string field_name, std::string field_id, bool default_value) {
      return GenerateParamField(field_name, field_id, default_value, "checkbox");
    }

    // TODO: Make checkbox input look nice.
    template<typename FieldType>
    std::string GenerateParamField(std::string field_name, std::string field_id, FieldType default_value, std::string input_type) {
      std::stringstream param_html;
        param_html << "<div class=\"input-group\">";
          param_html << "<span class=\"input-group-addon\" id=\"" << field_id << "-addon\">" << field_name << "</span>  ";
          param_html << "<input type=\"" << input_type << "\""
                     << " class=\"form-control\""
                     << " aria-describedby=\"" << field_id << "-addon\""
                     << " id=\"" << field_id << "-param\"";
                     if (input_type == "checkbox") param_html << "checked=\"" << default_value << "\">";
                     else param_html << " value=\"" << default_value << "\">";
        param_html << "</div>";
      return param_html.str();
    }

    // Call to initialize a new experiment with current config values.
    void InitializeExperiment() {
      // Setup the world.
      if (world != nullptr) delete world;
      if (random != nullptr) delete random; // World does not own *random. Delete it.
      random = new emp::Random(random_seed);
      world = new SimplePhysicsWorld(random, "simple-world");
      // Setup world view canvs.
      world_view.ClearChildren();
      world_view << web::Canvas(world_width, world_height, "simple-world-canvas") << "<br>";
      // Configure new experiment.
      world->ConfigPop(world_width, world_height, surface_friction,
                       max_pop_size, point_mutation_rate, max_organism_radius,
                       cost_of_repro, max_resource_age, max_resource_count,
                       resource_radius, resource_value, movement_noise);
      // Run a reset
      DoReset();
    }

    void ResetEvolution() {
      // Purge the world!
      world->Clear();
      world->update = 0;
      // Initialize the population.
      const emp::Point<double> mid_point(world_width / 2.0, world_height / 2.0);
      int org_radius = max_organism_radius;
      Organism_t ancestor(emp::Circle<double>(mid_point, org_radius), genome_length, detach_on_birth);
      // Randomize ancestor genome.
      for (int i = 0; i < ancestor.genome.GetSize(); i++) {
        if (random->P(0.5)) ancestor.genome[i] = !ancestor.genome[i];
      }
      // TODO: make mass dependent on density
      ancestor.GetBody().SetMass(10.0);
      ancestor.SetMembraneStrength(organism_membrane_strength);
      ancestor.SetBirthTime(-1);
      world->Insert(ancestor);
      // // For testing:
      // for (int i = 0; i < 10; i++) {
      //   const emp::Point<double> loc(random->GetInt(world_width), random->GetInt(world_height));
      //   Organism_t orgie(emp::Circle<double>(loc, org_radius), genome_length, detach_on_birth);
      //   orgie.SetColorID();
      //   orgie.GetBody().SetMass(10.0);
      //   orgie.SetMembraneStrength(organism_membrane_strength);
      //   orgie.SetBirthTime(-1);
      //   world->Insert(orgie);
      // }

    }

    // Single animation step for this interface.
    void Animate(const web::Animate &anim) {
      world->Update();
      // Draw
      web::Draw(world_view.Canvas("simple-world-canvas"), world->popM.GetPhysics().GetSurfaceSet(), emp::GetHueMap(360));
      stats_view.Redraw();
    }

    // Called on start/stop button press.
    bool DoToggleRun() {
      anim.ToggleActive();
      // Grab buttons to manipulate:
      auto start_but = dashboard.Button("start_but");
      auto step_but = dashboard.Button("step_but");
      // Update buttons
      if (anim.GetActive()) {
        // If active, set button to show 'stop' button.
        start_but.Label(PAUSE_GLYPH);
        start_but.SetAttr("class", "btn btn-danger");
        step_but.Disabled(true);
      } else {
        // If inactive, set button to show 'play' option.
        start_but.Label(PLAY_GLYPH);
        start_but.SetAttr("class", "btn btn-success");
        step_but.Disabled(false);
      }
      return true;
    }

    // Called on reset button press and when initializing the experiment.
    bool DoReset() {
      ResetEvolution();
      web::Draw(world_view.Canvas("simple-world-canvas"), world->popM.GetPhysics().GetSurfaceSet(), emp::GetHueMap(360));
      stats_view.Redraw();
      return true;
    }

    // Called on step button press.
    bool DoStep() {
      emp_assert(anim.GetActive() == false);
      anim.Step();
      return true;
    }

    // Called on run experiment button press (from config exp page).
    bool DoRunExperiment() {
      // Collect parameter values.
      UpdateParams();
      // Initialize the experiment.
      InitializeExperiment();
      // Update page
      page_mode = ChangePageView(PageMode::EXPERIMENT);
      return true;
    }

    // Called on reconfigure exp button press (from run exp page).
    bool DoReconfigureExperiment() {
      // Set page mode to CONFIG.
      if (anim.GetActive()) {
        anim.ToggleActive();
        auto start_but = dashboard.Button("start_but");
        auto step_but = dashboard.Button("step_but");
        start_but.Label(PLAY_GLYPH);
        start_but.SetAttr("class", "btn btn-success");
        step_but.Disabled(false);
      }
      page_mode = ChangePageView(PageMode::CONFIG);
      return true;
    }

    // Update parameter values from webpage using javascript.
    void UpdateParams() {
      // @amlalejini QUESTION: What is the best way to be collecting parameter values?
      // TODO: clean parameters (clip to max/min values, etc).
      // TODO: switch to config system, automate this.
      // -- General --
      random_seed = EM_ASM_INT_V({ return $("#random-seed-param").val(); });
      world_width = EM_ASM_INT_V({ return $("#world-width-param").val(); });
      world_height = EM_ASM_INT_V({ return $("#world-height-param").val(); });
      // -- Population-Specific --
      max_pop_size = EM_ASM_INT_V({ return $("#max-pop-size-param").val(); });
      // -- Organism-Specific --
      genome_length = EM_ASM_INT_V({ return $("#genome-length-param").val(); });
      point_mutation_rate = EM_ASM_DOUBLE_V({ return $("#point-mutation-rate-param").val(); });
      max_organism_radius = EM_ASM_DOUBLE_V({ return $("#max-organism-radius-param").val(); });
      cost_of_repro = EM_ASM_DOUBLE_V({ return $("#cost-of-repro-param").val(); });
      detach_on_birth = EM_ASM_INT_V({ return $("#detach-on-birth-param").is(":checked"); });
      organism_membrane_strength = EM_ASM_DOUBLE_V({ return $("#organism-membrane-strength-param").val(); });
      // -- Resource-Specific --
      max_resource_age = EM_ASM_INT_V({ return $("#max-resource-age-param").val(); });
      max_resource_count = EM_ASM_INT_V({ return $("#max-resource-count-param").val(); });
      resource_radius = EM_ASM_DOUBLE_V({ return $("#resource-radius-param").val(); });
      resource_value = EM_ASM_DOUBLE_V({ return $("#resource-value-param").val(); });
      // -- Physics-Specific --
      surface_friction = EM_ASM_DOUBLE_V({ return $("#surface-friction-param").val(); });
      movement_noise = EM_ASM_DOUBLE_V({ return $("#movement-noise-param").val(); });
    }

    PageMode ChangePageView(PageMode new_mode) {
      /* Calling this function updates the view to match the current page mode. */
      switch (new_mode) {
        case PageMode::CONFIG:
          // Transition to CONFIG view.
          // Hide: dashboard, stats-view, param-view
          EM_ASM( {
            $("#exp-config-view").show();
            $("#exp-run-view").hide();
          });
          break;
        case PageMode::EXPERIMENT:
          // Transition to EXPERIMENT view.
          stats_view.Redraw();
          param_view.Redraw();
          EM_ASM( {
            $("#exp-config-view").hide();
            $("#exp-run-view").show();
          });
          break;
      }
      return new_mode;
    }
};

EvoInPhysicsInterface *interface;

int main(int argc, char *argv[]) {
  interface = new EvoInPhysicsInterface(argc, argv);
  return 0;
}
