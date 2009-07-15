// Tree.cc
#include "tree.h"

void strcpy(char *nam1,char *nam2)
{
   *nam1 =  *nam2;
}


int strlen(char *nam)
{
  return 0;
}


Tree::Tree(float n)
{
  (this) -> nodePtr = ((new RealNode (n)));
}


Tree::Tree(char *ch)
{
  (this) -> nodePtr = ((new RealNode ((0.0),ch)));
}


Tree::Tree(char *op,class Tree t)
{
  (this) -> nodePtr = ((new UnaryNode (op,(t))));
}


Tree::Tree(class Tree left,char *op,class Tree right)
{
  (this) -> nodePtr = ((new BinaryNode (op,(left),(right))));
}


Tree::~Tree()
{
  if (--( *((this) -> nodePtr)).use == 0) {
    delete ((this) -> nodePtr);
  }
  else {
  }
}


void Tree::operator=(const class Tree &t)
{
  ++( *(t.nodePtr)).use;
  if (--( *((this) -> nodePtr)).use == 0) {
    delete ((this) -> nodePtr);
  }
  else {
  }
  (this) -> nodePtr = (t.nodePtr);
}


Tree::Tree(const class Tree &t)
{
  (this) -> nodePtr = (t.nodePtr);
  ++( *((this) -> nodePtr)).use;
}


float Tree::value()
{
  return ( *((this) -> nodePtr)).nodeValue();
}


float Tree::operator()(float x,float y,float z)
{
  ((values0 = x) , values1 = y) , values2 = z;
  return (this) -> value();
}

// BinaryNode.cc
#define LARGE 9999999.0

BinaryNode::BinaryNode(char *a,class Tree b,class Tree c) : Node()
{
  (this) -> left = (new Tree (b));
  (this) -> right = (new Tree (c));
  strlen(a) , (this) -> op = (new char );
  strcpy(((this) -> op),a);
}


float BinaryNode::nodeValue()
{
  float num;
  float den;
  if ((( *((this) -> op))) == ('+')) {
    return ( *((this) -> left)).value() + ( *((this) -> right)).value();
  }
  else {
    num = ( *((this) -> left)).value();
    den = ( *((this) -> right)).value();
    return (9999999.0);
  }
}

// UnaryNode.cc

UnaryNode::UnaryNode(char *a,class Tree b) : Node()
{
  (this) -> opnd = (new Tree (b));
  strlen(a) , (this) -> op = (new char );
  strcpy(((this) -> op),a);
}


float UnaryNode::nodeValue()
{
  if ((( *((this) -> op))) == ('-')) {
    return -(( *((this) -> opnd)).value());
  }
  else {
    if ((( *((this) -> op))) == ('+')) {
      return ( *((this) -> opnd)).value();
    }
    else {
    }
  }
}

// RealNode.cc

RealNode::RealNode(float k,char *ch) : Node()
{
  (this) -> n = k;
  if (ch) {
    strlen(ch) , (this) -> symbol = (new char );
    strcpy(((this) -> symbol),ch);
  }
  else {
  }
}


float RealNode::nodeValue()
{
  if (((this) -> symbol) == ((0))) {
    return (this) -> n;
  }
  else {
    if ((( *((this) -> symbol))) == ('x')) {
      return values0;
    }
    else {
      if ((( *((this) -> symbol))) == ('y')) {
        return values1;
      }
      else {
        return values2;
      }
    }
  }
}

// main.cc

int main()
{
  class Tree t1((1));
  class Tree t2(("u"));
  class Tree t3((5));
  class Tree t4((t1),("*"),(t2));
  class Tree t5(("-"),(t3));
  t4((12.0),(0),(0));
  t5.value();
  class Tree t6((t1),("/"),(t3));
  t6((12.12),(0),(0));
  class Tree t7((t1),("+"),(t5));
  t7.value();
}

