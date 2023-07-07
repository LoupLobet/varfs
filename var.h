typedef struct entry {
	ulong id;
	struct entry *next;
	struct entry *prev;
	char *name;
	char *val;
	File *file;
} Entry;

typedef struct {
	Entry *head;
	Entry *tail;
	ulong size;
} Bucket;

typedef struct {
	Bucket *buckets;
	ulong nentry;
	ulong size;
} Map;

typedef Entry Var;

Map	*mapalloc(void);
void	 mapfree(Map *);
Var	*varadd(Map *, ulong, char *, char *, File *);
int	 vardel(Map *, ulong);
Var	*varbyid(Map *, ulong);
Var *varbyname(Map *, char *);
Var	*varset(Map *, ulong, char *);
