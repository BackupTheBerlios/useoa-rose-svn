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

  /* !Reaching Consts x=TOP, y=TOP, z=TOP, i=TOP */
  y = 5;

  for( i = 1; i < 10; i++ ) {

    /*!Reaching Consts x=BOTTOM, y=5, z=5, i=BOTTOM */
    x = 2;

    /* !Reaching Consts x=2, y=5 z=5 */
    if ( x >= 0 ) {

      /*!Reaching Consts x=2, y=5, z=5 */
      y = 5;

      /* !Reaching Consts x=2, y=5, z=5 */
      x = y + y;
      
    } else {

      /* !Reaching Consts x=2, y=5, z=5 */
      y = 5;
      
    }

    /* !Reaching Consts x=BOTTOM, y=5, z=5 */
    z = y;
  }

   /* !Reaching Consts x=BOTTOM, y=5, z=5 */

  return 0;
}

