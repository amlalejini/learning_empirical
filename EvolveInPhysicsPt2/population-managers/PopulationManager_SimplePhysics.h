/*
  PopulationManager_SimplePhysics.h
*/

#ifndef POPULATION_MANAGER_SIMPLE_PHYSICS_H
#define POPULATION_MANAGER_SIMPLE_PHYSICS_H

#include <iostream>

#include "../geometry/Physics2D.h"
#include "../resources/SimpleResource.h"

#include "evo/PopulationManager.h"
#include "tools/vector.h"
#include "tools/random_utils.h"

/////////////////////////
// Population Manager
//  * Responsibilities:
//    - Organism removal
//    - Organism placement
//    - Resource removal
//    - Resource placement
//    - Population structure
/////////////////////////

namespace emp {
namespace evo {

template <typename ORG>
class PopulationManager_SimplePhysics {
  protected:
    // TODO How do I guarantee that ORG has a PHYSICS_BODY_TYPE?
    using Org_t = ORG;                  // Just here for consistency
    using Resource_t = SimpleResource;
    using PhysicsBody_t = CircleBody2D;
    using Physics_t = SimplePhysics2D<SimpleResource, ORG>;
    // TODO
    Physics_t physics;
    emp::vector<Org_t*> population;
    emp::vector<Resource_t*> resources;

    Random *random_ptr;   // This comes from world. PopManager does not own random_ptr.

    // Population manager parameters.
    int max_pop_size;
    double point_mutation_rate;
    double max_organism_radius;
    double cost_of_repro;

    int max_resource_age;
    int max_resource_count;
    double resource_radius;
    double resource_value;

    double movement_noise;

    // Useful things to not have to look up all of the time.
    static constexpr int RESOURCE_TYPE_ID = Physics_t::template GetTypeID<Resource_t>();
    static constexpr int ORG_TYPE_ID = Physics_t::template GetTypeID<ORG>();

  public:
    PopulationManager_SimplePhysics()
      : physics(),
        max_pop_size(1),
        point_mutation_rate(0.0075),
        max_organism_radius(1.0),
        cost_of_repro(1.0),
        max_resource_age(1),
        max_resource_count(1),
        resource_radius(1.0),
        resource_value(1.0),
        movement_noise(0.1)
    {
      physics.RegisterCollisionCallback([this](PhysicsBody_t *b1, PhysicsBody_t *b2) { this->PhysicsCollisionCallback(b1, b2); });
    }

    ~PopulationManager_SimplePhysics() { ; }

    // Allow this and derived classes to be identified as a population manager:
    static constexpr bool emp_is_population_manager = true;
    static constexpr bool emp_has_separate_generations = false;
    using value_type = Org_t*;

    // Setup iterator for the population.
    friend class PopulationIterator<PopulationManager_SimplePhysics<Org_t> >;
    using iterator = PopulationIterator<PopulationManager_SimplePhysics<Org_t> >;
    // Operator overloading.
    Org_t* & operator[](int i) { return population[i]; }

    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, population.size()); }

    uint32_t size() const { return population.size(); }
    int GetSize() const { return (int) this->size(); }
    int GetNumResources() const { return (int) resources.size(); }
    Physics_t & GetPhysics() { return physics; }

    // Add new organism. Return position in population.
    int AddOrg(Org_t *new_org) {
      int pos = this->GetSize();
      population.push_back(new_org);
      physics.AddOrgBody(new_org, new_org->GetBodyPtr());
      return pos;
    }

    int AddResource(Resource_t *new_res) {
      int pos = this->GetNumResources();
      resources.push_back(new_res);
      physics.AddResourceBody(new_res, new_res->GetBodyPtr());
      return pos;
    }

    void Setup(Random *r) { random_ptr = r; }

    void Clear() {
      physics.Clear();
      population.clear();
      resources.clear();
    }

