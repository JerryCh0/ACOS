LD_LIBRARY_PATH=/home/emil/Desktop #указывал текущий адрес в переменную, отвечающую за каталоги содержащие пользовательские динамические библиотеки
export LD_LIBRARY_PATH #подтверждение внесения изменений

gcc -c -fPIC sumAB.c #создали объектный файл для файла с кодом функции 
gcc -shared -o libsumAB.so sumAB.o #создали разделяемую, динамически подгружаемую библиотеку

gcc -c -fPIC printing.c #создали объектный файл для файла с кодом функции 
gcc -shared -o libprinting.so printing.o #создали разделяемую, динамически подгружаемую библиотеку

gcc plugIn.c -o plugIn -ldl #создали объектный файл для файла с кодом с dlopen и вызовом функции

./plugIn