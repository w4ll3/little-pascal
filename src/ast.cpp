#include "ast.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

#include "utils.h"
using namespace std;

// Code Gen section
std::vector<std::string> ast::Identifier::CodeGen(CodeGenContext& context) {
  std::vector<std::string> code;
  auto entry = Lookup(context, name);

  if (entry.address == -1) {
    red("Identifier '" + name + "' is not defined.");
    context.error = true;
    return code;
  }

  std::cout << "Loading identifier: " << name
            << ", addr: " << std::to_string(entry.address) << std::endl;
  code.push_back("CRVL " + std::to_string(entry.address));
  return code;
}

std::vector<std::string> ast::Attr::CodeGen(CodeGenContext& context) {
  std::vector<std::string> code;
  auto entry = Lookup(context, name);

  if (entry.address == -1) {
    red("Identifier '" + name + "' is not defined.");
    context.error = true;
    return code;
  }

  std::cout << "Save to identifier: " << name << std::endl;
  code.push_back("ARMZ " + std::to_string(entry.address));
  return code;
}

std::vector<std::string> ast::Param::CodeGen(CodeGenContext& context) {
  std::cout << "Creating param: " << name << std::endl;
  std::vector<std::string> code;
  auto entry = Lookup(context, name);

  if (entry.address == -1) {
    red("Identifier '" + name + "' is not defined.");
    context.error = true;
    return code;
  }
  std::cout << "Creating param: " << name << std::endl;
  return code;
}

std::vector<std::string> ast::Integer::CodeGen(CodeGenContext& context) {
  std::vector<std::string> code;
  std::cout << "Creating integer: " << val << std::endl;
  code.push_back("CRCT " + std::to_string(val));
  return code;
}

std::vector<std::string> ast::Boolean::CodeGen(CodeGenContext& context) {
  std::vector<std::string> code;
  std::cout << "Creating boolean: " << (val ? "true" : "false") << std::endl;
  code.push_back("CRCT " + std::to_string(val));
  return code;
}

std::vector<std::string> ast::Routine::CodeGen(CodeGenContext& context) {
  // Addr = number of params
  context.scope++;
  std::vector<std::string> code;

  auto entry = Lookup(context, routine_name->toString());

  if (entry.address != -1) {
    red("Routine '" + routine_name->toString() + "' is already defined.");
    context.error = true;
    return code;
  }

  std::cout << "------BEGIN " << (isFunction() ? "FUNCTION " : "PROCEDURE ")
            << routine_name->toString() << " Scope("
            << std::to_string(context.scope) << ")"
            << "--------" << std::endl;
  code.push_back(routine_name->toString() + ": ENPR " +
                 std::to_string(context.scope));

  int arg_addr = -4;
  int arg_c = 0;
  for (auto arg : *(argument_list)) {
    arg_c++;
    auto c = arg->CodeGen(context);
    for (auto el : c) {
      code.push_back(el);
    }
    context.lookup_table.emplace(
        arg->name, LPEntry("PF", arg_addr, context.scope, arg->type->raw_name));
    arg_addr--;
  }

  int var_num = 0;
  for (auto var_decl : *(var_part)) {
    var_num++;
    auto c = var_decl->CodeGen(context);
    for (auto el : c) {
      code.push_back(el);
    }
  }

  std::string ret;
  if (isProcedure()) {
    ret = "void";
  } else {
    ret = return_type->raw_name;
  }
  context.lookup_table.emplace(routine_name->toString(),
                               LPEntry("ROUTINE", arg_c, context.scope, ret));

  for (auto routine : *(routine_part)) {
    auto c = routine->CodeGen(context);
    for (auto el : c) {
      code.push_back(el);
    }
  }

  auto c = routine_body->CodeGen(context);
  for (auto el : c) {
    code.push_back(el);
  }
  for (auto var_decl : *(var_part)) {
    auto c = var_decl->CodeGenDmem(context);
    for (auto el : c) {
      code.push_back(el);
    }
  }

  context.scope--;

  code.push_back("RTPR " + std::to_string(context.scope) + ", " +
                 std::to_string(var_num));

  std::cout << "------END ROUTINE--------" << std::endl;
  for (auto i : code) std::cout << i << std::endl;

  return code;
}

