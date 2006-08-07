/*
  What is the difference in the Sage AST between a sizeof call on
  a type versus an object.
*/

class blah {
    int x;
    char c;
    int *y;
};

int main()
{
    int x;
    char c;
    blah b;

    sizeof(int);
    sizeof x;
    sizeof c;
    sizeof b;

}
