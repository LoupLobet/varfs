#define MIN(A, B) (((A) > (B)) ? (B) : (A))
#define MAX(A, B) (((A) < (B)) ? (B) : (A))

void	*emalloc(unsigned int);
void	*erealloc(void *, unsigned int);
void	*estrdup(char *);
