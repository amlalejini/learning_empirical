//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This is a modified version of Physics2D.
//
//  Physics2D - handles movement and collissions in a simple 2D world.
//  This describes environment physics.

// THIS IS ALL SCRATCH CODE

#ifndef EMP_PHYSICS_2D_H
#define EMP_PHYSICS_2D_H

#include <iostream>

#include "../geometry/Surface2D.h"
#include "Body2D.h"
#include "../geometry/Shape2D.h"
#include "../geometry/OwnedShape2D.h"

#include "tools/Random.h"
#include "tools/assert.h"
#include "tools/functions.h"
#include "tools/meta.h"

#include <vector>
#include <tuple>
#include <utility>
#include <type_traits>

// For each owner, create a Surface and add it to surface set.
// Surface set will be indexed by the order in which things were passed into the template.
// Note: OWNER_TYPES should be a **SET** -- no repeat types passed in.
// If repeats are passed in, things *should* work fine. But only the first surface of that type will ever
// be used.

// TODO: eventually make number of surfaces generic
namespace emp {

template <typename... OWNEDSHAPE_TYPES>
class SimplePhysics2D {
  protected:
    //using Shape_t = Circle;
    std::vector<Surface2D<OwnedShape<Circle, Body2D_Base>> *> surface_set;
    using Tuple_t = std::tuple<Surface2D<OWNEDSHAPE_TYPES>*...>;
    Tuple_t surface_tuple;
    static constexpr int num_surfaces = std::tuple_size<Tuple_t>::value;

    Point<double> *max_pos;   // Max position across all surfaces.
    bool configured;          // Have the physics been configured yet?
    emp::Random *random_ptr;

    //Signal<BODY_TYPE *, BODY_TYPE *> on_collision_sig;
    Signal<> on_update_sig;

    // The below functions (init_surfaces and destroy_surfaces)
    //  leverage: http://www.codeproject.com/Articles/857354/Compile-Time-Loops-with-Cplusplus-Creating-a-Gener
    void InitSurfaces(double width, double height, double friction, std::integral_constant<int, num_surfaces>)
    {
      std::cout << "Terminate INIT loop." << std::endl;
    }

    template<int index = 0>
    void InitSurfaces(double width, double height, double friction, std::integral_constant<int, index> = std::integral_constant<int, 0>())
    {
        std::cout << "Loop index: " << index << std::endl;
        using O_t = typename std::remove_pointer<typename std::tuple_element<index, Tuple_t>::type>::type;
        std::get<index>(surface_tuple) = new O_t(width, height, friction);
        InitSurfaces(width, height, friction, std::integral_constant<int, index + 1>());
    }

    void DestroySurfaces(std::integral_constant<int, num_surfaces>)
    {
      std::cout << "Terminate DESTROY loop." << std::endl;
    }

    template<int index = 0>
    void DestroySurfaces(std::integral_constant<int, index> = std::integral_constant<int, 0>())
    {
        std::cout << "Loop index: " << index << std::endl;
        //using O_t = typename std::remove_pointer<typename std::tuple_element<index, Tuple_t>::type>::type;
        delete std::get<index>(surface_tuple);
        DestroySurfaces(std::integral_constant<int, index + 1>());
    }


  public:
    SimplePhysics2D() :
     configured(false)
    { ; }

    SimplePhysics2D(double width, double height, emp::Random *r, double surface_friction)
    {
      ConfigPhysics(width, height, r, surface_friction);
    }

    ~SimplePhysics2D() {
      emp_assert(configured);
      DestroySurfaces();
      delete max_pos;
    }

    // Call GetTypeID<type_name>() to get the ID associated with owner type type_name.
    template<typename T>
    constexpr static int GetTypeID() { return get_type_index<T, OWNEDSHAPE_TYPES...>(); }
    // Call GetTypeID(owner) to get the ID associated with 'owner'.
    template <typename T>
    constexpr static int GetTypeID(const T &) { return get_type_index<T, OWNEDSHAPE_TYPES...>(); }

    double GetWidth() const { emp_assert(configured); return max_pos->GetX(); }
    double GetHeight() const { emp_assert(configured); return max_pos->GetY(); }

    SimplePhysics2D & Clear() {
      for (auto * surface : surface_set) surface->Clear();
      return *this;
    }

