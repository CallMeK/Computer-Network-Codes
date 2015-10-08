#include <string.h>
#include <stdio.h>

int main()
{
  char a[11];
  char b[5];
  memset(a,0,11);
  memset(b,0,5);
  b[0]='1';
  b[1]='2';
  b[2]='3';
  b[3]='4';
  b[4]='5';
  strcat(a,b);
  printf("%s\n",a);
  strcat(a,b);
  printf("%s\n",a);
  return 0;
}
