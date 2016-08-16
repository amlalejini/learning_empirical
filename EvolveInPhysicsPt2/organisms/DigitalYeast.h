/*
  organisms/DigitalYeast.h
    Defines the DigitalYeast class. Class used to represent Yeast from (Ratcliff et al., 2012).
*/

#ifndef DIGITALYEAST_H
#define DIGITALYEAST_H

class DigitalYeast {
  private:
    using Body = emp::CircleBody2D;

    Body* body;
    int offspring_count;

  public:

    DigitalYeast(const emp::Circle<double> &_p)
      : offspring_count(0)
    { body = new Body(_p); }

    // TODO: copy constructor.
    // DigitalYeast(DigitalYeast *parent)
    //   : body(parent)

    ~DigitalYeast() { delete body; }
}

#endif
