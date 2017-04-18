
CFLAGS ?= -g
LDFLAGS ?= 

TARGETS = plus3read plusdread deocp

all: $(TARGETS)


%: %.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) *.o
	$(RM) $(TARGETS)


