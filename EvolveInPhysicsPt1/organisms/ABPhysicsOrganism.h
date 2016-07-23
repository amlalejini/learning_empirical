/*
  organisms/ABPhysicsOrganism.h
   Defines the ABPhysicsOrganism class. Simple organism with bit string genome.
   Eats nutrients for energy. At some threshold of energy, it is able to replicate.
   Nutrients are collected by attracting them to the organism.
   Strength of nutrient A attraction: sum(0) in genome
   Strength of nutrient B attraction: sum(1) in genome

   TODO:
    * Implement update body, update links
*/

#ifndef ABPHYSICSORGANISM_H
#define ABPHYSICSORGANISM_H

#include <iostream>

#include "tools/BitVector.h"

#include "../modified_geometry/Body2D.h"

#include "../nutrients/ABPhysicsNutrient.h"

class ABPhysicsOrganism : public emp::CircleBody2D {
  private:
    int repro_count;
  public:
    emp::BitVector genome;
    std::vector<ABPhysicsNutrient> resources;

    ABPhysicsOrganism(const emp::Circle<double> &_p, int genome_length = 1)
      : emp::CircleBody2D(_p),
        repro_count(0),
        genome(genome_length, false)        
    {
      ;
    }

    ~ABPhysicsOrganism() { ; }

    ABPhysicsOrganism * BuildOffspring(emp::Point<double> offset) {
      /* Build and return an offspring from this organism given offset from parent. */
      // Offspring cannot be right on top of parent.
      emp_assert(offset.GetX() != 0 || offset.GetY() != 0);
      // Create the offspring as a paired link.
      auto *offspring = new ABPhysicsOrganism(emp::Circle<double>(this->GetCenter(), this->GetRadius()));
      offspring->Translate(offset);
      repro_count++;
      return offspring;
    }

    // ABPhysicsOrganism * BuildOffspring() {
    //   /* Build and return an offspring from this organism. Generate offset from parent. */
    //   return ;
    // }

    bool operator==(const ABPhysicsOrganism &other) const {
      /* Do these organisms have the same genotype? */
      return this->genome == other.genome;
    }

    bool operator<(const ABPhysicsOrganism &other) const {
      return this->genome < other.genome;
    }

    bool operator>(const ABPhysicsOrganism &other) const {
      return this->genome > other.genome;
    }

    bool operator!=(const ABPhysicsOrganism &other) const {
      return this->genome != other.genome;
    }

    bool operator>=(const ABPhysicsOrganism &other) const {
      return !this->operator<(other);
    }

    bool operator<=(const ABPhysicsOrganism &other) const {
      return !this->operator>(other);
    }

};

#endif
