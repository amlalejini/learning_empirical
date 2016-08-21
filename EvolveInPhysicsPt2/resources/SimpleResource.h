/*
  resources/SimpleResource.h
    Defines the SimpleResource class.
*/

#ifndef SIMPLERESOURCE_H
#define SIMPLERESOURCE_H

#include "../geometry/Body2D.h"

class SimpleResource;
class SimpleResourceBody;

// TODO: What's the best way to do this?
// Can I do a 'bodywrapper' class somehow? Can I make the base class a template?
// class SimpleResourceBody : public emp::CircleBody2D {
//   private:
//     SimpleResource *owner;
//   public:
//     using emp::CircleBody2D::CircleBody2D;
//     void SetOwner(SimpleResource *resource) { owner = resource; }
//     SimpleResource * GetOwner() { return owner; }
//
// };

class SimpleResource {
  using Body_t = emp::CircleBody2D;
  friend class emp::CircleBody2D;
  private:
    Body_t* body;                     // An organism/resource's body may be deleted by outside forces.
    double value;
    double age;
    bool has_body;

    void OnBodyDestruction() {
      has_body = false;
    }

  public:
    SimpleResource(const emp::Circle<double> &_p, double value = 1.0)
      : age(0.0)
    {
      AttachBody(new Body_t(_p));
      body->SetDetachOnRepro(true);
      body->SetMaxPressure(99999); // Big number.
      this->value = value;
    }

    SimpleResource(const SimpleResource &other)
      : value(other.GetValue()),
        age(0.0)
    {
      AttachBody(new Body_t(other.GetConstBody().GetPerimeter()));
      body->SetDetachOnRepro(other.GetConstBody().GetDetachOnRepro());
      body->SetMaxPressure(99999);
    }

    ~SimpleResource() {
      if (has_body) {
        body->InvalidateOwner();
        body->MarkForDestruction();
      }
    }

    double GetValue() const { return value; }
    double GetAge() const { return age; }
    Body_t * GetBodyPtr() { emp_assert(body); return body; }
    Body_t & GetBody() { emp_assert(body); return *body; }
    const Body_t & GetConstBody() const { emp_assert(has_body); return *body; }
    bool HasBody() const { return has_body; }
    void FlagBodyDestruction() { has_body = false; }

    void SetValue(double value) { this->value = value; }
    void SetAge(double age) { this->age = age; }
    int IncAge() { return ++age; }
    void SetColorID(int id) { emp_assert(has_body); body->SetColorID(id); }

    // TODO: should be able to point body to THIS as owner.
    // (These things need access to the lookup table that currently sits in the physics.)
    void AttachBody(Body_t * in_body) {
        body = in_body;
        has_body = true;
    }

    // operator overloads
    bool operator==(const SimpleResource &other) const {
      /* Do these organisms have the same genotype? */
      return this->GetValue() == other.GetValue();
    }

    bool operator<(const SimpleResource &other) const {
      return this->GetValue() < other.GetValue();
    }

    bool operator>(const SimpleResource &other) const {
      return this->GetValue() > other.GetValue();
    }

    bool operator!=(const SimpleResource &other) const {
      return this->GetValue() != other.GetValue();
    }

    bool operator>=(const SimpleResource &other) const {
      return !this->operator<(other);
    }

    bool operator<=(const SimpleResource &other) const {
      return !this->operator>(other);
    }
};

#endif
