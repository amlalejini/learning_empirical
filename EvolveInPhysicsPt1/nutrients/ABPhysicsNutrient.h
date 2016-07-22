/*
  nutrients/ABPhysicsNutrient.h
   Defines the ABPhysicsNutrient class.
*/

#ifndef ABPHYSICSNUTRIENT_H
#define ABPHYSICSNUTRIENT_H

#include <iostream>
#include <map>
#include <string>

#include "../modified_geometry/Body2D.h"


class ABPhysicsNutrient : public emp::CircleBody2D {
  private:
    static const std::map<int, std::string> type_names = { {0, "A"},
                                                               {1, "B"} };

    int type;
    double value;
  public:
    ABPhysicsNutrient(const emp::Circle<double> &_p, int type = 0, double value = 1.0)
      : emp::CircleBody2D(_p),
        this->type(type),
        this->value(value)
    {
      ;
    }

    ~ABPhysicsNutrient() { ; }

    // accessors
    double GetValue() { return value; }
    int GetType() { return type; }
    std::string GetTypeName() { return type_names[type]; }

    // mutators
    void SetValue(double value) { this->value = value; }
    void SetType(int type) { this->type = type; }

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

}

#endif
