/*
  organisms/SimpleOrganism.h
*/

#ifndef SIMPLEORGANISM_H
#define SIMPLEORGANISM_H

#include "tools/BitVector.h"
#include "tools/Random.h"

#include "../geometry/Body2D.h"

// TODO: copy constructor for bodies

class SimpleOrganism {
  private:
    using Body = emp::CircleBody2D;
    Body* body;
    int offspring_count;
    double birth_time;
    double pressure_threshold;  // How much pressure able to withstand before popping? TODO: should this be stored in body?

  public:
    emp::BitVector genome;

    SimpleOrganism(const emp::Circle<double> &_p, int genome_length = 1, bool detach_on_birth = true)
      : offspring_count(0),
        birth_time(0.0),
        pressure_threshold(1.0),
        genome(genome_length, false)
    {
      body = new Body(_p);
      body->SetDetachOnRepro(detach_on_birth);
      body->SetBodyLabel(BODY_LABEL::ORGANISM);
    }

    SimpleOrganism(SimpleOrganism *other)
       : offspring_count(other->GetOffspringCount()),
         birth_time(other->GetBirthTime()),
         pressure_threshold(other->GetPressureThreshold()),
         genome(other->genome)
    {
      body = new Body(other->GetBody()->GetPerimeter());
      body->SetDetachOnRepro(other->GetDetachOnBirth());
      body->SetBodyLabel(BODY_LABEL::ORGANISM);
    }

    ~SimpleOrganism() { delete body; }

    int GetOffspringCount() const { return offspring_count; }
    double GetBirthTime() const { return birth_time; }
    bool GetDetachOnBirth() const { return body->GetDetachOnRepro(); }
    double GetPressureThreshold() const { return pressure_threshold; }
    Body * GetBody() { return body; }

    void SetDetachOnBirth(bool detach) { body->SetDetachOnRepro(detach); }

    SimpleOrganism * Reproduce(emp::Random *r) {
      // TODO: Energy costs?
      // Build offspring
      // - TODO: assert not on top of parent
      auto *offspring = SimpleOrganism(this);
      // - TODO: translate given offsiet
      // TODO: Mutate offspring
      // TODO: Link offspring
      return offspring;
    }

    bool operator==(const SimpleOrganism &other) const {
      /* Do these organisms have the same genotype? */
      return this->genome == other.genome;
    }

    bool operator<(const SimpleOrganism &other) const {
      return this->genome < other.genome;
    }

    bool operator>(const SimpleOrganism &other) const {
      return this->genome > other.genome;
    }

    bool operator!=(const SimpleOrganism &other) const {
      return this->genome != other.genome;
    }

    bool operator>=(const SimpleOrganism &other) const {
      return !this->operator<(other);
    }

    bool operator<=(const SimpleOrganism &other) const {
      return !this->operator>(other);
    }
}

#endif
