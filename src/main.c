#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sysy.h"

static void usage(const char *name) {
  printf("usage: %s [-hv] [-o output] FILE...\n", name);
  printf("when FILE is -, reads from stdin\n");
}

static void version(const char *name) {
  printf("%s %s\n", name, VERSION);
}

int main(int argc, char *argv[]) {
  FILE *w = stdout;
  int opt;
  while((opt = getopt(argc, argv, "vho:")) != -1) {
    switch(opt) {
      case 'o':
        if(!(w = fopen(optarg, "w"))) {
          perror(optarg);
          exit(1);
        }
        break;
      case 'v': version(argv[0]); return 0;
      case 'h': usage(argv[0]); return 0;
    }
  }
  argc -= optind; argv += optind;

  if(argc == 0) return sysy(stdin, w);

  FILE *r;
  for(int i = 0; i < argc; i++) {
    r = !strcmp(argv[i], "-") ? stdin : fopen(argv[i], "r");
    if(!r) {
      perror(argv[i]);
      exit(1);
    }
    sysy(r, w);
    if(r != stdin) fclose(r);
  }
  if(w != stdout) fclose(w);
  return 0;
}
