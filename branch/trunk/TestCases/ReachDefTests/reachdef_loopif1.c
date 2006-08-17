
int main()
{
  double x, y, z;
  int i;
 
  i = 1; 
  while( i != 10 )
    {
      x = 2;
      if( x >= 0 )
	{
	  y = 5;
	  x = y + y;
	  y = x * 5;
	}
      else
        {
	  y = 3;
        }
      z = y;
      i++;
    }
  return 0;    
}

