// Listing 4.1 page 72, from "C++ Programming Style" by
// Tom Cargill.  Addison Wesley, 1992.
#include <stdio.h>
#include <string.h>

class Vehicle 
{
  protected: char *plate;
  

  public: inline Vehicle()
{
    (this) -> plate = ((0));
  }

  

  inline Vehicle(char *p)
{
    (this) -> plate = (new char [strlen((p)) + 1]);
    strcpy(((this) -> plate),(p));
  }

  

  inline ~Vehicle()
{
    delete []((this) -> plate);
  }

  

  virtual inline void identify()
{
    printf("generic vehicle\n");
  }

}

;

class Car : public Vehicle
{
  

  public: inline Car() : Vehicle()
{
  }

  

  inline Car(char *p) : Vehicle(p)
{
  }

  

  virtual inline void identify()
{
    printf("car with plate %s\n",((this) -> plate));
  }

}

;

class Truck : public Vehicle
{
  

  public: inline Truck() : Vehicle()
{
  }

  

  inline Truck(char *p) : Vehicle(p)
{
  }

  

  virtual inline void identify()
{
    printf("truck with plate %s\n",((this) -> plate));
  }

}

;

class Garage 
{
  private: int maxVehicles;
  class Vehicle **parked;
  public: Garage(int max);
  ~Garage();
  int accept(class Vehicle *);
  Vehicle *release(int bay);
  void listVehicles();
}

;

Garage::Garage(int max)
{
  (this) -> maxVehicles = max;
  (this) -> parked = (new Vehicle *[((this) -> maxVehicles)]);
  for (int bay = 0; bay < ((this) -> maxVehicles); ++bay) {
    ((this) -> parked)[bay] = ((0));
  }
}


Garage::~Garage()
{
  delete []((this) -> parked);
}


int Garage::accept(class Vehicle *veh)
{
  for (int bay = 0; bay < ((this) -> maxVehicles); ++bay) {
    if (!((((this) -> parked)[bay]))) {
      ((this) -> parked)[bay] = veh;
      return bay;
    }
    else {
    }
  }
// no bay available
  return (-1);
}


Vehicle *Garage::release(int bay)
{
  if ((bay < 0) || (bay >= ((this) -> maxVehicles))) {
    return (0);
  }
  else {
  }
  class Vehicle *veh = (((this) -> parked)[bay]);
  ((this) -> parked)[bay] = ((0));
  return veh;
}


void Garage::listVehicles()
{
  for (int bay = 0; bay < ((this) -> maxVehicles); ++bay) {
    if ((((this) -> parked)[bay])) {
      printf("Vehicle in bay %d is: ",bay);
      ( *(((this) -> parked)[bay])).identify();
    }
    else {
    }
  }
}

class Car c1(("RVR 101"));
class Car c2(("SPT 202"));
class Car c3(("CHP 303"));
class Car c4(("BDY 404"));
class Car c5(("BCH 505"));
class Truck t1(("TBL 606"));
class Truck t2(("IKY 707"));
class Truck t3(("FFY 808"));
class Truck t4(("PCS 909"));
class Truck t5(("SLY 000"));

int main()
{
  class Garage park(15);
  park.accept(((&c1)));
  int t2bay = park.accept(((&t2)));
  park.accept(((&c3)));
  park.accept(((&t1)));
  int c4bay = park.accept(((&c4)));
  park.accept(((&c5)));
  park.accept(((&t5)));
  park.release(t2bay);
  park.accept(((&t4)));
  park.accept(((&t3)));
  park.release(c4bay);
  park.accept(((&c2)));
  park.listVehicles();
  return 0;
}

