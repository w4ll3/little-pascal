%{
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "ast.h"
#include "utils.h"
#include "parser.hpp"
#include "ccalc.h"
using namespace std;

int yydebug = 1;
ast::Node* ast_root;
%}
%union{
  char*           debug;

    ast::Param*            ast_Param;
    ast::Statement*        ast_Statement;
    ast::Expression*       ast_Expression;
    ast::Program*          ast_Program;
    ast::Routine*          ast_Routine;
    ast::Type*             ast_Type;
    ast::AssignmentStmt*   ast_AssignmentStmt;
    ast::Const*            ast_Const;

    ast::Statements*       ast_Statements;
    ast::Variables*        ast_Variables;
    ast::Parameters*       ast_Parameters;
    ast::Routines*         ast_Routines;
    ast::NameList*         ast_NameList;
    ast::ExpressionList*   ast_ExpressionList;
}

%token PROGRAM IDD DOT EQUAL LTHAN LEQU GT GE PLUS MINUS MUL DIV RIGHTP LEFTP 
%token ASSIGN COLON COMMA SEMI OR AND READ UNEQUAL 
%token IF THEN ELSE WHILE DO BEGINN END INTEGER NOT 
%token SYS_FUNCT WRITE VAR PROCEDURE FUNCTION TYPE SYS_BOOL

%start program
%type <debug> NAME IDD TYPE WRITE READ SYS_BOOL INTEGER

%type <ast_Program>        program program_head routines routine_head sub_routine
%type <ast_Type>           type
%type <ast_Statement>      proc_stmt stmt else_clause while_stmt if_stmt
%type <ast_AssignmentStmt> assign_stmt 
%type <ast_Expression>     expression expr term factor
%type <ast_Routine>        function_decl function_head procedure_head procedure_decl
%type <ast_Const>          const_value

%type <ast_Parameters>     parameters para_decl_list para_type_list
%type <ast_Routines>       routine_part 
%type <ast_Statements>     routine_body compound_stmt stmt_list 
%type <ast_Variables>      var_part var_decl_list var_decl
%type <ast_NameList>       name_list
%type <ast_ExpressionList> expression_list

%%

NAME: IDD                                 { $$ = $1;                           }
;

program: program_head routines DOT        { ast_root = $2;                     }
;

program_head:
  PROGRAM IDD SEMI                        {                                    }
  |                                       {                                    }
;

routines:
  routine_head routine_body               { $$ = $1; $$->routine_body = $2;    }
;

sub_routine:
  routine_head routine_body               { $$ = $1; $$->routine_body = $2;    }
;

routine_head:
  var_part routine_part                   { $$ = new ast::Program($1, $2, nullptr); }
;

type: TYPE                                { $$ = new ast::Type($1);            }
;

const_value:
  INTEGER                                 { $$ = new ast::Integer(atoi($1));
                                            $$->debug = $1;
                                          }
  | SYS_BOOL                              { $$ = new ast::Boolean($1);         }
;

name_list: // ast_NameList
  name_list COMMA IDD                     { $$ = $1; $$->push_back($3);        }
  | IDD                                   { $$ = new ast::NameList();
                                            $$->push_back($1);
                                          }
;

var_part:
  VAR var_decl_list                       { $$ = $2;                           }
  |                                       { $$ = new ast::Variables();         }
;

var_decl_list:
  var_decl_list var_decl                  { $$ = $1;
                                            $1->insert($1->end(),
                                                       $2->begin(),
                                                       $2->end()
                                            );
                                          }
  | var_decl                              { $$ = $1;                           }
;

var_decl:
  name_list COLON type SEMI               { $$ = new ast::Variables();
                                            for(auto name : *($1)) {
                                              $$->push_back(new ast::Var(
                                                              new ast::Identifier(name),
                                                              new ast::Type(*($3))
                                                            )
                                              );
                                            }
                                          }
;

routine_part:
  routine_part function_decl              { $$ = $1; $1->push_back($2);        }
  | routine_part procedure_decl           { $$ = $1; $1->push_back($2);        }
  |                                       { $$ = new ast::Routines();          }
;

function_decl:
  function_head SEMI sub_routine SEMI     { $$ = new ast::Routine($1, $3);     }
;

function_head:
  FUNCTION IDD parameters COLON type      { $$ = new ast::Routine(
                                                  ast::Routine::RoutineType::function,
                                                  new ast::Identifier($2),
                                                  $3,
                                                  $5
                                                );
                                          }
;

procedure_decl:
  procedure_head SEMI sub_routine SEMI    { $$ = new ast::Routine($1, $3); }
;

procedure_head:
  PROCEDURE IDD parameters                { $$ = new ast::Routine(
                                                  ast::Routine::RoutineType::procedure,
                                                  new ast::Identifier($2),
                                                  $3,
                                                  nullptr
                                                );
                                          }
;

parameters:
  LEFTP para_decl_list RIGHTP             { $$ = $2; }
  | LEFTP RIGHTP                          { $$ = new ast::Parameters(); }
  |                                       { $$ = new ast::Parameters(); }
;

para_decl_list:
  para_decl_list SEMI para_type_list      { $$ = $1;
                                            $1->insert($1->end(), $3->begin(),
                                                       $3->end());
                                          }
  | para_type_list                        { $$ = $1; }
;

