//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines classes to represent bodies that exist on a 2D surface.
//  Each class should be able to:
//   * Maintain a pointer to information about the full organism associated with this body.
//   * provide a circular perimeter of the body (for phase1 of collision detection)
//   * Provide body an anchor point and center point of the body (typically the same)
//
//  Currently, the only type of body we have is:
//
//    CircleBody2D - One individual circular object in the 2D world.
//
//
//  Development notes:
//  * If we are going to have a lot of links, we may want a better data structure than vector.
//    (if we don't have a lot, vector may be the best choice...)

// TODO: Review how BodyUpdate and FinalizePosition are organized.

#ifndef EMP_BODY_2D_H
#define EMP_BODY_2D_H

#include "tools/assert.h"
#include "tools/alert.h"
#include "tools/mem_track.h"
#include "tools/Ptr.h"
#include "tools/vector.h"
#include "tools/signal.h"

#include "Angle2D.h"
#include "Circle2D.h"

#include <iostream>
#include <string>
#include <functional>

namespace emp {

  // TODO: is this still the best way to do this? Also, probably should not be
  // just sitting in the emp namespace.
  // Bodies can be linked in several ways.
  // DEFAULT -> Joined together with no extra meaning
  // REPRODUCTION -> "from" is gestating "to"
  // ATTACK -> "from" is trying to eat "to"
  // PARASITE -> "from" is stealing resources from "to"
  // CONSUME_RESOURCE -> "from" is eating "to" where "from" is an organism and "to" is a resource.
  enum class LINK_TYPE { DEFAULT, REPRODUCTION, ATTACK, PARASITE, CONSUME_RESOURCE };

  class Body2D_Base {
  protected:
    template <typename BODY_TYPE>
    struct BodyLink {
      LINK_TYPE type;       // DEFAULT, REPRODUCTION, ATTACK, PARASITE
      BODY_TYPE * from;     // Initiator of the connection (e.g., parent, attacker)
      BODY_TYPE * to;       // Target of the connection (e.g., offspring, prey/host)
      double cur_dist;      // How far are bodies currently being kept apart?
      double target_dist;   // How far should the be moved to? (e.g., if growing)
      double link_strength; // How strong is the link? (used to determine who wins in competive links)

      BodyLink() : type(LINK_TYPE::DEFAULT), from(nullptr), to(nullptr), cur_dist(0)
                 , target_dist(0), link_strength(0) { ; }
      BodyLink(LINK_TYPE t, BODY_TYPE * _frm, BODY_TYPE * _to, double cur=0, double target=0, double lnk_str=0)
        : type(t), from(_frm), to(_to), cur_dist(cur), target_dist(target), link_strength(lnk_str) { ; }
      BodyLink(const BodyLink &) = default;
      ~BodyLink() { ; }
    };

    double birth_time;        // At what time point was this organism born?
    Angle orientation;        // Which way is body facing?
    Point<double> velocity;   // Speed and direction of movement
    double mass;              // "Weight" of this object (@CAO not used yet..)
    double inv_mass;          // Inverse mass on this object (often useful to have precalculated)
    uint32_t color_id;        // Which color should this body appear?
    int repro_count;          // Number of offspring currently being produced.
    bool detach_on_repro;     // Should body detach when link type is REPRODUCTION?
    double growth_rate;       // Default rate (growth per update) at which body will grow if not at target size.

    Point<double> shift;            // How should this body be updated to minimize overlap.
    Point<double> cum_shift;        // Build up of shift not yet acted upon.
    Point<double> total_abs_shift;  // Total absolute-value of shifts (to calculate pressure)
    double pressure;                // Current pressure on this body.
    double max_pressure;            // Max amount of pressure this body can withstand.
    Signal<> destruction_sig;          // Triggered on body destruction.
    Signal<Body2D_Base*> collision_sig; // Triggered on collision with another body.
    bool is_colliding;   // Is currently colliding?
    bool to_destroy;

    void* owner_ptr;
    int owner_id;        // -1 means no owner has been assigned.
    std::function<void()> destruction_callback;

  public:
    Body2D_Base() : birth_time(0.0), mass(1.0), inv_mass(1 / mass), color_id(0),
                    repro_count(0), detach_on_repro(true), growth_rate(1.0),
                    pressure(0), max_pressure(1.0), is_colliding(false),
                    to_destroy(false), owner_id(-1) { ; }
    virtual ~Body2D_Base() {
      destruction_sig.Trigger();
      if (owner_ptr != nullptr) destruction_callback();
    }

