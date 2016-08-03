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
#include "../modified_geometry/Body2D.h"

#include "../nutrients/ABPhysicsNutrient.h"

#include "evo/PopulationManager.h"
#include "tools/vector.h"
#include "tools/random_utils.h"

/////////////////////////////
// Learning notes
//  * Functionality required of population managers:
//    - AddOrg, AddOrgBirth, Print,
//    - needs to maintain list of organisms
//  * Functionality required of Physics2D environment:
//    - Collision testing
//    - Update
//    - needs to maintain a Surface of bodies (organisms/[nutrients eventually])
/////////////////////////////
// TODO:
// -- Assert that organism is a physics body
// -- Bad: Many things in physics bodies should be moved to organisms (Charles was formally using them as organisms, not just physics bodies)
namespace emp {
namespace evo {

// Template must both be a physics body organism
template <typename ORG>
class PopulationManager_ABPhysics {
  protected:
    using ptr_t = ORG*;
    using RESOURCE = ABPhysicsNutrient;

    ABPhysics2D<ORG, RESOURCE> physics;

    Random *random_ptr;
    // TODO: pull these in from outside pop manager
    const double pop_pressure = 1.0;
    int minimum_population_size;
    int maximum_population_size;
    int resource_count;
    int max_resource_age;
    double reproduction_prob;
    double drift;
    double max_organism_radius;
    double reproduction_cost;
    double mutation_rate;

    int best_ones;
    int best_zeros;

  public:
    PopulationManager_ABPhysics()
      : physics(),
        minimum_population_size(1),
        maximum_population_size(100),
        resource_count(0),
        max_resource_age(100),
        reproduction_prob(0.003),
        drift(0.15),
        max_organism_radius(4.0),
        reproduction_cost(10.0),
        mutation_rate(0.01),
        best_ones(-1),
        best_zeros(-1)
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
    ptr_t & operator[](int i) { return physics.GetOrgBodySet()[i]; }
    //const ptr_t
    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, physics.GetOrgBodySet().size()); }

    uint32_t size() const { return physics.GetConstOrgBodySet().size(); }
    int GetSize() const { return (int) this->size(); }
    int GetNumResources() const {return (int) physics.GetConstResourceBodySet().size(); }
    int GetBestOnes() const { return best_ones; }
    int GetBestZeros() const { return best_zeros; }
    void SetRandom(Random *r) { random_ptr = r; }

    void Clear() { physics.Clear(); }

    void ConfigPop(double width, double height, double max_org_radius = 20, bool detach = true, int min_pop_size = 1, int max_pop_size = 100, int res_cnt = 0, int max_res_age = 10, double repro_cost = 10.0, double repro_prob = 0.003, double mut_rate = 0.01) {
      /*
        Configure the population manager (and the underlying physics) given the following parameters:
          * width: width of physics world (in world units)
          * height: height of physics world (in world units)
          * max_org_radius: max organism radius (in world units)
          * detach: should organisms be attached or detached to parent at birth?
      */
      // Configure population-specific variables
      minimum_population_size = min_pop_size;
      maximum_population_size = max_pop_size;
      resource_count = res_cnt;
      max_resource_age = max_res_age;
      max_organism_radius = max_org_radius;
      reproduction_cost = repro_cost;
      reproduction_prob = repro_prob;
      mutation_rate = mut_rate;
      // Configure the physics
      physics.ConfigPhysics(width, height, random_ptr, max_organism_radius, detach, max_resource_age);
    }

    ABPhysics2D<ORG, RESOURCE> & GetPhysics() { return physics; }

    int AddOrg(ORG *new_org) {
      // Returns position in physics?
      int pos = this->GetSize();
      physics.AddOrg(new_org);   // Add to physics body set.
      return pos;
    }

    int AddResource(RESOURCE *new_res) {
      int pos = this->GetNumResources();
      physics.AddResource(new_res);           // Add to physics body set.
      return 0;
    }

