all: myshell test

myshell: myshell.c lex.yy.c
	gcc myshell.c lex.yy.c -lfl -o myshell

test: test.c lex.yy.c
	gcc test.c lex.yy.c -lfl -o test

lex.yy.c: lex.c
	flex lex.c

debug: myshell.c lex.yy.c
	gcc -g myshell.c lex.yy.c -lfl -o myshell.debug

clean:
	$(RM) myshell test lex.yy.c
