#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //fork()
#include <fcntl.h> //open(), close(), create(), dup2() (для Файловых Дескрипторов)
#include <sys/types.h> //pid_t
#include <sys/stat.h>  //для ФД
#include <sys/wait.h> //wait()

//функция парсит команду по словам и возвращает количество слов в команде	
int prepare(char* cmd_line, char* arr_cmd[]) //на вход: введенная строка и массив для команд
{
	int qty_cmd = 0; //количество команд (слов) в строке
	int start_pos = 0; //позиция начала каждого слова
	int line_length = strlen(cmd_line); //длина всей введенной строки
	int cmd_length; //длина одной команды (одного слова)
	for (int i = 0; i <= line_length; ++i){
		if (cmd_line[i] == ' ' || i == line_length){ //идем по буквам, пока не пробел или не последний символ
			cmd_length = i - start_pos;
			arr_cmd[qty_cmd] = malloc(cmd_length + 1);//в последнюю ячейку положим детерминирующий ноль
			arr_cmd[qty_cmd][cmd_length] = '\0';
			strncpy(arr_cmd[qty_cmd], cmd_line + start_pos, cmd_length);
			++qty_cmd;
			start_pos = i + 1;
		}
	}
	arr_cmd[qty_cmd] = NULL; //этого требует функция execvp, которуб мы будем использовать в дальнейшем

	return qty_cmd;
}

int input(char *cmd_line) //возвращает 1, если была введена команда quit, иначе 0
{
	printf("> ");
	gets(cmd_line);
	if (cmd_line[0] == 'q' && cmd_line[1] == 'u' && cmd_line[2] == 'i' && cmd_line[3] == 't' && !cmd_line[4])
		return 1; //выходим по команде quit
	else return 0;
}

int main()
{
	while (1){
		char cmd_line[100]; //строка, вводимая пользователем
		char *arr_cmd[20]; //массив си-строк, в котором будут лежать распарсенные слова
		int qty_cmd; //количество команд (слов) в строке
		char buf0[50] = ""; //буфер для хранения имени источника ввода
		char buf1[50] = ""; //буфер для хранения имени источника вывода
		pid_t pid; //PID процесса

		if (input(cmd_line)) //считываем строку, выходим по команде quit
			break;
		qty_cmd = prepare(cmd_line, arr_cmd); //парсит команду (строку) по словам и считаем количество слов в команде
		while (qty_cmd > 1)
		{
			char *cmd = arr_cmd[qty_cmd - 1];
			if (cmd[0] == '<')
				strcpy(buf0, cmd + 1);
			else if (cmd[0] == '>')
				strcpy(buf1, cmd + 1);
			else
				break;
			--qty_cmd;
			free(arr_cmd[qty_cmd]);
			arr_cmd[qty_cmd] = NULL; //занулили для дальнейшего использования функции execvp
		}

		pid = fork();

		if (pid == 0){ //если данный процесс является потомком
			if (buf0[0]){ //если в буфере что-то написано
				int fd = open(buf0, O_RDONLY); //создаем ФД, указывающий на файл, имя которого уже считали, с параметром только на чтение
				dup2(fd, 0); //скопировали (перенаправили) тот ФД из файла в стд поток ввода
				close(fd);
			}
			if (buf1[0]){ //если в буфере что-то написано
				int fd = creat(buf1, 00666); //создаем ФД, указывающий на файл, имя которого уже считали, (если он не существует - создадим новый) с параметром чтение-запись
				dup2(fd, 1); //скопировали (перенаправили) тот ФД из файла в стд поток вsвода
				close(fd);
			}
			execvp(arr_cmd[0], arr_cmd);
		}
		else{ //если потомок - родитель
			for (int i = 0; i < qty_cmd; ++i)
				free(arr_cmd[i]);
			wait(0);
		}
	}
	return 0;
}
