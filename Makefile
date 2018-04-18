
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
HDRS := $(wildcard *.h)

CFLAGS := -Wall -Werror -g `pkg-config fuse --cflags`
LDLIBS := `pkg-config fuse --libs`

all: nufs tests

tests:
	(cd tests && make -f make)

nufs: $(OBJS)
	gcc $(CLFAGS) -o $@ $^ $(LDLIBS)

%.o: %.c $(HDRS)
	gcc $(CFLAGS) -c -o $@ $<

clean: unmount
	rm -f nufs *.o test.log data.nufs
	rmdir mnt || true
	(cd tests && make -f make clean)

mount: nufs
	mkdir -p mnt || true
	./nufs -s -f mnt data.nufs

unmount:
	fusermount -u mnt || true

test: all
	perl test.pl

gdb: nufs
	mkdir -p mnt || true
	gdb --args ./nufs -s -f mnt data.nufs

.PHONY: clean mount unmount gdb all tests

