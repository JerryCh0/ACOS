#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int main()
{
    void *handle; //указатель на на начало динамической бибилиотеки
    void (*summing)(); //объявление указателя на функцию
    void (*printing)();
    char *error;

	printf("Choose plugIn:\n1. Summing A,B\n2. Printing a message\n"); //предлагаем пользователю выбрать плагин
	int number;
	char* cmd1;
	scanf("%d", &number);
	if (number == 1)
		cmd1 = "libsumAB.so";
	if (number == 2)
		cmd1 = "libprinting.so";

	//dlopen загружает динамическую библиотеку, имя которой указано в строке filename,
	//и возвращает прямой указатель на начало динамической библиотеки.
	handle = dlopen(cmd1, RTLD_LAZY);
	//Значение flag (второй аргумент) должно быть одним из двух:
	//RTLD_LAZY, подразумевающим разрешение неопределенных символов в виде кода, содержащегося в исполняемой динамической библиотеке;
	//или RTLD_NOW, требующим разрешения всех неопределенных символов перед возвратом их из dlopen и возвращающим ошибку,
	//если разрешение не может быть выполнено. 
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        return 1;
    }

	// dlerror возвращает NULL, если не возникло ошибок с момента инициализации или его последнего вызова.
	//Если вызывать dlerror() дважды, то во второй раз результат выполнения всегда будет равен NULL.
	dlerror(); //Очищаем буфер ошибок, если таковые успели уже возникнуть
	
	//dlsym использует указатель на динамическую библиотеку, возвращаемую dlopen, и оканчивающееся нулем символьное имя,
	//а затем возвращает адрес, указывающий, откуда загружается этот символ.
	//Если символ не найден, то возвращаемым значением dlsym является NULL.
	
	//According to the ISO C standard, casting between function
    //pointers and 'void *', as (sumAB = ( void (*)() ) dlsym(handle, "cos");) produces undefined results.
	if (number == 1)
		*(void **) (&summing) = dlsym(handle, "sum");
	if (number == 2)
		*(void **) (&printing) = dlsym(handle, "print");
		
   if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", error);
        return 1;
    }

	if (number == 1)
		(*summing)(); //вызываем нашу функцию
	if (number == 2)
		(*printing)();
		
   dlclose(handle);
   return 0;
}
