#include "screen1.h"
// begin screen1.cpp implementation

void printf(char *s,int i,int j)
{
}


void cursor_controller::up(int rows)
{
  printf(("%dA"),rows,0);
}


void cursor_controller::down(int rows)
{
  printf(("%dB"),rows,0);
}


void cursor_controller::right(int cols)
{
  printf(("%dC"),cols,0);
}


void cursor_controller::left(int cols)
{
  printf(("%dD"),cols,0);
}


void cursor_controller::move(int row,int col)
{
  printf(("%d;%dH"),row,col);
}


void cursor_controller::clear_screen()
{
  printf(("2J"),0,0);
}


void cursor_controller::clear_eol()
{
  printf(("K"),0,0);
}


void cursor_controller::save()
{
  printf(("s"),0,0);
}


void cursor_controller::restore()
{
  printf(("u"),0,0);
}

// end screen1.cpp
#include "screen2.h"
//begin screen2 implementation

void cursor_controller2::normal()
{
  printf(("0"),0,0);
}


void cursor_controller2::high_intensity()
{
  printf(("1"),0,0);
}


void cursor_controller2::blink()
{
  printf(("5"),0,0);
}


void cursor_controller2::reverse()
{
  printf(("7"),0,0);
}


void cursor_controller2::invisible()
{
  printf(("8"),0,0);
}

#include "screen3.h"
// begin screen3 implementation
class screen_controller screen;

void error(char *message)
{
}


int strlen(char *str)
{
  return 0;
}


screen_controller::screen_controller(int rows,int cols) : rowmax(rows), colmax(cols), cursor_controller2()
{
  if (object_count) {
    error(("only one instance allowed"));
  }
  else {
    object_count = 1;
  }
  printf(("turn off wrap"),0,0);
}


void screen_controller::setrows(int rows)
{
  (this) -> rowmax = rows;
}


void screen_controller::setcols(int cols)
{
  (this) -> colmax = cols;
}


screen_controller::~screen_controller()
{
  ((class cursor_controller2 *)(this)) -> ~cursor_controller2();
  (this) -> normal();
  printf(("turn on line wrap"),0,0);
}


void screen_controller::upper_left()
{
  (this) -> move(1,1);
}


void screen_controller::lower_left()
{
  (this) -> move(((this) -> rowmax),1);
}


void screen_controller::upper_right()
{
  (this) -> move(1,((this) -> colmax));
}


void screen_controller::lower_right()
{
  (this) -> move(((this) -> rowmax),((this) -> colmax));
}


void screen_controller::draw_vertical(int row,int col,int length,char l_char)
{
  bool rose_sc_bool_0 = false;
  if ((row > ((this) -> rowmax))) {{
      rose_sc_bool_0 = true;
    }
  }
  else {
    if (((row + length) > ((this) -> rowmax))) {
      rose_sc_bool_0 = true;
    }
    else {
    }
  }
  if (rose_sc_bool_0) {
    error(("draw_vertical: row index out of bounds"));
  }
  else {
  }
  if (col > ((this) -> colmax)) {
    error(("draw_vertical: col index out of bounds"));
  }
  else {
  }
  for (int rrow = row; rrow <= (row + length); rrow++) {
    (this) -> cursor_controller::move(rrow,col);
    printf(("%c"),((int )l_char),0);
  }
}


void screen_controller::draw_horizontal(int row,int col,int length,char l_char)
{
  bool rose_sc_bool_1 = false;
  if ((col > ((this) -> colmax))) {{
      rose_sc_bool_1 = true;
    }
  }
  else {
    if (((col + length) > ((this) -> colmax))) {
      rose_sc_bool_1 = true;
    }
    else {
    }
  }
  if (rose_sc_bool_1) {
    error(("draw_horizontal: row index out of bounds"));
  }
  else {
  }
  if (row > ((this) -> rowmax)) {
    error(("draw_horizontal: col index out of bounds"));
  }
  else {
  }
  for (int ccol = col; ccol <= (col + length); ccol++) {
    (this) -> cursor_controller::move(row,ccol);
    printf(("%c"),((int )l_char),0);
  }
}


