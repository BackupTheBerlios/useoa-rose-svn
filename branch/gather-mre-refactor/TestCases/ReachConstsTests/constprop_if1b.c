/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
! A simple program to const reaching definitions when there is an if stmt
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

int main() {
    
/*    
!  double precision :: x, y, z
! for ConstProp, only ConstValBasicInterface that is defined
! and useful is Open64IntegerConstVal
! so am making x, y, and z into integers
!
*/
    
  int x, y, z;

  /* !Reaching Constant x=TOP, y=TOP, z=TOP */
  x = 2;

  /* !Reaching Constant x=2, y=TOP, z=TOP */
  if ( x > 0 ) {

    /* !Reaching Constant x=2, y=TOP, z=TOP  */
    y = 5;

    /* !Reaching Constant z=2, y=5, z=TOP */
    x = y + y ;
    
  } else {

    /* !Reaching Constant x=2, y=TOP, z=TOP */ 
    y = 3;

    /* !Reaching Constant x=2, y=3, z=TOP */
    x = 10;
  }

  /* !Reaching Constant x=10, y=BOTTOM, z=TOP */
  z = y;

  /*!Reaching Constant x=10, y=BOTTOM, z=BOTTOM */

  return 0;
}  

