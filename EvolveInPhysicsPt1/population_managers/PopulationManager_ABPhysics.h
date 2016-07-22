/*
 PopulationManager_ABPhysics.h
  * Implements population manager for my AB nutrient environment.

*/

#ifndef POPULATION_MANAGER_ABPHYSICS_H
#define POPULATION_MANAGER_ABPHYSICS_H

#include <iostream>
#include <functional>

#include "../modified_geometry/ABPhysics2D.h"
#include "../modified_geometry/Surface2D.h"
#include "../modified_geometry/Angle2D.h"

#include "evo/PopulationManager.h"
#include "tools/vector.h"

/////////////////////////////
// Learning notes
//  * Functionality required of population managers:
//    - AddOrg, AddOrgBirth, Print,
//    - needs to maintain list of organisms
//  * Functionality required of Physics2D environment:
//    - Collision testing
//    - Update
//    - needs to maintain a Surface of bodies (organisms/[nutrients eventually])
////////
// DEV NOTES
//  * I do not like the current relationship between physics2d and this population manager.
//    - Also related, I do not like the relationship between physics organism and CircleBody2D
/////////////////////////////
// TODO:
// -- Assert that organism is a physics body

namespace emp {
namespace evo {

// Template must both be a physics body organism
template <typename ORG>
class PopulationManager_ABPhysics {
  protected:
    using ptr_t = ORG*;

    ABPhysics2D<ORG> physics;
    Random *random_ptr;
    // TODO: pull these in from outside pop manager
    const double pop_pressure = 1.0;
    int minimum_population_size;
    double repro_prob;
    double drift;
    double max_org_radius;

  public:
    PopulationManager_ABPhysics()
      : physics(),
        repro_prob(0.003),
        drift(0.15),
        max_org_radius(4.0),
        minimum_population_size(1)
    {
      ;
    }

    ~PopulationManager_ABPhysics() { ; }

    // // Allow this and derived classes to be identified as a population manager
    static constexpr bool emp_is_population_manager = true;
    static constexpr bool emp_has_separate_generations = false;
    using value_type = ORG*;
    // // Setup iterator for population
    friend class PopulationIterator<PopulationManager_ABPhysics<ORG> >;
    using iterator = PopulationIterator<PopulationManager_ABPhysics<ORG> >;
    // Necessary overloads
    ptr_t & operator[](int i) { return physics.GetBodySet()[i]; }
    //const ptr_t
    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, physics.GetBodySet().size()); }

    uint32_t size() const { return physics.GetConstBodySet().size(); }
    void Clear() { physics.Clear(); }
    int GetSize() const { return (int) this->size(); }

    void SetRandom(Random *r) { random_ptr = r; }


    void ConfigPop(double width, double height, double max_org_diameter = 20, bool detach = true, int min_pop_size = 1) {
      /*
        Configure the population manager (and the underlying physics) given the following parameters:
          * width: width of physics world (in world units)
          * height: height of physics world (in world units)
          * max_org_diameter: max organism diameter (in world units)
          * detach: should organisms be attached or detached to parent at birth?
      */
      // Configure population-specific variables
      minimum_population_size = min_pop_size;
      // Configure the physics
      physics.ConfigPhysics(width, height, max_org_diameter, detach);
    }

    ABPhysics2D<ORG> & GetPhysics() { return physics; }

    int AddOrg(ORG *new_org) {
      int pos = this->GetSize();
      physics.AddBody(new_org);
      return pos;
    }

    void Update() {
      /* Calling this executes 1 tick of the world. */
      // Take a single timestep on the world physics
      physics.Update();

      // Test which organisms should replicate
      auto &pop = physics.GetBodySet();
      vector<ORG *> new_organisms;

      // Loop through all bodies to see which ones should replicate.
      for (auto *body : pop) {
        // if not an organism, continue
        // Add a small amount of Brownian motion
        body->IncSpeed(Angle(random_ptr->GetDouble() * (2.0 * emp::PI)).GetPoint(drift));
        // Organisms cannot reproduce if:
        //  * They are already reproducing
        //  * They are under too much pressure
        //  * They are attached to too many bodies
        if (body->IsReproducing()
            || (body->GetPressure() > pop_pressure)) continue;

        // Reproduction would happen here
        // TODO: right now it's based on random chance of repro, need to make it fitness-based (when I actually have fitness values)
        if (random_ptr->P(repro_prob) || ( (int) pop.size() < minimum_population_size) ) {
          emp::Angle repro_angle(random_ptr->GetDouble(2.0 * emp::PI)); // What angle should we put the offspring at?
          auto *baby_org = body->BuildOffspring(repro_angle.GetPoint(0.1));
          new_organisms.push_back(baby_org);  // Mark this baby org to be added to the world.
        }
      }
      // Adding new bodies to world would happen here
      for (auto new_organism : new_organisms) {
        AddOrg(new_organism);
      }
    }

};

}
}

#endif
