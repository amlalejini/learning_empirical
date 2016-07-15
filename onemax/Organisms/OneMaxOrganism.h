/*
  Organisms/OneMaxOrganism.h
    Defines the OneMaxOrganism class. Super simple organism used by onemax_evolve/onemax_web.
    I mostly made this just to learn how to use custom organisms in empirical.
*/

#ifndef ONEMAXORGANISM_H
#define ONEMAXORGANISM_H

#include <iostream>

#include "../../../Empirical/tools/BitVector.h"

class OneMaxOrganism {
  private:
  public:
    emp::BitVector genome;

    OneMaxOrganism(int genome_length = 1): genome(genome_length, false)  {
      /* OneMaxOrganism constructor.
          Given a specified genome length, initialize one max organism.
          * Genome: a bitstring. Initialized to all 0's
          * Fitness: sum of 1's in genome
          * Mutation: point mutations (bit flips) at each site with some some probability equal to the mutation rate.
      */
    }

    void Print() {
      /* Print information about this particular organism. */
      // Genome information
      std::cout << "Genome: ";
      genome.Print(std::cout);
      std::cout << std::endl;
    }

    bool operator==(const OneMaxOrganism &other) const {
      /* Do these organisms have the same genotype? */
      return this->genome == other.genome;
    }

    bool operator<(const OneMaxOrganism &other) const {
      /* Fitness comparison */
      return this->genome < other.genome;
    }

    bool operator>(const OneMaxOrganism &other) const {
      return this->genome > other.genome;
    }

    bool operator!=(const OneMaxOrganism &other) const {
      return this->genome != other.genome;
    }

    bool operator>=(const OneMaxOrganism &other) const {
      return !this->operator<(other);
    }

    bool operator<=(const OneMaxOrganism &other) const {
      return !this->operator>(other);
    }

};

#endif
