%{
void yyerror (char *s);  
#include "y.tab.h"
#include <string.h>
#include <wchar.h>
%}
%option nounput
%option noinput
%%
"true" {return tsym;}
"false" {return fsym;}
[a-zA-Z\+\-\*/=%]+ {yylval.id = strdup(yytext);return symbol;}
[1-9][0-9]* {yylval.num = atof(yytext);return number;}
"(" {return lparen;}
")" {return rparen;}
[ \t\n\r] {;}
<<EOF>> {return end;}
. {ECHO;yyerror ("unexpected char during parse!\n");}
%%
int yywrap(void) {return 1;}

/* dl-scratch.c ends here */
