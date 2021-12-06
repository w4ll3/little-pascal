#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <map>
#include <vector>

#include "ccalc.h"
#include "parser.hpp"
int parseError = 0;
void yyerror(char* s, ...) {
  PrintError(s);
  parseError = 1;
  // va_list ap;
  // static char errmsg[10000];
  // va_list args;
  // int start=nTokenStart;
  // int end=start + nTokenLength - 1;
  // int i;
  //  fprintf(stdout, "...... !");
  //  for (int i=0; i<nBuffer; i++)
  //    fprintf(stdout, ".");
  //  fprintf(stdout, "^\n");
  //  va_start(args, errorstring);
  // vsprintf(errmsg, errorstring, args);
  // va_end(args);
  // fprintf(stdout, "Error: %s\n", errmsg);
  // va_start(ap, s);
  // vfprintf(stdout, s, ap);
  // fprintf(stdout, "At line %d: %s\n", yylineno, yytext);
  // fprintf(stdout, "\n");
}

int count = 0;

const char* tailStr = "└── ";
const char* branchStr = "├── ";
const char* ch_tailStr = "    ";
const char* ch_branchStr = "|   ";

void ast_print(ast_node* node, char* prefix, int tail) {
  char* new_prefix = (char*)malloc(sizeof(char) * (strlen(prefix) + 4 + 1));
  strcpy(new_prefix, prefix);
  if (tail)
    strcat(new_prefix, tailStr);
  else
    strcat(new_prefix, branchStr);
  printf("%s %s\n", new_prefix, node->debug);
  char* ch_prefix = (char*)malloc(sizeof(char) * (strlen(prefix) + 4 + 1));
  strcpy(ch_prefix, prefix);
  if (tail)
    strcat(ch_prefix, ch_tailStr);
  else
    strcat(ch_prefix, ch_branchStr);
  ast_node** ch;
  for (ch = node->ch; *ch; ch++) {
    if ((*(ch + 1)) == NULL)
      ast_print(*ch, ch_prefix, 1);
    else
      ast_print(*ch, ch_prefix, 0);
  }
}

void ast_travel(ast_node* node) {
  char* ch_prefix = (char*)malloc(sizeof(char) * (4 + 1));
  strcpy(ch_prefix, "");
  strcat(ch_prefix, ch_tailStr);
  printf("%s\n", node->debug);
  ast_node** ch;
  for (ch = node->ch; *ch; ch++) {
    if ((*(ch + 1)) == NULL)
      ast_print(*ch, ch_prefix, 1);
    else
      ast_print(*ch, ch_prefix, 0);
  }
}

ast_node* ast_dbg(char* debug) {
  // printf("new %s\n",debug);
  int len = strlen(debug);
  ast_node* node = (ast_node*)calloc(1, sizeof(ast_node));
  node->debug = (char*)calloc(len + 3, sizeof(char));
  node->debug[0] = '\"';
  strncpy(node->debug + 1, debug, len);
  node->debug[1 + len] = '\"';
  node->debug[2 + len] = '\0';
  // node->debug = debug;
  node->ch = (ast_node**)calloc(1, sizeof(ast_node*));
  return node;
}

ast_node* ast_newNode1(ast_node* c1) {
  ast_node* node = (ast_node*)calloc(1, sizeof(ast_node));

  node->ch = (ast_node**)calloc(2, sizeof(ast_node*));
  node->ch[0] = c1;
  return node;
}

ast_node* ast_newNode2(ast_node* c1, ast_node* c2) {
  ast_node* node = (ast_node*)calloc(1, sizeof(ast_node));

  node->ch = (ast_node**)calloc(3, sizeof(ast_node*));
  node->ch[0] = c1;
  node->ch[1] = c2;
  return node;
}

ast_node* ast_newNode3(ast_node* c1, ast_node* c2, ast_node* c3) {
  ast_node* node = (ast_node*)calloc(1, sizeof(ast_node));

  node->ch = (ast_node**)calloc(4, sizeof(ast_node*));
  node->ch[0] = c1;
  node->ch[1] = c2;
  node->ch[2] = c3;
  return node;
}

ast_node* ast_newNode4(ast_node* c1, ast_node* c2, ast_node* c3, ast_node* c4) {
  ast_node* node = (ast_node*)calloc(1, sizeof(ast_node));

  node->ch = (ast_node**)calloc(5, sizeof(ast_node*));
  node->ch[0] = c1;
  node->ch[1] = c2;
  node->ch[2] = c3;
  node->ch[3] = c4;
  return node;
}

