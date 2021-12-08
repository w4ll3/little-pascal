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
std::vector<std::string> ast::Identifier::CodeGen(CodeGenContext &context) {
  std::vector<std::string> code;
  auto entry = Lookup(context, name);

  if (entry.address == -1 && !context.ignore) {
    red("Identifier '" + name + "' is not defined.");
    context.error = true;
    return code;
  }
  code.push_back("CRVL " + std::to_string(entry.scope) + ", " +
                 std::to_string(entry.address));
  return code;
}

std::vector<std::string> ast::Attr::CodeGen(CodeGenContext &context) {
  std::vector<std::string> code;
  auto entry = Lookup(context, name);

  if (entry.address == -1 && !context.ignore) {
    red("Identifier '" + name + "' is not defined.");
    context.error = true;
    return code;
  }
  if (strcmp(entry.type.c_str(), "ROUTINE")) {
    return code;
  }

  code.push_back("ARMZ " + std::to_string(entry.scope) + ", " +
                 std::to_string(entry.address));
  return code;
}

std::vector<std::string> ast::Param::CodeGen(CodeGenContext &context) {
  std::vector<std::string> code;
  return code;
}

std::vector<std::string> ast::Integer::CodeGen(CodeGenContext &context) {
  std::vector<std::string> code;
  code.push_back("CRCT " + std::to_string(val));
  return code;
}

std::vector<std::string> ast::Boolean::CodeGen(CodeGenContext &context) {
  std::vector<std::string> code;
  code.push_back("CRCT " + std::to_string(val));
  return code;
}

std::vector<std::string> ast::Routine::CodeGen(CodeGenContext &context) {
  // Addr = number of params
  context.scope = 1;
  int prev_addr = context.addr;
  context.addr = 0;
  std::vector<std::string> code;
  int label = ++context.label;

  auto entry = Lookup(context, routine_name->toString());

  if (entry.address != -1 && !context.ignore) {
    red("Routine '" + routine_name->toString() + "' is already defined.");
    context.error = true;
    return code;
  }

  code.push_back("R" + std::to_string(label) + ": ENPR " +
                 std::to_string(context.scope));

  int arg_addr = -3 - argument_list->size();
  int arg_c = 0;
  for (auto arg : *(argument_list)) {
    arg_c++;
    auto c = arg->CodeGen(context);
    for (auto el : c) {
      code.push_back(el);
    }
    context.lookup_table.emplace(
        arg->name, LPEntry("PF", arg_addr, context.scope, arg->type->raw_name));
    arg_addr++;
  }

  for (auto var_decl : *(var_part)) {
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
                               LPEntry("ROUTINE", label, context.scope,
                                       "0" + std::to_string(arg_c) + "" + ret));

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

  code.push_back("RTPR 1, " + std::to_string(argument_list->size()));

  context.addr = prev_addr;
  context.scope = 0;
  return code;
}

std::vector<std::string> ast::FuncCall::CodeGen(CodeGenContext &context) {
  context.scope = 1;
  int prev_addr = context.addr;
  context.addr = 0;
  std::vector<std::string> code;

  auto entry = Lookup(context, id->name);
  int argc = 0;
  if (entry.type.empty() && !context.ignore) {
    red("Function '" + id->name + "' is not defined.");
    context.error = true;
    return code;
  }

  if (!context.ignore) {
    argc = std::stoi(entry.sys_type);
  }
  code.push_back("AMEM 1");
  if (this->argument_list->size() < argc !=
          this->argument_list->size() > argc &&
      !context.ignore) {
    red("Expected " + std::to_string(argc) + " parameters," + " got " +
        std::to_string(argument_list->size()) + ".");
    context.error = true;
    return code;
  }

  for (auto arg : *(this->argument_list)) {
    auto els = arg->CodeGen(context);
    for (auto el : els) {
      code.push_back(el);
    }
  }

  context.addr = prev_addr;
  context.scope = 0;

  code.push_back("CHPR R" + std::to_string(entry.address) + ", " +
                 std::to_string(context.scope));

  return code;
}

std::vector<std::string> ast::ProcCall::CodeGen(CodeGenContext &context) {
  context.scope = 1;
  int prev_addr = context.addr;
  context.addr = 0;
  std::vector<std::string> code;

  auto entry = Lookup(context, id->name);
  int argc = 0;
  if (entry.type.empty() && !context.ignore) {
    red("Procedure '" + id->name + "' is not defined.");
    context.error = true;
    return code;
  }

  if (!context.ignore) {
    argc = std::stoi(entry.sys_type);
  }

  if (this->argument_list->size() < argc !=
      this->argument_list->size() > argc) {
    red("Expected " + std::to_string(argc) + " parameters," + " got " +
        std::to_string(argument_list->size()) + ".");
    context.error = true;
    return code;
  }

  for (auto arg : *(this->argument_list)) {
    auto els = arg->CodeGen(context);
    for (auto el : els) {
      code.push_back(el);
    }
  }

  context.addr = prev_addr;
  context.scope = 0;

  code.push_back("CHPR R" + std::to_string(entry.address) + ", " +
                 std::to_string(context.scope));

  return code;
}

std::vector<std::string> ast::Write::CodeGen(CodeGenContext &context) {
  std::vector<std::string> code;
  for (auto arg : *argument_list) {
    auto arg_val = arg->CodeGen(context);
    for (auto val : arg_val) {
      code.push_back(val);
    }
    code.push_back("IMPR");
  }

  return code;
}

