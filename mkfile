# plan9 uncomment
# </$objtype/mkfile
# plan9port uncomment
<$PLAN9/src/mkhdr

CFLAGS=-Wall -pedantic
LDFLAGS=

ALL=varfs

all:V:$ALL

varfs: varfs.$O err.$O
	$LD $LDFLAGS -o $target $prereq

%.$O: %.c
	$CC $CFLAGS -c $stem.c

clean:V:
	rm -f $ALL *.[$OS]