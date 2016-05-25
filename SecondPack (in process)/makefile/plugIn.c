#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int main()
{
    printf("Choose plugIn:\n1. Summing A,B\n2. Printing a message\n"); //предлагаем пользователю выбрать плагин
	int number;
	scanf("%d", &number);
	if (number == 1)
        sum();
	if (number == 2)
        print();

   return 0;
}
