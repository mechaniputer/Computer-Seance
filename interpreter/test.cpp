#include <stdio.h>
#include <string.h>
#include "interpret.h"

int main() {
  struct command *c;
  char line[500];

  while (!feof(stdin)) {
    printf("> ");
    fgets(line, 500, stdin);
    c = parse(line);
    dump_command(c);

    if (!strcmp(c->verb, "quit")) {
      break;
    }

    free_command(c);
  }

  return 0;
}