std::vector<std::string> ast::FuncCall::CodeGen(CodeGenContext& context) {
  context.scope++;
  std::vector<std::string> code;
  std::cout << "------BEGGIN FUNCCALL--------" << std::endl;

  auto entry = Lookup(context, id->name);
  if (entry.type.empty()) {
    red("Function '" + id->name + "' is not defined.");
    context.error = true;
    return code;
  }

  if (this->argument_list->size() != entry.address) {
    red("Expected " + std::to_string(entry.address) + " parameters," + " got " +
        std::to_string(argument_list->size()) + ".");
    context.error = true;
    return code;
  }

  std::vector<std::string> args_ids;
  for (auto arg : *(this->argument_list)) {
    auto els = arg->CodeGen(context);
    args_ids.push_back(arg->toString());
    for (auto el : els) {
      code.push_back(el);
    }
  }

  code.push_back("CHPR " + this->id->name + ", " +
                 std::to_string(context.scope));

  std::cout << this->id->name << implode_list_around("(", ")", args_ids, ", ")
            << std::endl;
  std::cout << "------END FUNCCALL--------" << std::endl;
  context.scope--;
  return code;
}

std::vector<std::string> ast::ProcCall::CodeGen(CodeGenContext& context) {
  context.scope++;
  std::vector<std::string> code;
  std::cout << "------PROC FUNCCALL--------" << std::endl;

  auto entry = Lookup(context, id->name);
  if (entry.type.empty()) {
    red("Procedure '" + id->name + "' is not defined.");
    context.error = true;
    return code;
  }

  if (this->argument_list->size() < entry.address !=
      this->argument_list->size() > entry.address) {
    red("Expected " + std::to_string(entry.address) + " parameters," + " got " +
        std::to_string(argument_list->size()) + ".");
    context.error = true;
    return code;
  }

  std::vector<std::string> args_ids;
  for (auto arg : *(this->argument_list)) {
    auto els = arg->CodeGen(context);
    args_ids.push_back(arg->toString());
    for (auto el : els) {
      code.push_back(el);
    }
  }

  for (auto e : code) std::cout << e << std::endl;

  std::cout << this->id->name << implode_list_around("(", ")", args_ids, ", ")
            << std::endl;
  std::cout << "------END PROCCALL--------" << std::endl;
  context.scope--;
  return code;
}

std::vector<std::string> ast::Write::CodeGen(CodeGenContext& context) {
  std::vector<std::string> code;
  std::cout << "Write " << id->name << " Call" << std::endl;
  for (auto arg : *argument_list) {
    auto arg_val = arg->CodeGen(context);
    for (auto val : arg_val) {
      code.push_back(val);
    }
    code.push_back("IMPR");
  }

  return code;
}

std::vector<std::string> ast::Read::CodeGen(CodeGenContext& context) {
  std::vector<std::string> code;
  std::cout << "Read " << id->name << " Call" << std::endl;
  for (auto arg : *argument_list) {
    auto arg_val = arg->CodeGen(context);
    for (auto val : arg_val) {
      code.push_back(val);
    }
    code.push_back("IMPR");
  }

  for (auto i : code) std::cout << i << std::endl;

  return code;
}

std::vector<std::string> ast::Operator::CodeGen(CodeGenContext& context) {
  std::vector<std::string> code;
  for (auto inst : op1->CodeGen(context)) code.push_back(inst);
  for (auto inst : op2->CodeGen(context)) code.push_back(inst);

  switch (op) {
    case OpType::plus:
      code.push_back("SOMA");
      break;
    case OpType::minus:
      code.push_back("SUBT");
      break;
    case OpType::mul:
      code.push_back("MULT");
      break;
    case OpType::div:
      code.push_back("DIVI");
      break;
    case OpType::bit_and: {
      code.push_back("DISJ");
      break;
    }
    case OpType::bit_or: {
      code.push_back("CONJ");
      break;
    }
    case OpType::eq: {
      code.push_back("CMIG");
      break;
    }
    case OpType::ne: {
      code.push_back("CMIG");
      code.push_back("NEGA");
      break;
    }
    case OpType::lt: {
      code.push_back("CMMA");
      code.push_back("NEGA");
      break;
    }
    case OpType::gt: {
      code.push_back("CMMA");
      break;
    }
    case OpType::le: {
      code.push_back("CMEG");
      break;
    }
    case OpType::ge: {
      code.push_back("CMAG");
      break;
    }
  }
  return code;
}

std::vector<std::string> ast::Type::CodeGen(CodeGenContext& context) {}

