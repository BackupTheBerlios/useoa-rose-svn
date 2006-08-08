float foo(char op)
{
  float x=2.;
  float y=3.;
  float z;

  switch(op)
  {
    //int i;

    case '+':
    case '-':
      z=x-y;
      //break;

    case '*':
      z=x*y;
      //break;

    case '/':
      z=x/y;
      break;

    default:
      z=0;
      break;      

  }
   return z;
}

