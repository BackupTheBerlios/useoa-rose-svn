/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
! A simple program to const reaching definitions when there is an if stmt
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

int main()
{
    /*
!  double precision :: x, y, z
! for ConstProp, only ConstValBasicInterface that is defined
! and useful is Open64IntegerConstVal
! so am making x, y, and z into integers
!
*/
    
  int x, y, z;

  /* !Reaching Consts x=TOP, y=TOP, z=TOP */
  x = 2;

  /* !Reaching Consts x=2, y=TOP, z=TOP */
  if ( x >= 0 ) {

    /* !Reaching Consts x=2, y=TOP, z=TOP */
    y = 5;

    /* !Reaching Consts x=2, y=5, z=TOP */
    x = y + y;

    /* !Reaching Consts x=10, y=5, z=TOP */
    z = 7;
  }  
  else {

    /* !Reaching Consts x=2, y=TOP, z=TOP */
    y = 3;

    /* !Reaching Consts x=2, y=3, z=TOP */
    x = 10;
  }

  /* !Reaching Consts x=10, y=BOTTOM, z=BOTTOM */
  z = y;

  return 0;
}

