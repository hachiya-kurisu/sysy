#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define emit(...) fprintf(w, __VA_ARGS__)

#include "sysy.h"

static void step(FILE *w, char c) {
  switch(c) {
    case '&': emit("&amp;"); break;
    case '<': emit("&lt;"); break;
    case '>': emit("&gt;"); break;
    default: putc(c, w);
  }
}

static void advance(FILE *w, char **pos, int skip) {
  *pos += skip;
  while(isspace(**pos)) (*pos)++;
  while(**pos) step(w, *(*pos)++);
}

static void transition(enum state *from, enum state to, FILE *w) {
  if(to == *from) { emit(to == TEXT || to == QUOTE ? "<br>" : ""); return; }
  switch(*from) {
    case LIST: emit("</ul>\n"); break;
    case PRE: emit("</pre>\n"); break;
    case QUOTE: emit("</blockquote>\n"); break;
    default: break;
  }
  switch(to) {
    case TEXT: emit("<p>"); break;
    case LIST: emit("<ul>\n"); break;
    case PRE: emit("<pre>"); break;
    case QUOTE: emit("<blockquote>\n<p>"); break;
    default: break;
  }
  *from = to;
}

static void line(enum state *state, char *pos, FILE *w) {
  if(strspn(pos, " \t") == strlen(pos)) return transition(state, VOID, w);
  if(strncmp(pos, "```", 3) == 0)
    return transition(state, *state == PRE ? VOID : PRE, w);
  if(*state == PRE) { while(*pos) step(w, *pos++); return; }
  if(strncmp(pos, "* ", 2) == 0) {
    transition(state, LIST, w);
    emit("<li>");
    return advance(w, &pos, 2);
  }
  if(pos[0] == '>') {
    transition(state, QUOTE, w);
    return advance(w, &pos, 1);
  }
  if(pos[0] == '#') {
    int level = 0;
    while(pos[level] == '#' && level < 3) level++;
    if(level > 0 && (pos[level] == ' ' || pos[level] == '\0')) {
      transition(state, VOID, w);
      emit("<h%d>", level);
      advance(w, &pos, level);
      emit("</h%d>", level);
      return;
    }
  }
  if(strncmp(pos, "=>", 2) == 0) {
    transition(state, TEXT, w);
    pos += 2 + strspn(pos + 2, " \t");
    char *text = strchr(pos, ' ');
    if(text) *text++ = '\0', text += strspn(text, " \t"); 
    if(strncmp(pos, "javascript:", 11) == 0 || strncmp(pos, "data:", 5) == 0)
      return advance(w, &pos, 0);
    emit("<a href=\"");
    char *url = pos;
    advance(w, &pos, 0);
    emit("\">");
    advance(w, text ? &text : &url, 0);
    emit("</a>");
    return;
  }
  transition(state, TEXT, w);
  while(*pos) step(w, *pos++);
}

int sysy(FILE *r, FILE *w) {
  enum state state = VOID;
  char buf[LINE_MAX] = {0};
  char *pos;
  while((pos = fgets(buf, LINE_MAX, r))) {
    char *nl = strchr(buf, '\n');
    if(!nl && !feof(r)) return (fprintf(stderr, "line too long\n"), 1);
    if(nl) *nl = '\0';
    line(&state, pos, w);
    emit("\n");
  }
  transition(&state, VOID, w);
  return 0;
}
