// main.c   -   simulation main driver

#include "iostream"
#include "sim.h"
#include "direction.h"
#include "vehicle.h"
#include "roadlet.h"
#include "check_functions.h"
#include "intersection.h"
#include <stdlib.h>

using namespace std;

#if 0
extern "C" 
{
    void *malloc();
    void srandom(int);
}
#endif


// connect r1->d1 to r2->d2
void connect(roadlet *r1, direction d1, 
	     roadlet *r2, direction d2, 
	     roadlet* (*r1_to_r2_filter_function)(roadlet *, vehicle *, direction))
{
    //cout << "connecting " << *r1 << " " << *r2 << '\n';
    r1->set_neighbor(d1, r2);
    r2->set_neighbor(d2, r1);
    r1->set_move_filter_function(d1, r1_to_r2_filter_function);
}

int main()
{
    car c("fred");
    roadlet *r1, *r2;
    roadlet *r3, *r4;
    char *buff;
    int i;

    srandom(5);

    // build a two lane one direction road

    r1 = new roadlet ("R start");
    r3 = new roadlet ("L start");
    connect(r1, W, r3, E, return_null); // there but can't move to

    c.set_location(r1);		// a car
    r1->arrive(&c);

    for(i=0; i<10; i++)
    {
	buff = (char *)malloc(4);
	sprintf(buff, "R%d", i);
	r2 = new roadlet(buff);

	buff = (char *)malloc(4);
	sprintf(buff, "L%d", i);
	r4 = new roadlet(buff);

        connect(r1, N, r2, S, &is_empty);
        connect(r3, N, r4, S, &is_empty);
        connect(r1, NW, r4, SE, &lane_switch_ok);
        connect(r3, NE, r2, SW, &lane_switch_ok);
        connect(r2, W, r4, E, return_null); // can't more sideways

	r1 = r2;
	r3 = r4;
    }

    car b2("blocker 2");
    b2.set_location(r1);		// a car
    r1->arrive(&b2);
    cout << b2 << '\n';

    intersection_4x4 i1("intersection ");

    i1.connectSin(r3, r1);

    broken_light l(3,1,4,1);

    for(i = 0; i < 100000; i++)
    {
	cout << l << "\n";
	l.tick();
    }

    for(i=0; i< 100000; i++)
    {
        i1.light()->tick();
    }


    r1 = new roadlet ("East Road R ");
    r3 = new roadlet ("East Road L ");
    connect(r1, N, r3, S, return_null); // there but can't move to

    car b("blocker");
    b.set_location(r1);		// a car
    r1->arrive(&b);
    cout << b << '\n';

    i1.connectEout(r3, r1);

    for(i=0; i< 100000; i++)
    {
        cout << '\n' << i << ' ' << c << '\n';
        c.tick();
    }
    cout << i << ' ' << c << '\n';
}
// check-functions.cc

#include "direction.h"
#include "roadlet.h"
#include "vehicle.h"
#include "iostream"


// roadlet r is the the 'd' of v->location()


roadlet * return_null(roadlet * r, vehicle *v, direction)
{
    return(NULL);
}


roadlet * is_empty(roadlet *r, vehicle *v, direction d)
{
    if (!r->occupied())
	return(r);
    else
	return(NULL);
}


//check both left and right.  This makes sense for roads of two or fewer lanes.
roadlet * lane_switch_ok(roadlet *r, vehicle *v, direction d)
{
    direction dir = v->direction();
    //cout << "dir = " << dir << "\n";
    cout << "lane switch for " << *v << " at " << *r <<'\n';

    if (r->occupied())
	return(NULL);

    /*if ((r->neighbor(dir.right()) != NULL) && 
	(r->neighbor(dir.right())->occupied()))
	return(NULL);

    if ((r->neighbor(dir.left()) != NULL) && 
	(r->neighbor(dir.left())->occupied()))
	return(NULL);
don;t check right or left */

    if ((r->neighbor(dir.back()) != NULL) && 
	(r->neighbor(dir.back())->occupied()))
	return(NULL);

cout << "lane switch said true \n";
    return(r);
}


//f from notes
roadlet * strait (roadlet *r, vehicle *v, direction d)
{
    if (!r->occupied() && (v->direction() == d))
	return(r);
    else
	return(NULL);


}


// h from notes
roadlet * strait_or_left (roadlet *r, vehicle *v, direction d)
{
    if (!r->occupied() && 
          ((v->direction() == d) || (v->direction().left() == d)))
	return(r);
    else
	return(NULL);
}


//g from notes
roadlet * strait_or_right (roadlet *r, vehicle *v, direction d)
{
    if (!r->occupied() && 
          ((v->direction() == d) || (v->direction().right() == d)))
	return(r);
    else
	return(NULL);
}

roadlet * green_light(intersection_roadlet* r, vehicle* v, direction d)
{
    if ((d == N) || (d == S))
    {
	if (!r->occupied() && (r->light()->greenNS()))
	    return(r);
        else
	    return(NULL);
    }
    else
    {
	if (!r->occupied() && (r->light()->greenEW()))
	    return(r);
        else
	    return(NULL);
    }
}