    // All physics bodies must indicate that they are indeed physics bodies.
    static constexpr bool emp_is_physics_body = true;

    double GetBirthTime() const { return birth_time; }
    const Angle & GetOrientation() const { return orientation; }
    const Point<double> & GetVelocity() const { return velocity; }
    double GetMass() const { return mass; }
    double GetInvMass() const { return inv_mass; }
    uint32_t GetColorID() const { return color_id; }
    bool IsReproducing() const { return repro_count; }
    bool IsColliding() const { return is_colliding; }
    bool ToDestroy() const { return to_destroy; }
    int GetReproCount() const { return repro_count; }
    bool GetDetachOnRepro() const { return detach_on_repro; }
    Point<double> GetShift() const { return shift; }
    double GetPressure() const { return pressure; }
    double GetMaxPressure() const { return max_pressure; }
    double GetGrowthRate() const { return growth_rate; }
    int GetOwnerID() const { return owner_id; }
    void* GetOwnerPtr() { return owner_ptr; }
    virtual bool ExceedsStressThreshold() const { return pressure > max_pressure; }

    void InvalidateOwner() { owner_ptr = nullptr; owner_id = -1; destruction_callback = [](){ ; }; }
    void MarkForDestruction() { to_destroy = true;  }
    void SetBirthTime(double in_time) { birth_time = in_time; }
    void SetMaxPressure(double mp) { max_pressure = mp; }
    void SetDetachOnRepro(bool detach) { detach_on_repro = detach; }
    void SetGrowthRate(double rate) { growth_rate = rate; }
    void SetColorID(uint32_t in_id) { color_id = in_id; }
    void SetOwner(void* owner, int id, std::function<void()> destruction_callback) {
      owner_ptr = owner; owner_id = id;
      this->destruction_callback = destruction_callback;
    }
    void RegisterDestructionCallback(std::function<void()> callback) {
      destruction_sig.AddAction(callback);
    }
    void RegisterCollisionCallback(std::function<void(Body2D_Base*)> callback) {
      collision_sig.AddAction(callback);
    }
    // Called on collision. Set is colliding flag.
    void TriggerCollision(Body2D_Base *other_body) {
      is_colliding = true;
      collision_sig.Trigger(other_body);
    }
    // Call to signal that the current collision has been resolved.
    void ResolveCollision() {
      is_colliding = false;
    }
    // Orientation control...
    void TurnLeft(int steps = 1) { orientation.RotateDegrees(steps * 45); }
    void TurnRight(int steps = 1) { orientation.RotateDegrees(steps * -45); }

    // Velocity control...
    void IncSpeed(const Point<double> & offset) { velocity += offset; }
    void IncSpeed() { velocity += orientation.GetPoint<double>(); }
    void DecSpeed() { velocity -= orientation.GetPoint<double>(); }
    void SetVelocity(double x, double y) { velocity.Set(x, y); }
    void SetVelocity(const Point<double> & v) { velocity = v; }
    void SetMass(double m) {
      mass = m;
      if (mass == 0.0) inv_mass = 0.0;
      else inv_mass = 1.0 / mass;
    }

    // Shift to apply next update.
    void AddShift(const Point<double> & s) { shift += s; total_abs_shift += s.Abs(); }
  };

  class CircleBody2D : public Body2D_Base {
  protected:
    Circle<double> perimeter;  // Includes position and size.
    double target_radius;      // For growing/shrinking

    // Information about other bodies that this one is linked to.
    emp::vector< BodyLink<CircleBody2D> * > from_links;   // Active links initiated by body
    emp::vector< BodyLink<CircleBody2D> * > to_links;   // Active links targeting body

    void RemoveFromLink(int link_id) {
      emp_assert(link_id >= 0 && link_id < (int) from_links.size());
      from_links[link_id] = from_links.back();
      from_links.pop_back();
    }
    void RemoveToLink(int link_id) {
      emp_assert(link_id >= 0 && link_id < (int) to_links.size());
      to_links[link_id] = to_links.back();
      to_links.pop_back();
    }

  public:
    CircleBody2D(const Circle<double> & _p, double mass = 1.0)
      : perimeter(_p), target_radius(_p.GetRadius())
    {
      //EMP_TRACK_CONSTRUCT(CircleBody2D);
    }
    ~CircleBody2D() {
      // Remove any remaining links from this body.
      while (from_links.size()) RemoveLink(from_links[0]);
      while (to_links.size()) RemoveLink(to_links[0]);
      //EMP_TRACK_DESTRUCT(CircleBody2D);
    }

