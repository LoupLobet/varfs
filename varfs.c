#include <u.h>
#include <libc.h>
#include <fcall.h>
#include <thread.h>
#include <9p.h>

#include "err.h"
#include "util.h"

#define TYPE(path)		((int)(path) & 0xFF)

typedef struct {
	char *name;
	ulong mode;
} FsFile;

typedef struct {
	char *name;
	char *data;
	int ndata;
} Var;

static void	 delvar(Req *);
static void	 destroy(File *);
static void	 newvar(Req *);
static void	 fsread(Req *);
static void	 fswrite(Req *);
static void	 fsinit(void);
static void	 writevar(Req *);

enum {
	Qnew,
	Qdel,
	Qend,
};

static FsFile fsfile[Qend] = {
	[Qnew]    = { "new",   0666 },
	[Qdel]    = { "del",   0666 },
};

static Srv fs = {
	.read = fsread,
	.write = fswrite,
	/* See 9p(3), for file tree handled functions */
};

void
threadmain(int argc, char *argv[])
{
	fsinit();
}

static void
delvar(Req *r)
{
	Err *e = nil;
	File *f;
	char *name;

	for (name = strtok(r->ifcall.data, "\n"); name != nil;
	     name = strtok(NULL, "\n"))
	{
		f = walkfile(fs.tree->root, name);
		if (f == nil || removefile(f)) {
			if (e == nil && (e = err("could not delete var:")) == nil)
				sysfatal("something went wrong ...\n");
			errcat(e, "\n", name);
			continue;
		}
	}
	if (e != nil)
		respond(r, e->data);
	else {
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
newvar(Req *r)
{
	Err *e = nil;
	File *f;
	Var *v;
	char *name;

	for (name = strtok(r->ifcall.data, "\n"); name != nil;
	     name = strtok(NULL, "\n"))
	{
		f = createfile(fs.tree->root, name, r->fid->uid, 0666, nil);
		if (f == nil) {
			if (e == nil && (e = err("could not create var:\n")) == nil)
				sysfatal("something went wrong ...\n");
			errcat(e, name, "\n");
			continue;
		}
		v = emalloc9p(sizeof(Var));
		v->name = estrdup9p(name);
		v->data = nil;
		f->aux = v;
	}
	if (e != nil)
		respond(r, e->data);
	else {
		r->ofcall.count = r->ifcall.count;
		respond(r, nil);
	}
}

static void
fsread(Req *r)
{
	Var *v;
	ulong path;

	v = r->fid->file->aux;
	path = r->fid->qid.path;
	switch (TYPE(path)) {
	case Qnew:
	case Qdel:
		respond(r, "this file isn't supposed to be read");
		break;
	default:
		readstr(r, v->data);
		respond(r, nil);
		break;
	}

}

static void
fswrite(Req *r)
{
	ulong path;

	path = r->fid->qid.path;
	switch (TYPE(path)) {
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
fsinit(void)
{
	int i;

	/* build file tree */
	fs.tree = alloctree(nil, nil, DMDIR|0777, destroy);
	for (i = Qnew; i < Qend; i++)
		createfile(fs.tree->root, fsfile[i].name, nil, fsfile[i].mode, nil);
	fs.foreground = 1;
	threadpostmountsrv(&fs, "varfs", nil, MREPL | MCREATE);
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
