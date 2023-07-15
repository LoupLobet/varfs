#include <u.h>
#include <libc.h>
/* for emalloc9p(3) */
#include <fcall.h>
#include <thread.h>
#include <9p.h>

#include "err.h"

Err *
err(void)
{
	Err *e;

	e = emalloc9p(sizeof(Err));
	e->ndata = 0;
	e->full = 0;
	e->cat = e->data;
	return e;
}

Err *
errcat(Err *e, char *fmt, ...)
{
	va_list ap;
	int n;

	if (!e->full) {
		va_start(ap, fmt);
		n = vsnprint(e->cat, ERRSIZE - e->ndata - 3, fmt, ap);
		if (n >= ERRSIZE - e->ndata -3) {
			strcpy(e->cat, "...");
			e->ndata += 3;
			e->cat += 3;
			e->full++;
		} else {
			e->ndata += n;
			e->cat += n;
		}
	}
	return e;
}

void
errfree(Err *e)
{
	free(e);
}

