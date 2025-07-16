#ifndef SYSY_H
#define SYSY_H
#define SYSY_MEDIA 1
enum state { VOID, TEXT, LIST, PRE, QUOTE };
int sysy(FILE*, FILE*, int);
#endif
