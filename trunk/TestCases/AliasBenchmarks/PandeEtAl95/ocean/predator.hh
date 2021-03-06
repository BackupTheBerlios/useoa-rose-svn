#ifndef PredatorDef
#define PredatorDef

class Predator : public Prey {
private:
  virtual Cell *reproduce(Coordinate anOffset);

protected:
  unsigned timeToFeed;

public:
  Predator(Coordinate aCoord) : Prey(aCoord) {
    timeToFeed = TimeToFeed;
    image = DefaultPredImage;
  }
  virtual ~Predator(void) {Prey::~Prey();}
  virtual void process(void);
};

#endif
