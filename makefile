talisp:  lex.yy.c y.tab.c 
	gcc -std=gnu99 -Wall -g -lpthread lex.yy.c y.tab.c -o talisp

lex.yy.c: talisp-syntax.l
	flex talisp-syntax.l

y.tab.c: talisp-parse.y lex.yy.c
	yacc -d talisp-parse.y

clean: 
	rm -f lex.yy.c y.tab.c y.tab.h talisp

default: talisp
