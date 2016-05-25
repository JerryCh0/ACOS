#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

int read_from_and_write_to(int fromfd, int tofd)
{
    int flags;
    //Получаем флаги фд, которому соответствует fromfd.
    //F_GETFL (void)
    //Получить права доступа к файлу и флаги состояния файла; arg игнорируется.
    if ((flags = fcntl(fromfd, F_GETFL, 0)) == -1)
        perror("get fcntl"), exit(1);
    //Ставим те же флаги и делаем его неблокирующим.
    //Если процесс пытается получить несовместимый доступ (например, read(2) и write(2)) к области файла, на которую установлена несовместимая обязательная блокировка, то результат зависит от состояния флага O_NONBLOCK в описании этого открытого файла. Если флаг O_NONBLOCK не установлен, то системный вызов блокируется до удаления блокировки или преобразуется в режим, который совместим с доступом. Если флаг O_NONBLOCK установлен, то системный вызов завершается с ошибкой EAGAIN.
    if (fcntl(fromfd, F_SETFL, flags | O_NONBLOCK) == -1)
        perror("set fcntl 1"), exit(1);
    
    int sumlen = 0;
    char buffer[4];
    int len;
    //Читаем из fromfd через buffer по 4 символа, записывам в tofd.
    while ((len = read(fromfd, buffer, sizeof(buffer))) > 0)
    {
        sumlen += len;
        int rc = write(tofd, buffer, len);
        if (rc == -1)
            perror("write"), exit(1);
    }
    //Если мы работали с файлом, не являющимся сокетом.
    //EAGAIN
    //Файловый дескриптор fd указывает на файл, не являющийся сокетом и который помечен как неблокирующий (O_NONBLOCK), а чтение вызвало бы блокировку.
    if (len == -1 && errno != EAGAIN)
        perror("read"), exit(1);
    //Снимает неблокирующий флаг.
    if (fcntl(fromfd, F_SETFL, flags) == -1)
        perror("set fcntl 2"), exit(1);
    //Возвращаем длину сообщения.
    return len;
}

