//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Physics2D - handles movement and collissions in a simple 2D world.
//  This describes environment physics.

// NOTES:
//  * Increase max bounding box size by factor of pull strength (organisms may collide with further away resources)
//  * At the moment, max org radius/diameter means nothing
#ifndef EMP_ABPHYSICS_2D_H
#define EMP_ABPHYSICS_2D_H

#include <vector>
#include <unordered_set>
#include <functional>
#include <iostream>

using namespace std::placeholders;

#include "Surface2D.h"
#include "Body2D.h"

namespace emp {

  template <typename ORG_TYPE, typename RESOURCE_TYPE> class ABPhysics2D {
    private:
      bool configured_physics;
    protected:
      using OrgSurface_t      = Surface2D<ORG_TYPE>;
      using ResourceSurface_t = Surface2D<RESOURCE_TYPE>;
      ResourceSurface_t *resource_surface;
      OrgSurface_t *org_surface;
      emp::vector<Surface2D<CircleBody2D> *> surface_set;

      bool detach_on_birth; // Should bodies detach from their parent when born?
      Point<double> *max_pos;
      int max_resource_age;
    public:
      ABPhysics2D()
        : configured_physics(false),
          detach_on_birth(true),
          max_resource_age(100)
      {
        ;
      }

      ABPhysics2D(double width, double height, double max_org_radius = 20, bool detach = true, int max_res_age = 100) {
        /*
          Something akin to the original Physics2D constructor.
        */
        ConfigPhysics(width, height, max_org_radius, detach, max_res_age);
      }

      ~ABPhysics2D() {
        delete max_pos;
        delete org_surface;
        delete resource_surface;
      }

      // Member accessors
      const OrgSurface_t & GetOrgSurface() const { return *org_surface; }
      const ResourceSurface_t & GetResourceSurface() const { return *resource_surface; }
      const emp::vector<Surface2D<CircleBody2D> *> & GetSurfaceSet() const { return surface_set; }
      bool GetDetach() const { return detach_on_birth; }
      double GetWidth() const { return max_pos->GetX(); }
      double GetHeight() const { return max_pos->GetY(); }

      ABPhysics2D & Clear() {
        org_surface->Clear();
        resource_surface->Clear();
        return *this;
      }

      void ConfigPhysics(double width, double height, double max_org_radius = 20, bool detach = true, int max_res_age = 100) {
        /*
          Configure physics. This function must be called before using Physics2D.
        */
        detach_on_birth = detach;
        max_resource_age = max_res_age;
        org_surface = new OrgSurface_t(width, height);
        resource_surface = new ResourceSurface_t(width, height);
        surface_set.push_back( (Surface2D<CircleBody2D> *) org_surface);
        surface_set.push_back( (Surface2D<CircleBody2D> *) resource_surface);
        max_pos = new Point<double>(width, height);
        configured_physics = true;
      }

      ABPhysics2D & AddOrg(ORG_TYPE *in_org) {
        org_surface->AddBody(in_org);
        return *this;
      }
      ABPhysics2D & AddResource(RESOURCE_TYPE *in_resource) {
        resource_surface->AddBody(in_resource);
        return *this;
      }

      bool CollideBodies(CircleBody2D *body1, CircleBody2D *body2) {
        // Handle all possible collision cases
        RESOURCE_TYPE *res_body;
        ORG_TYPE *org_body;
        if ( ( res_body = dynamic_cast<RESOURCE_TYPE*>(body1) ) && ( org_body = dynamic_cast<ORG_TYPE*>(body2) ) )
          return CollideOrgXResource(org_body, res_body);
        else if ( ( res_body = dynamic_cast<RESOURCE_TYPE*>(body2) ) && ( org_body = dynamic_cast<ORG_TYPE*>(body1) ) )
          return CollideOrgXResource(org_body, res_body);
        else
          return CollideCircles(body1, body2);
      }

      bool CollideCircles(CircleBody2D *body1, CircleBody2D *body2) {
        /* Given two circle bodies, test for collision. If there is a collision,
            handle the aftermath.
           Returns true if collision happens.
           Returns false if no collision.
        */
        if (body1->IsLinked(*body2)) return false; // Linked bodies can overlap.

        const Point<double> dist = body1->GetCenter() - body2->GetCenter();
        const double sq_pair_dist = dist.SquareMagnitude();
        const double radius_sum = body1->GetRadius() + body2->GetRadius();
        const double sq_min_dist = radius_sum * radius_sum;

        // If there was no collision, return false.
        if (sq_pair_dist >= sq_min_dist) { return false; }

        // Collision happened: Handle it and return true.
        if (sq_pair_dist == 0.0) {
          // If the shapes are on top of each other, we have a problem. Shift one!
          body2->Translate(Point<double>(0.01, 0.01));
        }

        // @CAO If objects can phase or explode, identify that here.

        // Re-adjust position to remove overlap.
        const double true_dist = sqrt(sq_pair_dist);
        const double overlap_dist = ((double) radius_sum) - true_dist;
        const double overlap_frac = overlap_dist / true_dist;
        const Point<double> cur_shift = dist * (overlap_frac / 2.0);
        body1->AddShift(cur_shift);   // Split the re-adjustment between the two colliding bodies.
        body2->AddShift(-cur_shift);

        // @CAO if we have inelastic collisions, we just take the weighted average of velocities
        // and let them move together.

        // Assume elastic: Re-adjust velocity to reflect bounce.
        double x1, y1, x2, y2;

        if (dist.GetX() == 0) {
          // If vertically aligned, just swap y velocities.
          x1 = body1->GetVelocity().GetX();  y1 = body2->GetVelocity().GetY();
          x2 = body2->GetVelocity().GetX();  y2 = body1->GetVelocity().GetY();

          body1->SetVelocity(Point<double>(x1, y1));
          body2->SetVelocity(Point<double>(x2, y2));
        } else if (dist.GetY() == 0) {
          // If horizontally aligned, just swap x velocities.
          x1 = body2->GetVelocity().GetX();  y1 = body1->GetVelocity().GetY();
          x2 = body1->GetVelocity().GetX();  y2 = body2->GetVelocity().GetY();

          body1->SetVelocity(Point<double>(x1, y1));
          body2->SetVelocity(Point<double>(x2, y2));
        } else {
          // General case.
          const Point<double> rel_velocity(body2->GetVelocity() - body1->GetVelocity());
          double normal_a = dist.GetY() / dist.GetX();
          x1 = ( rel_velocity.GetX() + normal_a * rel_velocity.GetY() )
            / ( normal_a * normal_a + 1 );
          y1 = normal_a * x1;
          x2 = rel_velocity.GetX() - x1;
          y2 = - (1 / normal_a) * x2;

          body2->SetVelocity(body1->GetVelocity() + Point<double>(x2, y2));
          body1->SetVelocity(body1->GetVelocity() + Point<double>(x1, y1));
        }
        return true;
      }

      bool CollideOrgXResource(ORG_TYPE *org_body, RESOURCE_TYPE *resource_body) {
        /*
          Handles collision test for organism X resource.
          If resource is in range of organism, attempt to add link from organism to resource.
          == Winner Takes All Method ==
          If link would be stronger than any existing link, add link and remove old link.
          Otherwise, resource consumption is unsuccessful.
          == Proportional Consumption ==
          No matter strengh, make link. Will hand out energy proportional to link strengths.

        */
        if (org_body->IsLinked(*resource_body)) return false; // This pair is already linked, move on.

        // Calculate distance between organism and resource.
        const Point<double> dist = org_body->GetCenter() - resource_body->GetCenter();
        const double sq_pair_dist = dist.SquareMagnitude();
        const double radius_sum = org_body->GetRadius() + resource_body->GetRadius(); // This is where I would put any radius extentions for the organism.
        const double sq_min_dist = radius_sum * radius_sum;

        // If there was no collision, return false.
        if (sq_pair_dist >= sq_min_dist) { return false; }

        // There is a collision, and resource and organism are not already linked!
        const double true_dist = sqrt(sq_pair_dist);
        // Determine strength of consumption:
        double strength;
        if (sq_pair_dist == 0.0) strength = sq_min_dist / 0.001;
        else strength = sq_min_dist / sq_pair_dist;
        org_body->BindResource(*resource_body, true_dist, sq_min_dist, strength);
        return true;
      }

      void TestCollisions() {
        /* Given a list of Surface2D objects and a physics object where the necessary collide functions are defined,
            test all possible collisions among all surfaces in surfaces vector.

            Required: all surfaces MUST be same width/height.
            Required: all bodies MUST be circle bodies (relying on radius function to calculate sector sizes).
        */

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
        // Calculate sector size
        const double sector_width = GetWidth() / (double) num_cols;
        const double sector_height = GetHeight() / (double) num_rows;
        emp::vector< emp::vector<CircleBody2D *> > sector_set(num_sectors);

        int hit_count = 0;
        int test_count = 0;

        // Loop through all of the bodies on each surface, placing them into sectors and testing for
        // collisions with other bodies already in nearby sectors.
        for (auto *surface : surface_set) {
          auto &surface_body_set = surface->GetBodySet();
          for (auto *body : surface_body_set) {
            emp_assert(body);
            // Determine which sector the current body is in.
            const int cur_col = emp::to_range<int>(body->GetCenter().GetX()/sector_width, 0, max_col);
            const int cur_row = emp::to_range<int>(body->GetCenter().GetY()/sector_height, 0, max_row);

            // See if this body may collide with any of the bodies previously put into sectors.
            for (int i = std::max(0, cur_col-1); i <= std::min(cur_col+1, num_cols-1); i++) {
              for (int j = std::max(0, cur_row-1); j <= std::min(cur_row+1, num_rows-1); j++) {
                const int sector_id = i + num_cols * j;
                if (sector_set[sector_id].size() == 0) continue;

                for (auto body2 : sector_set[sector_id]) {
                  test_count++;
                  if (CollideBodies(body, body2)) hit_count++;
                }

              }
            }

            // Add this body to the current sector for future tests to compare with.
            const int cur_sector = cur_col + cur_row * num_cols;
            emp_assert(cur_sector < (int) sector_set.size());

            sector_set[cur_sector].push_back(body);
          }
        }

        // Make sure all bodies are in a legal position on each surface.
        for (auto *surface : surface_set) {
          auto &surface_body_set = surface->GetBodySet();
          for (auto *cur_body : surface_body_set) {
            cur_body->FinalizePosition(*max_pos);
          }
        }
      }

      void Update() {
        /* Handle physics update. */
        auto &org_body_set = org_surface->GetBodySet();
        auto &resource_body_set = resource_surface->GetBodySet();
        // Update organism bodies
        for (auto *cur_body : org_body_set) {
          cur_body->BodyUpdate(0.25);                     // Let a body change size or shape, as needed
          cur_body->ProcessStep(0.00125);                 // Update position and velocity. (0.00125 -> friction)
        }
        // Update resource bodies
        for (auto *cur_body : resource_body_set) {
          cur_body->BodyUpdate(0.25);
          cur_body->ProcessStep(0.00125);
        }

        // Handle collisions
        //auto collide_fun = [this](CircleBody2D &b1, CircleBody2D &b2) { return this->CollideFunction(b1, b2); };
        TestCollisions();

        ///////////////////////////////
        // Organism removal
        ///////////////////////////////
        // Determine which organisms we should remove.
        int cur_size = (int) org_body_set.size();
        int cur_id = 0;
        while (cur_id < cur_size) {
          emp_assert(org_body_set[cur_id] != nullptr);
          const double cur_pressure = org_body_set[cur_id]->GetPressure();

          // @CAO Arbitrary pressure threshold!
          if (cur_pressure > 3.0) {                 // If pressure too high, burst this cell!
            delete org_body_set[cur_id];                // Delete the burst cell.
            cur_size--;                             // Indicate one fewer cells in population.
            org_body_set[cur_id] = org_body_set[cur_size];  // Move last cell to popped position.
          } else {
            cur_id++;
          }
        }
        // Now that some cells are removed, resize the number of bodies.
        org_body_set.resize(cur_size);
        ///////////////////////////////

        ///////////////////////////////
        // Resource removal
        ///////////////////////////////
        // Remove rotten (those that have passed their expiration date) resources
        // Remove consumed resources
        cur_size = (int) resource_body_set.size();
        cur_id = 0;
        while (cur_id < cur_size) {
          emp_assert(resource_body_set[cur_id] != nullptr);
          // Consumption
          auto consumption_links = resource_body_set[cur_id]->GetConsumptionLinks();
          // Who has the strongest link?
          // Feed to strongest link.
          // Remove this resource. (will clean up links)
          std::cout << "Consumption links: " << consumption_links.size() << std::endl;
          // Rotten removal
          const int cur_age = resource_body_set[cur_id]->GetAge();
          if (cur_age > max_resource_age) {
            delete resource_body_set[cur_id];
            cur_size--;
            resource_body_set[cur_id] = resource_body_set[cur_size];
          } else {
            cur_id++;
          }
        }
        resource_body_set.resize(cur_size);
        ///////////////////////////////
      }

      // Access to bodies
      std::vector<ORG_TYPE *> & GetOrgBodySet() {
        return org_surface->GetBodySet();
      }
      std::vector<RESOURCE_TYPE *> & GetResourceBodySet() {
        return resource_surface->GetBodySet();
      }
      // Access to bodies in a const physics...
      const std::vector<ORG_TYPE *> & GetConstOrgBodySet() const {
        return org_surface->GetConstBodySet();
      }
      const std::vector<RESOURCE_TYPE *> & GetConstResourceBodySet() const {
        return resource_surface->GetConstBodySet();
      }
  };
};

#endif