    // Configure physics. This must be called if default constructor was
    // used when creating this.
    void ConfigPhysics(double width, double height, emp::Random *r, double surface_friction) {
      std::cout << "Configuring physics." << std::endl;
      if (configured) {
        // If already configured, delete existing bits and remake them.
        DestroySurfaces();
      }

      std::cout << "Tuple size: " << num_surfaces << std::endl;
      InitSurfaces(width, height, surface_friction);
      max_pos = new Point<double>(width, height);
      random_ptr = r;
      configured = true;
    }

    template<typename BODY_TYPE>
    SimplePhysics2D & AddBody(BODY_TYPE * in_body) {
      emp_assert(configured);
      //constexpr int index = GetTypeID(in_body->GetConstShape());
      //(std::get<index>(surface_tuple))->AddShape(in_body->GetShapePtr());
      //surface_set[idx]->AddShape(in_body->GetShapePtr());
    }

    // SimpleCirclePhysics2D & RemoveBody(BODY_TYPE * in_body) {
    //   emp_assert(in_body->HasOwner(), configured);
    //   org_surface->RemoveBody(in_body);
    //   return *this;
    // }

    // Test for collisions in *this* physics.
    void TestCollisions() {

    }

    // Progress physics by a single time step.
    void Update() {
      emp_assert(configured);
      std::cout << "Physics update." << std::endl;
    }

};

//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This is a modified version of Physics2D.
//
//  Physics2D - handles movement and collissions in a simple 2D world.
//  This describes environment physics.

#ifndef EMP_PHYSICS_2D_H
#define EMP_PHYSICS_2D_H

#include <iostream>

#include "../geometry/Surface2D.h"
#include "Body2D.h"
#include "../geometry/Shape2D.h"
#include "../geometry/OwnedShape2D.h"

#include "tools/Random.h"
#include "tools/assert.h"
#include "tools/functions.h"
#include "tools/meta.h"

#include <vector>
#include <tuple>
#include <utility>
#include <type_traits>
#include <memory>
#include<typeinfo>

// For each owner, create a Surface and add it to surface set.
// Surface set will be indexed by the order in which things were passed into the template.
// Note: OWNER_TYPES should be a **SET** -- no repeat types passed in.
// If repeats are passed in, things *should* work fine. But only the first surface of that type will ever
// be used.

// TODO: eventually make number of surfaces generic
namespace emp {


  template <typename... BODY_TYPES>
  class CirclePhysics2D {
    protected:
      using Shape_t = Circle;
      // I know every type in surface_set by index (synced to BODY_TYPES): OwnedShape<Shape_t, BODY_TYPE>
      emp::vector<Surface*> surface_set;
      //Surface2D<Shape_t> *surface;
      Point<double> *max_pos;   // Max position across all surfaces.
      bool configured;          // Have the physics been configured yet?
      emp::Random *random_ptr;

      //Signal<BODY_TYPE *, BODY_TYPE *> on_collision_sig;
      Signal<> on_update_sig;

      // Template meta-programming magic to build surface set.
      template <typename FIRST_TYPE>
      void BuildSurfaceSet(double width, double height, double friction) {
        // Build set.
        std::cout << "Adding surface to the set!" << std::endl;
        surface_set.push_back(new Surface2D<OwnedShape<Shape_t, FIRST_TYPE>>(width, height, friction));
      }
      template <typename FIRST_TYPE, typename SECOND_TYPE, typename... MORE_TYPES>
      void BuildSurfaceSet(double width, double height, double friction) {
        std::cout << "Looping to build surface set!" << std::endl;
        BuildSurfaceSet<FIRST_TYPE>(width, height, friction);
        BuildSurfaceSet<SECOND_TYPE, MORE_TYPES...>(width, height, friction);
      }


    public:
      CirclePhysics2D()
        : configured(false)
      { ; }

      CirclePhysics2D(double width, double height, emp::Random *r, double surface_friction) {
        ConfigPhysics(width, height, r, surface_friction);
      }

      ~CirclePhysics2D() {
        emp_assert(configured);
        for (auto * surface : surface_set) delete surface;
        //delete surface;
        delete max_pos;
      }

