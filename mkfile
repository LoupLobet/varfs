<$PLAN9/src/mkhdr

CFLAGS=-Wall -pedantic
LDFLAGS=

ALL=varfs

all:V:$ALL

varfs: varfs.$O var.$O util.$O
	$LD $LDFLAGS -o $target $prereq

%.$O: %.c
	$CC $CFLAGS -c $stem.c

clean:V:
	rm -f $ALL *.[$OS]