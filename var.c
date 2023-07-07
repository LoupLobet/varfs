#include <u.h>
#include <libc.h>
#include <fcall.h>
#include <thread.h>
#include <9p.h>

#include "var.h"
#include "util.h"

static Entry 	*mapallocentry(Bucket *, ulong);
static void	 maprealloc(Map *, ulong);

static const ulong defmapsize = 8;
static const float loadf = 1;
static const float growthf = 1.5;

Map *
mapalloc(void)
{
	Map *m;

	m = emalloc9p(sizeof(Map));
	m->size = defmapsize;
	m->buckets = emalloc9p(m->size * sizeof(Bucket));
	return m;
}

Entry *
mapallocentry(Bucket *b, ulong n)
{
	Entry *e;

	e = emalloc9p(sizeof(Entry));
	e->next = nil;
	if (b[n].head == nil) {
		b[n].head = e;
		b[n].tail = e;
		e->prev = nil;
	} else {
		b[n].tail->next = e;
		e->prev = b[n].tail;
		b[n].tail = e;
	}
	b[n].size++;
	return e;
}

void
mapfree(Map *m)
{
	Entry *e;
	int i;

	for (i = 0; i < m->size; i++) {
		e = m->buckets[i].head;
		if (e == nil)
			continue;
		while(e->next != nil) {
			if (e->val != nil)
				free(e->val);
			removefile(e->file);
			free(e->name);
			e = e->next;
			free(e->prev);
		}
		if (e->val != nil)
			free(e->val);
		removefile(e->file);
		free(e->name);
		free(e);
	}
	free(m->buckets);
	free(m);
}

void
maprealloc(Map *m, ulong size)
{
	Bucket *b;
	Entry *e;
	int i, n;

	/*
	 * We only realloc buckets, and just rearange already
	 * allocated entries in the new buckets.
	 */
	b = emalloc9p(size * sizeof(Bucket));
	for (i = 0; i < m->size; i++) {
		e = m->buckets[i].head;
		while (e != nil) {
			n = e->id % size;
			if (b[n].head == nil) {
				b[n].head = e;
				b[n].tail = e;
				b[n].head->prev = nil;
			} else {
				b[n].tail->next = e;
				e->prev = b[n].tail;
				b[n].tail = b[n].tail->next;
			}
			e = e->next;
			b[n].tail->next = nil;
			b[n].size++;
		}
	}
	m->size = size;
	free(m->buckets);
	m->buckets = b;
}

Var *
varadd(Map *m, ulong id, char *name, char *val, File *file)
{
	ulong n, newsize;
	Entry *e;

	if ((float)m->nentry / (float)m->size >= loadf) {
		newsize = (ulong)(growthf * m->size);
		/*
		 * Breaks even numbers stacking on resizing, by
		 * remaping to a odd size map
		 */
		if (!(newsize % 2))
			newsize++;
		maprealloc(m, newsize);
	}
	n = id % m->size;
	e = mapallocentry(m->buckets, n);
	e->id = id;
	if (val != nil)
		e->val = estrdup9p(val);
	e->file = file;
	e->name = estrdup9p(name);
	m->nentry++;
	return e;
}

int
vardel(Map *m, ulong id)
{
	Entry *e, *del;
	int n;

	n = id % m->size;
	for (e = m->buckets[n].head; e != nil; e = e->next) {
		if (e->id == id) {
			if (e == m->buckets[n].head) {
				m->buckets[n].head = e->next;
				del = e;
			} else if (e->next == nil) {
				e = e->prev;
				del = e->next;
				e->next = nil;
			} else {
				del = e;
				e = e->prev;
				e->next = e->next->next;
				e->next->prev = e;
			}
			if (del->val != nil)
				free(del->val);
			removefile(del->file);
			free(del->name);
			free(del);
			return 0;
		}
	}
	return 1;
}



Var *
varbyid(Map *m, ulong id)
{
	Entry *e;
	int n;

	n = id % m->size;
	for (e = m->buckets[n].head; e != nil; e = e->next) {
		if (e->id == id)
			return e;
	}
	return nil;
}

Var *
varbyname(Map *m, char *name)
{
	Entry *e;
	int i;

	for (i = 0; i < m->size; i++) {
		for (e = m->buckets[i].head; e != nil; e = e->next) {
			if (!strcmp(e->name, name))
				return e;
		}
	}
	return nil;
}

Var *
varset(Map *m, ulong id, char *val)
{
	Entry *e;
	int n;

	n = id % m->size;
	for (e = m->buckets[n].head; e != nil; e = e->next) {
		if (e->id == id)
			/*
			 * If e->val is nil, realloc is fine anyway
			 * (because equivalent to malloc(3)).
			 */
			e->val = erealloc9p(e->val, strlen(val) + 1);
			strcpy(e->val, val);
			return e;
	}
	return nil;
}