std::vector<std::string> ast::AssignmentStmt::CodeGen(CodeGenContext& context) {
  std::cout << "Creating assignment for id " << getlhsName() << std::endl;
  std::vector<std::string> code;

  auto entry = Lookup(context, lhs->name);

  if (entry.address == -1) {
    red("Identifier '" + lhs->name + "' is not defined at scope " +
        std::to_string(context.scope) + ".");
    context.error = true;
    return code;
  }

  for (std::string exp : rhs->CodeGen(context)) {
    code.push_back(exp);
  }

  code.push_back("ARMZ " + std::to_string(entry.address));

  return code;
}

std::vector<string> ast::Var::CodeGen(CodeGenContext& context) {
  std::vector<std::string> code;

  auto entry = Lookup(context, name->name);
  if (entry.address != -1 && entry.scope == context.scope) {
    red("Identifier '" + name->name + "' is already defined at scope " +
        std::to_string(context.scope) + ".");
    context.error = true;
    return code;
  }

  std::cout << "AMEM for: " << name->name << " at "
            << std::to_string(context.addr) << std::endl;
  context.lookup_table.emplace(
      name->name, LPEntry("VS", context.addr, context.scope, type->raw_name));

  context.addr = context.addr + 1;
  // Varios amem pra abençoar o trabalho
  code.push_back("AMEM 1");

  return code;
}

std::vector<string> ast::Var::CodeGenDmem(CodeGenContext& context) {
  std::cout << "DMEM for: " << this->name->name << std::endl;
  std::vector<string> alloc;
  std::cout << "DMEM 1" << std::endl;
  alloc.push_back("DMEM 1");
  return alloc;
}

std::vector<string> ast::Program::CodeGen(CodeGenContext& context) {
  std::vector<string> code;
  std::cout << "------BEGIN PROGRAM--------" << std::endl;
  std::cout << "INPP" << std::endl;
  // deal with variable declaration
  std::cout << "------BEGIN PROGRAM VARIABLES--------" << std::endl;
  for (auto var_decl : *(this->var_part)) {
    auto vars = var_decl->CodeGen(context);
    for (auto el : vars) {
      code.push_back(el);
    }
  }
  std::cout << "------END PROGRAM VARIABLES--------" << std::endl;
  code.push_back("DSVS prog");
  std::cout << "------BEGIN PROGRAM METHODS--------" << std::endl;
  for (auto routine : *(this->routine_part)) {
    auto body = routine->CodeGen(context);
    for (auto el : body) {
      code.push_back(el);
    }
  }
  std::cout << "------END PROGRAM METHODS--------" << std::endl;

  code.push_back("prog: NADA");
  std::cout << "------BEGIN PROGRAM BODY--------" << std::endl;
  auto c = routine_body->CodeGen(context);
  for (auto el : c) {
    code.push_back(el);
  }
  std::cout << "------END PROGRAM BODY--------" << std::endl;
  for (auto var_decl : *(this->var_part)) {
    auto vars = var_decl->CodeGenDmem(context);
    for (auto el : vars) {
      code.push_back(el);
    }
  }
  std::cout << "PARA" << std::endl;
  std::cout << "------END PROGRAM--------" << std::endl;
  return code;
}

std::vector<std::string> ast::IfStmt::CodeGen(CodeGenContext& context) {
  std::vector<std::string> code = condition->CodeGen(context);
  string labelElse = random_string(16) + "else";
  string labelThen = random_string(16) + "then";

  // Else branch
  if (elseStmt != nullptr) {
    code.push_back("DSVF " + labelElse);
  }

  for (auto i : thenStmt->CodeGen(context)) {
    code.push_back(i);
  }

  // Then branch
  code.push_back("DSVF " + labelThen);

  if (elseStmt != nullptr) {
    code.push_back(labelElse + ": NADA");
    std::vector<std::string> elseStm = elseStmt->CodeGen(context);
    for (auto i : elseStm) {
      code.push_back(i);
    }
  }

  code.push_back(labelThen + ": NADA");
  return code;
}

std::vector<std::string> ast::WhileStmt::CodeGen(CodeGenContext& context) {
  auto beginLabel = random_string(16) + "beginW";
  auto endLabel = random_string(16) + "endW";
  std::vector<string> code;
  code.push_back(beginLabel + ": NADA");
  for (auto i : condition->CodeGen(context)) {
    code.push_back(i);
  }
  code.push_back("DSVF " + endLabel);
  for (auto i : loopStmt->CodeGen(context)) {
    code.push_back(i);
  }
  code.push_back("DSVI " + beginLabel);
  code.push_back(endLabel + ": NADA");
  return code;
}