// Test/example file for Geometry shapes.

#include "geometry/Shape2D.h"
#include "geometry/Surface2D.h"
#include "geometry/Point2D.h"
#include "physics/Body2D.h"
#include "web/web.h"
#include "web/canvas_utils.h"


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


  emp::Circle circle(emp::Point<double>(250, 250), 15);
  shapes.push_back(circle);

  for (auto & shape : shapes) {
    std::cout << shape.GetCenter() << std::endl;
  }

  // Test surfaces
  int width = 500;
  int height = 500;
  emp::Circle * c_ptr = new emp::Circle(emp::Point<double>(250, 250), 15);
  c_ptr->SetColorID(100);
  emp::Surface2D<emp::Circle> surface(width, height);
  surface.AddShape(c_ptr);
  std::cout << "Num shapes on surface: " << surface.GetShapeSet().size() << std::endl;

  // Test bodies


  doc << web::Canvas(width, height, "test-canvas") << "<br>";
  web::Draw(doc.Canvas("test-canvas"), surface, emp::GetHueMap(360));
}
