/*
  resources/TubeResource.h
    Defines the TubeResource class.
*/

#ifndef TUBERESOURCE_H
#define TUBERESOURCE_H

class TubeResource {
  private:
    using Body = emp::CircleBody2D;
    Body* body;
    double value;
    int age;
  public:
    TubeResource(const emp::Circle<double> &_p, int value = 1.0)
      : age(0)
    {
      body = new Body(_p);
      this->value = value;
    }

    ~TubeResource() { ; }
};

#endif
