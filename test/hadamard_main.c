#include <stdio.h>

extern int HadamardSAD8x8 (short* diff);

int main()
{
  short input[64] = {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8,
                     1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8,
                     1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8,
                     1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};
  int result;
  result = HadamardSAD8x8(input);
  printf("Result: %d\n", result);
  return 0;
}
