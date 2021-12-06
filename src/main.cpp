#include <iostream>
#include <stdexcept>

#include "ast.h"
#include "ccalc.h"
#include "parser.hpp"
#include "utils.h"

using namespace std;

extern int yyparse();
extern ast::Program* ast_root;

int main(int argc, char** argv) {
  init_buffer();
  yyparse();
  extern int parseError;
  if (parseError) return 0;
  green("syntax check success!");
  cout << ast_root << endl;
  ast_root->print_node("", true, true);
  CodeGenContext context;
  try {
    context.generateCode(*ast_root);
    if (context.error) {
      for (auto e : context.lookup_table)
        std::cout << "{ " << e.first << ", " << std::to_string(e.second.address)
                  << ", " << std::to_string(e.second.scope) << ", "
                  << e.second.sys_type << ", " << e.second.type << " }"
                  << std::endl;
      throw std::domain_error("Program failed due to the above errors.");
    }
  } catch (const std::domain_error& de) {
    red(de.what());
  } catch (const std::logic_error& le) {
    red(le.what());
  } catch (...) {
    cout << "other uncaught error" << endl;
  }
  return 0;
}
