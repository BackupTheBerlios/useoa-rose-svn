/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! Due to context insensativity across calls, no constants get through 
! either call
!
! Due to context insensitive alias analysis, m, p and v get placed 
! into the same Loc Id set, so they mayOverlap each other.
! Therefore, now, output is all BOTTOM, no constants get thru.
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

  void head(int *x, int *f);
  void bar(int *a,int *b,int *c,int *d);


  int main()
  {
      
      int x,f;
      head(&x, &f);
      return 0;
  }  
 
  void head(int *x, int *f)
  {
       int m, p, v;


       /* ! all BOTTOM */
       m = 2;

       /* ! all BOTTOM, m=2 */
       p = 5;
       
       /* ! all BOTTOM, m=2, p=5 */
       bar(&m, &p, &v, x);

       /*
       ! most precise: all BOTTOM, m=2, v=10 
       ! call-return interference causes: all BOTTOM, m=2
       !    context insensativity causes: all BOTTOM
       */
       m = 5;

       /*
       !                    most precise: all BOTTOM, m=2, v=10
       ! call-return interference causes: all BOTTOM, m=2
       !    context insensativity causes: all BOTTOM, m=2
       */
       p = 2;

       /*
       !                    most precise: all BOTTOM, m=2, p=5, v=10
       ! call-return interference causes: all BOTTOM, m=2, p=5
       !    context insensativity causes: all BOTTOM, m=2, p=5
       */
       
       bar(&p, &m, &v, x);

       /*
       !                    most precise: all BOTTOM, m=50, p=5
       ! call-return interference causes: all BOTTOM,       p=5
       !    context insensativity causes: all BOTTOM
       */
       (*f) = m + p + v + (*x);

       /*
       !                    most precise: all BOTTOM, m=50, p=5
       ! call-return interference causes: all BOTTOM,       p=5
       !    context insensativity causes: all BOTTOM
       */

  }    

  void bar(int *a,int *b,int *c,int *d)
  {
       
       /*    
       ! in context: call#1:               all BOTTOM, a=2, b=5
       ! in context: call#2: most precise: all BOTTOM, a=5, b=10, c=2
       ! in context: call#2: less precise: all BOTTOM, a=5,       c=2
       ! no context:                       all BOTTOM
       */
           
       (*c) = (*a) * (*b);

       /*
       ! in context: call#1:               all BOTTOM, a=2, b=5,  c=10
       ! in context: call#2: most precise: all BOTTOM, a=5, b=10, c=50
       ! in context: call#2: less precise: all BOTTOM, a=5
       ! no context:                       all BOTTOM
       */
       
       (*b) = (*c) - (*d);

       /*
       ! in context: call#1:               all BOTTOM, a=2, c=10
       ! in context: call#2: most precise: all BOTTOM, a=5, c=50
       ! in context: call#2: less precise: all BOTTOM, a=5
       ! no context:                       all BOTTOM
       */
       
       return;

       /*
       ! in context: call#1:               all BOTTOM, a=2, c=10
       ! in context: call#2: most precise: all BOTTOM, a=5, c=50
       ! in context: call#2: less precise: all BOTTOM, a=5
       ! no context:                       all BOTTOM
       */
  }
