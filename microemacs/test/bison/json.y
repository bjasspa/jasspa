%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int yylex();
extern int yyparse();
extern FILE *yyin;
extern char* yytext;
int errorPrinted = 0;
void yyerror(const char *detail);
%}
%union
{
int num;
4
char *str;
}
%token TRUE FALSE NULL_TOKEN
%token STRING NUMBER
%token OPEN_BRACE CLOSE_BRACE OPEN_BRACKET CLOSE_BRACKET COLON
COMMA
%type <num> NUMBER
%type <str> STRING
%start json
%%
json:
value { printf("\n\t\t\tJSON is valid!\n"); }
;
value:
STRING
| NUMBER
| TRUE
| FALSE
| NULL_TOKEN
| object
| array
;
object:
OPEN_BRACE members CLOSE_BRACE
;
members:
pair
| pair COMMA members
;
pair:
STRING COLON value
;
array:
OPEN_BRACKET elements CLOSE_BRACKET
;
elements:
value
| value COMMA elements
;
%%
void yyerror(const char *detail) {
if (!errorPrinted) {
printf("\n\t\t\tJSON is invalid\n\t\t\tError: %s\n", detail);
fprintf(stderr,"\t\t\tUnexpected or missing character near: %s\n", yytext);
errorPrinted = 1;
}
}
int main(int argc, char *argv[])
{
if (argc < 2)
{
fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
return 1;
}
yyin = fopen(argv[1], "r");
if (!yyin)
{
perror(argv[1]);
return 1;
}
printf("\n\n\t\t\t+
+\n");
printf("\t\t\t|\t\t\t\t\t\t\t\t\tJSON Parser\t\t\t\t\t\t\t\t\t|");
printf("\n\t\t\t+
+\n\n");
printf("\t\t\t**Input file content:**\n\n");
int ch;
int indentation = 28;
6
int isFirstCharacter = 1;
while ((ch = fgetc(yyin)) != EOF) {
if (isFirstCharacter) {
for (int i = 0; i < indentation; ++i) {
putchar(' ');
}
isFirstCharacter = 0;
}
putchar(ch);
if (ch == '\n') {
isFirstCharacter = 1;
}
}
rewind(yyin);
printf("\n\t\t\t+
+\n");
printf("\t\t\t|\t\t\t\t\t\t\t\t\tParsed Output\t\t\t\t\t\t\t\t\t|\n");
printf("\t\t\t+
+\n\n");
yyparse();
fclose(yyin);
printf("\n\n\n");
return 0;
}
