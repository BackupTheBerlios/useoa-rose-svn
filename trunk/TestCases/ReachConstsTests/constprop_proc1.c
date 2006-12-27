/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
! most precise results (not here yet)
! p should be constant before the call, and not after the call
! v should be constant after the call, but not before the call
! m should be constant before and after the call
!
! still at call-return interference ...
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

 void bar(int *a, int *b, int *c, int *d);


 int main()
 {
       int x;
       int f;
       int m, p, v;


       /* all BOTTOM */
       m = 2;

       /* all BOTTOM, m=2 */
       p = 5;
       
       /* all BOTTOM, m=2, p=5 */
       bar(&m,&p,&v,&x);

       /*  most precise: all BOTTOM, m=2, v=10
           call-return interference causes: all BOTTOM, m=2
       */
       
       f = m + p + v + x;

       /* most precise: all BOTTOM, m=2, v=10
          call-return interference causes: all BOTTOM, m=2
       */   

       return 0;
 }       


 void bar(int *a, int *b, int *c, int *d)
 {
       /* all BOTTOM, a=2, b=5 */
       (*c) = (*a) * (*b);

       /* all BOTTOM, a=2, b=5, c=10 */
       (*b) = (*c) - (*d);

       /* all BOTTOM, a=2, c=10 */
       return;

       /* all BOTTOM, a=2, c=10 */
 }
