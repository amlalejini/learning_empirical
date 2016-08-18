//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This is a modified version of Physics2D.
//
//  Physics2D - handles movement and collissions in a simple 2D world.
//  This describes environment physics.

// QUESTION: Using body labels vs. different body types to differentiate different types of bodies.

#ifndef EMP_PHYSICS_2D_H
#define EMP_PHYSICS_2D_H

#include <iostream>

#include "Surface2D.h"
#include "Body2D.h"

#include "tools/Random.h"
#include "tools/assert.h"
#include "tools/functions.h"

namespace emp {

  // Simple physics with CircleBody2D bodies.
  template <typename BODY_TYPE> class SimplePhysics2D {
    protected:
      using OrgSurface_t = Surface2D<BODY_TYPE>;
      using ResourceSurface_t = Surface2D<BODY_TYPE>;

      ResourceSurface_t *resource_surface;
      OrgSurface_t *org_surface;
      emp::vector<Surface2D<BODY_TYPE> *> surface_set;

      Point<double> *max_pos;   // Max position across all surfaces.
      bool configured;          // Have the physics been configured yet?
      emp::Random *random_ptr;

    public:
      SimplePhysics2D()
        : configured(false)
      { ; }

      SimplePhysics2D(double width, double height, emp::Random *r, double surface_friction) {
        ConfigPhysics(width, height, r, surface_friction);
      }

      ~SimplePhysics2D() {
        emp_assert(configured);
        delete max_pos;
        delete org_surface;
        delete resource_surface;
      }

      const OrgSurface_t & GetOrgSurface() const { emp_assert(configured); return *org_surface; }
      const ResourceSurface_t & GetResourceSurface() const { emp_assert(configured); return *resource_surface; }
      const emp::vector<Surface2D<BODY_TYPE> *> & GetSurfaceSet() const { return surface_set; }

      double GetWidth() const { emp_assert(configured); return max_pos->GetX(); }
      double GetHeight() const { emp_assert(configured); return max_pos->GetY(); }

      SimplePhysics2D & Clear() {
        if (configured) {
          org_surface->Clear();
          resource_surface->Clear();
        }
        return *this;
      }
      // Config needs to be able to be called multiple times..
      // Configure physics. This must be called if default constructor was
      // used when creating this.
      void ConfigPhysics(double width, double height, emp::Random *r, double surface_friction) {
        if (configured) {
          // If already configured, delete existing bits and remake them.
          surface_set.clear();
          delete org_surface;
          delete resource_surface;
          delete max_pos;
        }
        org_surface = new OrgSurface_t(width, height, surface_friction);
        resource_surface = new ResourceSurface_t(width, height, surface_friction);
        surface_set.push_back( (OrgSurface_t *) org_surface);
        surface_set.push_back( (ResourceSurface_t *) resource_surface);
        max_pos = new Point<double>(width, height);
        random_ptr = r;
        configured = true;
      }

      SimplePhysics2D & AddOrgBody(BODY_TYPE * in_body) {
        emp_assert(configured && (in_body->GetBodyLabel() == BODY_LABEL::ORGANISM));
        org_surface->AddBody(in_body);
        return *this;
      }

      SimplePhysics2D & AddResourceBody(BODY_TYPE * in_body) {
        emp_assert(configured && (in_body->GetBodyLabel() == BODY_LABEL::RESOURCE));
        resource_surface->AddBody(in_body);
        return *this;
      }


      //TODO: there has to be a way to make this better.
      bool CollideBodies(BODY_TYPE *body1, BODY_TYPE *body2) {
        emp_assert(configured);
        // If bodies are linked, no collision.
        if (body1->IsLinked(*body2)) return false;
        const Point<double> dist = body1->GetCenter() - body2->GetCenter();
        const double sq_pair_dist = dist.SquareMagnitude();
        const double radius_sum = body1->GetRadius() + body2->GetRadius();
        const double sq_min_dist = radius_sum * radius_sum;
        // If there was no collision, return false.
        if (sq_pair_dist >= sq_min_dist) return false;
        // Collision! Trigger body collision signals.
        body1->TriggerCollision(body2);
        body2->TriggerCollision(body1);
        // TODO: how should I determine if I'm done resolving collision or if I should continue with default physics response.
        //  TODO: QUESTION: is this approach reasonable? Better approaches?
        if (body1->IsColliding() || body2->IsColliding()) {
          // Default collision resolution.
          // If the shapes are on top of each other, we have a problem. Shift one!
          if (sq_pair_dist == 0.0) {
            body2->Translate(Point<double>(0.01, 0.01));
          }
          // Re-adjust position to remove overlap.
          const double true_dist = sqrt(sq_pair_dist);
          const double overlap_dist = ((double) radius_sum) - true_dist;
          const double overlap_frac = overlap_dist / true_dist;
          const Point<double> cur_shift = dist * (overlap_frac / 2.0);
          body1->AddShift(cur_shift);   // Split the re-adjustment between the two colliding bodies.
          body2->AddShift(-cur_shift);
          // Resolve collision using impulse resolution.
          const double coefficient_of_restitution = 1.0;
          const Point<double> collision_normal(dist / true_dist);
          const Point<double> rel_velocity(body1->GetVelocity() - body2->GetVelocity());
          const double velocity_along_normal = (rel_velocity.GetX() * collision_normal.GetX()) + (rel_velocity.GetY() * collision_normal.GetY());
          // If velocities are separating, no need to resolve anything further, but we'll still mark it as a collision.
          if (velocity_along_normal > 0) return true;
          double j = -(1 + coefficient_of_restitution) * velocity_along_normal; // Calculate j, the impulse scalar.
          j /= body1->GetInvMass() + body2->GetInvMass();
          const Point<double> impulse(collision_normal * j);
          // Apply the impulse.
          body1->SetVelocity(body1->GetVelocity() + (impulse * body1->GetInvMass()));
          body2->SetVelocity(body2->GetVelocity() - (impulse * body2->GetInvMass()));
          // Mark collision as resolved.
          body1->ResolveCollision(); body2->ResolveCollision();
        }
        return true;
      }

      // Test for collisions in *this* physics.
      void TestCollisions() {
        emp_assert(configured);
        // Find the size of the largest body to determine minimum sector size.
        double max_radius = 0.0;
        for (auto *surface : surface_set) {
          auto &surface_body_set = surface->GetBodySet();
          for (auto *body : surface_body_set) {
            if (body->GetRadius() > max_radius) max_radius = body->GetRadius();
          }
        }
        // Figure out the actual number of sectors to use (currently no more than 1024).
        const int num_cols = std::min<int>(GetWidth() / (max_radius * 2.0), 32);
        const int num_rows = std::min<int>(GetHeight() / (max_radius * 2.0), 32);
        const int max_col = num_cols - 1;
        const int max_row = num_rows - 1;
        const int num_sectors = num_cols * num_rows;
        // Calculate sector size.
        const double sector_width = GetWidth() / (double) num_cols;
        const double sector_height = GetHeight() / (double) num_rows;
        emp::vector<emp::vector<BODY_TYPE *>> sector_set(num_sectors);

        int hit_count = 0;
        int test_count = 0;

        // Loop through all bodies on each surface, placing them into sectors and
        // testing for collisions with other bodies already in nearby sectors.
        for (auto *surface : surface_set) {
          auto &surface_body_set = surface->GetBodySet();
          for (auto *body : surface_body_set) {
            emp_assert(body);
            // Determine which sector the current body is in.
            const int cur_col = emp::to_range<int>(body->GetCenter().GetX()/sector_width, 0, max_col);
            const int cur_row = emp::to_range<int>(body->GetCenter().GetY()/sector_height, 0, max_row);

            // See if this body may collide with any of the bodies previously put into sectors.
            for (int i = std::max(0, cur_col-1); i <= std::min(cur_col+1, max_col); i++) {
              for (int j = std::max(0, cur_row-1); j <= std::min(cur_row+1, max_row); j++) {
                const int sector_id = i + num_cols * j;
                if (sector_set[sector_id].size() == 0) continue; // no need to test this body with nothing!
                for (auto *body2 : sector_set[sector_id]) {
                  test_count ++;
                  if (CollideBodies(body, body2)) hit_count++;
                }
              }
            }
            // Add this body to the current sector for collision tests with subsequent bodies.
            const int cur_sector = cur_col + cur_row * num_cols;
            emp_assert(cur_sector < (int) sector_set.size());
            sector_set[cur_sector].push_back(body);
          }
        }
        // TODO: the below bit might be better to move elsewhere
        // Make sure all bodies are in a legal position on each surface.
        for (auto *surface : surface_set) {
          auto &surface_body_set = surface->GetBodySet();
          for (auto *cur_body : surface_body_set) {
            cur_body->FinalizePosition(*max_pos);
          }
        }
      }

      // Progress physics by a single time step.
      void Update() {
        std::cout << "Physics update!" << std::endl;
        emp_assert(configured);
        auto &org_body_set = org_surface->GetBodySet();
        auto &resource_body_set = resource_surface->GetBodySet();
        // Update organism bodies.
        double f = org_surface->GetFriction();
        for (auto *cur_body : org_body_set) {
          cur_body->BodyUpdate(f);
        }
        // Update resource bodies.
        f = resource_surface->GetFriction();
        for (auto *cur_body : resource_body_set) {
          cur_body->BodyUpdate(f);
        }

        // Test for and handle collisions.
        TestCollisions();

        // Body removal/post-collision updates: Are there any bodies that have
        // been physically destroyed?
        // * Organism body removal:
        int cur_size = (int) org_body_set.size();
        int cur_id = 0;
        while (cur_id < cur_size) {
          emp_assert(org_body_set[cur_id] != nullptr);
          const double cur_pressure = org_body_set[cur_id]->GetPressure();
          const double pop_pressure = org_body_set[cur_id]->GetMaxPressure();
          if (cur_pressure > pop_pressure) {  // If pressure is too high, burst the cell!
            delete org_body_set[cur_id];      // Delete the burst cell.
            cur_size--;                       // Indicate 1 fewer cell in body set.
            org_body_set[cur_id] = org_body_set[cur_size];
          } else {
            cur_id++;
          }
        }
        org_body_set.resize(cur_size);
        // * Resource body removal:
        cur_size = (int) resource_body_set.size();
        cur_id = 0;

      }

      emp::vector<BODY_TYPE *> & GetOrgBodySet() {
        emp_assert(configured);
        return org_surface->GetBodySet();
      }

      emp::vector<BODY_TYPE *> & GetResourceBodySet() {
        emp_assert(configured);
        return resource_surface->GetBodySet();
      }

      const emp::vector<BODY_TYPE *> & GetConstOrgBodySet() const {
        emp_assert(configured);
        return org_surface->GetConstBodySet();
      }

      const emp::vector<BODY_TYPE *> & GetConstResourceBodySet() const {
        emp_assert(configured);
        return resource_surface->GetConstBodySet();
      }

  };
}

#endif
