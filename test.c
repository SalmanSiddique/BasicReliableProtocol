#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main ()
{
  /* initialize random seed: */
  srand ( time(NULL) );

  printf("random number %d\n",rand());
  printf("random number %d\n",rand());
  printf("random number %d\n",rand());
  printf("random number %d\n",rand());

  return 0;
}
