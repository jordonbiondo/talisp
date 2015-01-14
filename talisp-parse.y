%{
void yyerror (char *s);
int yylex(void);
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 enum ltype {
   bt,
   it,
   st,
   et
 };
 
 union ldata {
   int n;
   char* sym;
 };
 
 struct lthing {
   enum ltype t;
   union ldata d;
 };

 struct sf {
   struct lthing o;
   struct sf* n;
   struct sf* p;
 };
 
 struct sf* sp;
 const struct sf* eos;

#define eachFrame(sym) for (struct sf* sym = sp; sym != eos; sym = sym->p)
#define forwardFrames(frame, name) for (struct sf* name = (frame); name ; name = name->n)
 void dumpFrames() {
   /* printf("stack:\n"); */
   eachFrame(f) {
     /* printf("at: %s\n", (f->o.t == bt) ? "bool" : (f->o.t == it) ? "int" : (f->o.t == st) ? "sym" : "expr"); */
   }
 }
 
 void pNum(int n) {
   /* printf("pushing %d\n", n); */
   struct sf* f = malloc(sizeof(struct sf));
   sp->n = f;
   f->p = sp;
   sp = f;
   f->o.t = it;
   f->o.d.n = n;
 }

 void pBool(int n) {
   /* printf("pushing %s\n", n == 1 ? "true" : "false"); */
   struct sf* f = malloc(sizeof(struct sf));
   sp->n = f;
   f->p = sp;
   sp = f;
   f->o.t = bt;
   f->o.d.n = (n == 1);
 }

 void execute (struct sf* frame) {
   char* string = frame->o.d.sym;
   struct sf* args = frame->n;
   /* printf("execing (%s ...)\n", string); */
   if (strcmp(string, "list") == 0)  {
   } else if (strcmp(string, "+") == 0) {
     int o = 0;
     // do type check
     forwardFrames(args, arg) { o += arg->o.d.n; }
     // free sym and arg frames
     pNum(o);
     printf("output is %d\n", o);
   } else if (strcmp(string, "-") == 0) {
   } else if (strcmp(string, "=") == 0) {
   } else if (strcmp(string, "*") == 0) {
   } else if (strcmp(string, "/") == 0) {
   } else if (strcmp(string, "%") == 0) {
   } else if (strcmp(string, "inc") == 0) {
   } else if (strcmp(string, "dec") == 0) {
   } else if (strcmp(string, "length") == 0) {
   } else if (strcmp(string, "not") == 0) {
   } else {
     printf("Unknown function!!!");
   }
 }

 void pExpr() {
   struct sf* f = sp;
   eachFrame(d) {
     if (d->o.t == st) {
       f = d;
       break;
     }
   }
   sp = f->p;
   execute(f);
 }

 void pSym(char* s) {
   /* printf("pushing sym %s\n", s); */
   struct sf* f = malloc(sizeof(struct sf));
   sp->n = f;
   f->p = sp;
   sp = f;
   f->o.d.sym = s;
   f->o.t = st;
   dumpFrames();
 }


%}

%union {int num; char* id;}
%start exprlist;
%token <num> number;
%token <id> symbol;
%token <id> lparen;
%token <id> rparen;
%token <id> tsym;
%token <id> fsym;
%token end;
%%
 
expr:
lparen func exprlist rparen {
} |
number {pNum($1);} |
tsym {pBool(1);} |
fsym {pBool(0);} |
end {
  YYACCEPT;
}

func: symbol {pSym($1);}

exprlist:
expr exprlist {;} |
expr {pExpr();}
%%

void yyerror (char *s) {
  fprintf (stderr, "%s\n", s);
}

int main(int argc, char* argv[]) {
  sp = malloc(sizeof(struct sf));
  eos = sp;
  yyparse();
}
