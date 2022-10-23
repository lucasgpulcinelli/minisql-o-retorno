CFLAGS  += -std=gnu99 -Wall -Wextra
LDFLAGS +=
VDFLAGS  = --track-origins=yes -v --leak-check=full

EXECUTABLE ?= build/main
ZIPFILE    ?= ../zipfile.zip
CFILES      = $(shell find src/ -type f |grep '\.c')
OFILES      = $(patsubst src/%.c,build/obj/%.o, $(CFILES))


.PHONY: all clean zip run debug gdb valgrind


all: $(EXECUTABLE)

clean:
	@rm -f $(ZIPFILE)
	@rm -rf build/

zip: clean
	7za a $(ZIPFILE) ./*

gdb: $(EXECUTABLE)
	gdb $(EXECUTABLE)

run: $(EXECUTABLE)
	@./$(EXECUTABLE) $(ARGS)

valgrind: debug
	valgrind $(VDFLAGS) $(EXECUTABLE)

debug: CFLAGS+=-g -O0
debug: clean
debug: $(EXECUTABLE)


$(EXECUTABLE): $(OFILES)
	@mkdir -p build
	$(CC) $(LDFLAGS) -o $@ $^

build/obj/%.o: src/%.c src/%.h
	@mkdir -p build
	@mkdir -p build/obj
	$(CC) $(CFLAGS) -c -o $@ $<

build/obj/%.o: src/%.c
	@mkdir -p build
	@mkdir -p build/obj
	$(CC) $(CFLAGS) -c -o $@ $<

-include src/src.mk
