
CFLAGS ?= -g
LDFLAGS ?= 

TARGETS = plus3read plusdread deocp plus3tap

all: $(TARGETS)


%: %.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) *.o
	$(RM) $(TARGETS)


