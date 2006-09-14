// Borland C++
/* VPOINT.CPP--Example from Getting Started */
// VPOINT.CPP contains the definitions for the Point and Location
// classes that are declared in the file vpoint.h
#include "vpoint.h"
// member functions for the Location class

Location::Location(int InitX,int InitY)
{
  (this) -> X = InitX;
  (this) -> Y = InitY;
}


int Location::GetX()
{
  return (this) -> X;
}


int Location::GetY()
{
  return (this) -> Y;
}

// member functions for the Point class: These assume
// the main program has initialized the graphics system

void Point::Show()
{
  (this) -> Visible = True;
//   putpixel(X, Y, getcolor());	 uses default color
}


void Point::Hide()
{
  (this) -> Visible = False;
//   putpixel(X, Y, getbkcolor());  uses background color to erase
}


enum Boolean Point::IsVisible()
{
  return (this) -> Visible;
}


void Point::MoveTo(int NewX,int NewY)
{
// make current point invisible
  (this) -> Hide();
// change X and Y coordinates to new location
  (this) -> X = NewX;
  (this) -> Y = NewY;
// show point at new location
  (this) -> Show();
}

// A Circle class derived from Point

class Circle : public Point
{
// private by default
  private: int Radius;
  

  public: inline Circle(int InitX,int InitY,int InitRadius) : Point(InitX,InitY)
{
    (this) -> Radius = InitRadius;
  }

  virtual void Show();
  virtual void Hide();
  void Expand(int ExpandBy);
  inline void Contract(int ContractBy);
}

;

void Circle::Show()
{
  (this) -> Visible = True;
//   circle(X, Y, Radius);       draw the circle using BGI function
}


void Circle::Hide()
{
// no need to hide
  if (!(((this) -> Visible))) {
    return ;
  }
  else {
  }
// to save current color
  unsigned int TempColor;
//   TempColor = getcolor();     set to current color
//   setcolor(getbkcolor());     set drawing color to background
  (this) -> Visible = False;
//   circle(X, Y, Radius);       draw in background color to erase
//   setcolor(TempColor);        set color back to current color
}


void Circle::Expand(int ExpandBy)
{
// is current circle visible?
  enum Boolean vis = ((this) -> Visible);
// if so, hide it
  if (vis) {
    (this) -> Hide();
  }
  else {
  }
// expand radius
  (this) -> Radius += ExpandBy;
// avoid negative radius
  if (((this) -> Radius) < 0) {
    (this) -> Radius = 0;
  }
  else {
  }
// draw new circle if previously visible
  if (vis) {
    (this) -> Show();
  }
  else {
  }
}


inline void Circle::Contract(int ContractBy)
{
// redraws with (Radius - ContractBy)
  (this) -> Expand((-ContractBy));
}

// test the functions

int main()
{
// declare a circle object
  class Circle MyCircle(50,100,25);
// show it
  MyCircle.Circle::Show();
// move the circle (tests hide
  MyCircle.MoveTo(100,125);
// and show also)
// make it bigger
  MyCircle.Expand(25);
// make it smaller
  MyCircle.Contract(35);
  return 0;
}

