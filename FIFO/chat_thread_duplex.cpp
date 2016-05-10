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

//Constant names.
const char* client_read_name = "/clientcanread";
const char* server_read_name = "/servercanread";
const char* fifo_busy_name = "/chatfifobusy";
const char* fifo_name = "chatfifo";

//Creating semaphores.
sem_t *client_can_read, *server_can_read, *fifo_busy;

//pthread_create
//Первый параметр этой функции представляет собой указатель на переменную типа pthread_t, которая служит идентификатором создаваемого потока. Второй параметр, указатель на переменную типа pthread_attr_t, используется для передачи атрибутов потока. Третьим параметром функции pthread_create() должен быть адрес функции потока. Эта функция играет для потока ту же роль, что функция main() – для главной программы. Четвертый параметр функции pthread_create() имеет тип void *. Этот параметр может использоваться для передачи значения, возвращаемого функцией потока. Вскоре после вызова pthread_create() функция потока будет запущена на выполнение параллельно с другими потоками программы.

//Sending data.
void SendData(int fout, const string& str) {
    int len = str.length();
    write(fout, &len, sizeof(int));
    write(fout, str.c_str(), str.length() * sizeof(char));
}

//Receiving data.
void ReceiveData(int fin, string& str) {
    int len = 0;
    read(fin, &len, sizeof(int));
    char *buf = new char[len];
    read(fin, buf, len);
    str = string(buf);
    delete buf;
}

//Checking semaphore.
void check_sem_open(sem_t* semaphore) {
    if (semaphore == SEM_FAILED) {
        cout << strerror(errno) << endl;;
        exit(0);
    }
}

//Pending data.
void* PendingData(void* data) {
    int fin = open(fifo_name, O_RDWR);
    bool is_server = *((bool*)data);
    cout << "Ready :) Pending" << endl;
    string str;
    while (str != "Goodbye") {
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
        
        sem_wait(fifo_busy);

        ReceiveData(fin, str);
        cout << "He : " << str << endl;

        sem_post(fifo_busy);
    }

    /*if (is_server)
        sem_close(server_can_read);
    else
        sem_close(client_can_read);
    sem_close(fifo_busy);*/
    
    close(fin);
    return NULL;
}

//Input data.
void* UserInput(void* data) {
    int fout = open(fifo_name, O_RDWR);
    bool is_server = *((bool*)data);

    cout << "Ready :) UserInput" << endl;
    string str;
    while (str != "Goodbye") {
        getline(cin, str);
        
        sem_wait(fifo_busy);

        SendData(fout, str);

        if (is_server) {
            sem_post(client_can_read);
        }
        else {
            sem_post(server_can_read);
        }
        
        sem_post(fifo_busy);
    }
    
    /*if (is_server)
        sem_close(client_can_read);
    else
        sem_close(server_can_read);
    sem_close(fifo_busy);*/
    
    close(fout);
    return NULL;
}

void Server() {
    //Создаем FIFO с максимальными правами.
    mkfifo(fifo_name, 0777); //as usual file
    //Создаем дескриптор потока.
    pthread_t temp;
    //Инициализируем указатель на bool - тип текущего запуска сервером является сервером.
    bool* is_server = new bool;
    *is_server = true;
    //
    pthread_create(&temp, NULL, PendingData, is_server);
    UserInput(is_server);
    pthread_cancel(temp);
}

void Client() {
    mkfifo(fifo_name, 0777); //as usual file
    pthread_t temp;
    bool* is_server = new bool;
    *is_server = false;
    pthread_create(&temp, NULL, PendingData, is_server);
    UserInput(is_server);
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
        puts("Unknow argument\n");
        exit(-1);
    }
    
    return 0;
}