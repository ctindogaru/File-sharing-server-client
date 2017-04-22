#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <dirent.h>
#include "errors.h"
#include "structures.h"

#define MAX_CLIENTS	5
#define BUFLEN 256

void error(char *msg)
{
	perror(msg);
	exit(1);
}

int search_file(char username[], char filename[], share sha[], int nr) {
	int i;
	int j;
	for (i = 0; i < nr; i++) {
		if (strcmp(sha[i].username, username) == 0)
			for (j = 0; j < sha[i].nr_files; j++)
				if (strcmp(sha[i].files[j], filename) == 0)
					return 1;
	}
	return 0;
}

void print(share sha[], int nr) {
	int i;
	int j;
	for (i = 0; i < nr; i++) {
		printf("%s ", sha[i].username);
		for (j = 0; j < sha[i].nr_files; j++)
			printf("%s ", sha[i].files[j]);
		printf("\n");
	}
}

char* get_password(char username[], account acc[], int nr) {
	int i;
	for (i = 0; i < nr; i++)
		if (strcmp(username, acc[i].username) == 0)
			return acc[i].password;
	return "null";
}

int main(int argc, char *argv[])
{
	int nr_attempts = 0;
	int sockfd, newsockfd, portno, clilen;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, j;
	fd_set read_fds;	
	fd_set tmp_fds;
	int fdmax;		


	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	portno = atoi(argv[1]);
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;	
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0)
		error("ERROR on binding");

	listen(sockfd, MAX_CLIENTS);

	
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;

	char *users_config = argv[2];
	char *shares_config = argv[3];

	FILE *users, *shares;
	int nr_acc;
	users = fopen(argv[2], "r");
	fscanf(users, "%d", &nr_acc);
	account acc[nr_acc];
	for (i = 0; i < nr_acc; i++)
		fscanf(users, "%s%s", acc[i].username, acc[i].password);
	fclose(users);

	int nr_sha;
	shares = fopen(argv[3], "r");
	fscanf(users, "%d", &nr_sha);
	share sha[nr_sha];
	int k = 0;
	for (i = 0; i < nr_sha; i++)
		sha[i].nr_files = 0;

	for (i = 0; i < nr_sha; i++) {
		char string[100];
		fscanf(shares, "%s", string);
		char *token;
		char *search = ":";
		token = strtok(string, search);
		int flag = 0;

		for (j = 0; j < k; j++)
			if (strcmp(sha[j].username, token) == 0) {
				token = strtok(NULL, search);
				strcpy(sha[j].files[sha[j].nr_files], token);
				sha[j].nr_files += 1;
				flag = 1;
			}
		if (flag == 0) {
			strcpy(sha[k].username, token);
			token = strtok(NULL, search);
			strcpy(sha[k].files[sha[k].nr_files], token);
			sha[k].nr_files += 1;
			k++;
		}

	}

	FD_SET(0, &read_fds);



	while (1) {
		tmp_fds = read_fds;
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1)
			error("ERROR in select");

		if (FD_ISSET(0, &tmp_fds)) {
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);
			if (strcmp(buffer, "quit\n") == 0)
				exit(0);

		}

		tmp_fds = read_fds;
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1)
			error("ERROR in select");

		for (i = 1; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {

				if (i == sockfd) {
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						error("ERROR in accept");
					}
					else {
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) {
							fdmax = newsockfd;
						}
					}
					printf("New connection from %s, port %d, socket_client %d\n ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
				}

				else {
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
						if (n == 0) {
							printf("selectserver: socket %d hung up\n", i);
						}
						else {
							error("ERROR in recv");
						}
						close(i);
						FD_CLR(i, &read_fds);
					}

					else { 

						const char s[] = " \n";
						char *token;
						char words[10][50];
						int words_len = 0;

						token = strtok(buffer, s);

						while (token != NULL) {
							strcpy(words[words_len], token);
							words_len++;
							token = strtok(NULL, s);
						}
						int j;

						if (strcmp(words[0], "login") == 0) {
							char password[100];
							strcpy(password, get_password(words[1], acc, nr_acc));

							if (strcmp(password, "null") == 0 || strcmp(words[2], password) != 0) {
								nr_attempts++;
								if (nr_attempts == 3) {
									send(i, "login-error-8", sizeof("login-error-8"), 0);
								}
								else send(i, "login-error-3", sizeof("login-error-3"), 0);

							}
							else {
								char aux[50];
								strcpy(aux, "conexiune ");
								strcat(aux, words[1]);
								send(i, aux, sizeof(aux), 0);
								nr_attempts = 0;
							}
						}
						else if (strcmp(words[0], "logout") == 0) {
							send(i, "logoutACK", sizeof("logoutACK"), 0);
						}
						else if (strcmp(words[0], "getuserlist") == 0) {
							char s[100000];
							strcpy(s, "getuserlistACK ");
							for (j = 0; j < nr_acc; j++) {
								strcat(s, acc[j].username);
								strcat(s, " ");
							}
							send(i, s, sizeof(s), 0);

						}

						else if (strcmp(words[0], "getfilelist") == 0) {
							if (strcmp(get_password(words[1], acc, nr_acc), "null") == 0)
								send(i, "getfilelist-error-11", sizeof("getfilelist-error-11"), 0);
							else {
								char s[100000];
								strcpy(s, "getfilelistACK ");

								DIR *dir;
								char all_files[50][50];
								int all_files_len = 0;

								struct dirent *ent;
								if ((dir = opendir(words[1])) != NULL) {
									while ((ent = readdir(dir)) != NULL) {
										if (strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0) {
											strcpy(all_files[all_files_len], ent->d_name);
											all_files_len++;
										}
									}
									closedir(dir);
								}
								else {
									perror("");
									return EXIT_FAILURE;
								}

								for (j = 0; j < all_files_len; j++) {
									struct stat st;
									char path[50];
									strcpy(path, words[1]);
									strcat(path, "//");
									strcat(path, all_files[j]);

									stat(path, &st);
									long int size = st.st_size;
									char str[10];
									sprintf(str, "%ld", size);
									strcat(s, all_files[j]);
									strcat(s, " ");
									strcat(s, str);
									strcat(s, " ");
									if (search_file(words[1], all_files[j], sha, nr_sha)) {
										strcat(s, "SHARED");
										strcat(s, " ");
									}
									else {
										strcat(s, "PRIVATE");
										strcat(s, " ");
									}
								}
								send(i, s, sizeof(s), 0);
							}

						}
						else if (strcmp(words[0], "share") == 0) {

							if (search_file(words[2], words[1], sha, nr_sha))
								send(i, "share-error-6", sizeof("share-error-6"), 0);
							else {
								DIR *dir;
								char all_files[50][50];
								int all_files_len = 0;

								struct dirent *ent;
								if ((dir = opendir(words[2])) != NULL) {
									while ((ent = readdir(dir)) != NULL) {
										if (strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0) {
											strcpy(all_files[all_files_len], ent->d_name);
											all_files_len++;
										}
									}
									closedir(dir);
								}
								else {
									perror("");
									return EXIT_FAILURE;
								}

								int flag = 0;
								for (j = 0; j < all_files_len; j++)
									if (strcmp(all_files[j], words[1]) == 0)
										flag = 1;
								if (flag == 0)
									send(i, "share-error-4", sizeof("share-error-4"), 0);
								else {
									char aux[100];
									strcpy(aux, "shareACK ");
									strcat(aux, words[1]);
									int k;
									for (k = 0; k < nr_sha; k++) {
										if (strcmp(sha[k].username, words[2]) == 0) {
											strcpy(sha[k].files[sha[k].nr_files], words[1]);
											sha[k].nr_files += 1;
										}

									}
									send(i, aux, sizeof(aux), 0);

								}
							}

						}

						else if (strcmp(words[0], "unshare") == 0) {

							DIR *dir;
							char all_files[50][50];
							int all_files_len = 0;

							struct dirent *ent;
							if ((dir = opendir(words[2])) != NULL) {
								while ((ent = readdir(dir)) != NULL) {
									if (strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0) {
										strcpy(all_files[all_files_len], ent->d_name);
										all_files_len++;
									}
								}
								closedir(dir);
							}
							else {
								perror("");
								return EXIT_FAILURE;
							}

							int flag = 0;
							for (j = 0; j < all_files_len; j++)
								if (strcmp(all_files[j], words[1]) == 0)
									flag = 1;
							if (flag == 0)
								send(i, "unshare-error-4", sizeof("unshare-error-4"), 0);
							else {

								if (search_file(words[2], words[1], sha, nr_sha) == 0)
									send(i, "unshare-error-7", sizeof("unshare-error-7"), 0);
								else {
									char aux[100];
									strcpy(aux, "unshareACK ");
									strcat(aux, words[1]);
									int k;
									for (k = 0; k < nr_sha; k++) {
										for (j = 0; j < sha[k].nr_files; j++)
											if (strcmp(sha[k].files[j], words[1]) == 0) {
												int x;
												for (x = j; x < sha[k].nr_files - 1; x++)
													strcpy(sha[k].files[x], sha[k].files[x + 1]);
												sha[k].nr_files -= 1;
												break;
											}

									}
									send(i, aux, sizeof(aux), 0);
								}
							}


						}
						else if (strcmp(words[0], "delete") == 0) {

							DIR *dir;
							char all_files[50][50];
							int all_files_len = 0;

							struct dirent *ent;
							if ((dir = opendir(words[2])) != NULL) {
								while ((ent = readdir(dir)) != NULL) {
									if (strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0) {
										strcpy(all_files[all_files_len], ent->d_name);
										all_files_len++;
									}
								}
								closedir(dir);
							}
							else {
								perror("");
								return EXIT_FAILURE;
							}

							int flag = 0;

							for (j = 0; j < all_files_len; j++)
								if (strcmp(all_files[j], words[1]) == 0) {
									flag = 1;
									char path[50];
									strcpy(path, words[2]);
									strcat(path, "//");
									strcat(path, words[1]);
									unlink(path);
									send(i, "deleteACK", sizeof("deleteACK"), 0);
									for (k = 0; k < nr_sha; k++) {
										for (j = 0; j < sha[k].nr_files; j++)
											if (strcmp(sha[k].files[j], words[1]) == 0) {
												int x;
												for (x = j; x < sha[k].nr_files - 1; x++)
													strcpy(sha[k].files[x], sha[k].files[x + 1]);
												sha[k].nr_files -= 1;
												break;
											}

									}
									break;
								}
							if (flag == 0)
								send(i, "delete-error-4", sizeof("delete-error-4"), 0);
						}
						else if (strcmp(words[0], "quit") == 0) {
							send(i, "quitACK", sizeof("quitACK"), 0);
						}
						else if (strcmp(words[0], "upload") == 0) {

							DIR *dir;
							char all_files[50][50];
							int all_files_len = 0;

							struct dirent *ent;
							if ((dir = opendir(words[2])) != NULL) {
								while ((ent = readdir(dir)) != NULL) {
									if (strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0) {
										strcpy(all_files[all_files_len], ent->d_name);
										all_files_len++;
									}
								}
								closedir(dir);
							}
							else {
								perror("");
								return EXIT_FAILURE;
							}

							int flag = 0;

							for (j = 0; j < all_files_len; j++)
								if (strcmp(words[1], all_files[j]) == 0) {
									send(i, "upload-error-9", sizeof("upload-error-9"), 0);
									flag = 1;
									break;
								}

							if (flag == 0) {
								char ch;
								FILE *source, *target;
								source = fopen(words[1], "r");
								char path[100];
								strcpy(path, words[2]);
								strcat(path, "//");
								strcat(path, words[1]);
								target = fopen(path, "w");

								while ((ch = fgetc(source)) != EOF)
									fputc(ch, target);

								fclose(source);
								fclose(target);
								char aux[100];
								strcpy(aux, "uploadACK ");
								strcat(aux, words[1]);
								send(i, aux, sizeof(aux), 0);

							}

						}
						else if (strcmp(words[0], "download") == 0) {

							DIR *dir;
							char all_files[50][50];
							int all_files_len = 0;

							struct dirent *ent;
							if ((dir = opendir(words[1])) != NULL) {
								while ((ent = readdir(dir)) != NULL) {
									if (strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0) {
										strcpy(all_files[all_files_len], ent->d_name);
										all_files_len++;
									}
								}
								closedir(dir);
							}
							else {
								perror("");
								return EXIT_FAILURE;
							}

							int flag = 0;
							for (j = 0; j < all_files_len; j++)
								if (strcmp(all_files[j], words[2]) == 0) {
									flag = 1;
									break;
								}

							if (flag == 0)
								send(i, "download-error-4", sizeof("download-error-4"), 0);
							else if (search_file(words[1], words[2], sha, nr_sha) == 0)
								send(i, "download-error-5", sizeof("download-error-5"), 0);
							else {
								int pid = getpid();
								char pid_str[10];
								sprintf(pid_str, "%d", pid);
								char file_name[100];
								strcpy(file_name, pid_str);
								strcat(file_name, words[2]);

								char ch;
								FILE *source, *target;
								char source_name[100];
								strcpy(source_name, words[1]);
								strcat(source_name, "//");
								strcat(source_name, words[2]);

								source = fopen(source_name, "r");
								char path[100];
								strcpy(path, words[3]);
								strcat(path, "//");
								strcat(path, file_name);
								target = fopen(path, "w");

								while ((ch = fgetc(source)) != EOF)
									fputc(ch, target);

								fclose(source);
								fclose(target);
								char aux[100];
								strcpy(aux, "downloadACK ");
								strcat(aux, words[2]);
								send(i, aux, sizeof(aux), 0);

							}

						}

					}
				}
			}
		}
	}


	close(sockfd);

	return 0;
}