roadlet * green_OR_plan_rightONred(intersection_roadlet* r, vehicle* v, direction d)
{
    int right_on_red ;
    int green_light;

    cout << "green or right on red\n";

    //if (r->light()->yellow() )
	// stop car;

    if ((d == N) || (d == S))
    {
        right_on_red = (r->light()->redNS()) ; // && no on coming traffic 
	green_light = r->light()->greenNS();
    }
    else
    {
        right_on_red = (r->light()->redEW()) ; // && no on coming traffic 
	green_light = r->light()->greenEW();
    }
       

    if (right_on_red)      //verify that there is a road to the right
    {
        if (r->neighbor(d.right()) == NULL)
	{
	    right_on_red = FALSE;
	}
	else
        {
           v->plan = d.right().as_int();
           cout << "plan for " << *v << " is " << direction(v->plan) << "\n";
        }
    }

    if (green_light) cout << "green!\n";
    if (right_on_red) cout << "right on red\n";
    if (!r->occupied() && (right_on_red || green_light))
    {
	return(r);
    }
    else
	return(NULL);
}
// direction.cc

#include "direction.h"

char *direction::as_string()
{
    char *dirs[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW", 
		     "No direction"};
    return(dirs[dir]);
}
int operator== (direction d1, direction d2)
{
    return (d1.dir == d2.dir);
}
int operator!= (direction d1, direction d2)
{
    return (d1.dir != d2.dir);
}


int operator<= (direction d1, direction d2)
{
    return (d1.dir <= d2.dir);
}


ostream& operator<< (ostream& o, direction d)
{
    o << d.as_string();
    return (o);
}


const direction N (0);
const direction NE(1);
const direction E (2);
const direction SE(3);
const direction S (4);
const direction SW(5);
const direction W (6);
const direction NW(7);
const direction NO_DIRECTION(8);
#ifdef direction_test
#include "iostream"
main()
{
    direction d(0);

    for(int i =0 ; i < 10; i++)
    {
	cout << d << ' ' << d.right() << ' ' << d.back() << ' ' << d.left() << '\n';
	d++;
    }
}
#endif

// build_intersection.cc

#include "intersection.h"
//extern "C" char *malloc();


void intersection_4x4::connectNin (roadlet *leftlane, roadlet *rightlane)
{
    connect(leftlane, S, roadlets[0][1], N, green_light);
    connect(rightlane, S, roadlets[0][0], N, green_OR_plan_rightONred);
}

void intersection_4x4::connectEin (roadlet *leftlane, roadlet *rightlane)
{
    connect(leftlane, W, roadlets[1][3], E, green_light);
    connect(rightlane, W, roadlets[0][3], E, green_OR_plan_rightONred);
}

void intersection_4x4::connectSin (roadlet *leftlane, roadlet *rightlane)
{
    connect(leftlane, N, roadlets[3][2], S, green_light);
    connect(rightlane, N, roadlets[3][3], S, green_OR_plan_rightONred);
}

void intersection_4x4::connectWin (roadlet *leftlane, roadlet *rightlane)
{
    connect(leftlane, E, roadlets[2][0], W, green_light);
    connect(rightlane, E, roadlets[3][0], W, green_OR_plan_rightONred);
}

void intersection_4x4::connectNout(roadlet *leftlane, roadlet *rightlane)
{
    connect(roadlets[0][2], N, leftlane, S, strait);
    connect(roadlets[0][3], N, rightlane, S, strait_or_right);
}
void intersection_4x4::connectEout(roadlet *leftlane, roadlet *rightlane)
{
    connect(roadlets[2][3], E, leftlane, W, strait);
    connect(roadlets[3][3], E, rightlane, W, strait_or_right);
}
void intersection_4x4::connectSout(roadlet *leftlane, roadlet *rightlane)
{
    connect(roadlets[3][1], S, leftlane, N, strait);
    connect(roadlets[3][0], S, rightlane, N, strait_or_right);
}
void intersection_4x4::connectWout(roadlet *leftlane, roadlet *rightlane)
{
    connect(roadlets[1][0], W, leftlane, E, strait);
    connect(roadlets[0][0], W, rightlane, E, strait_or_right);
}


