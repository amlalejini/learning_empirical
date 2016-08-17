/*
  PopulationManager_SimplePhysics.h
*/

#ifndef POPULATION_MANAGER_SIMPLE_PHYSICS_H
#define POPULATION_MANAGER_SIMPLE_PHYSICS_H

#include <iostream>

#include "../geometry/Physics2D.h"

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
    using RESOURCE = SimpleResource;
    using PHYSICS_BODY_TYPE = CircleBody2D;
    // TODO
    SimplePhysics2D<PHYSICS_BODY_TYPE> physics;
    emp::vector<ORG*> population;
    emp::vector<RESOURCE*> resources;

    Random *random_ptr;

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
    { ; }

    ~PopulationManager_SimplePhysics() { ; }

    // Allow this and derived classes to be identified as a population manager:
    static constexpr bool emp_is_population_manager = true;
    static constexpr bool emp_has_separate_generations = false;
    using value_type = ORG*;

    // Setup iterator for the population.
    friend class PopulationIterator<PopulationManager_SimplePhysics<ORG> >;
    using iterator = PopulationIterator<PopulationManager_SimplePhysics<ORG> >;
    // Operator overloading.
    ORG* & operator[](int i) { return population[i]; }

};

}
}

#endif
