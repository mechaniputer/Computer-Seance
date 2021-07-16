#ifndef INTERPRET_H
#define INTERPRET_H

struct command *parse(const char *text0);
void free_command(struct command *c);
void dump_command(const struct command *c);

enum preposition {
  TO = 0,
  ON,
  MAX
};

struct command {
  char *memory;
  char *verb;
  char *direct_object;
  char *indirect_object;
  char *phrases[MAX];
};


extern const char *prepositions[];
extern const char *articles[];

#endif
