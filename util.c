#include <u.h>
#include <libc.h>

void *
emalloc(unsigned int n)
{
	void *p = NULL;

	p = malloc(n);
	if (p == NULL)
		sysfatal("malloc");
	memset(p, 0, n);
	return p;
}

void *
erealloc(void *q, unsigned int n)
{
	void *p = NULL;

	p = realloc(q, n);
	if (p == NULL)
		sysfatal("realloc");
	return p;
}

void *
estrdup(char *s)
{
	void *p = NULL;

	p = strdup(s);
	if (p == NULL)
		sysfatal("strdup");
	return p;
}
