#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "interpret.h"

#define countof(X) (sizeof(X) / sizeof(*(X)))

#define INVALID (-1)

const char *prepositions[] = {
  "to",
  "on",
};

const char *articles[] = {
  "a",
  "an",
  "the",
};

static int strnicmp(const char *a, const char *b, size_t l) {
  // TODO unicode
  int d;
  while (l) {
    d = tolower(*a) - tolower(*b);
    if (d != 0 || !*a || !*b) {
      return d;
    }
    ++a;
    ++b;
    --l;
  }
  return 0;
}

static int _is_one_of(char *w, size_t l, const char *words[], size_t c) {
  size_t i;
  for (i = 0; i < c; i++) {
    if (!strnicmp(w, words[i], l)) {
      return 1;
    }
  }
  return 0;
}

#define is_one_of(W, L, WORDS) _is_one_of(W, L, WORDS, countof(WORDS))

static void whitespace(char **p) {
  while (isspace(**p)) {
    ++*p;
  }
}

static char *word(char **p) {
  char *w;

  whitespace(p);
  w = *p;
  while (**p && !isspace(**p)) {
    ++*p;
  }

  // No words left?
  if (w == *p) {
    return NULL;
  }

  return w;
}

static void cut(char **p) {
  // Null terminates something
  if (**p) {
    **p = 0;
    ++*p;
  }
}

static char *verb(char **p) {
  // TODO invalid verbs?

  char *v = word(p);

  if (v) {
    cut(p);
  }

  return v;
}

static int preposition(char **p) {
  size_t i;
  char *text;
  char *prep;

  text = *p;
  prep = word(&text);

  if (prep) {
    for (i = 0; i < countof(prepositions); i++) {
      if (!strnicmp(prep, prepositions[i], text - prep)) {
        *p = text;
        return i;
      }
    }
  }

  return INVALID;
}

static char *noun_phrase_tail(char **p) {
  char *np, *w;
  char *old_loc;

  old_loc = *p;
  np = word(p);

  if (np != NULL
      && !is_one_of(np, *p - np, prepositions)
      && !is_one_of(np, *p - np, articles)
  ) {

    do {
      old_loc = *p;
      w = noun_phrase_tail(p);
    } while (w != NULL);

  }

  // Want to cut before trailing whitespace
  *p = old_loc;
  return np;
}

static char *noun_phrase(char **p) {
  char *np, *old_loc;

  old_loc = *p;
  np = word(p);

  if (np != NULL && !is_one_of(np, *p - np, prepositions)) {

    noun_phrase_tail(p);
    cut(p);
  } else {
    *p = old_loc;
  }

  return np;
}

struct command *parse(const char *text0) {
  struct command *c;
  char *text, *np;
  int prep;

  // The need for this useless cast is C++ being dumb
  c = (struct command *)calloc(sizeof *c, 1);
  text = c->memory = strdup(text0);

  c->verb = verb(&text);

  if (c->verb == NULL) {
    fprintf(stderr, "Invalid verb\n");
    free_command(c);
    return NULL;
  }

  whitespace(&text);
  while (*text) {
    prep = preposition(&text);
    np = noun_phrase(&text);

    if (prep == INVALID) {
      // No preposition implies an object
      if (c->indirect_object != NULL) {
        fprintf(stderr, "Can't have more than 2 objects\n");
        free_command(c);
        return NULL;
      }

      if (c->direct_object != NULL) {
        c->indirect_object = c->direct_object;
      }

      c->direct_object = np;
    } else {
      if (c->phrases[prep] != NULL) {
        fprintf(stderr, "Duplicate prepositional phrases\n");
        free_command(c);
        return NULL;
      }

      c->phrases[prep] = np;
    }

    whitespace(&text);
  }

  return c;
}

void free_command(struct command *c) {
  free(c->memory);
  free(c);
}


void dump_command(const struct command *c) {
  size_t i;

  printf("verb = %s\n", c->verb);
  printf("direct object = %s\n", c->direct_object);
  printf("indirect object = %s\n", c->indirect_object);

  for (i = 0; i < countof(prepositions); i++) {
    printf("prepositional phrase '%s' = %s\n", prepositions[i], c->phrases[i]);
  }
}
