int show_error(FILE *log_file, int number) {
	switch(number) {
		case -1:
			printf("The client is not authenticated\n");
			fprintf(log_file, "The client is not authenticated\n");
			break;
		case -2:
			printf("Session already open\n");
			fprintf(log_file, "Session already open\n");
			break;
		case -3:
			printf("Wrong user/password\n");
			fprintf(log_file, "Wrong user/password\n");
			break;
		case -4:
			printf("File does not exist\n");
			fprintf(log_file, "File does not exist\n");
			break;
		case -5:
			printf("Forbidden download\n");
			fprintf(log_file, "Forbidden download\n");
			break;
		case -6:
			printf("The file is already shared\n");
			fprintf(log_file, "The file is already shared\n");
			break;
		case -7:
			printf("The file is already private\n");
			fprintf(log_file, "The file is already private\n");
			break;
		case -8:
			printf("Brute-force detected\n");
			fprintf(log_file, "Brute-force detected\n");
			break;
		case -9:
			printf("The file is already on server\n");
			fprintf(log_file, "The file is already on server\n");
			break;
		case -10:
			printf("File in transfer...\n");
			fprintf(log_file, "File in transfer...\n");
			break;
		case -11:
			printf("The user does not exist\n");
			fprintf(log_file, "The user does not exist\n");
			break;
	}
	return number;
}
