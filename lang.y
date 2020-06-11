%{
#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"

extern int yylex(void);
extern int yyparse(void);
%}

%union {
  int INT; float FLOAT; char STR[32]; node *NODE;
}

%type <NODE> PROGRAM COMP_STMT STMT_LIST STMT DECLARATION TYPE
%type <NODE> ID_LIST NULL_STMT ASSIGN_STMT ASSIGN_EXPR EXPR
%type <NODE> FOR_STMT OPASSIGN_EXPR OPBOOL_EXPR WHILE_STMT
%type <NODE> IF_STMT ELSE_PART BOOL_EXPR C_OP RVAL FACTOR TERM

%start PROGRAM
%token <STR> T_ID T_NUM
%token <INT> T_MAINCLASS T_PUBLIC T_STATIC T_VOID T_MAIN
%token <INT> T_INT T_FLOAT T_PRINTLN T_ELSE T_FOR T_IF T_WHILE
%token <INT> '(' ')' '{' '}' ';' ','
%right <INT> '='
%left <INT> T_EQEQ T_LEQ T_NE T_GE '>' '<'
%left <INT> '+' '-'
%left <INT> '*' '/'
%left <INT> T_UMINUS

%%                   /* beginning of rules section */

PROGRAM: T_MAINCLASS T_ID '{' T_PUBLIC T_STATIC T_VOID T_MAIN '(' ')' COMP_STMT '}'
  { root = create_node(PROGRAM, create_str_node($2), $10, NULL, NULL); }
;

COMP_STMT: '{' STMT_LIST '}'
  { $$ = create_node(COMP_STMT, $2, NULL, NULL, NULL); }
;

STMT_LIST: STMT_LIST STMT  { $$ = create_node(STMT_LIST, $1, $2, NULL, NULL); }
|  { $$ = NULL; };

STMT: ASSIGN_STMT  { $$ = $1; }
| FOR_STMT  { $$ = $1; }
| WHILE_STMT  { $$ = $1; }
| IF_STMT  { $$ = $1; }
| COMP_STMT  { $$ = $1; }
| DECLARATION  { $$ = $1; }
| NULL_STMT  { $$ = $1; }
| T_PRINTLN '(' EXPR ')' ';'  { $$ = create_node(T_PRINTLN, $3, NULL, NULL, NULL); }
;

DECLARATION: TYPE ID_LIST { $$ = create_node(DECLARATION, $1, $2, NULL, NULL); };

TYPE: T_INT { $$ = create_node(T_INT, NULL, NULL, NULL, NULL); }
| T_FLOAT { $$ = create_node(T_FLOAT, NULL, NULL, NULL, NULL); };

ID_LIST: T_ID ',' ID_LIST { $$ = create_node(ID_LIST, create_str_node($1), $3, NULL, NULL); }
| T_ID  { $$ = create_str_node($1); };

NULL_STMT: ';'  { $$ = NULL; };

ASSIGN_STMT: ASSIGN_EXPR ';'  { $$ = $1; };

ASSIGN_EXPR: T_ID '=' EXPR  { $$ = create_node(ASSIGN_EXPR, create_str_node($1), $3, NULL, NULL); };

EXPR: ASSIGN_EXPR { $$ = $1; }
| RVAL  { $$ = $1; };

FOR_STMT: T_FOR '(' OPASSIGN_EXPR ';' OPBOOL_EXPR ';' OPASSIGN_EXPR ')' STMT  { $$ = create_node(FOR_STMT, $3, $5, $7, $9); };

OPASSIGN_EXPR: ASSIGN_EXPR  { $$ = $1; }
| { $$ = NULL; };

OPBOOL_EXPR: BOOL_EXPR  { $$ = $1; }
| { $$ = NULL; };

WHILE_STMT: T_WHILE '(' BOOL_EXPR ')' STMT  { $$ = create_node(WHILE_STMT, $3, $5, NULL, NULL); };

IF_STMT: T_IF '(' BOOL_EXPR ')' STMT ELSE_PART  { $$ = create_node(IF_STMT, $3, $5, $6, NULL); };

ELSE_PART: T_ELSE STMT  { $$ = $2; }
| { $$ = NULL; };

BOOL_EXPR: EXPR C_OP EXPR { $$ = create_node(BOOL_EXPR, $1, $2, $3, NULL); };

C_OP: T_EQEQ  { $$ = create_node($1, NULL, NULL, NULL, NULL); }
| '<'  { $$ = create_node($1, NULL, NULL, NULL, NULL); }
| '>'  { $$ = create_node($1, NULL, NULL, NULL, NULL); }
| T_LEQ  { $$ = create_node($1, NULL, NULL, NULL, NULL); }
| T_GE  { $$ = create_node($1, NULL, NULL, NULL, NULL); }
| T_NE  { $$ = create_node($1, NULL, NULL, NULL, NULL); };

RVAL: RVAL '+' TERM { $$ = create_node($2, $1, $3, NULL, NULL); }
| RVAL '-' TERM { $$ = create_node($2, $1, $3, NULL, NULL); }
| TERM  { $$ = $1; };

TERM: TERM '*' FACTOR { $$ = create_node($2, $1, $3, NULL, NULL); }
| TERM '/' FACTOR { $$ = create_node($2, $1, $3, NULL, NULL); }
| FACTOR  { $$ = $1; };

FACTOR: '(' EXPR ')'  { $$ = $2; }
| T_UMINUS FACTOR { $$ = create_node(T_UMINUS, $2, NULL, NULL, NULL); }
| T_ID   { $$ = create_str_node($1); }
| T_NUM { $$ = create_num_node($1); };

%%
