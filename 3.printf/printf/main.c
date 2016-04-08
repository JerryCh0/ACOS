#include<stdarg.h>
#include<string.h>
#include<stdio.h>

void my_printf(char* s, ...)
{
	va_list L;
	va_start(L, s);
	while (*s != '\0')
	{
		if (*s != '%') printf("%c", *s);
		if (*s == '%') {
			s++;
			switch (*s) {
			case 's':
				printf("%s", va_arg(L, char *));
				break;
			case 'd':
				printf("%d", atoi(va_arg(L, char *)));
				break;
			}
			s++;
		}
	}
	va_end(L);
}



void main()
{
	char a[250];
	int len = 0;
	gets(a);
	my_printf("%d", a);
}
