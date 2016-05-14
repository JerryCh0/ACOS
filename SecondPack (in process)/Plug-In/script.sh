#LD_LIBRARY_PATH=/home/emil/Desktop #указывал текущий адрес в переменную, отвечающую за каталоги содержащие пользовательские динамические библиотеки
#LD_LIBRARY_PATH=&pwd
#export LD_LIBRARY_PATH #подтверждение внесения изменений
#echo $LD_LIBRARY_PATH

gcc -c -fPIC sumAB.c #создали объектный файл для файла с кодом функции 
gcc -shared -o libsumAB.so sumAB.o #создали разделяемую, динамически подгружаемую библиотеку

gcc -c -fPIC printing.c #создали объектный файл для файла с кодом функции 
gcc -shared -o libprinting.so printing.o #создали разделяемую, динамически подгружаемую библиотеку

gcc plugIn.c -o plugIn -ldl #создали объектный файл для файла с кодом с dlopen и вызовом функции

./plugIn
