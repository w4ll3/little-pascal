#ifndef __PASCAL_H__
#define __PASCAL_H__

#include <cstring>
#include <iostream>
#include <map>
#include <random>
#include <stack>
#include <typeinfo>
#include <vector>

#include "ast.h"
class CodeGenBlock {
 public:
  std::string returnValue;
  CodeGenBlock* parent;
};

typedef struct YYLTYPE {
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  char* filename;
} YYLTYPE;

struct LookupEntry {
  std::string type = "";
  int address = -1;
  int scope = 0;
  std::string sys_type = "";
};

typedef std::multimap<std::string, LookupEntry> LookupTable;

class CodeGenContext {
 public:
  LookupTable lookup_table;
  bool error = false;
  int scope = 0;
  int addr = 0;

 public:
  void generateCode(ast::Program& root);
};

#define YYLTYPE_is_declared

extern int yylex();
// extern int yyparse();
extern int yylineno;
extern char* yytext;

void yyerror(char* s, ...);

typedef struct ast_struct ast_tree;
typedef ast_tree ast_node;

struct ast_struct {
  ast_node** ch;  // children
  char* debug;    // debug info
  int val;
};

#define NDEBUG

#ifdef NDEBUG
#define DEBUG_TEST_COND 0
#else
#define DEBUG_TEST_COND 1
#endif

/* get only filename */
#define __FILENAME__ \
  (std::strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : __FILE__)

/* show more precise infomartion */
#ifdef DBINFO_POS
#define __DB_POS__ "[" << __FILENAME__ << ":" << __LINE__ << "] " <<
#else
#define __DB_POS__ "" <<
#endif

/* define print macros */

#define DBVAR(var)                                                \
  do {                                                            \
    if (DEBUG_TEST_COND)                                          \
      std::cout << "[DEBUG] " << __DB_POS__ #var << "\t" << (var) \
                << std::endl;                                     \
  } while (0)

#define DBMSG(msg)                                            \
  do {                                                        \
    if (DEBUG_TEST_COND)                                      \
      std::cout << "[DEBUG] " << __DB_POS__ msg << std::endl; \
  } while (0)

#define LOG(msg)                                                               \
  do {                                                                         \
    if (DEBUG_TEST_COND) std::cout << "[LOG] " << __DB_POS__ msg << std::endl; \
  } while (0)

void ast_travel(ast_node* node);
ast_node* ast_dbg(char* debug);
ast_node* ast_newNode1(ast_node* c1);
ast_node* ast_newNode2(ast_node* c1, ast_node* c2);
ast_node* ast_newNode3(ast_node* c1, ast_node* c2, ast_node* c3);
ast_node* ast_newNode4(ast_node* c1, ast_node* c2, ast_node* c3, ast_node* c4);
ast_node* ast_newNode5(ast_node* c1, ast_node* c2, ast_node* c3, ast_node* c4,
                       ast_node* c5);
ast_node* ast_newNode6(ast_node* c1, ast_node* c2, ast_node* c3, ast_node* c4,
                       ast_node* c5, ast_node* c6);
ast_node* ast_newNode7(ast_node* c1, ast_node* c2, ast_node* c3, ast_node* c4,
                       ast_node* c5, ast_node* c6, ast_node* c7);
ast_node* ast_newNode8(ast_node* c1, ast_node* c2, ast_node* c3, ast_node* c4,
                       ast_node* c5, ast_node* c6, ast_node* c7, ast_node* c8);
std::string implode_list(std::vector<std::string> vec, std::string delimiter);
std::string implode_list_around(std::string prefix, std::string suffix,
                                std::vector<std::string> vec,
                                std::string delimiter);
std::string random_string(std::size_t length);
void red(const std::string& str);
void green(const std::string& str);

LookupEntry LPEntry(std::string type, int address, int scope,
                    std::string sys_type);

LookupEntry Lookup(CodeGenContext& context, std::string type);
#endif