para_type_list: //TODO: var is different
  VAR name_list COLON type                { $$ = new ast::Parameters();
                                            for(auto name: *($2)) {
                                              $$->push_back(
                                                  new ast::Param(name, $4)
                                              );
                                            }
                                          }
  | name_list COLON type                  { $$ = new ast::Parameters();
                                            for(auto name: *($1)) {
                                              $$->push_back(
                                                  new ast::Param(name, $3)
                                              );
                                            }
                                          }
;

routine_body:  
  compound_stmt                           { $$ = $1;                           }
;

compound_stmt : 
  BEGINN stmt_list END                    { $$ = $2;                           }
;

stmt_list : 
  stmt_list stmt SEMI                     { $$ = $1;
                                            $$->getlist()->push_back($2);
                                          }
  |                                       { $$ = new ast::Statements();        }
;

stmt : 
  assign_stmt                             { $$ = (ast::Statement *) $1;        }
  | proc_stmt                             { $$ = (ast::Statement *) $1;        }  
  | compound_stmt                         { $$ = $1;                           }
  | if_stmt                               { $$ = (ast::Statement *) $1;        }    
  | while_stmt                            { $$ = (ast::Statement *) $1;        }  
;

assign_stmt : 
  IDD ASSIGN expression                   { $$ = new ast::AssignmentStmt(
                                                  new ast::Identifier($1), $3
                                                );
                                          }
;

proc_stmt : 
  IDD LEFTP RIGHTP                        { $$ = new ast::ProcCall(
                                                  new ast::Identifier($1)
                                                );
                                          }
  | IDD LEFTP expression_list RIGHTP    { $$ = new ast::ProcCall(
                                                  new ast::Identifier($1), $3
                                                );
                                          }
  | WRITE LEFTP expression_list RIGHTP    { $$ = new ast::Write(
                                                  new ast::Identifier($1), $3
                                                );
                                          }
  | READ  LEFTP name_list RIGHTP          { std::vector<ast::Expression*> params;
                                            for (auto e: *($3)) {
                                              params.push_back(new ast::Attr(e));
                                            }
                                            $$ = new ast::Read(
                                                  new ast::Identifier($1),
                                                  &params
                                                );
                                          }

if_stmt : 
  IF expression THEN stmt else_clause     { $$ = (ast::Statement*)
                                                      new ast::IfStmt($2,$4,$5);
                                          }
;

else_clause : 
  ELSE stmt                               { $$ = $2;                           }
  |                                       { $$ = nullptr;                      }
;

while_stmt : 
  WHILE  expression  DO stmt              { $$ = (ast::Statement*)
                                                      new ast::WhileStmt($2,$4);
                                          }
;

expression_list: 
  expression_list COMMA expression        { $$ = $1; $1->push_back($3);        }
  | expression                            { $$ = new ast::ExpressionList();
                                            $$->push_back($1);
                                          }

expression:
  expr                                    { $$ = $1;                           }
  | expression  GE  expr                  { $$ = new ast::Operator($1,
                                                  ast::Operator::OpType::ge,
                                                  $3
                                                );
                                          }
  | expression  GT  expr                  { $$ = new ast::Operator($1,
                                                  ast::Operator::OpType::gt,
                                                  $3
                                                );
                                          }
  | expression  LEQU  expr                { $$ = new ast::Operator($1,
                                                  ast::Operator::OpType::le,
                                                  $3
                                                );
                                          }
  | expression  LTHAN  expr               { $$ = new ast::Operator($1,
                                                  ast::Operator::OpType::lt, $3
                                                );
                                          }
  | expression  EQUAL  expr               { $$ = new ast::Operator($1,
                                                  ast::Operator::OpType::eq, $3
                                                );
                                          }                 
  | expression  UNEQUAL  expr             { $$ = new ast::Operator($1,
                                                  ast::Operator::OpType::ne, $3
                                                );
                                          }  
;

expr: 
  expr PLUS term                          { $$ = new ast::Operator($1,
                                                  ast::Operator::OpType::plus,
                                                  $3
                                                );
                                          }
  | expr  MINUS  term                     { $$ = new ast::Operator($1,
                                                  ast::Operator::OpType::minus,
                                                  $3
                                                );
                                          }  
  | expr  OR  term                        { $$ = new ast::Operator($1,
                                                  ast::Operator::OpType::bit_or,
                                                  $3
                                                );
                                          }
  | term { $$ = $1; }
;

term: 
  factor                                  { $$ = $1;                           }
  | term  MUL  factor                     { $$ = new ast::Operator($1,
                                                  ast::Operator::OpType::mul,
                                                  $3
                                                );
                                          }
  | term  DIV  factor                     { $$ = new ast::Operator($1,
                                                  ast::Operator::OpType::div,
                                                  $3
                                                );
                                          }
  | term  AND  factor                     { $$ = new ast::Operator($1,
                                                  ast::Operator::OpType::bit_and,
                                                  $3
                                                );
                                          }
;

factor: 
  NAME                                    { $$ = new ast::Identifier($1);      }
  | const_value                           { $$ = $1;                           }
  | NAME LEFTP expression_list RIGHTP     { $$ = new ast::FuncCall(
                                                  new ast::Identifier($1), $3
                                                );
                                          }
  | LEFTP  expression  RIGHTP             { $$ = $2;                           }
  | MINUS  factor                         { $$ = new ast::Operator(
                                                  new ast::Integer(0),
                                                  ast::Operator::OpType::minus,
                                                  &(*($2))
                                                );
                                          }
;
%%


