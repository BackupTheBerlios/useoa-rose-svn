/*
 * from Reuse-Driven Interprocedural Slicing in the Presence of
 * Pointers and Recursion.
 * Donglin Liang and Mary Jean Harrold.
 * Proceedings of the International Conference on Software Maintanence'99.
 * Figure 5.
 */

int g1, g2, g, e;

void f(int a)
{
  if (a > 0) {
    e = e + 1;
    g1 = g2;
  } else {
    g2 = g1 + g;
    f(a++);
  }
}

int main()
{
  g1 = 0;
  g2 = 0;
  g = 0;
  e = 0;
  f(0);
  g1++;
  return 0;
}

