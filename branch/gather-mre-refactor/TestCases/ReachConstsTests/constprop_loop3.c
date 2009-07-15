/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
! A simple program to test reaching constants when there is a loop and 
! if stmt
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
  
int main()
{
  int x, y, z;
  int i;

  /*
  !Set all proc def locations initially to bottom as OUT set of Entry node
  !Set all proc def locations initially to top at OUT set of all other nodes
  !This facilitates constants defined above a loop reaching into a loop
  !  when they are not re-defined within the loop to a different value, as
  !  well as correctly handling branches outside of the loop.
  */

  /* !Reaching Consts x=BOTTOM, y=BOTTOM, z=BOTTOM, i=BOTTOM */
  x = 2;

  /*!Reaching Consts x=2, y=BOTTOM, z=BOTTOM, i=BOTTOM
  !i = 1    ! compiler supplied */

  for( i = 1; i < 10; i++ ) {

    /* !Reaching Consts x=2, y=BOTTOM, z=BOTTOM, i=BOTTOM */
    y = 3;

    /* !Reaching Consts x=2, y=3, z=BOTTOM */
    if ( x >= 0 ) {

      /* !Reaching Consts x=2, y=3, z=BOTTOM   */
      y = 5;

      /* !Reaching Consts x=2, y=5, z=BOTTOM */
      z = y + y;
      
    } else {

      /* !Reaching Consts x=2, y=3, z=BOTTOM */
      y = 5;
      
    }

    /* !Reaching Consts x=2, y=5, z=BOTTOM */
    z = y;

    /*
    !Reaching Consts x=2, y=5, z=5
    !i = i + 1    ! compiler supplied
    */
  }

    /* !Reaching Consts x=2, y=BOTTOM, z=BOTTOM */
  return 0;

}

