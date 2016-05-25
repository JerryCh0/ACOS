#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>

#include <sys/socket.h> //socket
#include <sys/select.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

void in(const char *s) //какое назначение у этой функции?
{
	return;
	printf("%s...", s);
	scanf("%*c");
}

int read_from_and_write_to(int fromfd, int tofd)
{
	//printf("from %d to %d\n", fromfd, tofd);

	int flags;
	if ((flags = fcntl(fromfd, F_GETFL, 0)) == -1)
		perror("get fcntl"), exit(1);
	if (fcntl(fromfd, F_SETFL, flags | O_NONBLOCK) == -1)
		perror("set fcntl 1"), exit(1);

	int sumlen = 0;
	char buffer[4];
	int len;
	while ((len = read(fromfd, buffer, sizeof(buffer))) > 0)
	{
		sumlen += len;
		int rc = write(tofd, buffer, len);
		if (rc == -1)
			perror("write"), exit(1);
	}
	if (len == -1 && errno != EAGAIN)
		perror("read"), exit(1);

	if (fcntl(fromfd, F_SETFL, flags) == -1)
		perror("set fcntl 2"), exit(1);
	return len;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Usage: %s host port\n", argv[0]);
		exit(1);
	}

	char *host = argv[1];
	//почему мы просто не считываем готовое чилсо?
	int port = strtol(argv[2], NULL, 10); //возвращает число из строки (1 арг) в десятичной системе (10 - третий аргумент), где null - там обычно end (указатель на символ, стоящий после числа)
	if (errno == ERANGE)//это код ошибки выдаваемой, Если результат не может быть представлен как значение типа long int
		perror("Invalid port"), exit(1);

	in("socket"); //что происходит?
	int sockfd;
	//socket() creates an endpoint for communication and returns a file
	//	descriptor that refers to that endpoint.

	/*The domain argument specifies a communication domain; this selects
		the protocol family which will be used for communication.These
		families are defined in <sys / socket.h>.The currently understood
		formats include :
		AF_UNIX, AF_LOCAL   Local communication              unix(7)
	    AF_INET             IPv4 Internet protocols(4 байта) ip(7) -------------- мы его используем
        AF_INET6            IPv6 Internet protocols          ipv6(7)*/


	/*     The socket has the indicated type, which specifies the communication
       semantics.  Currently defined types are:

	   SOCK_STREAM     Provides sequenced, reliable, two-way, connection- ------------мы его используем
	   based byte streams.  An out-of-band data transmission
	   mechanism may be supported.

	   SOCK_DGRAM      Supports datagrams (connectionless, unreliable ---- это для дата-грам (UDP)
	   messages of a fixed maximum length).

	   SOCK_SEQPACKET  Provides a sequenced, reliable, two-way connection-
	   based data transmission path for datagrams of fixed
	   maximum length; a consumer is required to read an
	   entire packet with each input system call.
	*/
	// int socket(int domain, int type, int protocol);
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		//tcp_socket = socket(AF_INET, SOCK_STREAM, 0);  
		//udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
		//raw_socket = socket(AF_INET, SOCK_RAW, protocol);
		perror("socket"), exit(1);

	/*
	// don't leave the socket in a TIME_WAIT state if we close the connection
	struct linger fix_ling;
	fix_ling.l_onoff = 1;
	fix_ling.l_linger = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &fix_ling, sizeof(fix_ling)) == -1)
	perror("setsockopt"), exit(1);
	//*/


	//В семействе адреса в интернете, структура SOCKADDR_IN используется Windows Sockets,
	//чтобы указать локальный или удаленный адрес конечной точки, к которому подключение сокет.
	
	//struct sockaddr_in {
	//	short            sin_family;   // e.g. AF_INET					//Тип адреса (должно быть AF_INET ).
	//	unsigned short   sin_port;     // e.g. htons(3490)				//Порт IP-адресов.
	//	struct in_addr   sin_addr;										//IP-адрес.
	//	char             sin_zero[8];  // zero this if you want to		//Заполнение, чтобы сделать структуру одного размера c SOCKADDR.
	//};

	//Структура sockaddr_in описывает сокет для работы с протоколами IP.
	//Значение поля sin_family всегда равно AF_INET.
	//Поле sin_port содержит номер порта который намерен занять процесс.Если значение этого поля равно нулю, то операционная система сама выделит свободный номер порта для сокета.
	//Поле sin_addr содержит IP адрес к которому будет привязан сокет.
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port); //htons - преобразует узловой порядок (от хоста) расположения байтов положительного короткого целого hostshort в сетевой порядок (TCP/IP) расположения байтов.

	struct hostent *h;
	//struct hostent
	//{
	//	char FAR * h_name;			// имя хоста
	//	char FAR * FAR * h_aliases;		// дополнительные названия
	//	short h_addrtype;			// тип адреса
	//	short h_length;			// длинна каждого адреса в байтах
	//	char FAR * FAR * h_addr_list;	// список адресов
	//};
	if ((h = gethostbyname(host)) == NULL) //gethostbyname - Получает информацию о хосте по его имени. Результат работы помещается в специальную структуру hostent
		//В эту функцию надо передать имя хоста.Если функция выполнится неудачно или с ошибкой, то вернется NULL.
		//Иначе указатель на структуру.Вы не должные изменять эту структуру.
		//Структура hostent используется функциями, чтобы хранить информацию о хосте : его имя, тип, IP адрес, и т.д.
		//Вы никогда не должны пытаться изменять эту структуру или освобождать любой из компонентов.
		//Кроме того, только одна копия структуры hostent должна быть связана с потоком.


		herror("gethostbyname"), exit(1);
	memcpy(&addr.sin_addr, h->h_addr, h->h_length);

	in("connect");
	if (connect(sockfd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1)
		perror("connect"), exit(1);

	while (1)
	{
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);
		FD_SET(sockfd, &readfds);
		//printf("select...\n");
		if (select(sockfd + 1, &readfds, NULL, NULL, NULL) == -1)
			perror("select"), exit(1);

		if (FD_ISSET(sockfd, &readfds))
		{
			int len = read_from_and_write_to(sockfd, 0);
			if (len == 0)
				break;
		}
		if (FD_ISSET(0, &readfds))
			read_from_and_write_to(0, sockfd);

		/*
		int error = 0;
		socklen_t len = sizeof(error);
		if (getsockopt(sockfd, SOL_SOCKET, , &error, &len) == -1)
		perror("getsockopt"), exit(1);
		if (error == -1)
		perror("SO_ERROR"), exit(1);//*/
	}

	return 0;
}
