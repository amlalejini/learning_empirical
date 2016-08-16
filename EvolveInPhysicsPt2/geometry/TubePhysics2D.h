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

namespace emp {
  template <typename ORG_TYPE, typename RESOURCE_TYPE> class TubePhysics2D {
    protected:
      using OrgSurface_t = Surface2D<ORG_TYPE>;
      using ResourceSurface_t = Surface2D<RESOURCE_TYPE>;
      using Body_t = CircleBody2D;

      ResourceSurface_t *resource_surface;
      OrgSurface_t *org_surface;
      emp::vector<Surface2D<Body_t> *> surface_set;

    public:
      TubePhysics2D() {

      }

      ~TubePhysics2D() {
        
      }
  };
}

#endif
