struct mystruct 
{ 
  int x; 
  struct mystruct *q, *r; 
}; 
 
int main(struct mystruct * p)
{
  p->q->r->x=9;
  return p->q->x;
}
