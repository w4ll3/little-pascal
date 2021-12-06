%option noyywrap yylineno
%{
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "ast.h"
#include "utils.h"

#include "parser.hpp"
#include "ccalc.h"

#define DEBUG 1

#define YY_INPUT(buf,result,max_size)  {\
    result = GetNextChar(buf, max_size); \
    if (  result <= 0  ) \
      result = YY_NULL; \
    }
#define YYLVAL_DEBUG yylval.debug = strdup(yytext); 
extern YYLTYPE yylloc;
int yycolumn = 1;
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;\
	yylloc.first_column = yycolumn; yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng; 
    
%}

comment "//".*
mult_comm [/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]

%%
{comment}   { }
{mult_comm} { }
"="      { BeginToken(yytext);YYLVAL_DEBUG return EQUAL; }
"<"      { BeginToken(yytext);YYLVAL_DEBUG return LTHAN; }
"<="     { BeginToken(yytext);YYLVAL_DEBUG return LEQU; }
">"      { BeginToken(yytext);YYLVAL_DEBUG return GT; }
">="     { BeginToken(yytext);YYLVAL_DEBUG return GE; }
"<>"     { BeginToken(yytext);YYLVAL_DEBUG return UNEQUAL; }
"+"      { BeginToken(yytext);YYLVAL_DEBUG return PLUS; }
"-"      { BeginToken(yytext);YYLVAL_DEBUG return MINUS; }
"*"      { BeginToken(yytext);YYLVAL_DEBUG return MUL; }
"/"|"div"      { BeginToken(yytext);YYLVAL_DEBUG return DIV; }		
")"      { BeginToken(yytext);YYLVAL_DEBUG return RIGHTP; }
"("      { BeginToken(yytext);YYLVAL_DEBUG return LEFTP; }
":="     { BeginToken(yytext);YYLVAL_DEBUG return ASSIGN; }
";"      { BeginToken(yytext);YYLVAL_DEBUG return SEMI; }
","		 { BeginToken(yytext);YYLVAL_DEBUG return COMMA; }
":"		 { BeginToken(yytext);YYLVAL_DEBUG return COLON; }
"begin"  { BeginToken(yytext);YYLVAL_DEBUG return BEGINN; }
"end"	 { BeginToken(yytext);YYLVAL_DEBUG return END; }
"and"    { BeginToken(yytext);YYLVAL_DEBUG return AND; }
"or"     { BeginToken(yytext);YYLVAL_DEBUG return OR; }
"not"	 { BeginToken(yytext);YYLVAL_DEBUG return NOT; }
"read"   { BeginToken(yytext);YYLVAL_DEBUG return READ; }
"if"     { BeginToken(yytext);YYLVAL_DEBUG return IF; }
"then"   { BeginToken(yytext);YYLVAL_DEBUG return THEN; }
"else"   { BeginToken(yytext);YYLVAL_DEBUG return ELSE; }
"while"  { BeginToken(yytext);YYLVAL_DEBUG return WHILE; }
"do"     { BeginToken(yytext);YYLVAL_DEBUG return DO; }
"program" { BeginToken(yytext);YYLVAL_DEBUG return PROGRAM; }
"function" { BeginToken(yytext);YYLVAL_DEBUG return FUNCTION; }
"procedure" { BeginToken(yytext);YYLVAL_DEBUG return PROCEDURE;}
"var"    { BeginToken(yytext);YYLVAL_DEBUG return VAR;}
[0-9]+   { BeginToken(yytext);YYLVAL_DEBUG  return INTEGER; }
"."      { BeginToken(yytext);YYLVAL_DEBUG return DOT; }
"write" {BeginToken(yytext);YYLVAL_DEBUG return WRITE; }
"false"|"true"   { BeginToken(yytext);YYLVAL_DEBUG return SYS_BOOL; }
"abs"|"chr"|"ord"|"pred"|"sqr"|"succ"    { BeginToken(yytext);YYLVAL_DEBUG return SYS_FUNCT; }
"boolean"|"integer"    {BeginToken(yytext);YYLVAL_DEBUG return TYPE;}
([_a-zA-Z])([_a-zA-Z0-9])* { BeginToken(yytext);yylval.debug = strdup(yytext);return IDD; }

[ \n\t]+ {BeginToken(yytext);}
.		{ printf("Invalid character\n");} 

%%
