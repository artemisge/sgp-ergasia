%{
#include <string.h>
#include "compiler.h"
#include "y.tab.h"
%}

letter [a-zA-Z]
ws [ \f\n\r\t]+
id {letter}({letter}|[0-9]|'_')*
num (0|[1-9][0-9]*)(\.[0-9]*)?

%%

"else" {
    debugl(T_ELSE, yytext);
    return(T_ELSE);
}
"float" {
    debugl(T_FLOAT, yytext);
    return(T_FLOAT);
}
"for" {
    debugl(T_FOR, yytext);
    return(T_FOR);
}
"if" {
    debugl(T_IF, yytext);
    return(T_IF);
}
"int" {
    debugl(T_INT, yytext);
    return(T_INT);
}
"main" {
    debugl(T_MAIN, yytext);
    return(T_MAIN);
}
"mainclass" {
    debugl(T_MAINCLASS, yytext);
    return(T_MAINCLASS);
}
"println" {
    debugl(T_PRINTLN, yytext);
    return(T_PRINTLN);
}
"public" {
    debugl(T_PUBLIC, yytext);
    return(T_PUBLIC);
}
"static" {
    debugl(T_STATIC, yytext);
    return(T_STATIC);
}
"void" {
    debugl(T_VOID, yytext);
    return(T_VOID);
}
"while" {
    debugl(T_WHILE, yytext);
    return(T_WHILE);
}
[+-/*(){}=><;,] {
    debugl(yytext[0], yytext);
    yylval.INT=yytext[0];
    return(yytext[0]);
}
"==" {
    debugl(T_EQEQ, yytext);
    return(T_EQEQ);
}
"<=" {
    debugl(T_LEQ, yytext);
    return(T_LEQ);
}
"!=" {
    debugl(T_NE, yytext);
    return(T_NE);
}
">=" {
    debugl(T_GE, yytext);
    return(T_GE);
}
{id} {
    debugl(T_ID, yytext);
    strcpy(yylval.STR, yytext);
    return(T_ID);
}
{num} {
    debugl(T_NUM, yytext);
    strcpy(yylval.STR, yytext);
    return(T_NUM);
}
{ws} {
}
. {
    die("Invalid input: %s\n", yytext);
}

%%