void screen_controller::center(int row,char *text)
{
  (this) -> move(row,((((this) -> colmax) - strlen(text)) / 2));
  printf(("%c"),((int )( *text)),0);
}


void screen_controller::move(int row,int col)
{
  if (row > ((this) -> rowmax)) {
    error(("move: row index out of bounds"));
  }
  else {
  }
  if (col > ((this) -> colmax)) {
    error(("move: col index out of bounds"));
  }
  else {
  }
  (this) -> cursor_controller::move(row,col);
}


void screen_controller::pause(int seconds)
{
}


void screen_controller::drawbox(int hor,int ver,int ul,int ur,int ll,int lr)
{
  (this) -> draw_vertical(0,0,((this) -> maxrow() - 1),(ver));
  (this) -> draw_vertical(0,(this) -> maxcol(),((this) -> maxrow() - 1),(ver));
  (this) -> draw_horizontal(0,0,((this) -> maxcol() - 1),(hor));
  (this) -> draw_horizontal((this) -> maxrow(),0,((this) -> maxcol() - 1),(hor));
  (this) -> upper_left();
  printf(("%d"),ul,0);
  (this) -> lower_left();
  printf(("%d"),ll,0);
  (this) -> upper_right();
  printf(("%d"),ur,0);
  (this) -> lower_right();
  printf(("%d"),lr,0);
}

// end screen3 implementation
#include "simulate.h"
class simulation_unit *s_grid[25][80];

class init_grid 
{
  public: init_grid();
  

  ~init_grid()
{
  }

  

  init_grid(class init_grid &rhs)
{
  }

  

  init_grid &operator=(class init_grid &rhs)
{
    if ((this) == &rhs) {
      return  *(this);
    }
    else {
    }
    return  *(this);
  }

}

;
class init_grid grid_initializer;

init_grid::init_grid()
{
  for (int i = 0; i < 25; i++) {
    for (int j = 0; j < 80; j++) {
      (s_grid[i])[j] = ((0));
    }
  }
}


void simulation_unit::move(int x_steps,int y_steps)
{
  int x_new = (((this) -> x) + x_steps);
  int y_new = (((this) -> y) + y_steps);
  bool rose_sc_bool_2 = false;
  if ((x_new < 0)) {{
      rose_sc_bool_2 = true;
    }
  }
  else {
    if ((x_new >= 25)) {
      rose_sc_bool_2 = true;
    }
    else {
    }
  }
  if (rose_sc_bool_2) {
    error(("move: x coord out of bounds"));
  }
  else {
  }
  bool rose_sc_bool_3 = false;
  if ((y_new < 0)) {{
      rose_sc_bool_3 = true;
    }
  }
  else {
    if ((y_new >= 80)) {
      rose_sc_bool_3 = true;
    }
    else {
    }
  }
  if (rose_sc_bool_3) {
    error(("move: y coord out of bounds"));
  }
  else {
  }
  if (((s_grid[x_new])[y_new]) == ((0))) {
    (s_grid[(this) -> x])[(this) -> y] = ((0));
    (this) -> erase();
    (s_grid[(this) -> x = x_new])[(this) -> y = y_new] = (this);
    (this) -> display();
  }
  else {
  }
}

// now the test of simulation

class pop_around : public simulation_unit
{
  

  public: virtual inline void display()
{
    screen.move(((this) -> x),((this) -> y));
    printf(("@"),0,0);
  }

  

  virtual inline void erase()
{
    screen.move(((this) -> x),((this) -> y));
    printf((""),0,0);
  }

  

  inline pop_around(int xi,int yi) : simulation_unit(xi,yi)
{
    (this) -> pop_around::display();
  }

  