    void Update() {
      /* Calling this executes 1 tick of the world. */
      // Take a single timestep on the world physics
      physics.Update(); // update physics (which may remove things?)
      // Test which organisms should replicate
      auto &pop = physics.GetOrgBodySet();
      auto &resources = physics.GetResourceBodySet();
      vector<ORG *> new_organisms;
      best_ones = -1;   // How many 1's does the best 1 org in the population have?
      best_zeros = -1;  // How many 0's does the best 0 org in the population have?
      // Loop through all bodies to see which ones should replicate.
      for (auto *org : pop) {
        // Add a small amount of Brownian motion
        org->IncSpeed(Angle(random_ptr->GetDouble() * (2.0 * emp::PI)).GetPoint(drift));
        // Update organism color based on energy levels! (ALERT! MAGIC NUMBER HERE)
        int num_ones = org->genome.CountOnes();
        int num_zeros = org->genome.GetSize() - num_ones;
        if (num_ones > best_ones) best_ones = num_ones;
        if (num_zeros > best_zeros) best_zeros = num_zeros;
        org->SetColorID((int)((num_ones / (double) org->genome.GetSize()) * 200));  // This should happen elsewhere, pop manager doesn't care about drawing... But for now, this is easy.
        // Organisms cannot reproduce if:
        //  * They are already reproducing
        //  * They are under too much pressure
        //  * They are attached to too many bodies
        if (org->IsReproducing()
            || (org->GetPressure() > pop_pressure)) continue;

        // Reproduction happens here.
        // If organism has enough energy, reproduce with Probability(repro_prob) || for sure reproduce if pop size is below minimum.
        if ( (org->GetEnergy() >= reproduction_cost) ) {
          emp::Angle repro_angle(random_ptr->GetDouble(2.0 * emp::PI)); // What angle should we put the offspring at?
          auto *baby_org = org->Reproduce(repro_angle.GetPoint(0.1), random_ptr, reproduction_cost, mutation_rate);
          baby_org->SetMass(5.0);
          new_organisms.push_back(baby_org);  // Mark this baby org to be added to the world.
        }
      } // end population loop
      // Adding new bodies to world would happen here
      // Make room in population for new organisms.
      if ((int)(pop.size() + new_organisms.size()) > maximum_population_size) {
        // We need to make room.
        int needed_room = (int)(pop.size() + new_organisms.size()) - maximum_population_size;
        int new_size = (int)pop.size() - needed_room;
        emp::Shuffle<ORG *>(*random_ptr, pop, new_size);
        // Delete all excess organisms, resize population.
        for (int i = new_size; i < (int) pop.size(); ++i) delete pop[i];
        pop.resize(new_size);
      }
      for (auto new_organism : new_organisms) {
        AddOrg(new_organism);
      }

      // Move the resources round (TODO: surface should define flow force matrix -- precalculated static forces for each location)
      for (auto *res : resources) {
        res->IncSpeed(Angle(random_ptr->GetDouble() * (2.0 * emp::PI)).GetPoint(drift * 0.5));  // Some brownian motion for resources
      }
      // While there aren't enough resources in the environment, add more randomly
      while (GetNumResources() < resource_count) {
        emp::Point<double> res_center(random_ptr->GetDouble(10.0, physics.GetWidth() - 10.0), random_ptr->GetDouble(10.0, physics.GetHeight()));
        RESOURCE *new_resource = new RESOURCE(emp::Circle<double>(emp::Circle<double>(res_center, 5)));
        new_resource->SetValue(1);
        if (random_ptr->P(0.5)) {
          // 1 resource
          new_resource->SetType(1);
          new_resource->SetColorID(200);
          new_resource->SetMass(1.0);
        } else {
          // 0 resource
          new_resource->SetType(0);
          new_resource->SetColorID(0);
          new_resource->SetMass(1.0);
        }
        AddResource(new_resource);
      }
    }
};

}
}

#endif
