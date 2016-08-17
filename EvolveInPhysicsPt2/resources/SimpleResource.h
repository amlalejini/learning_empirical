/*
  resources/SimpleResource.h
    Defines the SimpleResource class.
*/

#ifndef SIMPLERESOURCE_H
#define SIMPLERESOURCE_H

#include "../geometry/Body2D.h"

class SimpleResource {
  private:
    using Body = emp::CircleBody2D;
    Body* body;                     // An organism/resource's body may be deleted by outside forces.
    double value;
    double age;

  public:
    SimpleResource(const emp::Circle<double> &_p, double value = 1.0)
      : age(0.0)
    {
      body = new Body(_p);
      body->SetDetachOnRepro(true);
      body->SetBodyLabel(emp::BODY_LABEL::RESOURCE);
      this->value = value;
    }

    SimpleResource(SimpleResource *other)
      : value(other->GetValue()),
        age(0.0)
    {
      body = new Body(other->GetBody().GetPerimeter());
      body->SetDetachOnRepro(other->GetBody().GetDetachOnRepro());
      body->SetBodyLabel(emp::BODY_LABEL::RESOURCE);
    }

    ~SimpleResource() {
      if (body != nullptr) delete body;
    }

    double GetValue() const { return value; }
    double GetAge() const { return age; }
    Body * GetBodyPtr() { return body; }
    Body & GetBody() { emp_assert(body); return *body; }

    void SetValue(double value) { this->value = value; }
    void SetAge(double age) { this->age = age; }
    int IncAge() { return ++age; }

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
