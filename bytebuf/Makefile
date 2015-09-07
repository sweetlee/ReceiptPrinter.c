
CC ?= cc
VALGRIND ?= valgrind
CFLAGS = -std=c99 -Wall -Wextra -I../../deps -I.
LDFLAGS =""
VALGRIND_OPTS ?= --leak-check=full
SRC = $(wildcard *.c)
#SRC += $(wildcard ../../deps/*/*.c)

SRC += $(wildcard ../../deps/describe/*.c)
SRC += $(wildcard ../../deps/console-colors/*.c)

OBJS = $(SRC:.c=.o)
TESTS = $(wildcard test/*.c)

test: $(TESTS)
	$(foreach t, $(TESTS), @rm -f $(basename $(t));)
	@rm -rf *.o

$(TESTS): $(OBJS)
	@$(CC) $(CFLAGS) -o $(basename $@) $@ $(OBJS) $(LDFLAGS)
	./$(basename $@)
# @$(TEST_RUNNER) ./$(basename $@)

# grind:
# 	@TEST_RUNNER="$(VALGRIND) $(VALGRIND_OPTS)" \
# 		$(MAKE) test

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

clean:
	$(foreach t, $(TESTS), rm -f $(basename $(t));)
	rm -rf *.o
# rm -f example $(OBJS)
# rm -rf test/fixtures

.PHONY: test $(TESTS)
#clean #grind
