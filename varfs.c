#include <u.h>
#include <libc.h>
#include <fcall.h>
#include <thread.h>
#include <9p.h>

#include "err.h"

typedef struct {
	char *name;
	ulong mode;
} FsFile;

typedef struct {
	char *data;
	char *name;
	u32int ndata;
} Var;

static void	 delvar(Req *);
static void	 destroy(File *);
static void	 fsread(Req *);
static void	 fswrite(Req *);
static void	 newvar(Req *);
static void	 readvar(Req *);
static void	 usage(void);
static void	 writevar(Req *);

enum {
	Qnew,
	Qdel,
	Qend,
};

static Srv fs = {
	/*
	 * Variable (file) creation is achieved by writing to
	 * /new, hence, fscreate doesn't have any implementation
	 * and file variable can't be created this way.
	 *
	 * See 9p(3), for file tree handled functions.
	 */
	.read = fsread,
	.write = fswrite,
};

static FsFile fsfile[Qend] = {
	[Qnew]    = { "new",   0222 },
	[Qdel]    = { "del",   0222 },
};


void
threadmain(int argc, char *argv[])
{
	Err *e;
	File *f;
	Var *v;
	int i;

	ARGBEGIN {
	default:
		usage();
	} ARGEND

	fs.tree = alloctree(nil, nil, DMDIR|0777, destroy);
	for (i = Qnew; i < Qend; i++) {
		createfile(fs.tree->root, fsfile[i].name, nil, fsfile[i].mode, nil);
	}
	/* create command line passed variables */
	e = err();
	for (i = 0; i < argc; i++) {
		f = createfile(fs.tree->root, argv[i], nil, 0666, nil);
		if (f == nil) {
			errcat(e, "could not create variable: %s\n", argv[i]);
			continue;
		}
		v = emalloc9p(sizeof(Var));
		v->name = estrdup9p(argv[i]);
		v->data = nil;
		f->aux = v;
	}
	if (e->ndata)
		fprint(2, e->data);
	fs.foreground = 1;
	threadpostmountsrv(&fs, "varfs", nil, MREPL | MCREATE);
}

static void
delvar(Req *r)
{
	Err *e;
	File *f;
	char *name;

	e = err();
	for (name = strtok(r->ifcall.data, " \n"); name != nil;
	     name = strtok(NULL, " \n"))
	{
		f = fs.tree->root;
		incref((Ref *)f);
		f = walkfile(f, name);
		if (f == nil) {
			errcat(e, "\nno such variable: %s", name);
			continue;
		}
		closefile(f);
		if (removefile(f)) {
			errcat(e, "could not delete variable: %s\n", name);
			continue;
		}
	}
	if (e->ndata) {
		respond(r, e->data);
	} else {
		r->ofcall.count = r->ifcall.count;
		respond(r, nil);
	}
}

static void
destroy(File *f)
{
	Var *v;

	v = f->aux;
	if (v != nil) {
		free(v->name);
		if (v->ndata)
			free(v->data);
		free(v);
	}
}

static void
fsread(Req *r)
{
	ulong path;

	path = r->fid->qid.path;
	switch (path) {
	case Qnew:
	case Qdel:
		respond(r, "this file isn't supposed to be read");
		break;
	default:
		readvar(r);
		break;
	}

}

static void
fswrite(Req *r)
{
	ulong path;

	path = r->fid->qid.path;
	switch (path) {
	case Qnew:
		newvar(r);
		break;
	case Qdel:
		delvar(r);
		break;
	default:
		writevar(r);
		break;
	}
}

static void
newvar(Req *r)
{
	Err *e;
	File *f;
	Var *v;
	char *name;

	e = err();
	for (name = strtok(r->ifcall.data, " \n"); name != nil;
	     name = strtok(NULL, " \n"))
	{
		f = createfile(fs.tree->root, name, r->fid->uid, 0666, nil);
		if (f == nil) {
			errcat(e, "\ncould not create variable: %s", name);
			continue;
		}
		v = emalloc9p(sizeof(Var));
		v->name = estrdup9p(name);
		v->data = nil;
		f->aux = v;
	}
	if (e->ndata) {
		respond(r, e->data);
	} else {
		r->ofcall.count = r->ifcall.count;
		respond(r, nil);
	}
}

static void
readvar(Req *r)
{
	Var *v;

	v = r->fid->file->aux;
	readstr(r, v->data);
	respond(r, nil);
}

static void
usage(void)
{
	fprint(2, "usage: varfs [var ...]\n");
	threadexitsall("usage");
}

static void
writevar(Req *r)
{
	Var *v;

	v = r->fid->file->aux;
	v->data = erealloc9p(v->data, r->ifcall.count + 1);
	strcpy(v->data, r->ifcall.data);
	v->ndata = r->ifcall.count;
	r->ofcall.count = r->ifcall.count;
	respond(r, nil);

}