    const Circle<double> & GetPerimeter() const { return perimeter; }
    const Point<double> & GetAnchor() const { return perimeter.GetCenter(); }
    const Point<double> & GetCenter() const { return perimeter.GetCenter(); }
    double GetRadius() const { return perimeter.GetRadius(); }
    double GetTargetRadius() const { return target_radius; }

    void SetPosition(const Point<double> & p) { perimeter.SetCenter(p); }
    void SetRadius(double r) { perimeter.SetRadius(r); }
    void SetTargetRadius(double t) { target_radius = t; }

    // Translate immediately (ignoring physics)
    void Translate(const Point<double> & t) { perimeter.Translate(t); }

    // Creating, testing, and unlinking other organisms
    bool IsLinkedFrom(const CircleBody2D & link_body) const {
      for (auto * cur_link : from_links) if (cur_link->to == &link_body) return true;
      return false;
    }
    bool IsLinkedTo(const CircleBody2D & link_body) const { return link_body.IsLinkedFrom(*this); }
    bool IsLinked(const CircleBody2D & link_body) const {
      return IsLinkedFrom(link_body) || IsLinkedTo(link_body);
    }

    int GetLinkCount() const { return (int) (from_links.size() + to_links.size()); }

    // Add link FROM this TO link_body.
    void AddLink(LINK_TYPE type, CircleBody2D & link_body, double cur_dist, double target_dist, double link_strength = 0) {
      emp_assert(!IsLinked(link_body));  // Don't link twice!

      // Build connections in both directions.
      auto * new_link = new BodyLink<CircleBody2D>(type, this, &link_body, cur_dist, target_dist, link_strength);
      from_links.push_back(new_link);
      link_body.to_links.push_back(new_link);
    }

    void RemoveLink(BodyLink<CircleBody2D> * link) {
      if (link->to == this) {
        link->from->RemoveLink(link);
        return;
      }

      // Remove the FROM link.
      for (int i = 0; i < (int) from_links.size(); i++) {
        if (from_links[i]->to == link->to) { RemoveFromLink(i); break; }
      }

      // Remove the TO link.
      const int to_size = (int) link->to->to_links.size();
      for (int i = 0; i < to_size; i++) {
        if (link->to->to_links[i]->from == this) { link->to->RemoveToLink(i); break; }
      }

      delete link;
    }

    const BodyLink<CircleBody2D> & FindLink(const CircleBody2D & link_org) const {
      emp_assert(IsLinked(link_org));
      for (auto * link : from_links) if ( link->to == &link_org) return *link;
      return link_org.FindLink(*this);
    }

    BodyLink<CircleBody2D> & FindLink(CircleBody2D & link_org)  {
      emp_assert(IsLinked(link_org));
      for (auto * link : from_links) if ( link->to == &link_org) return *link;
      return link_org.FindLink(*this);
    }

    emp::vector<BodyLink<CircleBody2D> *> GetLinksToByType(LINK_TYPE link_type) {
      emp::vector<BodyLink<CircleBody2D> *> links;
      for (auto *link : this->to_links) {
        if (link->type == link_type) links.push_back(link);
      }
      return links;
    }

    emp::vector<BodyLink<CircleBody2D> *> GetLinksFromByType(LINK_TYPE link_type) {
      emp::vector<BodyLink<CircleBody2D> *> links;
      for (auto *link : this->from_links) {
        if (link->type == link_type) links.push_back(link);
      }
      return links;
    }

    double GetLinkDist(const CircleBody2D & link_org) const {
      emp_assert(IsLinked(link_org));
      return FindLink(link_org).cur_dist;
    }
    double GetTargetLinkDist(const CircleBody2D & link_org) const {
      emp_assert(IsLinked(link_org));
      return FindLink(link_org).target_dist;
    }
    void ShiftLinkDist(CircleBody2D & link_org, double change) {
      auto & link = FindLink(link_org);
      link.cur_dist += change;
    }

    CircleBody2D * BuildLinkedCircleBody2D(emp::Point<double> offset) {
      // Offspring cannot be right on top of parent.
      emp_assert(offset.GetX() != 0 || offset.GetY() != 0);

      // Create the offspring as a paired link.
      auto * offspring = new CircleBody2D(perimeter);
      AddLink(LINK_TYPE::REPRODUCTION, *offspring, offset.Magnitude(), perimeter.GetRadius()*2.0);
      offspring->Translate(offset);
      repro_count++;

      return offspring;
    }

    // See BodyUpdate(double, double).
    void BodyUpdate(double friction) {
      this->BodyUpdate(friction, growth_rate);
    }

