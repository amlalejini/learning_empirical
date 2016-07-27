/*
  nutrients/ABPhysicsNutrient.h
   Defines the ABPhysicsNutrient class.

   TODO:
    * Implement update body, update links
    * Eventually: nutrient clumping
*/

#ifndef ABPHYSICSNUTRIENT_H
#define ABPHYSICSNUTRIENT_H

#include <iostream>
#include <map>
#include <string>

#include "../modified_geometry/Body2D.h"


class ABPhysicsNutrient : public emp::CircleBody2D {
  private:
    std::map<int, std::string> type_names = { {0, "A"},
                                              {1, "B"} };

    int type;
    double value;
    int age;
  public:
    ABPhysicsNutrient(const emp::Circle<double> &_p, int nutrient_type = 0, double nutrient_value = 1.0)
      : emp::CircleBody2D(_p),
        type(nutrient_type),
        value(nutrient_value),
        age(0)
    { this->color_id = 270; }

    ~ABPhysicsNutrient() { ; }
    // accessors
    double GetValue() const { return value; }
    int GetType() const { return type; }
    std::string GetTypeName() const { return type_names.at(type); }
    int GetAge() const { return age; }
    // mutators
    void SetValue(double value) { this->value = value; }
    void SetType(int type) { this->type = type; }
    void Age(int inc = 1) { this->age += inc; }

    // If a body is not at its target radius, grow it or shrink it, as needed.
    void BodyUpdate(double change_factor=1, bool detach_on_birth=true) {
      CircleBody2D::BodyUpdate(change_factor, detach_on_birth);
      this->Age(1); // Age by 1 update
    }

    // operator overloads
    bool operator==(const ABPhysicsNutrient &other) const {
      /* Do these organisms have the same genotype? */
      return this->GetValue() == other.GetValue();
    }

    bool operator<(const ABPhysicsNutrient &other) const {
      return this->GetValue() < other.GetValue();
    }

    bool operator>(const ABPhysicsNutrient &other) const {
      return this->GetValue() > other.GetValue();
    }

    bool operator!=(const ABPhysicsNutrient &other) const {
      return this->GetValue() != other.GetValue();
    }

    bool operator>=(const ABPhysicsNutrient &other) const {
      return !this->operator<(other);
    }

    bool operator<=(const ABPhysicsNutrient &other) const {
      return !this->operator>(other);
    }

};

#endif
