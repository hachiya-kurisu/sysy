#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "sysy.h"

#define emit(...) fprintf(w, __VA_ARGS__)

static void step(FILE *w, char c) {
  switch(c) {
    case '&': emit("&amp;"); break;
    case '<': emit("&lt;"); break;
    case '>': emit("&gt;"); break;
    case '"': emit("&quot;"); break;
    default: putc(c, w);
  }
}

static void advance(FILE *w, char **pos, int skip) {
  *pos += skip; *pos += strspn(*pos, " \t"); while(**pos) step(w, *(*pos)++);
}

static void shift(FILE *w, enum state *from, enum state to) {
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

static void line(FILE *w, enum state *s, char *pos, int opts) {
  if(strncmp(pos, "```", 3) == 0) return shift(w, s, *s == PRE ? VOID : PRE);
  if(*s == PRE) { while(*pos) step(w, *pos++); return; }
  if(strspn(pos, " \t") == strlen(pos)) return shift(w, s, VOID);
  if(strncmp(pos, "* ", 2) == 0)
    return shift(w, s, LIST), emit("<li>"), advance(w, &pos, 2), (void)0;
  if(pos[0] == '>') return shift(w, s, QUOTE), advance(w, &pos, 1);
  if(pos[0] == '#') {
    int level = strspn(pos, "#"); level = level > 3 ? 3 : level;
    return shift(w, s, VOID), emit("<h%d>", level),
           advance(w, &pos, level), emit("</h%d>", level), (void)0;
  }
  if(strncmp(pos, "=>", 2) == 0) {
    shift(w, s, TEXT); pos += 2 + strspn(pos + 2, " \t");
    char *text = strchr(pos, ' ');
    if(text) *text++ = '\0', text += strspn(text, " \t"); 
    if(!strncmp(pos, "javascript:", 11) || !strncmp(pos, "data:", 5))
      return advance(w, text ? &text : &pos, 0);
    char *url = pos, *ext = strrchr(pos, '.');
    if(!ext || !(opts & SYSY_MEDIA)) goto fallback;
    if((!strcmp(ext, ".jpg") || !strcmp(ext, ".png") || !strcmp(ext, ".gif")))
      return emit("<img src=\""), advance(w, &pos, 0), emit("\" alt=\""),
             advance(w, text ? &text : &url, 0), emit("\">"), (void)0;
    if((!strcmp(ext, ".mp4") || !strcmp(ext, ".webm")))
      return emit("<video src=\""), advance(w, &pos, 0),
             emit("\" preload=metadata controls></video>"), (void)0;
    if((!strcmp(ext, ".m4a") || !strcmp(ext, ".mp3")))
      return emit("<audio src=\""), advance(w, &pos, 0),
             emit("\" preload=metadata controls></audio>"), (void)0;
fallback:
    return emit("<a rel=nofollow href=\""), advance(w, &pos, 0), emit("\">"),
           advance(w, text ? &text : &url, 0), emit("</a>"), (void)0;
  }
  shift(w, s, TEXT); while(*pos) step(w, *pos++);
}

int sysy(FILE *w, FILE *r, int opts) {
  enum state s = VOID;
  char buf[LINE_MAX] = {0}, *pos;
  while((pos = fgets(buf, LINE_MAX, r))) {
    char *nl = strchr(buf, '\n');
    if(!nl && !feof(r)) return (fprintf(stderr, "line too long\n"), 1);
    if(nl) *nl = '\0'; line(w, &s, pos, opts); emit("\n");
  }
  return shift(w, &s, VOID), 0;
}
