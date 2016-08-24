// Test/example file for Geometry shapes.

#include "geometry/Shape2D.h"
#include "geometry/Surface2D.h"
#include "geometry/Point2D.h"
#include "geometry/OwnedShape2D.h"
#include "physics/Body2D.h"
#include "web/web.h"
#include "web/canvas_utils.h"
#include "physics/Physics2D.h"
#include "tools/Random.h"


#include <iostream>

namespace web = emp::web;

web::Document doc("emp_base");

struct ShapeOwner {
  int value;
};


int main() {
  doc << "<h1>Shape example!</h1>" << "<br>";
  ShapeOwner owner;
  emp::vector<emp::Shape> shapes;

  // Test shape
  emp::Circle circle(emp::Point<double>(250, 250), 15);
  shapes.push_back(circle);

  for (auto & shape : shapes) {
    std::cout << shape.GetCenter() << std::endl;
  }

  // Test surfaces
  int width = 500;
  int height = 500;
  //emp::Surface2D<emp::Circle> surface(width, height);
  using Shape_t = emp::Circle;
  using Body_t = emp::Body<Shape_t>;
  //emp::Surface2D< emp::OwnedShape<Shape_t, Body_t> > surface(width, height);

  emp::Circle * c_ptr = new emp::Circle(emp::Point<double>(250, 250), 15);
  c_ptr->SetColorID(100);
  //surface.AddShape(c_ptr);

  // Test owned shape
  using OwnedShape_t = emp::OwnedShape<Shape_t, Body_t>;
  OwnedShape_t * ocircle = new OwnedShape_t(emp::Point<double>(100, 100), 10);
  //surface.AddShape(ocircle);

  // Test bodies
  Body_t * body1 = new Body_t(emp::Point<double>(50, 50), 8);
  OwnedShape_t * ocircle1 = new OwnedShape_t(body1, emp::Point<double>(50, 50), 8);
  //surface.AddShape(body1->GetShapePtr());

  emp::Body<emp::Circle, ShapeOwner> * body2 = new emp::Body<emp::Circle, ShapeOwner>(emp::Point<double>(50, 50), 8);

  // Test physics
  emp::Random *random_ptr = new emp::Random(1);
  //emp::CirclePhysics2D<emp::OwnedShape<Shape_t, Body_t>, emp::OwnedShape<Shape_t, emp::Body<emp::Circle, ShapeOwner> >> physics(width, height, random_ptr, 0.0001);
  emp::CirclePhysics2D<Body_t, emp::Body<emp::Circle, ShapeOwner>> physics(width, height, random_ptr, 0.0001);
  physics.AddBody(body1);
  physics.AddBody(body2);

  //std::cout << "Num shapes on surface: " << surface.GetShapeSet().size() << std::endl;
  doc << web::Canvas(width, height, "test-canvas") << "<br>";
  //web::Draw(doc.Canvas("test-canvas"), surface, emp::GetHueMap(360));
}