intersection_4x4::intersection_4x4(char *name)
{
    char *buff;

    int i,j;

    light_type *l;

    l = new light();
    the_light = l;

    for(i=0; i<4; i++)
	for(j=0; j<4; j++)
	{
	    buff = malloc(strlen(name) + 7);
	    sprintf(buff, "%s %d %d", name, i, j);
	    roadlets[i][j] = new intersection_roadlet(buff, l);
	}


    for(i=3; i>0; i--)
    {
	connect(roadlets[i][3], N, roadlets[i-1][3], S, &strait);
	connect(roadlets[i][2], N, roadlets[i-1][2], S, &strait);
    }
    for(i=0; i<3; i++)
    {
	connect(roadlets[i][0], S, roadlets[i+1][0], N, &strait);
	connect(roadlets[i][1], S, roadlets[i+1][1], N, &strait);
    }
    for(j=0; j<3; j++)
    {
	connect(roadlets[2][j], E, roadlets[2][j+1], W, &strait);
	connect(roadlets[3][j], E, roadlets[3][j+1], W, &strait);
    }
    for(j=3; j>0; j--)
    {
	connect(roadlets[0][j], W, roadlets[0][j-1], E, &strait);
	connect(roadlets[1][j], W, roadlets[1][j-1], E, &strait);
    }

    // override center square
// FIX ME i think car can circle forever in the middle of the intersection!
// FIX with PLAN of lefty is strait
    connect(roadlets[2][2], N, roadlets[1][2], S, &strait_or_left);
    connect(roadlets[1][2], W, roadlets[1][1], E, &strait_or_left);
    connect(roadlets[1][1], S, roadlets[2][1], N, &strait_or_left);
    connect(roadlets[2][1], E, roadlets[2][2], W, &strait_or_left);

car b("blocker");
b.set_location(roadlets[0][2]);         // a car
roadlets[0][2]->arrive(&b);
cout << b << '\n';


}
// light.cc

#include "broken_light.h"
#include "iostream"

void light::tick()
{
    time_remaining_in_current_state--;
    if (time_remaining_in_current_state <= 0)
    {
        current_state = this->next_state();

	cout << "current state = "<< current_state << '\n';
	time_remaining_in_current_state = time_in_state[current_state];
    }
}



void light::init(int t1, int t2, int t3, int t4) 
{
    __ = LIGHT_ID;
    current_state = 0;
    time_in_state[0] = t1;
    time_in_state[1] = t2;
    time_in_state[2] = t3;
    time_in_state[3] = t4;
    time_remaining_in_current_state = time_in_state[current_state];
}


ostream& operator<< (ostream & o, light l)
{
    o << l.current_state 
      << " " << l.redNS() << l.yellowNS() << l.greenNS() 
      << " " << l.redEW() << l.yellowEW() << l.greenEW();
    return(o);
}

#ifdef test_light
#include "iostream"

void main()
{
    light l(3,1,4,1);

    for(int i = 0; i < 20; i++)
    {
	cout << l << "\n";
	l.tick();
    }
}
#endif
// roadlet.cc

#include <iostream>
#include "roadlet.h"

void roadlet::init(char *n)      
{
    occupant = NULL;
    for(int i=0;i<8;i++) 
        neighbors[i] = NULL;
    name = n; 
    for(int i=0;i<8;i++) 
	the_moves[i] = return_null; 
}


ostream& operator<<(ostream& o, roadlet r)
{
    o << "roadlet " << r.name;
    return (o);
}
// vehicle.cc
// speed in (ticks per roadlet) / 100
// speed < 100 yields multiple ticks per move.
// speed > 100 yields multiple moves per tick.

#include <iostream>
#include <stdio.h>
#include "sim.h"
#include "direction.h"
#include "vehicle.h"
#include "roadlet.h"


ostream& operator<<(ostream& o, vehicle v)
{
    o << v.name << " at " << *(v.location) << " going " << v.direction();
    return (o);
}


void vehicle::tick()
{
    move_points = move_points + 100;
    while (move_points >= speed)
    {
	move();
	move_points = move_points - speed;
    }
}


void vehicle::move()
{
    direction d = NO_DIRECTION;

    if (plan != -1)
    {
	cout << "there is a plan! "<< plan << " \n";
        if (location->moves()[plan](location->neighbor(plan),this,plan) != NULL)
	{
	    d = direction(plan);
	    plan = -1;
	}  // else wait for plan to be possible
    }
    else
        d = select_move();

    if (d != NO_DIRECTION)
    {
        roadlet *r;

        location->depart();
        r = location->neighbor(d);
        r->arrive(this);
        location = r;
	if ((d == N) || (d == S) || (d == E) || (d == W))
	    dir = d;	// else = lane change only

    }
}


direction vehicle::select_move()
{
    roadlet *(**possible_moves)(roadlet*, vehicle *, type_direction);
    direction dir, move_to_the[8];
    int used;

    possible_moves = location->moves();

    for(used=0, dir=N; dir<=NW; dir ++)
    {
	// cout << "possible_moves[" << dir << "] " << possible_moves[dir.as_int()] << '\n';
	if (possible_moves[dir.as_int()](location->neighbor(dir), this, dir) != NULL)
	{
	    move_to_the[used] = dir;
	    used++;
	}
    }

    if (used == 0)
    {
        cout << "vehicle::select_move "<< *this << " has no where to go!\n";
	return(NO_DIRECTION);
    }
    else
    {
        int use = (int)random() % used;
	return(move_to_the[use]);
    }
}


// Car stuff
ostream& operator<< (ostream& o, car c)
{
    o << "Car " << (vehicle)c; 
    return (o);
}


// Truck stuff
ostream& operator<< (ostream& o, truck t)
{
    o << "Truck " << (vehicle)t; 
    return (o);
}
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
#include "iostream"
#endif
