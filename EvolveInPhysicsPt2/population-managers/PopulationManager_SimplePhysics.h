/*
  PopulationManager_SimplePhysics.h
*/

#ifndef POPULATION_MANAGER_SIMPLE_PHYSICS_H
#define POPULATION_MANAGER_SIMPLE_PHYSICS_H

#include <iostream>
#include <limits>

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
// TODO: Speed up body removal when removing body owner.
// TODO: make dummy default collision callback, allow users to register collision callbacks to be registered with physics.

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
      // TODO: allow collision callbacks (multiple?) to be registered.
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
      std::cout << "Physics collision callback@!" << std::endl;
      const bool is_resource = (body1->GetOwnerID() == RESOURCE_TYPE_ID || body2->GetOwnerID() == RESOURCE_TYPE_ID);
      const bool is_org = (body1->GetOwnerID() == ORG_TYPE_ID || body2->GetOwnerID() == ORG_TYPE_ID);
      if (is_resource && is_org) {
        // This is the special case where a resource body and org body collide.
        Resource_t *res;
        Org_t *org;
        if (body1->GetOwnerID() == ORG_TYPE_ID) {
          org = static_cast<Org_t*>(body1->GetOwnerPtr());
          res = static_cast<Resource_t*>(body2->GetOwnerPtr());
        } else {
          org = static_cast<Org_t*>(body2->GetOwnerPtr());
          res = static_cast<Resource_t*>(body1->GetOwnerPtr());
        }
        // If organism manages to eat resource and they are not already linked,
        // add a link org--->res.
        if (random_ptr->P(org->GetResourceConsumptionProb(*res))) {
          // Organism consumes resource!
          if (!org->GetBody().IsLinked(res->GetBody())) {
            double strength;
            // TODO: repeated math, can speed up!
            const double sq_pair_dist = (body1->GetCenter() - body2->GetCenter()).SquareMagnitude();
            const double radius_sum = body1->GetRadius() + body2->GetRadius();
            const double sq_min_dist = radius_sum * radius_sum;
            // strength is a function of how close the two organisms are
            sq_pair_dist == 0.0 ? strength = std::numeric_limits<double>::max() : strength = sq_min_dist / sq_pair_dist;
            // Add link FROM org TO resource.
            org->GetBody().AddLink(LINK_TYPE::CONSUME_RESOURCE, res->GetBody(), sqrt(sq_pair_dist), sqrt(sq_min_dist), strength);
          }
          body1->ResolveCollision();
          body2->ResolveCollision();
        }
      }
    }

    // Progress time by one step.
    void Update() {
      std::cout << "POPM UPdate ===================================" << std::endl;
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
          delete org;
          cur_size--;
          population[cur_id] = population[cur_size];
          continue;
        }
        // Organism has body, proceed.
        if (!org->GetBody().ExceedsStressThreshold() && org->GetEnergy() >= cost_of_repro) {
          auto *baby_org = org->Reproduce(random_ptr, point_mutation_rate, cost_of_repro);
          baby_org->GetBody().SetMass(10.0);
          new_organisms.push_back(baby_org);
        }
        // Add some noise to movement.
        org->GetBody().IncSpeed(Angle(random_ptr->GetDouble() * (2.0 * emp::PI)).GetPoint(movement_noise));
        cur_id++;
      }
      population.resize(cur_size);
      // Add new offspring to population.
      int total_size = (int)(population.size() + new_organisms.size());
      if (total_size > max_pop_size) {
        // Cull population to make room for new organisms.
        int new_size = ((int) population.size()) - (total_size - max_pop_size);
        emp::Shuffle<Org_t *>(*random_ptr, population, new_size);
        for (int i = new_size; i < (int) population.size(); i++) {
          //physics.RemoveOrgBody(population[i]->GetBodyPtr());
          delete population[i];
        }
        population.resize(new_size);
      }
      for (auto *new_organism : new_organisms) {
        AddOrg(new_organism);
      }
      // Manage resources.
      cur_size = GetNumResources();
      cur_id = 0;
      while (cur_id < cur_size) {
        std::cout << "-- managing a resource (popm) --" << std::endl;
        Resource_t *resource = resources[cur_id];
        // Remove resources with no body.
        if (!resource->HasBody()) {
          std::cout << "Res no body. Delete." << std::endl;
          delete resource;
          cur_size--;
          resources[cur_id] = resources[cur_size];
          continue; // Done handling this resource.
        }
        // Resource has body, proceed.
        // Grab consumption links TO this resource.
        auto consumption_links = resource->GetBody().GetLinksToByType(LINK_TYPE::CONSUME_RESOURCE);
        // Is anyone trying to consume this resource?
        if ((int) consumption_links.size() > 0) {
          // Find the strongest link!
          auto *max_link = consumption_links[0];
          for (auto *link : consumption_links) {
            if (link->link_strength > max_link->link_strength) max_link = link;
          }
          // Feed resource to the strongest link (if it's an organism)!
          if (max_link->from->GetOwnerID() == ORG_TYPE_ID) {
            Org_t *hungry_org = static_cast<Org_t*>(max_link->from->GetOwnerPtr());
            hungry_org->ConsumeResource(*resource);
            // Remove the consumed resource.
            // TODO: this is inefficient. Should work to make this easier. (flags of some sort?)
            //physics.RemoveResourceBody(resource->GetBodyPtr());
            delete resource;
            cur_size--;
            resources[cur_id] = resources[cur_size];
            continue; // Done handling this resource.
          }
        }
        // Check aging.
        std::cout << "Res age: " << resource->GetAge() << std::endl;
        if (resource->GetAge() > max_resource_age) {
          std::cout << "deleting an old resource" << std::endl;
          //physics.RemoveResourceBody(resource->GetBodyPtr());
          delete resource;
          cur_size--;
          resources[cur_id] = resources[cur_size];
          continue;
        }
        resource->IncAge();
        // Add some noise to movement.
        resource->GetBody().IncSpeed(Angle(random_ptr->GetDouble() * (2.0 * emp::PI)).GetPoint(movement_noise));
        cur_id++;
      }
      resources.resize(cur_size);
      // Pump resources in as necessary to max capacity.
      std::cout << "Adding new resources..\nNum resources? " << GetNumResources() << std::endl;
      std::cout << "RS size: " << physics.GetResourceBodySet().size() << std::endl;
      while (GetNumResources() < max_resource_count) {
        std::cout <<"--------Adding a new resource-------" << std::endl;
        emp_assert((physics.GetWidth() > resource_radius * 2.0) && (physics.GetHeight() > resource_radius * 2.0));
        emp::Point<double> res_loc(random_ptr->GetDouble(resource_radius, physics.GetWidth() - resource_radius), random_ptr->GetDouble(resource_radius, physics.GetHeight() - resource_radius));
        Resource_t *new_resource = new Resource_t(emp::Circle<double>(res_loc, resource_radius));
        new_resource->SetValue(resource_value);
        // TODO: make the below values not magic numbers.
        new_resource->SetColorID(180);
        new_resource->GetBody().SetMass(1);
        std::cout << "new res has body? " << new_resource->HasBody() << std::endl;
        int p = AddResource(new_resource);
        std::cout << "new res has body2? " << resources[p]->HasBody() << std::endl;
      }
      std::cout << "...done adding new resources. " <<  std::endl;
      std::cout << "RS size: " << physics.GetResourceBodySet().size() << std::endl;
    }
};

}
}

#endif
