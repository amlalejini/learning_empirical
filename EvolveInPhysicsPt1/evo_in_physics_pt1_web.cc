/*
  evo_in_physics_pt1_web.cc
    * Web version of evo_in_physics_pt1.cc. See that file for description.
  Still me learning how the old Evoke code works.
*/

#include "population_managers/PopulationManager_ABPhysics.h"
#include "organisms/ABPhysicsOrganism.h"

#include "web/Document.h"
#include "web/keypress.h"
#include "web/web.h"
#include "web/Animate.h"

#include "evo/World.h"
#include "tools/Random.h"

namespace web = emp::web;

// For now, use these parameters.
const int RANDOM_SEED = 101;

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
    web::Animate anim;
    web::KeypressManager keypress_manager;

    enum class MapMode { BLANK, MAKE_BLANK, BASIC } map_mode;

  public:
    EvoInPhysicsInterface()
      : random(RANDOM_SEED),
        world(random, "EvoInPhysicsPt1"),
        dashboard("dashboard-panel-body"),
        world_view("world-view"),
        anim([this]() { EvoInPhysicsInterface::Animate(anim); }),
        map_mode(MapMode::BASIC)
    {
      std::cout << "Interface constructor." << std::endl;
      // Link keypresses to the proper handlers
      keypress_manager.AddKeydownCallback(std::bind(&EvoInPhysicsInterface::OnKeydown, this, _1));

    }

    void Animate(const web::Animate &anim) {
      std::cout << "Animate!" << std::endl;
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
