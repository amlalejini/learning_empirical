/*
  organisms/ABPhysicsOrganism.h
   Defines the ABPhysicsOrganism class. Simple organism with bit string genome.
   Eats nutrients for energy. At some threshold of energy, it is able to replicate.
   Nutrients are collected by attracting them to the organism.
   Strength of nutrient A attraction: sum(0) in genome
   Strength of nutrient B attraction: sum(1) in genome
*/

#ifndef ABPHYSICSORGANISM_H
#define ABPHYSICSORGANISM_H

#include <iostream>

#include "tools/BitVector.h"

#include "../modified_geometry/Body2D.h"

class ABPhysicsOrganism : public emp::CircleBody2D {
  private:
  public:
    emp::BitVector genome;

    ABPhysicsOrganism(const emp::Circle<double> &_p, int genome_length = 1)
      : emp::CircleBody2D(_p),
        genome(genome_length, false)
    {
      std::cout << "Organism constructor!" << std::endl;
    }

    ~ABPhysicsOrganism() { ; }

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
