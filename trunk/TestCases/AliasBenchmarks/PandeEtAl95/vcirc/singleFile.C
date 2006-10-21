// Borland C++

/* vpoint.h--Example from Getting Started */

// version of point.h with virtual functions for use with VCIRCLE
// vpoint.h contains two classes:
// class Location describes screen locations in X and Y coordinates
// class Point describes whether a point is hidden or visible

enum Boolean { False, True };

class Location {
protected:          // allows derived class to access private data
   int X;
   int Y;

public:             // these functions can be accessed from outside
   Location(int InitX, int InitY);
   int GetX(void);
   int GetY(void);
};

class Point : public Location {      // derived from class Location
// public derivation means that X and Y are protected within Point

   protected:
   Boolean Visible;  // classes derived from Point will need access    

public:
   Point(int InitX, int InitY) : Location(InitX,InitY) { // constructor
	Visible = False;                  // make invisible by default
   }
   virtual void Show(void);
   virtual void Hide(void);
   Boolean IsVisible(void);
   void MoveTo(int NewX, int NewY);
};
// Borland C++

/* VPOINT.CPP--Example from Getting Started */

// VPOINT.CPP contains the definitions for the Point and Location
// classes that are declared in the file vpoint.h

// member functions for the Location class

Location::Location(int InitX, int InitY) {
   X = InitX;
   Y = InitY;
}

int Location::GetX() {
   return X;
}

int Location::GetY() {
   return Y;
}

// member functions for the Point class: These assume
// the main program has initialized the graphics system

void Point::Show() {
   Visible = True;
//   putpixel(X, Y, getcolor());	 uses default color
}

void Point::Hide(void) {
   Visible = False;
//   putpixel(X, Y, getbkcolor());  uses background color to erase
}

Boolean Point::IsVisible() {
   return Visible;
}

void Point::MoveTo(int NewX, int NewY) {
   Hide();         // make current point invisible
   X = NewX;       // change X and Y coordinates to new location
   Y = NewY;
   Show();         // show point at new location
}

// A Circle class derived from Point

class Circle : public Point {
   int Radius;           // private by default

public:
   Circle(int InitX, int InitY, int InitRadius) : Point(InitX,InitY) {
	Radius = InitRadius;
   }
   void Show(void);
   void Hide(void);
   void Expand(int ExpandBy);
   void Contract(int ContractBy);
};

void Circle::Show()
{
   Visible = True;
//   circle(X, Y, Radius);       draw the circle using BGI function
}

void Circle::Hide()
{
   if (!Visible) return;      // no need to hide
   unsigned int TempColor;    // to save current color
//   TempColor = getcolor();     set to current color
//   setcolor(getbkcolor());     set drawing color to background
   Visible = False;
//   circle(X, Y, Radius);       draw in background color to erase
//   setcolor(TempColor);        set color back to current color
}

void Circle::Expand(int ExpandBy)
{
   Boolean vis = Visible;  // is current circle visible?
   if (vis) Hide();        // if so, hide it
   Radius += ExpandBy;     // expand radius
   if (Radius < 0)         // avoid negative radius
      Radius = 0;
   if (vis) Show();        // draw new circle if previously visible
}

inline void Circle::Contract(int ContractBy)
{
   Expand(-ContractBy);       // redraws with (Radius - ContractBy)
}

main()                        // test the functions
{
   Circle MyCircle(50, 100, 25);    // declare a circle object
   MyCircle.Show();                 // show it
   MyCircle.MoveTo(100, 125);       // move the circle (tests hide
                                    // and show also)
   MyCircle.Expand(25);             // make it bigger
   MyCircle.Contract(35);           // make it smaller
   return 0;
}
