#include<stdarg.h>
#include<string.h>
#include<stdio.h>

void my_printf(char* s, ...)
{
	va_list L;
	va_start(L, s);
	while (*s != '\0')
	{
        if (*s != '%') {
            printf("%c", *s);
            s++;
        }
		if (*s == '%') {
			s++;
			switch (*s) {
			case 's':
				printf("%s", va_arg(L, char *));
				break;
			case 'd':
				printf("%d", va_arg(L, int));
				break;
            case 'q':
                    printf(" ");
                    char t = va_arg(L, char);
                    break;
            }
			s++;
		}
	}
	va_end(L);
}



int main()
{
    char *a;
    int y;
    double x;
	gets(a);
    x = 5.0;
    y = 10;
	my_printf("%s %q %d", a, x, y);
    return 0;
}
