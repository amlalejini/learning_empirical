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
        std::cout << "Simple physics destructor!" << std::endl;
        delete max_pos;
        delete org_surface;
        delete resource_surface;
      }

      const OrgSurface_t & GetOrgSurface() const { emp_assert(configured); return *org_surface; }
      const ResourceSurface_t & GetResourceSurface() const { emp_assert(configured); return *resource_surface; }
      const emp::vector<Surface2D<BODY_TYPE> *> & GetSurfaceSet() const { return surface_set; }

      bool GetWidth() const { emp_assert(configured); return max_pos->GetX(); }
      bool GetHeight() const { emp_assert(configured); return max_pos->GetY(); }

      SimplePhysics2D & Clear() {
        std::cout << "Simple physics clear() " << std::endl;
        if (configured) {
          std::cout << " -- is configured" << std::endl;
          org_surface->Clear();
          std::cout << "after org surface clear()" << std::endl;
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

      //TODO: CollideBodies
      bool CollideBodies(BODY_TYPE &body1, BODY_TYPE &body2) {
        emp_assert(configured);
        return false;
      }

      void TestCollisions() {
        emp_assert(configured);
        std::cout << "Test For Collisions." << std::endl;
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

        // Test for collisions.
        TestCollisions();

        // Body removal: are there any bodies that have been physically destroyed?
        // * Organism body removal:

        // * Resource body removal:

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
