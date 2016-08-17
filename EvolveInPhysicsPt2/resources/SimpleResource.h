/*
  resources/SimpleResource.h
    Defines the SimpleResource class.
*/

#ifndef SIMPLERESOURCE_H
#define SIMPLERESOURCE_H

class SimpleResource {
  private:
    using Body = emp::CircleBody2D;
    Body* body;
    double value;
    int age;
  public:
    SimpleResource(const emp::Circle<double> &_p, int value = 1.0)
      : age(0)
    {
      body = new Body(_p);
      this->value = value;
    }

    ~SimpleResource() { ; }
};

#endif
