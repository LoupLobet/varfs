#include <u.h>
#include <libc.h>
#include <fcall.h>
#include <thread.h>
#include <9p.h>

#include "util.h"
#include "var.h"

#define TYPE(path)		((int)(path) & 0xFF)

typedef struct {
	char *name;
	ulong mode;
} FsFile;

static void	 fsread(Req *);
static void	 fswrite(Req *);
static void	 fsinit(void);

enum {
	Qnew,
	Qdel,
	Qend,
};

static FsFile fsfile[Qend + 1] = {
	[Qvar]    = { "var",   0666|DMDIR },
	[Qnew]    = { "new",   0222 },
	[Qdel]    = { "del",   0222 },
};

static Srv fs = {
	.read = fsread,
	.write = fswrite,
};

static uvlong npath = 0;
static Map *vars;

void
threadmain(int argc, char *argv[])
{
	vars = mapalloc();
	fsinit();
}

static void
fsread(Req *r)
{
	Var *var;
	ulong path;

	path = r->fid->qid.path;
	switch (TYPE(path)) {
	case Qnew:
	case Qdel:
		sysfatal("varfs fatal error");
		break;
	default:
		if ((var = varbyid(vars, path)) != nil) {
			readstr(r, var->val);
			respond(r, nil);
		} else
			respond(r, "no such variable");
		break;
	}
}

static void
fswrite(Req *r)
{
	File *file;
	ulong path;
	char *name;
	Var *var;

	path = r->fid->qid.path;
	switch (TYPE(path)) {
	case Qnew:
		name = r->ifcall.data;
		for (name = strtok(r->ifcall.data, "\n"); name != nil;
		     name = strtok(NULL, "\n"))
		{
			file = createfile(fs.tree->root, name, nil, 0777, nil);
			if (file != nil) {
				(void)varadd(vars, npath, name, nil, file);
				npath++;
			}
		}
		r->ofcall.count = r->ifcall.count;
		respond(r, nil);
		break;
	case Qdel:
		name = r->ifcall.data;
		for (name = strtok(r->ifcall.data, "\n"); name != nil;
		     name = strtok(NULL, "\n"))
		{
			/*
			 * walkfile(3) then removefile(3) could have been used
			 * spot the var exists, but if the var exists, we must
			 * go through vars map anyway to spot variable name.
			 * But it's possible to retrieve the File pointer in
			 * the Var struct and loop once in total.
			 */
			if ((var = varbyname(vars, name)) != nil) {
				if (removefile(var->file))
					print("can't delete file %s\n", name);
				if (vardel(vars, var->id))
					print("can't delete variable %s\n", name);
			}
		}
		r->ofcall.count = r->ifcall.count;
		respond(r, nil);
		break;
	default:
		varset(vars, path, r->ifcall.data);
		r->ofcall.count = r->ifcall.count;
		respond(r, nil);
		break;
	}
}

static void
fsinit(void)
{
	int i;

	/* build file tree */
	fs.tree = alloctree(nil, nil, DMDIR|0777, nil);
	for (i = Qnew; i < Qend; i++) {
		createfile(fs.tree->root, fsfile[i].name, nil, fsfile[i].mode, nil);
		npath++;
	}
	fs.foreground = 1;
	threadpostmountsrv(&fs, "varfs", nil, MREPL | MCREATE);
}
