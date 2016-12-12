#include "syscall.h"

int main()
{
	int n=0,i;
	for(i=0 ; i < 1000000 ; i++)
    {
        n += i;
        int a=1;
        int b = 2;
        a = a + b;
    }
}
