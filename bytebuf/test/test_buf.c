
//
// test.c
//
// Copyright (c) 2012 TJ Holowaychuk <tj@vision-media.ca>
//

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bytebuf.h"

void
equal(char *a, char *b) {
  if (strcmp(a, b)) {
    printf("\n");
    printf("  expected: '%s'\n", a);
    printf("    actual: '%s'\n", b);
    printf("\n");
    exit(1);
  }
}

char *bbuffer_data(bytebuf *buf) {
    int len = 0;
    return bytebuf_convert_to_bytes(buf, &len);
}

int
bbuffer_append_str(bytebuf *buf, char *str) {
    int r = bytebuf_append_bytes(buf, str, (int)strlen(str));
    if (r == 0) return -1;
    return 0;
}

bytebuf *
bbuffer_new_with_str(char *str) {
  return bytebuf_create_with_data(str, (int)strlen(str));
}

// Tests

void
test_bbuffer_new() {
  bytebuf *buf = bytebuf_create();
  assert(0 == bytebuf_size(buf));

}

void
test_bbuffer_append_str() {
  bytebuf *buf = bytebuf_create();
  assert(0 == bbuffer_append_str(buf, "Hello"));
  assert(0 == bbuffer_append_str(buf, " World"));
  assert(strlen("Hello World") == bytebuf_size(buf));
  equal("Hello World", bbuffer_data(buf));
}

void
test_bbuffer_append_str__grow() {
  bytebuf *buf = bytebuf_create();
  assert(0 == bbuffer_append_str(buf, "Hello"));
  assert(0 == bbuffer_append_str(buf, " tobi"));
  assert(0 == bbuffer_append_str(buf, " was"));
  assert(0 == bbuffer_append_str(buf, " here"));

  char *str = "Hello tobi was here";
  equal(str, bbuffer_data(buf));
  assert(strlen(str) == (size_t)bytebuf_size(buf));
}

void
test_bbuffer_clear() {
  bytebuf *buf = bbuffer_new_with_str("Hello");
  assert(5 == bytebuf_size(buf));

  bytebuf_clear(buf);
  assert(0 == bytebuf_size(buf));
}

int
main(){
  test_bbuffer_new();
  test_bbuffer_append_str();
  test_bbuffer_append_str__grow();
  test_bbuffer_clear();
  printf("\n  \e[32m\u2713 \e[90mok\e[0m\n\n");
  return 0;
}