      // Call GetTypeID<type_name>() to get the ID associated with owner type type_name.
      template<typename T>
      constexpr static int GetTypeID() { return get_type_index<T, BODY_TYPES...>(); }
      // Call GetTypeID(owner) to get the ID associated with 'owner'.
      template <typename T>
      constexpr static int GetTypeID(const T &) { return get_type_index<T, BODY_TYPES...>(); }

      //const emp::vector<Surface2D<BODY_TYPE> *> & GetSurfaceSet() const { return surface_set; }

      double GetWidth() const { emp_assert(configured); return max_pos->GetX(); }
      double GetHeight() const { emp_assert(configured); return max_pos->GetY(); }

      CirclePhysics2D & Clear() {
        for (auto * surface : surface_set) delete surface->Clear();
        //surface->Clear();
        return *this;
      }

      // Config needs to be able to be called multiple times..
      // Configure physics. This must be called if default constructor was
      // used when creating this.
      void ConfigPhysics(double width, double height, emp::Random *r, double surface_friction) {
        if (configured) {
          // If already configured, delete existing bits and remake them.
          for (auto * surface : surface_set) delete surface;
          //delete surface;
        }
        //surface = new Surface2D<Shape_t>(width, height, surface_friction);
        BuildSurfaceSet<BODY_TYPES...>(width, height, surface_friction);
        max_pos = new Point<double>(width, height);
        random_ptr = r;
        configured = true;
      }

      template<typename BODY_TYPE>
      CirclePhysics2D & AddBody(BODY_TYPE * in_body) {
        emp_assert(configured);
        std::cout << "CirclePhysics2D::--- Adding a body! ---" << std::endl;
        int idx = GetTypeID(*in_body);
        std::cout << "  Body type index: " << idx << std::endl;
        // std::cout << typeid(pack_id<idx, BODY_TYPES...>).name() << std::endl;
        //std::cout << type_factory(idx) << std::endl;
        static_cast<Surface2D<Shape_t>*>(surface_set[idx])->AddShape(in_body->GetShapePtr());
        //surface->AddShape(in_body->GetShapePtr());
        return *this;
      }

      // SimpleCirclePhysics2D & RemoveBody(BODY_TYPE * in_body) {
      //   emp_assert(in_body->HasOwner(), configured);
      //   org_surface->RemoveBody(in_body);
      //   return *this;
      // }

      // Test for collisions in *this* physics.
      void TestCollisions() {

      }

      // Progress physics by a single time step.
      void Update() {
        emp_assert(configured);
        std::cout << "Physics update." << std::endl;
      }

      // TODO: get OWNERXsurface
      // TODO: get OWNERxBodySet
      // emp::vector<BODY_TYPE *> & GetBodySet() {
      //   emp_assert(configured);
      //   return org_surface->GetBodySet();
      // }
      //
      // emp::vector<BODY_TYPE *> & GetResourceBodySet() {
      //   emp_assert(configured);
      //   return resource_surface->GetBodySet();
      // }
      //
      // const emp::vector<BODY_TYPE *> & GetConstOrgBodySet() const {
      //   emp_assert(configured);
      //   return org_surface->GetConstBodySet();
      // }
      //
      // const emp::vector<BODY_TYPE *> & GetConstResourceBodySet() const {
      //   emp_assert(configured);
      //   return resource_surface->GetConstBodySet();
      // }

  };
  // TODO: check that all owned shape types

  // Simple physics with CircleBody2D bodies.
  template <typename... BODY_TYPES>
  class CirclePhysics2D {
    protected:
      using Shape_t = Circle;
      // I know every type in surface_set by index (synced to BODY_TYPES): OwnedShape<Shape_t, BODY_TYPE>
      //emp::vector<Surface*> surface_set;
      Surface2D<Shape_t> surface;
      Point<double> *max_pos;   // Max position across all surfaces.
      bool configured;          // Have the physics been configured yet?
      emp::Random *random_ptr;

      //Signal<BODY_TYPE *, BODY_TYPE *> on_collision_sig;
      Signal<> on_update_sig;

