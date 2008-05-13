int main () {

  int count, n;
  char *to, *from;
  int a,b;
  to = &a;
  from = &b;

  count=7;
  switch (count % 8)
  {
   case 0:        do {  *to = *from++;
   case 7:              *to = *from++; printf("7"); 
   case 6:              *to = *from++;
   case 5:              *to = *from++;
   case 4:              *to = *from++;
   case 3:              *to = *from++;
   case 2:              *to = *from++;
   case 1:              *to = *from++;
                      } while (--n > 0);
  }

  return 0;
}
