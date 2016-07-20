//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Physics2D - handles movement and collissions in a simple 2D world.


#ifndef EMP_ABPHYSICS_2D_H
#define EMP_ABPHYSICS_2D_H

#include <vector>
#include <unordered_set>
#include <functional>
#include <iostream>

using namespace std::placeholders;

#include "Surface2D.h"

namespace emp {

  // TODO:
  //  * Will need to configure using a ConfigPhysics()
  template <typename BODY_TYPE> class ABPhysics2D {
    private:
      bool configured_physics;
    protected:
      using Surface_t = Surface2D<BODY_TYPE>;

      Surface_t *surface;    // Bodies that can collide.
      Surface_t *background; // Bodies that cannot collide.

      bool detach_on_birth; // Should bodies detach from their parent when born?

    public:
      ABPhysics2D()
        : configured_physics(false),
          detach_on_birth(true)
      {
        std::cout << "Constructing physics..." << std::endl;
      }

      ABPhysics2D(double width, double height, double max_org_diameter = 20, bool detach = true) {
        /*
          Something akin to the original Physics2D constructor.
        */
        ConfigPhysics(width, height, max_org_diameter, detach);
      }

      ~ABPhysics2D() {
        delete surface;
        delete background;
      }

      // Member accessors
      const Surface_t & GetSurface() const { return *surface; }
      const Surface_t & GetBackground() const { return *background; }
      bool GetDetach() const { return detach_on_birth; }

      ABPhysics2D & Clear() { surface->Clear(); background->Clear(); return *this; }

      void ConfigPhysics(double width, double height, double max_org_diameter = 20, bool detach = true) {
        /*
          Configure physics. This function must be called before using Physics2D.
        */
        detach_on_birth = detach;
        surface = new Surface_t(width, height);
        background = new Surface_t(width, height);

        std::cout << "SHeight: " << surface->GetHeight() << std::endl;
        std::cout << "SWidth: " << surface->GetWidth() << std::endl;

        configured_physics = true;
      }

      ABPhysics2D & AddBody(BODY_TYPE *in_body) {
        surface->AddBody(in_body);
        return *this;
      }
      ABPhysics2D & AddBackground(BODY_TYPE *in_body) {
        background->AddBody(in_body);
        return *this;
      }

      bool TestPairCollision(BODY_TYPE &body1, BODY_TYPE &body2) {
        /* Test for collision between body1 and body2.
            Return: true if collision, false otherwise
        */
        if (body1.IsLinked(body2)) return false; // Linked bodies can overlap.

        const Point<double> dist = body1.GetCenter() - body2.GetCenter(); // Calc distance between two center points: P1 - P2 = (x1, y1) - (x2, y2) = ([x1 - x2], [y1 - y2])
        const double sq_pair_dist = dist.SquareMagnitude();               // d.x^2 + d.y^2
        const double radius_sum = body1.GetRadius() + body2.GetRadius();  // Need to be at least as far apart as sum of radiuses (comparing centers here)
        const double sq_min_dist = radius_sum * radius_sum;               // Let's square that last value so we don't have to take any square roots

        // If there was no collision, return false.
        if (sq_pair_dist >= sq_min_dist) { return false; }

        if (sq_pair_dist == 0.0) {
          // If the shapes are on top of each other, we have a problem. Shift one!
          body2.Translate(emp::Point<double>(0.01, 0.01));
        }

        // @CAO Here is where we would identify phasing objects, exploding objects, etc.

        // Re-adjust position to remove overlap.
        const double true_dist = sqrt(sq_pair_dist);
        const double overlap_dist = ((double) radius_sum) - true_dist;
        const double overlap_frac = overlap_dist / true_dist;
        const Point<double> cur_shift = dist * (overlap_frac / 2.0);
        body1.AddShift(cur_shift);
        body2.AddShift(-cur_shift);

        // @CAO If there are inelastic collisions, we just take the weighted average of velocities
        //  and let them move together

        // Assume elastic: re-adjust velocity to reflect bounce.
        double x1, y1, x2, y2;

        if (dist.GetX() == 0) {
          // If X distance between center points is 0, swap Y velocities.
          x1 = body1.GetVelocity().GetX(); y1 = body2.GetVelocity().GetY();
          x2 = body2.GetVelocity().GetX(); y2 = body1.GetVelocity().GetY();

          body1.SetVelocity(Point<double>(x1, y1));
          body2.SetVelocity(Point<double>(x2, y2));
        } else if (dist.GetY() == 0) {
          // If Y distance between center points is 0, swap X velocities.
          x1 = body2.GetVelocity().GetX(); y1 = body1.GetVelocity().GetY();
          x2 = body1.GetVelocity().GetX(); y2 = body2.GetVelocity().GetY();

          body1.SetVelocity(Point<double>(x1, y1));
          body2.SetVelocity(Point<double>(x2, y2));
        } else {
          // Bodies are not totally aligned in some direction.
          const Point<double> rel_velocity(body2.GetVelocity() - body1.GetVelocity()); // Calculate relative velocity
          double normal_a = dist.GetY() / dist.GetX();
          x1 = ( rel_velocity.GetX() + normal_a * rel_velocity.GetY() ) / ( normal_a * normal_a + 1 );
          y1 = normal_a * x1;
          x2 = rel_velocity.GetX() - x1;
          y2 = - (1 / normal_a) * x2;

          body2.SetVelocity(body1.GetVelocity() + Point<double>(x2, y2));
          body1.SetVelocity(body1.GetVelocity() + Point<double>(x1, y1));
        }

        return true;
      }

      void Update() {
        /* Handle physics update. */
        std::cout << "Physics: UPDATE" << std::endl;
        auto &body_set = surface->GetBodySet();

        // Update bodies
        for (auto *cur_body : body_set) {
          cur_body->BodyUpdate(0.25, detach_on_birth);    // Let a body change size or shape, as needed
          cur_body->ProcessStep(0.00125);                 // Update position and velocity. (0.00125 -> friction)
        }

        // Handle collisions
        auto collide_fun = [this](BODY_TYPE &b1, BODY_TYPE &b2) { return this->TestPairCollision(b1, b2); };
        surface->TestCollisions(collide_fun);

        // Determine which bodies we should remove.
        int cur_size = (int) body_set.size();
        int cur_id = 0;
        while (cur_id < cur_size) {
          emp_assert(body_set[cur_id] != nullptr);
          const double cur_pressure = body_set[cur_id]->GetPressure();

          // @CAO Arbitrary pressure threshold!
          if (cur_pressure > 3.0) {                 // If pressure too high, burst this cell!
            delete body_set[cur_id];                // Delete the burst cell.
            cur_size--;                             // Indicate one fewer cells in population.
            body_set[cur_id] = body_set[cur_size];  // Move last cell to popped position.
          } else {
            cur_id++;
          }
        }

        // Now that some cells are removed, resize the number of bodies.
        body_set.resize(cur_size);

      }

      // Access to bodies
      std::vector<BODY_TYPE *> & GetBodySet() {
        return surface->GetBodySet();
      }
      std::vector<BODY_TYPE *> & GetBackgroundSet() {
        return background->GetBodySet();
      }
      // Access to bodies in a const physics...
      const std::vector<BODY_TYPE *> & GetConstBodySet() const {
        return surface->GetConstBodySet();
      }
      const std::vector<BODY_TYPE *> & GetConstBackgroundSet() const {
        return background->GetConstBodySet();
      }
  };
};

#endif
