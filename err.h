#define ERRSIZE 2048

typedef struct {
	char data[ERRSIZE];
	long ndata;
	char *cat;
	char full;
} Err;

Err	*err(void);
Err	*errcat(Err *, char *, ...);
void	 errfree(Err *);