    void ConfigPop(double width, double height, double surface_friction,
                   int max_pop_size, double point_mutation_rate, double max_organism_radius,
                   double cost_of_repro, int max_resource_age, int max_resource_count,
                   double resource_radius, double resource_value, double movement_noise) {
      // Config pop-specific variables.
      this->max_pop_size = max_pop_size;
      this->point_mutation_rate = point_mutation_rate;
      this->max_organism_radius = max_organism_radius;
      this->cost_of_repro = cost_of_repro;
      this->max_resource_age = max_resource_age;
      this->max_resource_count = max_resource_count;
      this->resource_radius = resource_radius;
      this->resource_value = resource_value;
      this->movement_noise = movement_noise;
      // Config the physics.
      physics.ConfigPhysics(width, height, random_ptr, surface_friction);
    }

    // Physics collision callback (called when )
    void PhysicsCollisionCallback(PhysicsBody_t *body1, PhysicsBody_t *body2) {
      if (body1->GetOwnerID() == physics.template GetTypeID<Resource_t>()) std::cout << "Body1 is a Resource." << std::endl;
      if (body2->GetOwnerID() == physics.template GetTypeID<Resource_t>()) std::cout << "Body2 is a Resource." << std::endl;
      if (body1->GetOwnerID() == physics.template GetTypeID<ORG>()) std::cout << "Body1 is an Org." << std::endl;
      if (body2->GetOwnerID() == physics.template GetTypeID<ORG>()) std::cout << "Body2 is an Org." << std::endl;

      if (body1->GetOwnerID() == RESOURCE_TYPE_ID && body2->GetOwnerID() == ORG_TYPE_ID) {
        std::cout << "Special case! bod1 is res & bod2 is org" << std::endl;
      } else if (body1->GetOwnerID() == ORG_TYPE_ID && body2->GetOwnerID() == RESOURCE_TYPE_ID) {
        std::cout << "Special case! bod1 is org & bod2 is res" << std::endl;
      } else {
        return;
      }



    }

    // Progress time by one step.
    void Update() {
      // Progress physics (progress each physics body a single time step).
      physics.Update();
      emp::vector<Org_t *> new_organisms;
      // TODO: Manage population.
      int cur_size = GetSize();
      int cur_id = 0;
      while (cur_id < cur_size) {
        Org_t *org = population[cur_id];
        // Remove organisms with no body.
        if (!org->HasBody()) {
          std::cout << "This org has no body!" << std::endl;
          delete org;
          cur_size--;
          population[cur_id] = population[cur_size];
          continue;
        }
        // Organism has body, proceed.
        // TODO: Feed e'body.
        // TODO: Reproduction.
        // Add some noise to movement.
        org->GetBody().IncSpeed(Angle(random_ptr->GetDouble() * (2.0 * emp::PI)).GetPoint(movement_noise));
        cur_id++;
      }
      population.resize(cur_size);

      // Manage resources.
      cur_size = GetNumResources();
      cur_id = 0;
      while (cur_id < cur_size) {
        Resource_t *resource = resources[cur_id];
        // Remove resources with no body.
        if (!resource->HasBody()) {
          delete resource;
          cur_size--;
          resources[cur_id] = resources[cur_size];
          continue;
        }
        // Resource has body, proceed.
        // Add some noise to movement.
        resource->GetBody().IncSpeed(Angle(random_ptr->GetDouble() * (2.0 * emp::PI)).GetPoint(movement_noise));
        cur_id++;
      }
      resources.resize(cur_size);
      // Pump resources in as necessary to max capacity.
      while (GetNumResources() < max_resource_count) {
        emp_assert((physics.GetWidth() > resource_radius * 2.0) && (physics.GetHeight() > resource_radius * 2.0));
        emp::Point<double> res_loc(random_ptr->GetDouble(resource_radius, physics.GetWidth() - resource_radius), random_ptr->GetDouble(resource_radius, physics.GetHeight() - resource_radius));
        Resource_t *new_resource = new Resource_t(emp::Circle<double>(res_loc, resource_radius));
        new_resource->SetValue(resource_value);
        // TODO: make the below values not magic numbers.
        new_resource->SetColorID(180);
        new_resource->GetBody().SetMass(1);
        AddResource(new_resource);
      }

    }


};

}
}

#endif
