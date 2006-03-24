/*
 * from Commutativity Analysis: A New Analysis Framework for Parallelizing 
 * Compilers, Martin C. Rinard and Pedro C. Diniz
 * SIGPLAN Conference on Programming Language Design and Implementation 1996.
 * Figure 4.
 */

#include <math.h>
#include <stdlib.h>

const int NDIM=3;

class vector {
 public:
  double val[NDIM];
  int i;
  void vecAdd(double v[NDIM]) {
    for(i = 0; i < NDIM; i++)
      val[i] += v[i];
  }
};

class node {
 public:
  virtual void someThing() { }
  double mass;
  vector pos;
};

const int NSUB = 8;

class cell : public node {
 public:
  node *subp[NSUB];
};

const int LEAFMAXBODIES = 16;

class body;

class leaf : public node {
 public:
  int numbodies;
  body *bodyp[LEAFMAXBODIES];
};

class body : public node {
  vector vel;
  vector acc;
  double phi;
 public:
  double subdivp(node *p, double dsq);
  void gravsub(node *n);
  double computeInter(node *n, double *res);
  void openCell(cell *c, double dsq);
  void openLeaf(leaf *l);
  void walksub(node *n, double dsq);
};

class nbody {
 public:
  int numbodies;
  body **bodies;
  node *BH_root;
  double size;
  void computeForces();
};

class parms {
 public:
  double tolSq;
  double eps;
  double epsSq;
};

parms Parms;
nbody Nbody;

double body::subdivp(node *n, double dsq)
{
  double drsq, d;
  drsq = Parms.epsSq;
  for(int i = 0; i < NDIM; i++) {
    d = n->pos.val[i] - pos.val[i];
    drsq += d*d;
  }

  return ((Parms.tolSq * drsq ) < dsq);
}

double body::computeInter(node *n, double *res)
{
  double inc, r, drsq, d;
  drsq = Parms.eps;
  for(int i = 0; i < NDIM; i++) {
    d = n->pos.val[i] - pos.val[i];
    drsq += d*d;
  }
  
  inc = n->mass/sqrt(drsq);
  r = inc/drsq;
  for(int i = 0; i < NDIM; i++)
    res[i] *= r;
  return inc;
}

void body::gravsub(node *n) 
{
  double d;
  double tmpv[NDIM];

  d = this->computeInter(n, tmpv);
  phi -= d;
  //  acc.vecAdd(tmpv);
}

void body::openCell(cell *c, double dsq)
{
  node *n;
  for(int i = 0; i < NSUB; i++) {
    n = c->subp[i];
    if (n != NULL)
      this->walksub(n,(dsq/4.0));
  }
}

void body::openLeaf(leaf *l)
{
  body *b;
  for(int i = 0; i < l->numbodies; i++) {
    b = l->bodyp[i];
    if (b != this)
      this->gravsub(b);
  }
}

void body::walksub(node *n, double dsq)
{
  cell *c;
  leaf *l;

  if (this->subdivp(n,dsq)) {
    c = dynamic_cast<cell*>(n);
    if (c != NULL) {
      this->openCell(c,dsq);
    } else {
      l = dynamic_cast<leaf*>(n);
      if (l != NULL)
	this->openLeaf(l);
    }
  } else {
    this->gravsub(n);
  }
}

void nbody::computeForces() 
{
  body *b;
  for(int i = 0; i < numbodies; i++) {
    b = bodies[i];
    b->walksub(BH_root, size*size);
  }
}
