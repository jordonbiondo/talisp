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
    lt,
    ft
  };
  
  char* ttos (int o) {
    switch (o) {
    case bt: return "bool";
    case it: return "int";
    case ft: return "func";
    case st: return "symbol";
    case lt: return "list";
    }
    return "unknown";
  }
  
  union ldata {
    int n;
    char* sym;
    char* str;
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

  
  void oval (struct lthing t) {
    switch (t.t) {
    case bt: printf("%s", t.d.n ? "true" : "false") ; return;
    case it: printf("%d", t.d.n); return;
    case ft:
    case st: printf("%s", t.d.sym); return;
    case lt: printf("not yet"); return;
    }
  }
  
  struct sf* sp;
  const struct sf* eos;
  
#define eachFrame(sym) for (struct sf* sym = sp; sym != eos; sym = sym->p)
#define forwardFrames(frame, name) for (struct sf* name = (frame); name ; name = name->n)
  void dumpFrames() {
    printf("stack:\n");
    eachFrame(f) {
      printf("at: %s\n", (f->o.t == bt) ? "bool" : (f->o.t == it) ? "int" : (f->o.t == st) ? "sym" : "expr");
    }
  }
  
  void finfo(struct sf* f) {
    printf("==== frame: %p ====\n", f);
    printf("      next: %p\n", f->n);
    printf("      prev: %p\n", f->p);
    printf("      type: %s\n", ttos(f->o.t));
    printf("       val: "); oval(f->o); printf("\n");
  }
  
  struct sf* mframe() {
    struct sf* f = malloc(sizeof(struct sf));
    f->p = NULL;
    f->n = NULL;
    return f;
  }
  
  void pNum(int n) {
    /* printf("pushing %d\n", n); */
    struct sf* f = mframe();
    sp->n = f;
    f->p = sp;
    sp = f;
    f->o.t = it;
    f->o.d.n = n;
  }
  
  void pBool(int n) {
    /* printf("pushing %s\n", n == 1 ? "true" : "false"); */
    struct sf* f = mframe();
    sp->n = f;
    f->p = sp;
    sp = f;
    f->o.t = bt;
    f->o.d.n = (n == 1);
  }

  void dplus (struct sf* args) {
    int o = 0;
    // do type check
    forwardFrames(args, arg) { o += arg->o.d.n; }
    // free sym and arg frames
    pNum(o);
  }

  void dminus (struct sf* args) {
    int o = args->o.d.n;
    if (!args->n) {
      o = -o;
    } else {
      forwardFrames(args->n, arg) { o -= arg->o.d.n; }
    }
    pNum(o);
  }

  void dmult (struct sf* args) {
    int o = 1;
    // do type check
    forwardFrames(args, arg) { o *= arg->o.d.n; }
    // free sym and arg frames
    pNum(o);
  }

  void deql (struct sf* args) {
    enum ltype t = args->o.t;
    int equal = 1;
    forwardFrames(args->n, arg) {
      if (arg->o.t == t) {
        switch(t) {
        case bt:
        case it: equal = arg->o.d.n == args->o.d.n;  break;
        case lt: printf("noope"); break;
        case ft:
        case st: equal = (strcmp(arg->o.d.sym, args->o.d.sym) == 0); break;
        }
      }
      if (!equal) break;
    }
    pBool(equal);
  }

  void execute (struct sf* frame) {
    char* string = frame->o.d.sym;
    printf("executing '(%s ", string);
    struct sf* args = frame->n;
    forwardFrames(args, arg) {
      oval(arg->o);
      if (arg->n) printf(" ");
    }
    printf(")'\n");
    if (strcmp(string, "list") == 0)  {
    } else if (strcmp(string, "+") == 0) {
      return dplus(args);
    } else if (strcmp(string, "-") == 0) {
      return dminus(args);
    } else if (strcmp(string, "=") == 0) {
      return deql(args);
    } else if (strcmp(string, "*") == 0) {
      return dmult(args);
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
      if (d->o.t == ft) {
        f = d;
        break;
      }
    }
    sp = f->p;
    execute(f);
  }

  void pSym(char* s) {
    printf("pushing sym %s\n", s);
    struct sf* f = mframe();
    sp->n = f;
    f->p = sp;
    sp = f;
    f->o.d.sym = s;
    f->o.t = st;
  }

  void pStr(char* s) {
    printf("pushing string \"%s\"\n", s);
    struct sf* f = mframe();
    sp->n = f;
    f->p = sp;
    sp = f;
    f->o.d.str = s;
    f->o.t = st;
  }


  void pFunc(char* s) {
    printf("pushing func %s\n", s);
    pSym(s);
    sp->o.t = ft;
  }


%}

%union {int num; char* id;}
%start exprlist;
%token <num> number;
%token <id> symbol;
%token <id> astr;
%token <id> lparen;
%token <id> rparen;
%token <id> tsym;
%token <id> fsym;
%token end;
%%
expr:
lparen func exprlist rparen {
  pExpr();
} |
lparen func rparen {
  pExpr();
} |
number {pNum($1);} |
symbol {pSym($1);} |
astr {pStr($1);} |
tsym {pBool(1);} |
fsym {pBool(0);} |
end {
  YYACCEPT;
}


func:
  symbol {pFunc($1);}

exprlist:
  expr exprlist {} |
  expr {}

%%

void yyerror (char *s) {
  fprintf (stderr, "%s\n", s);
}

int main(int argc, char* argv[]) {
  sp = mframe();
  eos = sp;
  yyparse();
  finfo(sp);
}
