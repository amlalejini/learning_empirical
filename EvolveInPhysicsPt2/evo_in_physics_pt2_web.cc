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

#include "web/web.h"
#include "web/Document.h"
#include "web/Animate.h"

#include "emtools/emfunctions.h"

#include "tools/Random.h"

namespace web = emp::web;

// Some useful macros:
#define PLAY_GLYPH        "<span class=\"glyphicon glyphicon-play\" aria-hidden=\"true\"></span>"
#define PAUSE_GLYPH       "<span class=\"glyphicon glyphicon-pause\" aria-hidden=\"true\"></span>"
#define REFRESH_GLYPH     "<span class=\"glyphicon glyphicon-refresh\" aria-hidden=\"true\"></span>"
#define SINGLE_STEP_GLYPH "<span class=\"glyphicon glyphicon-step-forward\" aria-hidden=\"true\"></span>"
#define SETTINGS_GLYPH    "<span class=\"glyphicon glyphicon-cog\" aria-hidden=\"true\"></span>"
#define LOLLY_GLYPH       "<span class=\"glyphicon glyphicon-ice-lolly\" aria-hidden=\"true\"></span>"

// Default experiment settings.
//  -- General Settings --
const int DEFAULT_RANDOM_SEED = 1;
const int DEFAULT_WORLD_WIDTH = 500;
const int DEFAULT_WORLD_HEIGHT = 500;
//  -- Organism-specific --
const int DEFAULT_GENOME_LENGTH = 10;
const double DEFAULT_POINT_MUTATION_RATE = 0.01;

class EvoInPhysicsInterface {
  private:
    emp::Random *random;
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
    int random_seed;
    int world_width;
    int world_height;
    int genome_length;
    double point_mutation_rate;

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
      random_seed = DEFAULT_RANDOM_SEED;
      world_width = DEFAULT_WORLD_WIDTH;
      world_height = DEFAULT_WORLD_HEIGHT;
      genome_length = DEFAULT_GENOME_LENGTH;
      point_mutation_rate = DEFAULT_POINT_MUTATION_RATE;

      // Required setup.
      random = new emp::Random(random_seed);

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
      // --- Setup Canvas. ---
      world_view << web::Canvas(1, 1, "evo-in-physics-pt2-world") << "<br>";
      // --- Setup Stats View. ---
      stats_view.SetAttr("class", "well");
      stats_view << "Update: " << "<br>";
      stats_view << "Organism Count: " << "<br>";
      stats_view << "Resource Count: " << "<br>";
      // --- Setup Config View. ---
      param_view.SetAttr("class", "well");
      param_view << "Random Seed: " << web::Live([this]() { return random_seed; }) << "<br>";

      // - Setup EXPERIMENT CONFIG mode view. -
      // -- random seed --
      exp_config << web::Button([this]() { DoRunExperiment(); }, LOLLY_GLYPH, "start_exp_but");
      auto start_exp_but = exp_config.Button("start_exp_but");
      start_exp_but.SetAttr("class", "btn btn-danger");
      exp_config << "<br>";
      exp_config << GenerateParamNumberField("Random Seed", "random-seed", random_seed);
      exp_config << GenerateParamNumberField("World Width", "world-width", world_width);
      exp_config << GenerateParamNumberField("World Height", "world-height", world_height);
      exp_config << GenerateParamNumberField("Genome Length", "genome-length", genome_length);
      exp_config << GenerateParamNumberField("Point Mutation Rate", "point-mutation-rate", point_mutation_rate);

      // Configure page view.
      UpdatePageView();
    }

    template<typename FieldType>
    std::string GenerateParamTextField(std::string field_name, std::string field_id, FieldType default_value) {
      std::stringstream param_html;
      param_html << "<div class =\"input-group\">";
        param_html << "<span class=\"input-group-addon\" id=\"" << field_id << "-addon\">" << field_name << "</span>  ";
        param_html << "<input type=\"text\" class=\"form-control\" placeholder=\"" << default_value << "\"aria-describedby=\"" << field_id << "-addon\" id=\"" << field_id << "-param\">";
      param_html << "</div>";
      return param_html.str();
    }

    template<typename FieldType>
    std::string GenerateParamNumberField(std::string field_name, std::string field_id, FieldType default_value) {
      std::stringstream param_html;
      param_html << "<div class =\"input-group\">";
        param_html << "<span class=\"input-group-addon\" id=\"" << field_id << "-addon\">" << field_name << "</span>  ";
        param_html << "<input type=\"number\" class=\"form-control\" placeholder=\"" << default_value << "\"aria-describedby=\"" << field_id << "-addon\" id=\"" << field_id << "-param\">";
      param_html << "</div>";
      return param_html.str();
    }

    void Animate(const web::Animate &anim) {
      /* Single animation step for this interface. */
      std::cout << "Animate!" << std::endl;
    }

    bool DoToggleRun() {
      /* Called on start/stop button press. */
      std::cout << "Do Toggle Run!" << std::endl;
      return true;
    }

    bool DoReset() {
      /* Called on reset button press. */
      std::cout << "Do Reset!" << std::endl;
      return true;
    }

    bool DoStep() {
      /* Called on step button. */
      std::cout << "Do Step!" << std::endl;
      return true;
    }

    bool DoRunExperiment() {
      /* Called after experiment configuration. */
      std::cout << "Do Run Experiment!" << std::endl;
      // Collect parameter values.
      // @amlalejini: What is the best way to be collecting parameter values?
      random_seed = EM_ASM_INT_V({ return $("#random-seed-param").val(); });
      world_width = EM_ASM_INT_V({ return $("#world-width-param").val(); });
      world_height = EM_ASM_INT_V({ return $("#world-height-param").val(); });
      genome_length = EM_ASM_INT_V({ return $("#genome-length-param").val(); });
      point_mutation_rate = EM_ASM_INT_V({ return $("#point-mutation-rate-param").val(); });
      // TODO: clean parameters (clip to max/min values, etc).
      //                     ------------
      // Set page mode to EXPERIMENT.
      page_mode = PageMode::EXPERIMENT;
      UpdatePageView();
      return true;
    }

    bool DoReconfigureExperiment() {
      /* Called on experiment reconfiguration. */
      std::cout << "Do Reconfigure Experiment!" << std::endl;
      // Set page mode to CONFIG.
      page_mode = PageMode::CONFIG;
      UpdatePageView();
      return true;
    }

    void UpdatePageView() {
      /* Calling this function updates the view to match the current page mode. */
      switch (page_mode) {
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
          EM_ASM( {
            $("#exp-config-view").hide();
            $("#exp-run-view").show();
          });
          break;
      }
    }

};

EvoInPhysicsInterface *interface;

int main(int argc, char *argv[]) {
  interface = new EvoInPhysicsInterface(argc, argv);
  return 0;
}