    // * If a body is not at its target radius, grow it or shrink it, as needed.
    // * Move body by its velocity.
    // * Reduce velocity by based on friction.
    // TODO: refactor this function
    //  * growth rate doesn't make sense to use for updating link distances
    void BodyUpdate(double friction, double change_factor) {
      // Test if this body needs to grow or shrink.
      if ((int) target_radius > (int) GetRadius()) SetRadius(GetRadius() + change_factor);
      else if ((int) target_radius < (int) GetRadius()) SetRadius(GetRadius() - change_factor);

      // Test if the link distance for this body needs to be updated
      for (int i = 0; i < (int) from_links.size(); i++) {
        auto * link = from_links[i];
        if (link->cur_dist == link->target_dist) continue; // No adjustment needed.

        // If we're within the change_factor, just set pair_dist to target.
        if (std::abs(link->cur_dist - link->target_dist) <= change_factor) {
          link->cur_dist = link->target_dist;
          if (link->type == LINK_TYPE::REPRODUCTION) {
            emp_assert(repro_count > 0);
            repro_count--;
            if (detach_on_repro) {   // Flag link for removal!
              RemoveLink(link);      // Remove the link.
              i--;                   // Check this position again.
            }
          }
        }
        else {
          if (link->cur_dist < link->target_dist) link->cur_dist += change_factor;
          else link->cur_dist -= change_factor;
        }
      }

      // Move body by its velocity and reduce velocity based on friction.
      if (velocity.NonZero()) {
        perimeter.Translate(velocity);
        const double velocity_mag = velocity.Magnitude();

        // If body is close to stopping stop it!
        if (friction > velocity_mag) { velocity.ToOrigin(); }

        // Otherwise slow it down proportionately in the x and y directions.
        else { velocity *= 1.0 - ((double) friction) / ((double) velocity_mag); }
      }
    }

    // Determine where the circle will end up and force it to be within a bounding box.
    void FinalizePosition(const Point<double> & max_coords) {
      const double max_x = max_coords.GetX() - GetRadius();
      const double max_y = max_coords.GetY() - GetRadius();

      // TODO: Update the caclulcation for pressure.
      // Act on the accumulated shifts only when they add up enough.
      cum_shift += shift;
      if (cum_shift.SquareMagnitude() > 0.25) {
        perimeter.Translate(cum_shift);
        cum_shift.ToOrigin();
      }
      pressure = (total_abs_shift - shift.Abs()).SquareMagnitude();
      shift.ToOrigin();              // Clear out the shift for the next round.
      total_abs_shift.ToOrigin();

      // If this body is linked to another, enforce the distance between them.
      for (auto * link : from_links) {
        if (GetAnchor() == link->to->GetAnchor()) {
          // If two organisms are on top of each other... shift one.
          Translate(emp::Point<double>(0.01, 0.01));
        }
        // Figure out how much each oragnism should move so that they will be properly spaced.
        const double start_dist = GetAnchor().Distance(link->to->GetAnchor());
        const double link_dist = link->cur_dist;
        const double frac_change = (1.0 - ((double) link_dist) / ((double) start_dist)) / 2.0;

        emp::Point<double> dist_move = (GetAnchor() - link->to->GetAnchor()) * frac_change;

        perimeter.Translate(-dist_move);
        link->to->perimeter.Translate(dist_move);
      }

      // Adjust the organism so it stays within the bounding box of the world.
      if (GetCenter().GetX() < GetRadius()) {
        perimeter.SetCenterX(GetRadius());     // Put back in range...
        velocity.NegateX();                    // Bounce off left side.
      } else if (GetCenter().GetX() > max_x) {
        perimeter.SetCenterX(max_x);           // Put back in range...
        velocity.NegateX();                    // Bounce off right side.
      }

      if (GetCenter().GetY() < GetRadius()) {
        perimeter.SetCenterY(GetRadius());     // Put back in range...
        velocity.NegateY();                    // Bounce off top.
      } else if (GetCenter().GetY() > max_y) {
        perimeter.SetCenterY(max_y);           // Put back in range...
        velocity.NegateY();                    // Bounce off bottom.
      }
    }

    // Check to make sure there are no obvious issues with this object.
    bool OK() {
      for (auto * link : from_links) {
        (void) link;
        emp_assert(link->cur_dist >= 0);          // Distances cannot be negative.
        emp_assert(link->target_dist >= 0);       // Distances cannot be negative.
      }
      return true;
    }
  };
};

#endif
