//Usual libraries.
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <cstdio>
#include <string.h>
//Sys libraries.
#include <unistd.h> //Including POSIX API
#include <fcntl.h> // open(),fcntl()
#include <sys/stat.h> //mkdir, mkfifo and etc...
#include <semaphore.h> //Including semaphores
#include <errno.h> //Error numbers
#include <sys/types.h> //pthread and etc...

using namespace std;

//Имена семафоров.
const char* client_read_name = "/clientcanread";
const char* server_read_name = "/servercanread";
const char* fifo_busy_name = "/chatfifobusy";
//Имя FIFO.
const char* fifo_name = "chatfifo";

//Создаем семафоры для клиента, сервера, FIFO.
sem_t *client_can_read, *server_can_read, *fifo_busy;

//pthread_create
//Первый параметр этой функции представляет собой указатель на переменную типа pthread_t, которая служит идентификатором создаваемого потока. Второй параметр, указатель на переменную типа pthread_attr_t, используется для передачи атрибутов потока. Третьим параметром функции pthread_create() должен быть адрес функции потока. Эта функция играет для потока ту же роль, что функция main() – для главной программы. Четвертый параметр функции pthread_create() имеет тип void *. Этот параметр может использоваться для передачи значения, возвращаемого функцией потока. Вскоре после вызова pthread_create() функция потока будет запущена на выполнение параллельно с другими потоками программы.

//Запись сообщения в FIFO.
//Функция write(handle, buf, count) переписывает count байт из буфера, на который указывает buf в файл, соот­ветствующий дескриптору файла handle.
void SendData(int fout, const string& str) {
    int len = str.length();
    //Записываем так же длину сообщения, чтобы его можно было корректно считать из FIFO.
    write(fout, &len, sizeof(int));
    write(fout, str.c_str(), str.length() * sizeof(char));
}

//Считывание сообщения из FIFO.
//Функция read(fd, buf, count) считывает count байт из файла, описываемого аргументом fd, в буфер, на который указывает аргумент buf.
void ReceiveData(int fin, string& str) {
    int len = 0;
    //Cчитываем размер сообщения.
    read(fin, &len, sizeof(int));
    //Создаем буфер для самого сообщения.
    char *buf = new char[len];
    //Считываем сообщение в буфер.
    read(fin, buf, len);
    str = string(buf);
    //Удаляем буфер.
    delete buf;
}

//Проверяем, корректно ли открыт семафор.
void check_sem_open(sem_t* semaphore) {
    if (semaphore == SEM_FAILED) {
        cout << strerror(errno) << endl;;
        exit(0);
    }
}

//Pending data.
void* PendingData(void* data) {
    //Открываем FIFO для чтения и записи.
    int fin = open(fifo_name, O_RDWR);
    //Создаем флаг типа запуска.
    bool is_server = *((bool*)data);
    //Уведомление об ожидании ввода.
    sleep(1);
    cout << "Ready, pending." << endl;
    //Инициализируем строку, в которую будем получать информацию.
    string str;
    //Пока строка не совпадает с командой закрытия "exit", захватываем нужный семафор (в зависимости от типа).
    while (str != "exit") {
        if (is_server) {
            if (sem_wait(server_can_read) < 0) {
                perror("sem_wait");
            }
        }
        else {
            if (sem_wait(client_can_read) < 0){
                perror("sem_wait");
            }
        }
        //Захватываем семафор FIFO.
        sem_wait(fifo_busy);
        //Получаем информацию.
        ReceiveData(fin, str);
        //Выводим сообщение.
        cout << "The other chatmate: " << str << endl;
        //Освобождаем семафор FIFO.
        sem_post(fifo_busy);
    }
    
    //Закрываем FIFO, возвращем пустое значение.
    close(fin);
    return NULL;
}

//Input data.
void* UserInput(void* data) {
    //Открываем FIFO для чтения и записи.
    int fout = open(fifo_name, O_RDWR);
    //Создаем флаг типа запуска.
    bool is_server = *((bool*)data);
    //Уведомление об ожидании ввода.
    cout << "Ready, Input." << endl;
    //Инициализируем строку, в которую будем получать информацию.
    string str;
    while (str != "exit") {
        //Считываем из стандартного потока сообщение.
        getline(cin, str);
        //Захватываем семафор FIFO.
        sem_wait(fifo_busy);
        //Отправляем в FIFO сообщение.
        SendData(fout, str);
        //Освобождаем семафор в зависимости от типа запуска.
        if (is_server) {
            sem_post(client_can_read);
        }
        else {
            sem_post(server_can_read);
        }
        //Освобождаем семафор FIFO.
        sem_post(fifo_busy);
    }
    
    //Закрываем FIFO, возвращаем пустое значение.
    close(fout);
    return NULL;
}

void Server() {
    //Создаем FIFO с максимальными правами.
    mkfifo(fifo_name, 0777); //as usual file
    //Создаем дескриптор потока.
    pthread_t temp;
    //Инициализируем указатель на bool - тип текущего запуска является сервером.
    bool* is_server = new bool;
    *is_server = true;
    //Создаем поток с атрибутами по умолчанию, выполняющий функцию PendingData, принимающую аргумент is_server.
    pthread_create(&temp, NULL, PendingData, is_server);
    //Ввод информации пользователем.
    UserInput(is_server);
    //Удаление потока управления.
    pthread_cancel(temp);
}

void Client() {
    //Создаем FIFO с максимальными правами.
    mkfifo(fifo_name, 0777); //as usual file
    //Создаем дескриптор потока.
    pthread_t temp;
    //Инициализируем указатель на bool - тип текущего запуска не является сервером (является клиентом).
    bool* is_server = new bool;
    *is_server = false;
    //Создаем поток с атрибутами по умолчанию, выполняющий функцию PendingData, принимающую аргумент is_server.
    pthread_create(&temp, NULL, PendingData, is_server);
    //Ввод информации пользователем.
    UserInput(is_server);
    //Удаление потока управления.
    pthread_cancel(temp);
}

int main(int argc, char **argv) {

    //Создаем семафор для клиента (или открываем, если он уже создан). Проверяем корректное открытие.
    client_can_read = sem_open(client_read_name, O_CREAT | O_EXCL, 0777, 0);
    if (client_can_read == SEM_FAILED) {
        client_can_read = sem_open(client_read_name, 0);
    }
    check_sem_open(client_can_read);

    //Создаем семафор для сервера (или открываем, если он уже создан). Проверяем корректное открытие.
    server_can_read = sem_open(server_read_name, O_CREAT | O_EXCL, 0777, 0);
    if (server_can_read == SEM_FAILED) {
        server_can_read = sem_open(server_read_name, 0);
    }
    check_sem_open(server_can_read);

    //Создаем семафор, определяющий занятость FIFO (или открываем, если он уже создан). Проверяем корректное открытие.
    fifo_busy = sem_open(fifo_busy_name, O_CREAT, 0777, 1);
    if (fifo_busy == SEM_FAILED) {
        fifo_busy = sem_open(fifo_busy_name, 0);
    }
    check_sem_open(fifo_busy);
    
    //Проверяем, в каком режиме запущен чат. Если аргумент отсутствует или введен несуществующий режим - выводим сообщение об ошибке.
    //Иначе - запускаем нужный режим.
    if (argc != 2) {
        puts("Error! Arguments wasn't found\n");
        exit(-1);
    }
    
    string name_pr(argv[1]);

    if (name_pr == "server") {
        Server();
    }
    else if (name_pr == "client") {
        Client();
    }
    else {
        puts("Unknown argument\n");
        exit(-1);
    }
    
    return 0;
}