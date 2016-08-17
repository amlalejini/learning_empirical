//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This is a modified version of Physics2D.
//
//  Physics2D - handles movement and collissions in a simple 2D world.
//  This describes environment physics.

#ifndef EMP_TUBEPHYSICS_2D_H
#define EMP_TUBEPHYSICS_2D_H

#include "Surface2D.h"
#include "Body2D.h"

#include "tools/Random.h"
#include "tools/assert.h"

namespace emp {
  template <typename ORG_TYPE, typename RESOURCE_TYPE> class TubePhysics2D {
    protected:
      // @amlalejini QUESTION: How can I ensure that ORG_TYPE and RESOURCE_TYPE are both derived from Body_t?
      using OrgSurface_t = Surface2D<ORG_TYPE>;
      using ResourceSurface_t = Surface2D<RESOURCE_TYPE>;
      using Body_t = CircleBody2D;

      ResourceSurface_t *resource_surface;
      OrgSurface_t *org_surface;
      emp::vector<Surface2D<Body_t> *> surface_set;

      Point<double> *max_pos;   // Max position across all surfaces.
      bool configured;          // Have the physics been configured yet?
      emp::Random *random_ptr;

    public:
      TubePhysics2D()
        : configured(false)
      { ; }

      TubePhysics2D(double width, double height, emp::Random *r) {
        ConfigPhysics(width, height, r);
      }

      ~TubePhysics2D() {
        emp_assert(configured);
        delete max_pos;
        delete org_surface;
        delete resource_surface;
      }

      const OrgSurface_t & GetOrgSurface() const { emp_assert(configured); return *org_surface; }
      const ResourceSurface_t & GetResourceSurface() const { emp_assert(configured); return *resource_surface; }
      const emp::vector<Surface2D<Body_t> *> & GetSurfaceSet() const { return surface_set; }

      bool GetWidth() const { emp_assert(configured); return max_pos->GetX(); }
      bool GetHeight() const { emp_assert(configured); return max_pos->GetY(); }

      TubePhysics2D & Clear() {
        emp_assert(configured);
        org_surface->Clear();
        resource_surface->Clear();
        return *this;
      }

      // Configure physics. This must be called if default constructor was
      // used when creating this.
      void ConfigPhysics(double width, double height, emp::Random *r) {
        org_surface = new OrgSurface_t(width, height);
        resource_surface = new ResourceSurface_t(width, height);
        surface_set.push_back( (Surface2D<Body_t> *) org_surface);
        surface_set.push_back( (Surface2D<Body_t> *) resource_surface);
        max_pos = new Point<double>(width, height);
        random_ptr = r;
        configured = true;
      }

      TubePhysics2D & AddOrg(ORG_TYPE *in_org) {
        emp_assert(configured);
        org_surface->AddBody(in_org);
        return *this;
      }

      TubePhysics & AddResource(RESOURCE_TYPE *in_resource) {
        emp_assert(configured);
        resource_surface->AddBody(in_resource);
        return *this;
      }

      //TODO: CollideBodies
      bool CollideBodies(Body_t *body1, Body_t *body2) {
        emp_assert(configured);
        return false;
      }

      bool ResolveCollision(Body_t *body1, Body_t *body2) {
        emp_assert(configured);
        return false;
      }

      bool ResolveCollision(ORG_TYPE *org_body, RESOURCE_TYPE *resource_body) {
        emp_assert(configured);
        return false;
      }

      // Test all possible collisions between bodies managed by physics (stored
      // in surface_set).
      //  REQUIREMENT: all surfaces must be same width/height.
      //  REQUIREMENT: assumes Body_t = CircleBody2D
      void TestCollisions() {
        emp_assert(configured);

      }

      // Progress physics by a single time step.
      void Update() {
        emp_assert(configured);
        auto &org_body_set = org_surface->GetBodySet();
        auto &resource_body_set = org_surface-GetBodySet();
        // Update organism bodies.
        for (auto *cur_body : org_body_set) {
          cur_body->BodyUpdate();
        }
        // Update resource bodies.

        // Test for collisions.

        //
      }

      emp::vector<ORG_TYPE *> & GetOrgBodySet() {
        emp_assert(configured);
        return org_surface->GetBodySet();
      }

      emp::vector<RESOURCE_TYPE *> & GetResourceBodySet() {
        emp_assert(configured);
        return resource_surface->GetBodySet();
      }

      const emp::vector<ORG_TYPE *> & GetConstOrgBodySet() const {
        emp_assert(configured);
        return org_surface->GetConstBodySet();
      }

      const emp::vector<RESOURCE_TYPE *> & GetConstResourceBodySet() const {
        emp_assert(configured);
        return resource_surface->GetConstBodySet();
      }

  };
}

#endif
