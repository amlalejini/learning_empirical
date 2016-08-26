/*
  PopulationManager_SimplePhysics.h
*/

#ifndef POPULATION_MANAGER_SIMPLE_PHYSICS_H
#define POPULATION_MANAGER_SIMPLE_PHYSICS_H

#include <iostream>
#include <limits>

#include "physics/Physics2D.h"
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
    using Physics_t = CirclePhysics2D<Body<Circle, SimpleResource>, Body<Circle, SimpleOrganism>>;
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
    int resource_in_flow_rate;  // how many resources per update will the resources restock?
    double resource_radius;
    double resource_value;

    double movement_noise;

    // Useful things to not have to look up all of the time.
    static constexpr int RESOURCE_PHYSICS_BODY_TYPE_ID = Physics_t::template GetTypeID<Body<Circle, SimpleResource>>();
    static constexpr int ORG_PHYSICS_BODY_TYPE_ID = Physics_t::template GetTypeID<Body<Circle, SimpleOrganism>>();

  public:
    PopulationManager_SimplePhysics()
      : physics(),
        max_pop_size(1),
        point_mutation_rate(0.0075),
        max_organism_radius(1.0),
        cost_of_repro(1.0),
        max_resource_age(1),
        max_resource_count(1),
        resource_in_flow_rate(1),
        resource_radius(1.0),
        resource_value(1.0),
        movement_noise(0.1)
    {
      physics.RegisterOnCollisionCallback([this](Body2D_Base *body1, Body2D_Base *body2) { this->OnCollisionCallback(body1, body2); });
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
      physics.AddBody(new_org->GetBodyPtr());
      return pos;
    }

    int AddResource(Resource_t *new_res) {
      int pos = this->GetNumResources();
      resources.push_back(new_res);
      physics.AddBody(new_res->GetBodyPtr());
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
                   int resource_in_flow_rate, double resource_radius, double resource_value,
                   double movement_noise) {
      // Config pop-specific variables.
      this->max_pop_size = max_pop_size;
      this->point_mutation_rate = point_mutation_rate;
      this->max_organism_radius = max_organism_radius;
      this->cost_of_repro = cost_of_repro;
      this->max_resource_age = max_resource_age;
      this->max_resource_count = max_resource_count;
      this->resource_in_flow_rate = resource_in_flow_rate;
      this->resource_radius = resource_radius;
      this->resource_value = resource_value;
      this->movement_noise = movement_noise;
      // Config the physics.
      physics.ConfigPhysics(width, height, random_ptr, surface_friction);
    }

    // Called on physics on_collision_sig trigger.
    void OnCollisionCallback(Body2D_Base *body1, Body2D_Base *body2) {
      const bool is_resource = (body1->GetPhysicsBodyTypeID() == RESOURCE_PHYSICS_BODY_TYPE_ID || body2->GetPhysicsBodyTypeID() == RESOURCE_PHYSICS_BODY_TYPE_ID);
      const bool is_org = (body1->GetPhysicsBodyTypeID() == ORG_PHYSICS_BODY_TYPE_ID || body2->GetPhysicsBodyTypeID() == ORG_PHYSICS_BODY_TYPE_ID);
      if (is_resource && is_org) {
        // Special case where a resource body and an org body collide.
        using ResBody_t = Body<Circle, SimpleResource>;
        using OrgBody_t = Body<Circle, SimpleOrganism>;
        ResBody_t *res_body;
        OrgBody_t *org_body;

        if (body1->GetPhysicsBodyTypeID() == ORG_PHYSICS_BODY_TYPE_ID) {
          org_body = static_cast<OrgBody_t*>(body1);
          res_body = static_cast<ResBody_t*>(body2);
        } else {
          org_body = static_cast<OrgBody_t*>(body2);
          res_body = static_cast<ResBody_t*>(body1);
        }
        Resource_t *res = res_body->GetBodyOwnerPtr();
        Org_t *org = org_body->GetBodyOwnerPtr();
        // If an organism manages to eat a resource and they are not already linked, add a link
        // org_body---->res_body.
        if (random_ptr->P(org->GetResourceConsumptionProb(*res))) {
          if (!org_body->IsLinked(*res_body)) {
            double strength;
            // TODO: repeated math -- can speed up by having a collision info struct.
            const double sq_pair_dist = (body1->GetShape().GetCenter() - body2->GetShape().GetCenter()).SquareMagnitude();
            const double radius_sum = body1->GetShape().GetRadius() + body2->GetShape().GetRadius();
            const double sq_min_dist = radius_sum * radius_sum;
            // Strength is a function of how close the two organisms are.
            sq_pair_dist == 0.0 ? strength = std::numeric_limits<double>::max() : strength = sq_min_dist / sq_pair_dist;
            // Add link FROM org TO resource.
            org_body->AddLink(BODY_LINK_TYPE::CONSUME_RESOURCE, *res_body, sqrt(sq_pair_dist), sqrt(sq_min_dist), strength);
          }
          // If resource was consumed, mark collision as resolved.
          body1->ResolveCollision();
          body2->ResolveCollision();
        }
      }
    }

    // Progress time by one step.
    void Update() {
      physics.Update();
      // Place for new organisms.
      emp::vector<Org_t*> new_organisms;
      // Manage the population.
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
        // Reproduction. Organisms that have sufficient energy and are not too stressed may reproduce.
        if (!org->GetBody().ExceedsStressThreshold() && org->GetEnergy() >= cost_of_repro) {
          auto *baby_org = org->Reproduce(random_ptr, point_mutation_rate, cost_of_repro);
          new_organisms.push_back(baby_org);
        }
        // Add some noise to movement.
        org->GetBody().IncSpeed(Angle(random_ptr->GetDouble() * (2.0 * emp::PI)).GetPoint(movement_noise));
        cur_id++;
      }
      population.resize(cur_size);
      // Cull population to make room for new offspring.
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
      // Add new organisms to the population.
      for (auto * new_organism : new_organisms) {
        AddOrg(new_organism);
      }
      // Manage the resources.
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
        // Handle resource consumption.
        auto consumption_links = resource->GetBody().GetLinksToByType(BODY_LINK_TYPE::CONSUME_RESOURCE);
        if ((int) consumption_links.size() > 0) {
          // Find the strongest link!
          auto *max_link = consumption_links[0];
          for (auto *link : consumption_links) {
            if (link->link_strength > max_link->link_strength) max_link = link;
          }
          // Feed resource to the strongest link.
          //  * When an organism eats a resource...
          if (max_link->from->GetPhysicsBodyTypeID() == ORG_PHYSICS_BODY_TYPE_ID) {
            Body<Circle, Org_t> *hungry_org_body = static_cast<Body<Circle, Org_t>*>(max_link->from);
            //Org_t *hungry_org = static_cast<Org_t*>(max_link->from->GetOwnerPtr());
            hungry_org_body->GetBodyOwnerPtr()->ConsumeResource(*resource);
            delete resource;
            cur_size--;
            resources[cur_id] = resources[cur_size];
            continue;
          }
        }
        // Check resource aging.
        if (resource->GetAge() > max_resource_age) {
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
      // Pump resources (at inflow rate) in as necessary to max capacity.
      for (int i = 0; (i < resource_in_flow_rate) && (GetNumResources() < max_resource_count); i++) {
        emp_assert((physics.GetWidth() > resource_radius * 2.0) && (physics.GetHeight() > resource_radius * 2.0));
        Point<double> res_loc(random_ptr->GetDouble(resource_radius, physics.GetWidth() - resource_radius),
                                   random_ptr->GetDouble(resource_radius, physics.GetHeight() - resource_radius));
        Resource_t *new_resource = new Resource_t(Circle(res_loc, resource_radius));
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