ast_node* ast_newNode5(ast_node* c1, ast_node* c2, ast_node* c3, ast_node* c4,
                       ast_node* c5) {
  ast_node* node = (ast_node*)calloc(1, sizeof(ast_node));

  node->ch = (ast_node**)calloc(6, sizeof(ast_node*));
  node->ch[0] = c1;
  node->ch[1] = c2;
  node->ch[2] = c3;
  node->ch[3] = c4;
  node->ch[4] = c5;
  return node;
}

ast_node* ast_newNode6(ast_node* c1, ast_node* c2, ast_node* c3, ast_node* c4,
                       ast_node* c5, ast_node* c6) {
  ast_node* node = (ast_node*)calloc(1, sizeof(ast_node));

  node->ch = (ast_node**)calloc(7, sizeof(ast_node*));
  node->ch[0] = c1;
  node->ch[1] = c2;
  node->ch[2] = c3;
  node->ch[3] = c4;
  node->ch[4] = c5;
  node->ch[5] = c6;
  return node;
}

ast_node* ast_newNode7(ast_node* c1, ast_node* c2, ast_node* c3, ast_node* c4,
                       ast_node* c5, ast_node* c6, ast_node* c7) {
  ast_node* node = (ast_node*)calloc(1, sizeof(ast_node));

  node->ch = (ast_node**)calloc(8, sizeof(ast_node*));
  node->ch[0] = c1;
  node->ch[1] = c2;
  node->ch[2] = c3;
  node->ch[3] = c4;
  node->ch[4] = c5;
  node->ch[5] = c6;
  node->ch[6] = c7;
  return node;
}

ast_node* ast_newNode8(ast_node* c1, ast_node* c2, ast_node* c3, ast_node* c4,
                       ast_node* c5, ast_node* c6, ast_node* c7, ast_node* c8) {
  ast_node* node = (ast_node*)calloc(1, sizeof(ast_node));

  node->ch = (ast_node**)calloc(9, sizeof(ast_node*));
  node->ch[0] = c1;
  node->ch[1] = c2;
  node->ch[2] = c3;
  node->ch[3] = c4;
  node->ch[4] = c5;
  node->ch[5] = c6;
  node->ch[6] = c7;
  node->ch[7] = c8;
  return node;
}

std::string implode_list(std::vector<std::string> vec, std::string delimiter) {
  if (vec.size() == 0) return "";
  if (vec.size() == 1) return vec.front();
  std::string result = vec.front();
  for (auto i = 1; i < vec.size() - 1; i++) result += delimiter + vec[i];
  result += delimiter + vec.back();
  return result;
}

std::string implode_list_around(std::string prefix, std::string suffix,
                                std::vector<std::string> vec,
                                std::string delimiter) {
  if (vec.size() == 0) return prefix + suffix;
  if (vec.size() == 1) return prefix + vec.front() + suffix;
  std::string result = prefix + vec.front();
  for (auto i = 1; i < vec.size() - 1; i++) result += delimiter + vec[i];
  result += delimiter + vec.back() + suffix;
  return result;
}

std::string random_string(std::size_t length) {
  const std::string CHARACTERS =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  std::random_device random_device;
  std::mt19937 generator(random_device());
  std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

  std::string random_string;

  for (std::size_t i = 0; i < length; ++i) {
    random_string += CHARACTERS[distribution(generator)];
  }

  return random_string;
}

LookupEntry LPEntry(std::string type, int address, int scope,
                    std::string sys_type) {
  LookupEntry lp;
  lp.type = type;
  lp.address = address;
  lp.scope = scope;
  lp.sys_type = sys_type;
  return lp;
}

void red(const std::string& str) {
  std::string red_b = "\033[1;31m";
  std::string red_e = "\033[0m";
  std::cout << red_b + str + red_e << std::endl;
}

void green(const std::string& str) {
  std::string red_b = "\033[1;32m";
  std::string red_e = "\033[0m";
  std::cout << red_b + str + red_e << std::endl;
}

LookupEntry Lookup(CodeGenContext& context, std::string name) {
  LookupTable::iterator match;
  LookupEntry entry;
  std::pair<LookupTable::iterator, LookupTable::iterator> el =
      context.lookup_table.equal_range(name);
  for (LookupTable::iterator it = el.first; it != el.second; it++) {
    match = context.lookup_table.find(it->first);
    if (match != context.lookup_table.end()) {
      entry = match.operator*().second;
    }
  }
  return entry;
}

void CodeGenContext::generateCode(ast::Program& root) {
  std::cout << "Generating code...\n";

  /* Push a new variable/block context */
  auto code = root.CodeGen(*this); /* emit bytecode for the toplevel block */

  /* Print the bytecode in a human-readable format
     to see if our program compiled properly
   */
  std::cout << "code is gen~~~\n";
  for (auto ln : code) std::cout << ln << std::endl;
  std::cout << "code is gen~!~\n";
}