  virtual inline ~pop_around()
{
    ((class simulation_unit *)(this)) -> ~simulation_unit();
  }

  

  virtual inline void cycle()
{
    int x_direction;
    int y_direction;
    if (((this) -> x) == 0) {
      x_direction = 0;
    }
    else {
      if (((this) -> x) == (25 - 1)) {
        x_direction = 1;
      }
      else {
        x_direction = 2;
      }
    }
    if (((this) -> y) == 0) {
      y_direction = 0;
    }
    else {
      if (((this) -> y) == (80 - 1)) {
        y_direction = 1;
      }
      else {
        y_direction = 2;
      }
    }
    int x_jump;
    if (x_direction) {
      x_jump = (-1);
    }
    else {
      x_jump = 1;
    }
    int y_jump;
    if (y_direction) {
      y_jump = (-1);
    }
    else {
      y_jump = 1;
    }
    (this) -> move(x_jump,y_jump);
  }

  

  pop_around() : simulation_unit()
{
  }

  

  pop_around(class pop_around &rhs) : simulation_unit(rhs)
{
  }

  

  pop_around &operator=(class pop_around &rhs)
{
    (*((class simulation_unit *)(this)))=((class simulation_unit &)rhs);
    if ((this) == &rhs) {
      return  *(this);
    }
    else {
    }
    return  *(this);
  }

}

;

class crawl_around : public simulation_unit
{
  

  public: virtual inline void display()
{
    screen.move(((this) -> x),((this) -> y));
    printf(("*"),0,0);
  }

  

  virtual inline void erase()
{
    screen.move(((this) -> x),((this) -> y));
    printf((""),0,0);
  }

  

  inline crawl_around(int xi,int yi) : simulation_unit(xi,yi)
{
    (this) -> crawl_around::display();
  }

  

  virtual inline ~crawl_around()
{
    ((class simulation_unit *)(this)) -> ~simulation_unit();
  }

  

  virtual inline void cycle()
{
    int x_step = ((0)?(-1):1);
    if ((((this) -> x) + x_step) < 0) {
      x_step = (-x_step);
    }
    else {
    }
    if ((((this) -> x) + x_step) >= 25) {
      x_step = (-x_step);
    }
    else {
    }
    int y_step = ((0)?(-1):1);
    if ((((this) -> y) + y_step) < 0) {
      y_step = (-y_step);
    }
    else {
    }
    if ((((this) -> y) + y_step) >= 80) {
      y_step = (-y_step);
    }
    else {
    }
    (this) -> move(x_step,y_step);
  }

  

  crawl_around() : simulation_unit()
{
  }

  

  crawl_around(class crawl_around &rhs) : simulation_unit(rhs)
{
  }

  

  crawl_around &operator=(class crawl_around &rhs)
{
    (*((class simulation_unit *)(this)))=((class simulation_unit &)rhs);
    if ((this) == &rhs) {
      return  *(this);
    }
    else {
    }
    return  *(this);
  }

}

;

int main()
{
  screen.clear_screen();
  int pop_factor = 1;
  int crawl_factor = 2;
  for (int x = 0; x < 25; x++) {
    for (int y = 0; y < 80; y++) {
      if (pop_factor) {
        if (!(((s_grid[x])[y]))) {
          (s_grid[x])[y] = ((new pop_around (x,y)));
        }
        else {
        }
      }
      else {
      }
      if (crawl_factor) {
        if (!(((s_grid[x])[y]))) {
          (s_grid[x])[y] = ((new crawl_around (x,y)));
        }
        else {
        }
      }
      else {
      }
    }
  }
  int xrand = 5;
  int yrand = 6;
  while((1)){
    int x_location = xrand;
    int y_location = yrand;
    if (((s_grid[x_location])[y_location])) {
      ( *((s_grid[x_location])[y_location])).cycle();
    }
    else {
    }
  }
}

