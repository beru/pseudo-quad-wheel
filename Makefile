ACFILES	= parser.c
MAINC 	= shell.c
CFILES = $(ACFILES) lexer.c code.c eval.c func.c value.c regexp.c pstate.c \
		rbtree.c scope.c utils.c proto.c filesys.ex.c proto.global.c unichar.c proto.string.c \
		number.c proto.number.c proto.array.c mempool.c \
	regex_ecma.c malloc/dlmalloc.c  $(MAINC) load.ex.c
OBJS    = $(CFILES:.c=.o)
DEFIN	= -DUSE_FILESYS_EX -DDONT_USE_POOL
#CFLAGS	= -g -Wl  $(DEFIN) -DYYDEBUG=0 -DDEBUG -DONLY_MSPACES -ansi
YACC	= bison -v
TARGET	= quadwheel
CFLAGS 	= -g $(DEFIN)

.PHONY: all clean cleanall

all: debug

opt: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -lm
	mv a.out $(TARGET)

debug: $(OBJS)
	gcc -g $(CFLAGS) $(OBJS) -lm

stepdebug: $(OBJS)
	gcc -g -Wall -DDEBUG $(DEFIN) $(CFILES) -o $(TARGET) -lm

parser.c: parser.y
	$(YACC) -oparser.c -d parser.y

clean:
	rm -f *.o *.output *.stackdump $(TARGET) a.out

cleanall:
	rm -f $(ACFILES) *.o parser.h *.output *.stackdump $(TARGET)


upload:
	support-read-only/scripts/googlecode_upload.py -u username -w passwd -s "see ChangeLog" -p blue-quad-wheel blue-quad-wheel-0.4.tar.gz	

