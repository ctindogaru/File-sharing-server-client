#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <dirent.h>
#include "errors.h"


#define BUFLEN 256

void error(char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[BUFLEN];
	if (argc < 3) {
		fprintf(stderr, "Usage %s server_address server_port\n", argv[0]);
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1], &serv_addr.sin_addr);


	if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");

	fd_set read_fds;	
	fd_set tmp_fds;	

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	FD_SET(sockfd, &read_fds);
	FD_SET(0, &read_fds);

	int login = 0;
	char currentUser[50] = "";
	FILE *log_file = fopen("log.txt", "w");

	while (1) {
		tmp_fds = read_fds;
		memset(buffer, 0, sizeof(buffer));

		if (select(sockfd + 1, &tmp_fds, NULL, NULL, NULL) == -1)
			perror("Error in select");

		if (FD_ISSET(0, &tmp_fds)) {
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);

			const char s[] = " \n";
			char *token;
			char words[5][50];
			int words_len = 0;
			char original_buffer[BUFLEN];
			strcpy(original_buffer, buffer);

			token = strtok(original_buffer, s);

			while (token != NULL) {
				strcpy(words[words_len], token);
				words_len++;
				token = strtok(NULL, s);
			}

			fprintf(log_file, "%s> %s", currentUser, buffer);

			if (strcmp(words[0], "login") == 0) {
				if (login == 0)
					send(sockfd, buffer, strlen(buffer), 0);
				else show_error(log_file, -2);
			}
			else if (strcmp(words[0], "logout") == 0) {
				if (login == 0)
					show_error(log_file, -1);
				else send(sockfd, buffer, strlen(buffer), 0);
			}
			else if (strcmp(words[0], "getuserlist") == 0) {
				if (login == 1)
					send(sockfd, buffer, strlen(buffer), 0);
			}
			else if (strcmp(words[0], "getfilelist") == 0) {
				if (login == 1)
					send(sockfd, buffer, strlen(buffer), 0);
			}
			else if (strcmp(words[0], "share") == 0) {
				if (login == 1) {
					strcat(buffer, " ");
					strcat(buffer, currentUser);
					send(sockfd, buffer, strlen(buffer), 0);
				}
			}
			else if (strcmp(words[0], "unshare") == 0) {
				if (login == 1) {
					strcat(buffer, " ");
					strcat(buffer, currentUser);
					send(sockfd, buffer, strlen(buffer), 0);
				}
			}
			else if (strcmp(words[0], "unshare") == 0) {
				if (login == 1) {
					strcat(buffer, " ");
					strcat(buffer, currentUser);
					send(sockfd, buffer, strlen(buffer), 0);
				}
			}
			else if (strcmp(words[0], "delete") == 0) {
				if (login == 1) {
					strcat(buffer, " ");
					strcat(buffer, currentUser);
					send(sockfd, buffer, strlen(buffer), 0);
				}
			}
			else if (strcmp(words[0], "quit") == 0) {
				send(sockfd, buffer, strlen(buffer), 0);

			}
			else if (strcmp(words[0], "upload") == 0) {
				FILE *file = fopen(words[1], "r");
				if (file == 0)
					show_error(log_file, -4);
				else if (login == 1) {
					strcat(buffer, " ");
					strcat(buffer, currentUser);
					send(sockfd, buffer, strlen(buffer), 0);
				}
			}
			else if (strcmp(words[0], "download") == 0) {
				if (login == 1) {
					strcat(buffer, " ");
					strcat(buffer, currentUser);
					send(sockfd, buffer, strlen(buffer), 0);
				}
			}

		}

		tmp_fds = read_fds;
		memset(buffer, 0, sizeof(buffer));

		if (select(sockfd + 1, &tmp_fds, NULL, NULL, NULL) == -1)
			perror("Error in select");


		if (FD_ISSET(sockfd, &tmp_fds)) {
			int n = recv(sockfd, buffer, BUFLEN, 0);
			if (n <= 0) {
				if (n == 0) {
					printf("Server shutdown!\n");
					fprintf(log_file, "Server shutdown!\n");
				}
				else {
					error("ERROR in recv");
				}
				close(sockfd);
				FD_CLR(sockfd, &read_fds); 
			}
			else {
				const char s[] = " \n";
				char *token;
				char words[100000][50];
				int words_len = 0;
				char original_buffer[BUFLEN];
				strcpy(original_buffer, buffer);

				token = strtok(original_buffer, s);

				while (token != NULL) {
					strcpy(words[words_len], token);
					words_len++;
					token = strtok(NULL, s);
				}


				if (strcmp(words[0], "login-error-8") == 0) {
					show_error(log_file, -8);
					exit(0);
				}
				else if (strcmp(words[0], "login-error-3") == 0) {
					show_error(log_file, -3);
				}
				else if (strcmp(words[0], "getfilelist-error-11") == 0) {
					show_error(log_file, -11);
				}
				else if (strcmp(words[0], "share-error-4") == 0) {
					show_error(log_file, -4);
				}
				else if (strcmp(words[0], "share-error-6") == 0) {
					show_error(log_file, -6);
				}
				else if (strcmp(words[0], "unshare-error-4") == 0) {
					show_error(log_file, -4);
				}
				else if (strcmp(words[0], "unshare-error-7") == 0) {
					show_error(log_file, -7);
				}
				else if (strcmp(words[0], "delete-error-4") == 0) {
					show_error(log_file, -4);
				}
				else if (strcmp(words[0], "delete-error-10") == 0) {
					show_error(log_file, -10);
				}
				else if (strcmp(words[0], "upload-error-9") == 0) {
					show_error(log_file, -9);
				}
				else if (strcmp(words[0], "download-error-4") == 0) {
					show_error(log_file, -4);
				}
				else if (strcmp(words[0], "download-error-5") == 0) {
					show_error(log_file, -5);
				}
				else if (strcmp(words[0], "conexiune") == 0) {
					printf("Successful connection\n");
					fprintf(log_file, "Successful connection\n");
					login = 1;
					strcpy(currentUser, words[1]);
				}
				else if (strcmp(words[0], "logoutACK") == 0) {
					printf("You've been logged out from the system\n");
					fprintf(log_file, "You've been logged out from the system\n");
					login = 0;
					strcpy(currentUser, "");
				}
				else if (strcmp(words[0], "getuserlistACK") == 0) {
					int j;
					for (j = 1; j < words_len; j++) {
						printf("%s\n", words[j]);
						fprintf(log_file, "%s\n", words[j]);
					}
				}
				else if (strcmp(words[0], "getfilelistACK") == 0) {
					int j;
					for (j = 1; j < words_len; j += 3) {
						printf("%s %s %s\n", words[j], words[j + 1], words[j + 2]);
						fprintf(log_file, "%s %s %s\n", words[j], words[j + 1], words[j + 2]);
					}
				}
				else if (strcmp(words[0], "shareACK") == 0) {
					printf("The file %s is now shared\n", words[1]);
					fprintf(log_file, "The file %s is now shared\n", words[1]);
				}
				else if (strcmp(words[0], "unshareACK") == 0) {
					printf("The file %s is now private\n", words[1]);
					fprintf(log_file, "The file %s is now private\n", words[1]);
				}
				else if (strcmp(words[0], "deleteACK") == 0) {
					printf("File deleted\n");
					fprintf(log_file, "File deleted\n");
				}
				else if (strcmp(words[0], "quitACK") == 0) {
					exit(0);
				}
				else if (strcmp(words[0], "uploadACK") == 0) {
					printf("%s: Successful upload\n", words[1]);
					fprintf(log_file, "%s: Successful upload\n", words[1]);
				}
				else if (strcmp(words[0], "downloadACK") == 0) {
					printf("%s: Successful download\n", words[1]);
					fprintf(log_file, "%s: Successful download\n", words[1]);
				}
			}
		}
	}
	fclose(log_file);

	return 0;
}


