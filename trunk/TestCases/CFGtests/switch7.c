float foo(char op)
{
  float x=2.;
  float y=3.;
  float z;

  switch(op)
  {
    //int i;
    case '+':
      break;

    default:
      z=0;
      break;      

    case '-':
      z=x-y;
      //break;

    case '*':
      z=x*y;
      //break;

    case '/':
      z=x/y;
      break;

  }
   return z;
}

