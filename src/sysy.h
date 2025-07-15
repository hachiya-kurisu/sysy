#ifndef SYSY_H
#define SYSY_H
enum state { VOID, TEXT, LIST, PRE, QUOTE };
int sysy(FILE *r, FILE *w);
#endif
