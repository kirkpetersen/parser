%{
#include <ctype.h>
%}

%token DIGIT

%%

E  : T E2
   ;

E2 : '+' T E2
   |
   ;

T  : F T2
   ;

T2 : '*' F T2
   |
   ;

F  : '(' E ')'
   | DIGIT
   ;

%%

yylex(void)
{
    int c;

    c = getchar();

    if(isdigit(c)) {
	yylval = c - '0';
	return DIGIT;
    }

    return c;
}