std::vector<std::string> ast::Read::CodeGen(CodeGenContext &context) {
  std::vector<std::string> code;
  for (auto arg : *argument_list) {

    auto entry = Lookup(context, arg->toString());

    if (entry.address == -1 && !context.ignore) {
      red("Identifier '" + arg->toString() + "' is not defined at scope " +
          std::to_string(context.scope) + ".");
      context.error = true;
      return code;
    }
    code.push_back("LEIT");
    code.push_back("ARMZ " + std::to_string(entry.scope) + ", " +
                   std::to_string(entry.address));
  }

  return code;
}

std::vector<std::string> ast::Operator::CodeGen(CodeGenContext &context) {
  std::vector<std::string> code;
  for (auto inst : op1->CodeGen(context))
    code.push_back(inst);
  for (auto inst : op2->CodeGen(context))
    code.push_back(inst);

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
    code.push_back("CONJ");
    break;
  }
  case OpType::bit_or: {
    code.push_back("DISJ");
    break;
  }
  case OpType::eq: {
    code.push_back("CMIG");
    break;
  }
  case OpType::ne: {
    code.push_back("CMDG");
    break;
  }
  case OpType::lt: {
    code.push_back("CMME");
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

std::vector<std::string> ast::Type::CodeGen(CodeGenContext &context) {}

std::vector<std::string> ast::AssignmentStmt::CodeGen(CodeGenContext &context) {
  std::vector<std::string> code;

  auto entry = Lookup(context, lhs->name);

  if (entry.address == -1 && !context.ignore) {
    red("Identifier '" + lhs->name + "' is not defined at scope " +
        std::to_string(context.scope) + ".");
    context.error = true;
    return code;
  }

  for (std::string exp : rhs->CodeGen(context)) {
    code.push_back(exp);
  }

  if (lhs->isFuncAssig && entry.type.compare("ROUTINE") != 0) {
    return code;
  }

  if (entry.type.compare("ROUTINE") == 0) {
    int argc = std::stoi(entry.sys_type);
    code.push_back("ARMZ 1, " + std::to_string(-4 - argc));
    return code;
  }

  code.push_back("ARMZ " + std::to_string(entry.scope) + ", " +
                 std::to_string(entry.address));

  return code;
}

std::vector<string> ast::Var::CodeGen(CodeGenContext &context) {
  std::vector<std::string> code;

  auto entry = Lookup(context, name->name);
  if (entry.address != -1 && entry.scope == context.scope && !context.ignore) {
    red("Identifier '" + name->name + "' is already defined at scope " +
        std::to_string(context.scope) + ".");
    context.error = true;
    return code;
  }

  context.lookup_table.emplace(
      name->name, LPEntry("VS", context.addr, context.scope, type->raw_name));

  context.addr = context.addr + 1;
  // Varios amem pra abençoar o trabalho
  code.push_back("AMEM 1");

  return code;
}

std::vector<string> ast::Var::CodeGenDmem(CodeGenContext &context) {
  std::vector<string> alloc;
  alloc.push_back("DMEM 1");
  return alloc;
}

std::vector<string> ast::Program::CodeGen(CodeGenContext &context) {
  std::vector<string> code;
  code.push_back("INPP");
  for (auto var_decl : *(this->var_part)) {
    auto vars = var_decl->CodeGen(context);
    for (auto el : vars) {
      code.push_back(el);
    }
  }
  code.push_back("DSVS R00");
  for (auto routine : *(this->routine_part)) {
    auto body = routine->CodeGen(context);
    for (auto el : body) {
      code.push_back(el);
    }
  }

  code.push_back("R00: NADA");
  auto c = routine_body->CodeGen(context);
  for (auto el : c) {
    code.push_back(el);
  }
  for (auto var_decl : *(this->var_part)) {
    auto vars = var_decl->CodeGenDmem(context);
    for (auto el : vars) {
      code.push_back(el);
    }
  }
  for (auto el : context.lookup_table)
    green("Resolved entry: { id: " + el.first +
          ", addr: " + std::to_string(el.second.address) +
          ", scope: " + std::to_string(el.second.scope) +
          ", type: " + el.second.sys_type + ", sys: " + el.second.type + " }");
  code.push_back("PARA");
  return code;
}

std::vector<std::string> ast::IfStmt::CodeGen(CodeGenContext &context) {
  std::vector<std::string> code = condition->CodeGen(context);
  string labelElse = "R" + std::to_string(++context.label);
  string labelThen = "R" + std::to_string(++context.label);

  if (elseStmt != nullptr) {
    // Invert result
    code.push_back("NEGA");

    // Else branch
    code.push_back("DSVF " + labelElse);

    for (auto i : elseStmt->CodeGen(context)) {
      code.push_back(i);
    }

    // Then branch
    code.push_back("DSVS " + labelThen);

    code.push_back(labelElse + ": NADA");
  } else {
    // Then branch
    code.push_back("DSVF " + labelThen);
  }

  for (auto i : thenStmt->CodeGen(context)) {
    code.push_back(i);
  }

  code.push_back(labelThen + ": NADA");

  return code;
}

std::vector<std::string> ast::WhileStmt::CodeGen(CodeGenContext &context) {
  auto beginLabel = "R" + std::to_string(++context.label);
  auto endLabel = "R" + std::to_string(++context.label);
  std::vector<string> code;
  code.push_back(beginLabel + ": NADA");
  for (auto i : condition->CodeGen(context)) {
    code.push_back(i);
  }
  code.push_back("DSVF " + endLabel);
  for (auto i : loopStmt->CodeGen(context)) {
    code.push_back(i);
  }
  code.push_back("DSVS " + beginLabel);
  code.push_back(endLabel + ": NADA");
  return code;
}