int main(int argc, char *argv[])
{
    //Проверяем корректность запуска
    if (argc != 3)
    {
        printf("Usage: %s host port\n", argv[0]);
        exit(1);
    }
    
    //Кастуем host и port к нужным нам форматам.
    char *host = argv[1];
    int port = strtol(argv[2], NULL, 10);
    //ERANGE
    //The value to be returned is not representable.
    if (errno == ERANGE)
        perror("Invalid port"), exit(1);
    

    int sockfd;
    //socket() создаёт конечную точку соединения и возвращает её дескриптор.
    //AF_INET	Протоколы Интернет IPv4
    //SOCK_STREAM
    //Обеспечивает создание двусторонних, надёжных потоков байтов на основе установления соединения. Может также поддерживаться механизм внепоточных данных.
    //Сокеты типа SOCK_STREAM являются соединениями полнодуплексных байтовых потоков, похожими на каналы. Они не сохраняют границы записей. Потоковый сокет должен быть в состоянии соединения перед тем, как из него можно будет отсылать данные или принимать их. Соединение с другим сокетом создается с помощью системного вызова connect(2). После соединения данные можно передавать с помощью системных вызовов read(2) и write(2) или одного из вариантов системных вызовов send(2) и recv(2). Когда сеанс закончен, выполняется команда close(2). Внепоточные данные могут передаваться, как описано в send(2), и приниматься, как описано в recv(2).
    
    //Протоколы связи, которые реализуют SOCK_STREAM, следят, чтобы данные не были потеряны или дублированы. Если часть данных, для которых имеется место в буфере протокола, не может быть передана за определённое время, соединение считается разорванным. Когда в сокете включен флаг SO_KEEPALIVE, протокол каким-либо способом проверяет, не отключена ли ещё другая сторона. Если процесс посылает или принимает данные, пользуясь «разорванным» потоком, ему выдаётся сигнал SIGPIPE; это приводит к тому, что процессы, не обрабатывающие этот сигнал, завершаются. Сокеты SOCK_SEQPACKET используют те же самые системные вызовы, что и сокеты SOCK_STREAM. Единственное отличие в том, что вызовы read(2) возвращают только запрошенное количество данных, а остальные данные пришедшего пакета будут отброшены. Границы сообщений во входящих дейтаграммах сохраняются.
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror("socket"), exit(1);
    
    
    /*struct sockaddr_in{
     short sin_family;
     unsigned short sin_port;
     struct in_addr sin_addr;
     char sin_zero[8];
    };*/
    
    //sin_family
    //Тип адреса (должно быть AF_INET ).
    //sin_port
    //Порт IP-адресов.
    //sin_addr
    //IP-адрес.
    //sin_zero
    //Заполнение, чтобы сделать структуру одного размера c SOCKADDR.
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    //Функция htons() преобразует значение короткого беззнакового целого hostshort из узлового порядка расположения байтов в сетевой порядок расположения байтов.
    addr.sin_port = htons(port);
    
    //Структура hostent используется функциями, чтобы хранить информацию о хосте: его имя, тип, IP адрес, и т.д. Вы никогда не должны пытаться изменять эту структуру или освобождать любой из компонентов. Кроме того, только одна копия структуры hostent должна быть связана с потоком.
    
    /*struct hostent
    {
        char FAR * h_name;			// имя хоста
        char FAR * FAR * h_aliases;		// дополнительные названия
        short h_addrtype;			// тип адреса
        short h_length;			// длинна каждого адреса в байтах
        char FAR * FAR * h_addr_list;	// список адресов
    };*/

    struct hostent *h;
    //Функция gethostbyname() Получает информацию о хосте по его имени. Результат работы помещается в специальную структуру hostent.
    if ((h = gethostbyname(host)) == NULL)
        herror("gethostbyname"), exit(1);
    memcpy(&addr.sin_addr, h->h_addr, h->h_length);
    
    //int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    //Системный вызов connect() устанавливает соединение с сокетом, заданным файловый дескриптором sockfd, ссылающимся на адрес addr. Аргумент addrlen определяет размер addr.
    if (connect(sockfd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1)
        perror("connect"), exit(1);
    
    while (1)
    {
        fd_set readfds;
        //FD_ZERO() очищает набор;
        FD_ZERO(&readfds);
        //FD_SET() добавляет заданный файловый дескриптор к набору
        FD_SET(0, &readfds);
        FD_SET(sockfd, &readfds);
        //Вызовы select() и pselect() позволяют программам отслеживать изменения нескольких файловых дескрипторов ожидая, когда один или более файловых дескрипторов станут «готовы» для операции ввода-вывода определённого типа (например, ввода). Файловый дескриптор считается готовым, если к нему возможно применить соответствующую операцию ввода-вывода (например, read(2)) без блокировки или очень маленький write(2)).
        //int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
        //Отслеживаются 3 независимых набора файловых дескрипторов. В тех, что перечислены в readfds, будет отслеживаться появление символов, доступных для чтения (говоря более точно, проверяется доступность чтения без блокировки; в частности, файловый дескриптор готов для чтения, если он указывает на конец файла)
        //Значение nfds на единицу больше самого большого номера файлового дескриптора из всех трёх наборов.
        if (select(sockfd + 1, &readfds, NULL, NULL, NULL) == -1)
            perror("select"), exit(1);
        
        //FD_ISSET() проверяет, является ли файловый дескриптор частью набора
        if (FD_ISSET(sockfd, &readfds))
        {
            int len = read_from_and_write_to(sockfd, 0);
            if (len == 0)
                break;
        }
        if (FD_ISSET(0, &readfds))
            read_from_and_write_to(0, sockfd);
    }
    
    return 0;
}
