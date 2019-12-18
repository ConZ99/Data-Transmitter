typedef struct {
	char data[MSGSIZE - 2*sizeof(int)];
	int index, size;
} package;