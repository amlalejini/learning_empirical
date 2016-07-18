/*
 PopulationManager_ABPhysics.h
  * Implements population manager for my AB nutrient environment.

*/

#ifndef POPULATION_MANAGER_ABPHYSICS_H
#define POPULATION_MANAGER_ABPHYSICS_H

#include <iostream>

#include "../modified_geometry/Physics2D.h"
#include "../modified_geometry/Surface2D.h"

#include "evo/PopulationManager.h"

using emp::Physics2D;
/////////////////////////////
// Learning notes
//  * Functionality required of population managers:
//    - AddOrg, AddOrgBirth, Print,
//    - needs to maintain list of organisms
//  * Functionality required of Physics2D environment:
//    - Collision testing
//    - Update
//    - needs to maintain a Surface of bodies (organisms/[nutrients eventually])
////////
// DEV NOTES
//  * I do not like the current relationship between physics2d and this population manager.
//    - Also related, I do not like the relationship between physics organism and CircleBody2D
/////////////////////////////

template <typename ORG>
class PopulationManager_ABPhysics : public Physics2D<ORG>, public emp::evo::PopulationManager_Base<ORG> {
  protected:

  public:
    PopulationManager_ABPhysics()
      : Physics2D<ORG>(100.0, 100.0, 20, true) // Some default values. Will be overridden by a ConfigPop
    {
      /* PopulationManager_ABPhysics constructor.
          * width: width of physics world (in world units)
          * height: height of physics world (in world units)
          * max_org_diameter: max organism diameter (in world units)
          * detach: should organisms be attached or detached to parent at birth?
      */
      std::cout << "Population Manager (AB Physics) Contruction!" << std::endl;
      std::cout << " Just checking on detach_on_birth: " << this->detach_on_birth << std::endl;
    }

    // ConfigPop(double width, double height, double max_org_diameter = 20, bool detach = true) {
    //
    // }
    void Clear() {
      std::cout << "I should be clearing things..." << std::endl;
    }
};

#endif
