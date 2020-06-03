%{
#include <stdio.h>
#include <stdlib.h>
%}

%union {
  int INT; float FLOAT; char ID;
}
%start PROGRAM
%token <ID> T_ID
%token T_MAINCLASS T_PUBLIC T_STATIC T_VOID T_MAIN
%token T_PRINTLN T_ELSE T_FOR T_IF T_WHILE T_LEQ T_NE T_GE
%token T_NUM T_EQEQ T_INT T_FLOAT
%left '+' '-'
%left '*' '/'
%left T_UMINUS

%%                   /* beginning of rules section */

PROGRAM: T_MAINCLASS T_ID '{' T_PUBLIC T_STATIC T_VOID T_MAIN '(' ')' COMP_STMT '}';

COMP_STMT: '{' STMT_LIST '}';

STMT_LIST: STMT_LIST STMT 
| ;

STMT: ASSIGN_STMT 
| FOR_STMT 
| WHILE_STMT
| IF_STMT
| COMP_STMT
| DECLARATION
| NULL_STMT
| T_PRINTLN '(' EXPR ')' ';';

DECLARATION: TYPE ID_LIST;

TYPE: T_INT
| T_FLOAT;

ID_LIST: T_ID ',' ID_LIST
| T_ID;

NULL_STMT: ';';

ASSIGN_STMT: ASSIGN_EXPR ';';

ASSIGN_EXPR: T_ID '=' EXPR;

EXPR: ASSIGN_EXPR
| RVAL;

FOR_STMT: T_FOR '(' OPASSIGN_EXPR ';' OPBOOL_EXPR ';' OPASSIGN_EXPR ')' STMT;

OPASSIGN_EXPR: ASSIGN_EXPR
| ;

OPBOOL_EXPR: BOOL_EXPR
| ;

WHILE_STMT: T_WHILE '(' BOOL_EXPR ')' STMT;

IF_STMT: T_IF '(' BOOL_EXPR ')' STMT ELSE_PART;

ELSE_PART: T_ELSE STMT
| ;

BOOL_EXPR: EXPR C_OP EXPR;

C_OP: T_EQEQ | '<' | '>' | T_LEQ | T_GE | T_NE;

RVAL: RVAL '+' TERM
| RVAL '-' TERM
| TERM;

TERM: TERM '*' FACTOR
| TERM '/' FACTOR
| FACTOR;

FACTOR: '(' EXPR ')'
| T_UMINUS FACTOR
| T_ID
| T_NUM;

%%

void yyerror(const char *msg)
{
  fprintf(stderr, "McChris language error: %s\n", msg);
  exit(1);
}

int yywrap(void) {
    return 1;
}

int main ()
{
  return yyparse();
}