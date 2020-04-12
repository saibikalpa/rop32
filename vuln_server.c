#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define MSG_RCV "Message received. OK\n"
#define MSG_HNDL "Handling your request....\r\nPlease enter a message: "
#define MSG_CONN_OK "Welcome! You have been connected to the server\r\n"
#define MSG_NO_ACPT "Connection error! Connection couln't be accepted!!\r\n"
#define MSG_NO_BND "Address failed to bind with socket!!\r\n"
#define MSG_NO_SCKT "Socket couldn't be created!!\r\n"
#define MSG_NO_PRT "No port number provided!!\n"
#define MSG_BNNR "This is a vulnerable server! Never use it for production purpose..\n"
#define MSG_NO_LSTN "Listen error!!"

void exit_server(const char *msg){

	perror(msg);
	exit(errno);
}
void serve_client(int fd){

	char buf[1024];
	write(fd, MSG_HNDL, strlen(MSG_HNDL));
	read(fd, buf, 2048);
	write(fd, MSG_RCV, strlen(MSG_RCV) );
	return;
}
void sigchild(int x){

	while (waitpid(-1, NULL, WNOHANG) != -1);
}
int main(int argc, char** argv){

	int sock_fd = -1, conn_fd = -1;
	struct sockaddr_in addr;
	int one = 1;
	char buf[500];
	
	if(argc !=2 ){

		printf(MSG_NO_PRT);
		return -1;
	}
	printf(MSG_BNNR);

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){

		exit_server(MSG_NO_SCKT);
	}
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1]));
	addr.sin_addr.s_addr = INADDR_ANY;
	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	
	if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){

		exit_server(MSG_NO_BND);
	}
	if (listen(sock_fd, 10) < 0){

		exit_server(MSG_NO_LSTN);
		signal(SIGCHLD, sigchild);
	}
	while(1) {
		if ((conn_fd = accept(sock_fd, NULL, 0)) < 0 && errno != EINTR){

			exit_server(MSG_NO_ACPT);
		}
		if (conn_fd < 0){

			continue;
		}
		if (fork() == 0) {
			write(conn_fd, MSG_CONN_OK, strlen(MSG_CONN_OK));
			serve_client(conn_fd);
			exit(0);
		}
		close(conn_fd);
	}
	return 0;
}
