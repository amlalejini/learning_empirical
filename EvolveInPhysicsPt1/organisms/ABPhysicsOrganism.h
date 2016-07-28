/*
  organisms/ABPhysicsOrganism.h
   Defines the ABPhysicsOrganism class. Simple organism with bit string genome.
   Eats nutrients for energy. At some threshold of energy, it is able to replicate.
   Nutrients are collected by attracting them to the organism.
   Strength of nutrient A attraction: sum(0) in genome
   Strength of nutrient B attraction: sum(1) in genome

   TODO:
    * currently IsReproducing just returns repro count (should separate how many offspring org has had vs. if it is currently reproducing)
*/

#ifndef ABPHYSICSORGANISM_H
#define ABPHYSICSORGANISM_H

#include <iostream>
#include <string>
#include <algorithm>

#include "tools/BitVector.h"
#include "tools/Random.h"
#include "tools/functions.h"

#include "../modified_geometry/Body2D.h"

#include "../nutrients/ABPhysicsNutrient.h"

class ABPhysicsOrganism : public emp::CircleBody2D {
  private:
    int offspring_count;
    using emp::CircleBody2D::from_links;
    using emp::CircleBody2D::to_links;
    double energy;
    int resources_collected;

  public:
    emp::BitVector genome;

    ABPhysicsOrganism(const emp::Circle<double> &_p, int genome_length = 1, bool random_genome = false)
      : emp::CircleBody2D(_p),
        offspring_count(0),
        energy(0),
        resources_collected(0),
        genome(genome_length, false)
    {
      ;
    }

    ABPhysicsOrganism(ABPhysicsOrganism *parent)
      : emp::CircleBody2D(parent->GetPerimeter()),
        offspring_count(0),
        energy(0),
        resources_collected(0),
        genome(parent->genome)
    {
      ;
    }

    ~ABPhysicsOrganism() { ; }

    double GetEnergy() const { return energy; }
    int GetNumResourcesCollected() const { return resources_collected; }
    int GetOffspringCount() const { return offspring_count; }
    double GetEffectorRadius() {
      /*
        Return the largest effector radius.
        Effector radius for organisms is a function of the number of 1s/0s in their genome.
        Each 1/0 increases radius by some factor.
      */
      int ones = genome.CountOnes();
      return std::max( { CalculateEffectorReach(ones), CalculateEffectorReach(genome.GetSize() - ones), GetRadius() } );
    }

    double GetEffectorRadius(int effector) {
      if (effector == 1) {
        // 1 attractor
        return CalculateEffectorReach(genome.CountOnes());
      } else if (effector == 0) {
        // 0 attractor
        return CalculateEffectorReach(genome.GetSize() - genome.CountOnes());
      } else {
        return GetRadius();
      }
    }

    double CalculateEffectorReach(int effector_strength) {
      /*
        Convert effector strength value (here, 1/0 count in genome) into a reach value.
        This reach value will be used to determine the effective resource eating radius for the organism.
        TODO: allow this function to be passed into the organism.
      */
      return (GetRadius() + 2 * effector_strength);
    }

    ABPhysicsOrganism * Reproduce(emp::Point<double> offset, emp::Random *r, double cost = 0.0, double mut_rate = 0.0) {
      /* Handles organism reproduction!
        For now, trust caller to respect reproduction costs. Perhaps in the future, we return a
        nullptr if reproduction fails (not enough energy, too old, etc.).
      */
      // Update energy (can become negative..)
      energy -= cost;
      offspring_count++;
      repro_count++;
      // Build offspring
      auto *offspring = this->BuildOffspring(offset);
      // Mutate offspring
      for (int i = 0; i < offspring->genome.GetSize(); i++) {
        if (r->P(mut_rate)) offspring->genome[i] = !offspring->genome[i];
      }
      // Link offspring
      AddLink(LINK_TYPE::REPRODUCTION, *offspring, offset.Magnitude(), this->GetRadius() * 2.0);
      return offspring;
    }

    ABPhysicsOrganism * BuildOffspring(emp::Point<double> offset) {
      /* Build and return an offspring from this organism given offset from parent. */
      // Offspring cannot be right on top of parent.
      emp_assert(offset.GetX() != 0 || offset.GetY() != 0);
      // Create the offspring as a paired link.
      auto *offspring = new ABPhysicsOrganism(this);
      offspring->Translate(offset);
      return offspring;
    }

    bool ConsumeResource(ABPhysicsNutrient &resource) {
      /*
        Attempt to consume resource.
        If success, return true; otherwise, return false.
        Currently no reason so fail.
      */
      energy += resource.GetValue();
      resources_collected++;
      return true;
    }

    void BindResource(ABPhysicsNutrient &resource, double cur_dist, double target_dist, double consumption_strength) {
      emp_assert(!IsLinked(resource));  // Don't link twice!
      this->AddLink(LINK_TYPE::CONSUME_RESOURCE, resource, cur_dist, target_dist, consumption_strength);
    }

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
