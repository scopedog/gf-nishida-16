CC		= cc
EXECUTABLE	= gf-bench
MAIN		= gf-bench.c
INTERFACES	= mt19937-64.c
SRCS		= $(MAIN) $(INTERFACES)
OBJS		= $(SRCS:.c=.o)
#LIBS		= -lgf_complete
LIBS		= /home/osu/gf-bench/lib/libgf_complete.a
#LIBPATH		= -L/home/osu/gf-bench/lib
INCPATH		= -I/home/osu/gf-bench/include
CFLAGS		= -O3 -mmmx -msse -DINTEL_SSE -msse2 -DINTEL_SSE2 -msse3 -DINTEL_SSE3 -mssse3 -DINTEL_SSSE3 -fPIC $(INCPATH) 

##################################################################

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

$(EXECUTABLE) : $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBPATH) $(LIBS)

all : $(EXECUTABLE)

clean:
	rm -f *.o *.core $(EXECUTABLE)

depend:
	mkdep $(CFLAGS) $(SRCS)

bench : $(EXECUTABLE)
	@basename `pwd`
	@./$(EXECUTABLE)


