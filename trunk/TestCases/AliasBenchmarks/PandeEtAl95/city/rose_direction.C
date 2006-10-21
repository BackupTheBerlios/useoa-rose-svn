// direction.cc
#include "direction.h"

char *direction::as_string()
{
  char *dirs[] = {("N"), ("NE"), ("E"), ("SE"), ("S"), ("SW"), ("W"), ("NW"), ("No direction")};
  return dirs[(this) -> dir];
}


int operator==(class direction d1,class direction d2)
{
  return ((d1.dir) == (d2.dir));
}


int operator!=(class direction d1,class direction d2)
{
  return ((d1.dir) != (d2.dir));
}


int operator<=(class direction d1,class direction d2)
{
  return ((d1.dir) <= (d2.dir));
}


std::ostream &operator<<(std::ostream &o,class direction d)
{
  o<<((d.as_string()));
  return o;
}

extern const class direction N(0);
extern const class direction NE(1);
extern const class direction E(2);
extern const class direction SE(3);
extern const class direction S(4);
extern const class direction SW(5);
extern const class direction W(6);
extern const class direction NW(7);
extern const class direction NO_DIRECTION(8);
#ifdef direction_test
#include "stream.h"
#endif
