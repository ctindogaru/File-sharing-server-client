typedef struct {
	char username[50];
	char password[50];
}account;

typedef struct {
	int nr_files;
	char username[50];
	char files[100][100];
}share;