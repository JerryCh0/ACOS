#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h> //printf
#include <stdlib.h> //free
#include <string.h> //для вывода ошибок
#include <unistd.h> //fork()
#include <fcntl.h> //open(), close(), create(), dup2() (для Файловых Дескрипторов)
#include <sys/types.h> //pid_t
#include <sys/stat.h>  //для ФД
#include <sys/wait.h> //wait()

//функция парсит команду по словам и возвращает количество слов в строке команд	
int line_parcing(char* cmd_line, char* arr_cmd[]) //на вход: введенная строка и массив для команд
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
	return qty_cmd;
}

//функция находит, обрабатывает и извлекает из основной строки команд перенаправления потоков ввода-вывода и возвращает количество слов в уже измененной строке	
int find_streams(char* arr_cmd[], int qty_cmd, char buf0[], char buf1[])
{
	for (int i = 0; i < qty_cmd; ++i){
		char *arg = arr_cmd[i];
		if (arg[0] == '<')
			strcpy(buf0, arg + 1);
		else if (arg[0] == '>')
			strcpy(buf1, arg + 1);
		else
			continue; //начинается новая итерация цикла
		free(arr_cmd[i]); //извлекаем слово, содержащее символ перенаправления потоков
		for (int j = i; j < qty_cmd - 1; ++j) //сдвигаем все слова, идущие после текущего, на одно влево, чтобы выровнять после сдвига
			arr_cmd[j] = arr_cmd[j + 1];
		--qty_cmd; //изменяем параметры, так как был сдвиг
		--i; //изменяем параметры, так как был сдвиг
	}
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
		pid_t arr_pid[20]; //массив pid-ов запущенных процессов
		int pid_counter = 0; //счетчик запущенных процессов
		int next_arg_fd = -10; //переменная для хранения ФД, предназаначенного для input следующей команды
		int start_position = 0; //хранит первое слово, после последнего пайпа (или самое первое)
		int fd; //ФД

		if (input(cmd_line)) //считываем строку, выходим по команде quit
			break;
		qty_cmd = line_parcing(cmd_line, arr_cmd); //парсит команду (строку) по словам и считаем количество слов в команде
		qty_cmd = find_streams(arr_cmd, qty_cmd, buf0, buf1); //находит, обрабатывает и извлекает из основной строки команд перенаправления потоков ввода - вывода
		for (int i = 0; i <= qty_cmd; ++i)
			if (i == qty_cmd || strcmp(arr_cmd[i], "|") == 0){
				int pipefd[2] = { -1, -1 };
				if (i != qty_cmd){ //создаем pipe в каждом случае, кроме конца строки
					if (pipe(pipefd) == -1){
						perror("Can't pipe");
						exit(1);
					}
				}
				int temp = next_arg_fd; //свопаем next_arg_fd и pipefd[0], чтобы сохранить ФД из pipefd[0] для следующего процесса
				next_arg_fd = pipefd[0];
				pipefd[0] = temp;

				pid = fork();
				if (pid != 0) //кладем pid нового процесса в массив, чтобы потом их все закрыть
					arr_pid[pid_counter++] = pid;
				if (pid == -1){
					perror("Can't fork");
					exit(1);
				}
				if (pid == 0){ //если мы в процессе-потомке
					if (i != qty_cmd){ //если мы работаем с последним словом в строке, то переменная для хранения ФД, предназаначенного для input следующей команды, так как следующей команды ужене будет
						close(next_arg_fd); //закрываем за ненадобностью в дочернем потоке
					}

					//это input команды (откуда она считывает)
					fd = pipefd[0];
					if (start_position == 0 && buf0[0]){ //если мы в начале строки, и есть перенаправление ввода (если в буфере что-то написано)
						fd = open(buf0, O_RDONLY); //создаем ФД, указывающий на файл, имя которого уже считали, с параметром только на чтение
						if (fd == -1){
							perror("Can't open buf0");
							exit(1);
						}
					}
					if (start_position != 0 || buf0[0]){ //чтобы в первом пайпе на вход ничего не менять (только если при этом нет перенаправления ввода)
						dup2(fd, 0); //скопировали (перенаправили) ФД в стд поток ввода
						close(fd);
					}
					
					//это out команды (куда она выводит)
					fd = pipefd[1];
					if (i == qty_cmd && buf1[0]){ //если мы в конце строки, и есть перенаправление вsвода (если в буфере что-то написано)
						fd = creat(buf1, 00666); //создаем ФД, указывающий на файл, имя которого уже считали, (если он не существует - создадим новый) с параметром чтение-запись
						if (fd == -1){
							perror("Can't open buf1");
							exit(1);
						}
					}
					if (i != qty_cmd || buf1[0]){ //чтобы в последнем пайпе на выход ничего не менять (только если при этом нет перенаправления вывода)
						dup2(fd, 1); //скопировали (перенаправили) ФД в стд поток вывода
						close(fd);
					}
					
					arr_cmd[i] = 0; //занулили последнюю свободную ячейку для дальнейшего использования функции execvp
					if (execvp(arr_cmd[start_position], arr_cmd + start_position) == -1){ //запускаем команду, расположенную на start_position
						perror("Can't exec");
						exit(1);
					}
				}
				//если мы в процессе - родителе
				else{ //если были созданы какие-то ФД в массиве pipefd (отличны от -1) - закрываем их
					if (pipefd[0] != -1)
						close(pipefd[0]);
					if (pipefd[1] != -1)
						close(pipefd[1]);

					start_position = i + 1;
				}
			}
		for (int i = 0; i < qty_cmd; ++i) //в конце освождаем память
			free(arr_cmd[i]);
		for (int i = 0; i <= pid_counter; ++i) //делаем wait для всех дочерних процессов
			waitpid(arr_pid[i], 0, 0);
	}
	return 0;
}