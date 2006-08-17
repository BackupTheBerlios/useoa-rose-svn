/* This test case models an issue I found in smg2000 that was causing
 * UseOA-ROSE to raise an assertion.
 * - Andy Stone
 */

struct A;

typedef struct A
{
    int field;
} B;

void foo(A *ptr)
{
    ptr->field;
}

