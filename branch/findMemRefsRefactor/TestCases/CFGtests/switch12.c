float foo(char op)
{
  float x=2.;
  float y=3.;
  float z;

  switch(op)
  {
    //int i;
    case 'g':
      if (z>4) { z = x*y; }
      //break;

    default:
      z=0;

    case '-':
      z=x-y;

  }
   return z;
}

