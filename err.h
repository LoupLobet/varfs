#define ERRSIZE 2048

typedef struct {
	char data[ERRSIZE];
	long ndata;
	char *cat;
	char full;
} Err;

Err	*err(char *);
Err *errcat(Err *, char *, char *);
void	 errfree(Err *);
