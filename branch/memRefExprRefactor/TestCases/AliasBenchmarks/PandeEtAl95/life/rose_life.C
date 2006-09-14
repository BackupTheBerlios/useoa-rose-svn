// Section 9.6, "Object-Oriented Programming using C++"
// by Ira Pohl.  Benjamin/Cummings Publishing Company, 1993
// size of square board
static const int N = 40;
static const int STATES = 4;
static const int DRAB = 3;
static const int DFOX = 8;
static const int CYCLES = 10;
enum state {EMPTY,GRASS,RABBIT,FOX};
// forward decl
// world is simulation
class living ;
typedef class living *world[40][40];
// what lives in world

class living 
{
// location
  protected: int row;
  int column;
// sm[#states] used by next()
  void sums(world w,int sm[]);
  

  public: inline living(int r,int c) : row(r), column(c)
{
  }

// state identification
  virtual enum state who() = 0;
// compute next
  virtual living *next(world w) = 0;
}

;

void living::sums(class living *(*w)[40],int *sm)
{
  int i;
  int j;
  sm[EMPTY] = (sm[GRASS] = (sm[RABBIT] = (sm[FOX] = 0)));
  for (i = (-1); i <= 1; ++i) {
    for (j = (-1); j <= 1; ++j) {
      sm[( *((w[((this) -> row) + i])[((this) -> column) + j])).who()]++;
    }
  }
}


class fox : public living
{
  protected: int age;
  

  public: inline fox(int r,int c,int a=0) : living(r,c), age(a)
{
  }

  

  virtual inline enum state who()
{
    return FOX;
  }

  virtual living *next(world w);
}

;

class rabbit : public living
{
  protected: int age;
  

  public: inline rabbit(int r,int c,int a=0) : living(r,c), age(a)
{
  }

  

  virtual inline enum state who()
{
    return RABBIT;
  }

  virtual living *next(world w);
}

;

class grass : public living
{
  

  public: inline grass(int r,int c) : living(r,c)
{
  }

  

  virtual inline enum state who()
{
    return GRASS;
  }

  virtual living *next(world w);
}

;

class empty : public living
{
  

  public: inline empty(int r,int c) : living(r,c)
{
  }

  

  virtual inline enum state who()
{
    return EMPTY;
  }

  virtual living *next(world w);
}

;

living *grass::next(class living *(*w)[40])
{
  int sum[4];
  (this) -> sums(w,sum);
// eat grass
  if ((sum[GRASS]) > (sum[RABBIT])) {
    return (new grass (((this) -> row),((this) -> column)));
  }
  else {
    return (new empty (((this) -> row),((this) -> column)));
  }
}


living *rabbit::next(class living *(*w)[40])
{
  int sum[4];
  (this) -> sums(w,sum);
// eat rabbits
  if ((sum[FOX]) >= (sum[RABBIT])) {
    return (new empty (((this) -> row),((this) -> column)));
  }
  else 
// rabbit too old
{
    if (((this) -> age) > DRAB) {
      return (new empty (((this) -> row),((this) -> column)));
    }
    else {
      return (new rabbit (((this) -> row),((this) -> column),(((this) -> age) + 1)));
    }
  }
}


living *fox::next(class living *(*w)[40])
{
  int sum[4];
  (this) -> sums(w,sum);
// too many foxes
  if ((sum[FOX]) > 5) {
    return (new empty (((this) -> row),((this) -> column)));
  }
  else 
// fox too old
{
    if (((this) -> age) > DFOX) {
      return (new empty (((this) -> row),((this) -> column)));
    }
    else {
      return (new fox (((this) -> row),((this) -> column),(((this) -> age) + 1)));
    }
  }
}


living *empty::next(class living *(*w)[40])
{
  int sum[4];
  (this) -> sums(w,sum);
  if ((sum[FOX]) > 1) {
    return (new fox (((this) -> row),((this) -> column)));
  }
  else 
// fox too old
{
    if ((sum[RABBIT]) > 1) {
      return (new rabbit (((this) -> row),((this) -> column)));
    }
    else {
      if ((sum[GRASS])) {
        return (new grass (((this) -> row),((this) -> column)));
      }
      else {
        return (new empty (((this) -> row),((this) -> column)));
      }
    }
  }
}

// world is empty

void init(class living *(*w)[40])
{
  int i;
  int j;
  for (i = 0; i < N; ++i) {
    for (j = 0; j < N; ++j) {
      (w[i])[j] = ((new empty (i,j)));
    }
  }
}

// new world from old world

void update(class living *(*w_new)[40],class living *(*w_old)[40])
{
  int i;
  int j;
  for (i = 1; i < (N - 1); ++i) {
    for (j = 1; j < (N - 1); ++j) {
      (w_new[i])[j] = ( *((w_old[i])[j])).next(w_old);
    }
  }
}

// clean world up

void dele(class living *(*w)[40])
{
  int i;
  int j;
  for (i = 1; i < N; ++i) {
    for (j = 1; j < N; ++j) {
      :: delete ((w[i])[j]);
    }
  }
}


void eden(class living *(*w)[40])
{
  int i;
  int j;
  for (i = 1; i < N; ++i) {
    for (j = 1; j < N; ++j) {
      if (i == j) {
        (w[i])[j] = ((new fox (i,j)));
      }
      else {
        if (i < j) {
          (w[i])[j] = ((new rabbit (i,j)));
        }
        else {
          (w[i])[j] = ((new grass (i,j)));
        }
      }
    }
  }
}


int main()
{
  world odd;
  world even;
  int i;
  init(odd);
  init(even);
  eden(even);
// simulation
  for (i = 0; i < CYCLES; ++i) {
    if ((i % 2)) {
      update(even,odd);
      dele(odd);
    }
    else {
      update(odd,even);
      dele(even);
    }
  }
}

