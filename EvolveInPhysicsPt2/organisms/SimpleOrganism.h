/*
  organisms/SimpleOrganism.h
*/

#ifndef SIMPLEORGANISM_H
#define SIMPLEORGANISM_H

#include "tools/BitVector.h"
#include "tools/Random.h"

#include "../geometry/Body2D.h"

// TODO: make SimpleOrganism bodies compatible with surface
class SimpleOrganism {
  private:
    using Body_t = emp::CircleBody2D;
    // TODO: make body a shared ptr
    Body_t *body;
    int offspring_count;
    double birth_time;
    double pressure_threshold;  // How much pressure able to withstand before popping? TODO: should this be stored in body?
    bool has_body;
  public:
    emp::BitVector genome;

    SimpleOrganism(const emp::Circle<double> &_p, int genome_length = 1, bool detach_on_birth = true)
      : offspring_count(0),
        birth_time(0.0),
        pressure_threshold(1.0),
        genome(genome_length, false)
    {
      std::cout << "Simple Organism arged constructor. Making new body. " << std::endl;
      body = new Body_t(_p);
      body->SetDetachOnRepro(detach_on_birth);
      body->SetBodyLabel(emp::BODY_LABEL::ORGANISM);
      body->AddDestructionCallback([this]() { std::cout << "bod dest cb!" << std::endl; this->has_body = false; });
      has_body = true;
    }

    SimpleOrganism(const SimpleOrganism &other)
       : offspring_count(other.GetOffspringCount()),
         birth_time(other.GetBirthTime()),
         pressure_threshold(other.GetPressureThreshold()),
         genome(other.genome)
    {
      std::cout << "Simple Organism copy constructor 2. Making new body." << std::endl;
      body = new Body_t(other.GetConstBody().GetPerimeter());
      body->SetDetachOnRepro(other.GetDetachOnBirth());
      body->SetBodyLabel(emp::BODY_LABEL::ORGANISM);
      body->AddDestructionCallback([this]() { std::cout << "bod dest cb!" << std::endl; this->has_body = false; });
      has_body = true;
    }

    ~SimpleOrganism() { std::cout << "Org destructor pt1!" << std::endl; if (has_body) delete body;  std::cout << "Org destructor pt2!" << std::endl;}

    int GetOffspringCount() const { return offspring_count; }
    double GetBirthTime() const { return birth_time; }
    bool GetDetachOnBirth() const { emp_assert(has_body); return body->GetDetachOnRepro(); }
    double GetPressureThreshold() const { return pressure_threshold; }
    Body_t * GetBodyPtr() { emp_assert(has_body); return body; }
    Body_t & GetBody() { emp_assert(has_body); return *body; }
    const Body_t & GetConstBody() const { emp_assert(has_body) return *body; }
    bool HasBody() const { return has_body; }

    void SetDetachOnBirth(bool detach) { emp_assert(has_body); body->SetDetachOnRepro(detach); }
    void SetColorID(int id) { emp_assert(has_body); body->SetColorID(id); }
    void SetColorID() {
      emp_assert(has_body);
      if (genome.GetSize() > 0) body->SetColorID((genome.CountOnes() / (double) genome.GetSize()) * 200);
      else body->SetColorID(0);
    }

    SimpleOrganism * Reproduce(emp::Random *r) {
      // TODO: Energy costs?
      // Build offspring
      // - TODO: assert not on top of parent
      auto *offspring = new SimpleOrganism(*this);
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
};

#endif
