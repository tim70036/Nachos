#include "syscall.h"

int main()
{
	int n=0,i,j;
  for(j=0; j < 100; j++)
  {
     n++; 
  }
  PrintInt(n);
	for(i=0 ; i < 10000 ; i++)
    {
        n += i;
    }
}
