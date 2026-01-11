%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json.tab.h"  /* Include this file here generated after executing json.y file */
%}
%%
[ \t\n]
"true" { return TRUE; }
"false" { return FALSE; }
"null" { return NULL_TOKEN; }
\"[^"]*\" { yylval.str = strdup(yytext); return STRING; }
[-]?[0-9]+(\.[0-9]+)? { yylval.num = atof(yytext); return NUMBER; }
"{" { return OPEN_BRACE; }
"}" { return CLOSE_BRACE; }
"[" { return OPEN_BRACKET; }
"]" { return CLOSE_BRACKET; }
":" { return COLON; }
"," { return COMMA; }
. { yyerror("Invalid character"); }
%%
int yywrap()
{
3
return 1;
}
