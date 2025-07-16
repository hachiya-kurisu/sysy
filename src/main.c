#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sysy.h"

int main(int argc, char *argv[]) {
  FILE *w = stdout;
  int opt, opts = 0;
  while((opt = getopt(argc, argv, "vhmo:")) != -1) {
    switch(opt) {
      case 'm': opts = SYSY_MEDIA; break;
      case 'o': if(!(w = fopen(optarg, "w"))) perror(optarg), exit(1); break;
      case 'v': printf("%s %s\n", argv[0], VERSION); return 0;
      case 'h': printf("%s [-hv] [-o output] FILE...\n", argv[0]); return 0;
    }
  }
  argc -= optind; argv += optind;
  if(argc == 0) return sysy(w, stdin, opts);
  for(int i = 0; i < argc; i++) {
    FILE *r = !strcmp(argv[i], "-") ? stdin : fopen(argv[i], "r");
    if(!r) perror(argv[i]), exit(1);
    sysy(w, r, opts);
    if(r != stdin) fclose(r);
  }
  return w == stdout ? 0 : fclose(w);
}
