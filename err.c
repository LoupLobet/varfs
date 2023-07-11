#include <u.h>
#include <libc.h>

#include "err.h"
#include "util.h"

Err *
err(char *s)
{
	Err *e;


	if (strlen(s) + 3 >= ERRMAX)
		return nil;
	e = emalloc(sizeof(Err));
	strcpy(e->data, s);
	e->cat = e->data + strlen(s);
	return e;
}

Err *
errcat(Err *e, char *s1, char *s2)
{
	if (!e->full && e->ndata + strlen(s1) + strlen(s2) + 3 >= ERRMAX) {
		strcpy(e->cat, "...");
		e->ndata += 3;
		e->cat += 3;
		e->full++;
	} else {
		strcpy(e->cat, s1);
		e->cat += strlen(s1);
		e->ndata += strlen(s1);
		strcpy(e->cat, s2);
		e->cat += strlen(s2);
		e->ndata += strlen(s2);
	}
	return e;
}

void
errfree(Err *e)
{
	free(e);
}