      // Template meta-programming magic to build surface set.
      // template <typename FIRST_TYPE>
      // void BuildSurfaceSet(double width, double height, double friction) {
      //   // Build set.
      //   std::cout << "Adding surface to the set!" << std::endl;
      //   surface_set.push_back(new Surface2D<OwnedShape<Shape_t, FIRST_TYPE>>(width, height, friction));
      // }
      // template <typename FIRST_TYPE, typename SECOND_TYPE, typename... MORE_TYPES>
      // void BuildSurfaceSet(double width, double height, double friction) {
      //   std::cout << "Looping to build surface set!" << std::endl;
      //   BuildSurfaceSet<FIRST_TYPE>(width, height, friction);
      //   BuildSurfaceSet<SECOND_TYPE, MORE_TYPES...>(width, height, friction);
      // }


    public:
      CirclePhysics2D()
        : configured(false)
      { ; }

      CirclePhysics2D(double width, double height, emp::Random *r, double surface_friction) {
        ConfigPhysics(width, height, r, surface_friction);
      }

      ~CirclePhysics2D() {
        emp_assert(configured);
        for (auto * surface : surface_set) delete surface;
        delete max_pos;
      }

      // Call GetTypeID<type_name>() to get the ID associated with owner type type_name.
      template<typename T>
      constexpr static int GetTypeID() { return get_type_index<T, BODY_TYPES...>(); }
      // Call GetTypeID(owner) to get the ID associated with 'owner'.
      template <typename T>
      constexpr static int GetTypeID(const T &) { return get_type_index<T, BODY_TYPES...>(); }

      //const emp::vector<Surface2D<BODY_TYPE> *> & GetSurfaceSet() const { return surface_set; }

      double GetWidth() const { emp_assert(configured); return max_pos->GetX(); }
      double GetHeight() const { emp_assert(configured); return max_pos->GetY(); }

      CirclePhysics2D & Clear() {
        for (auto * surface : surface_set) surface->Clear();
        return *this;
      }

      // Config needs to be able to be called multiple times..
      // Configure physics. This must be called if default constructor was
      // used when creating this.
      void ConfigPhysics(double width, double height, emp::Random *r, double surface_friction) {
        if (configured) {
          // If already configured, delete existing bits and remake them.
          for (auto * surface : surface_set) delete surface;
        }
        BuildSurfaceSet<BODY_TYPES...>(width, height, surface_friction);
        max_pos = new Point<double>(width, height);
        random_ptr = r;
        configured = true;
      }

      template<typename BODY_TYPE>
      CirclePhysics2D & AddBody(BODY_TYPE * in_body) {
        emp_assert(configured);
        std::cout << "CirclePhysics2D::--- Adding a body! ---" << std::endl;
        int idx = GetTypeID(*in_body);
        std::cout << "  Body type index: " << idx << std::endl;
        std::cout << typeid(pack_id<idx, BODY_TYPES...>).name() << std::endl;
        //std::cout << type_factory(idx) << std::endl;
        //static_cast<(type_factory(idx))*>(surface_set[idx]);//->AddShape(in_body->GetShapePtr());
        return *this;
      }

      // SimpleCirclePhysics2D & RemoveBody(BODY_TYPE * in_body) {
      //   emp_assert(in_body->HasOwner(), configured);
      //   org_surface->RemoveBody(in_body);
      //   return *this;
      // }

      // Test for collisions in *this* physics.
      void TestCollisions() {

      }

      // Progress physics by a single time step.
      void Update() {
        emp_assert(configured);
        std::cout << "Physics update." << std::endl;
      }

      // TODO: get OWNERXsurface
      // TODO: get OWNERxBodySet
      // emp::vector<BODY_TYPE *> & GetBodySet() {
      //   emp_assert(configured);
      //   return org_surface->GetBodySet();
      // }
      //
      // emp::vector<BODY_TYPE *> & GetResourceBodySet() {
      //   emp_assert(configured);
      //   return resource_surface->GetBodySet();
      // }
      //
      // const emp::vector<BODY_TYPE *> & GetConstOrgBodySet() const {
      //   emp_assert(configured);
      //   return org_surface->GetConstBodySet();
      // }
      //
      // const emp::vector<BODY_TYPE *> & GetConstResourceBodySet() const {
      //   emp_assert(configured);
      //   return resource_surface->GetConstBodySet();
      // }

  };
}

#endif


}

#endif
