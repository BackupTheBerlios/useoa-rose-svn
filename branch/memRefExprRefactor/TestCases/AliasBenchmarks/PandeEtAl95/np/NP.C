// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
//  Written by Hemant Pande, Rutgers University.  December 1994.  May be
//  distributed freely, provided this comment is displayed at the top.
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
class True {
public:
 True() {}
 virtual True *And (True *arg)
 {
   return arg;
 }
} trueObj;

class False : public True {
public:
  False() {}
  True *And (True *arg);
} falseObj;

True *False::And (True *arg)
{
   return &falseObj;
}

True *v1, *nv1, *v2, *nv2, *v3, *nv3;
True *c;

main () {
   if (0) {v1 = &trueObj; nv1 = &falseObj;} else {v1 = &falseObj; nv1= &trueObj;}
   if (0) {v2 = &trueObj; nv2 = &falseObj;} else {v2 = &falseObj; nv2 = &trueObj;}
   if (0) {v3 = &trueObj; nv3 = &falseObj;} else {v3 = &falseObj; nv3 = &trueObj;}
   if (0) c = nv2; else if (0) c = v1; else c = v2;
   if (0) c = c->And(nv3); else if (0) c = c->And(nv2); else c = c->And(v1);
   if (0) c = c->And(v2); else if (0) c = c->And(nv1); else c = c->And(v3);
}
