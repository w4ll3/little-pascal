#ifndef __AST_H__
#define __AST_H__

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// used forward-declaration to deal with cross-reference issue
class CodeGenContext;

// namespace ast start
namespace ast {

// forward declaration
class Var;
class Statement;
class Identifier;
class Routine;
class Expression;
class Statements;
class Integer;
class Boolean;
class Const;
class Type;
class Param;

typedef std::vector<Var*> Variables;
typedef std::vector<Param*> Parameters;
typedef std::vector<Identifier*> IdentifierList;
typedef std::vector<Routine*> Routines;
typedef std::vector<std::string> NameList;
typedef std::vector<Expression*> ExpressionList;

/**
 * @brief Class to map all nodes of the tree
 */
class Node {
 public:
  std::string debug;

  void print_node(std::string prefix, bool tail, bool root) {
    std::string tailStr = "└── ";
    std::string branchStr = "├── ";
    std::string ch_tailStr = "    ";
    std::string ch_branchStr = "|   ";
    std::cout << (root ? prefix : (prefix + (tail ? tailStr : branchStr)))
              << (this->toString()) << std::endl;
    auto children_list = this->getChildren();
    auto ch_prefix = tail ? prefix + ch_tailStr : prefix + ch_branchStr;
    for (size_t i = 0; i < children_list.size(); i++) {
      children_list[i] ? children_list[i]->print_node(
                             ch_prefix, i == children_list.size() - 1, false)
                       : []() {}();
    }
  }

  virtual std::vector<Node*> getChildren() {
    return *(new std::vector<Node*>());
  }
  virtual ~Node() {}
  virtual std::string toString() = 0;
  virtual std::vector<std::string> CodeGen(CodeGenContext& context) = 0;
};
class Statement : public Node {
 public:
  Statement(){};

  virtual ~Statement(){};
  virtual std::vector<std::string> CodeGen(CodeGenContext& context){};
  virtual std::vector<Statement*>* getlist() {}
};

class Statements : public Statement {
 public:
  std::vector<Statement*> list;
  virtual std::vector<std::string> CodeGen(CodeGenContext& context) {
    std::vector<std::string> code;
    for (auto stmt : list) {
      auto c = stmt->CodeGen(context);
      for (auto el : c) {
        code.push_back(el);
      };
    }
    return code;
  }
  virtual std::string toString() { return "stmt_list"; }
  virtual std::vector<Statement*>* getlist() { return &list; }
};

class Program : public Node {
 public:
  Variables* var_part;
  Routines* routine_part;
  Statements* routine_body;

  Program() {}
  virtual ~Program() {}
  Program(Variables* vp, Routines* rp, Statements* rb)
      : var_part(vp), routine_part(rp), routine_body(rb) {}
  virtual std::vector<Node*> getChildren() {
    std::vector<Node*> list;
    for (auto i : *(var_part)) list.push_back((Node*)i);
    for (auto i : *(routine_part)) list.push_back((Node*)i);
    for (auto i : *(routine_body->getlist())) list.push_back((Node*)i);
    return list;
  }
  virtual std::string toString() { return "Program start"; }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
};

class Routine : public Program {
 public:
  enum class RoutineType { function, procedure, error };
  Identifier* routine_name;
  Type* return_type;
  Parameters* argument_list;
  RoutineType routine_type;
  Routine(RoutineType rt, Identifier* rn, Parameters* vdl, Type* td)
      : Program(),
        routine_name(rn),
        return_type(td),
        argument_list(vdl),
        routine_type(rt) {}

  Routine(Routine* r, Program* p)
      : Program(*p),
        routine_name(r->routine_name),
        return_type(r->return_type),
        argument_list(r->argument_list),
        routine_type(r->routine_type) {}
  virtual ~Routine() {}
  bool isFunction() { return routine_type == RoutineType::function; }
  bool isProcedure() { return routine_type == RoutineType::procedure; }

  virtual std::vector<Node*> getChildren() {
    std::vector<Node*> list;
    list.push_back((Node*)routine_name);
    list.push_back((Node*)return_type);
    for (auto i : *(argument_list)) list.push_back((Node*)i);
    for (auto i : *(var_part)) list.push_back((Node*)i);
    for (auto i : *(routine_part)) list.push_back((Node*)i);
    for (auto i : *(routine_body->getlist())) list.push_back((Node*)i);

    return list;
  }
  virtual std::string toString() {
    return routine_type == RoutineType::function ? "Function" : "Procedure";
  }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
};

class Type : public Statement {
 public:
  enum class TypeName : int {
    error,
    integer,
    boolean,
  };

  std::string raw_name = "";
  TypeName sys_name = TypeName::error;

  void init() {
    if (sys_name != TypeName::error) return;
    if (raw_name == "integer")
      sys_name = TypeName::integer;
    else if (raw_name == "boolean")
      sys_name = TypeName::boolean;
    else
      throw std::logic_error("There is no such type dude.");
  }

  static std::string TypeString(TypeName type) {
    if (type == TypeName::integer) {
      return "integer";
    } else {
      return "boolean";
    }
  }
  Type(TypeName tpname) : sys_name(tpname) { init(); }
  Type(const std::string& str) : raw_name(str) { init(); }
  Type(const char* ptr_c) : raw_name(*(new std::string(ptr_c))) { init(); }

  virtual std::string toString() { return raw_name; }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
};

class Expression : public Node {
 public:
  virtual std::vector<std::string> CodeGen(CodeGenContext& context){};
  virtual Type::TypeName getType(){};
};

class Identifier : public Expression {
 public:
  std::string name;

  Identifier(const std::string& name) : name(name) {}
  Identifier(const char* ptr_s) : name(*(new std::string(ptr_s))) {}
  virtual std::string toString() { return name; }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
  virtual Type::TypeName getType(){};
};

class Attr : public Expression {
 public:
  std::string name;

  Attr(const std::string& name) : name(name) {}
  Attr(const char* ptr_s) : name(*(new std::string(ptr_s))) {}
  virtual std::string toString() { return name; }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
};

class Const : public Expression {
 public:
  int address;

  Const() : address(-1){};
  Const(int address) : address(address){};
};

class Var : public Statement {
 public:
  Identifier* name;
  Type* type;
  int address;

  Var(Identifier* name, Type* type) : name(name), type(type) {}
  Var(Identifier* name, Type* type, int address)
      : name(name), type(type), address(address) {}
  virtual std::vector<Node*> getChildren() {
    std::vector<Node*> list;
    list.push_back((Node*)name);
    list.push_back((Node*)type);
    list.push_back((Node*)new Identifier(std::to_string(address)));
    return list;
  }
  std::string toString() { return "Var"; }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
  virtual std::vector<std::string> CodeGenDmem(CodeGenContext& context);
};

class Integer : public Const {
 public:
  int val;

  Integer(int val) : val(val) {}
  virtual std::string toString() {
    return [=]() {
      std::stringstream oss;
      oss << val;
      return oss.str();
    }();
  }
  virtual Type::TypeName getType() { return Type::TypeName::integer; }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
};

class Param : public Const {
 public:
  std::string name;
  Type* type;

  Param(const std::string& name, Type* type) : name(name), type(type) {}
  virtual Type::TypeName getType() { return type->sys_name; }
  virtual std::string toString() { return "Param"; }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
};
class Boolean : public Const {
 public:
  int val;

  Boolean(const char* str) : val(std::string(str) == "true" ? 1 : 0) {}
  virtual Type::TypeName getType() { return Type::TypeName::boolean; }
  virtual std::string toString() {
    std::stringstream oss;
    oss << val;
    return oss.str();
  }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
};

class FuncCall : public Expression, public Statement {
 public:
  Identifier* id;
  ExpressionList* argument_list;

  FuncCall(Identifier* id) : id(id), argument_list(nullptr) {}
  FuncCall(Identifier* id, ExpressionList* argument_list)
      : id(id), argument_list(argument_list) {}

  virtual std::vector<Node*> getChildren() {
    std::vector<Node*> list;
    list.push_back((Node*)id);
    for (auto i : *(argument_list)) list.push_back((Node*)i);
    return list;
  }
  virtual std::string toString() { return "FuncCall " + id->name; }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
};

class ProcCall : public Statement {
 public:
  Identifier* id;
  ExpressionList* argument_list;
  NameList* name_list;

  ProcCall(Identifier* id) : id(id), argument_list(new ExpressionList) {}
  ProcCall(Identifier* id, ExpressionList* argument_list)
      : id(id), argument_list(argument_list) {}

  virtual std::vector<Node*> getChildren() {
    std::vector<Node*> list;
    for (auto i : *(argument_list)) list.push_back((Node*)i);
    return list;
  }
  virtual std::string toString() { return "ProcCall " + id->name; }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
};

class Read : public ProcCall {
 public:
  Read(Identifier* id) : ProcCall(id) {}
  Read(Identifier* id, ExpressionList* al) : ProcCall(id, al) {}

  virtual std::string toString() { return "Read"; }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
};

class Write : public ProcCall {
 public:
  Write(Identifier* id) : ProcCall(id) {}
  Write(Identifier* id, ExpressionList* al) : ProcCall(id, al) {}

  virtual std::string toString() { return "Write"; }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
};

class Operator : public Expression {
 public:
  enum class OpType : int {
    plus,
    minus,
    mul,
    div,
    bit_and,
    bit_or,
    eq,
    ne,
    lt,
    gt,
    le,
    ge
  };

  Expression *op1, *op2;
  OpType op;

  Operator(Expression* op1, OpType op, Expression* op2)
      : op1(op1), op(op), op2(op2) {}

  virtual std::vector<Node*> getChildren() {
    std::vector<Node*> list;
    list.push_back((Node*)op1);
    list.push_back((Node*)op2);
    return list;
  }
  virtual std::string toString() {
    return (std::map<OpType, std::string>){
        {OpType::plus, "+"},      {OpType::minus, "-"},
        {OpType::mul, "*"},       {OpType::div, "/ div"},
        {OpType::bit_and, "and"}, {OpType::bit_or, "or"},
        {OpType::eq, "=="},       {OpType::ne, "<>"},
        {OpType::lt, "<"},        {OpType::gt, ">"},
        {OpType::le, "<="},       {OpType::ge, ">="}}[op];
  }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
};

class AssignmentStmt : public Statement {
 public:
  Identifier* lhs = nullptr;  // left-hand side
  Expression* rhs = nullptr;
  AssignmentStmt(Identifier* lhs, Expression* rhs) : lhs(lhs), rhs(rhs) {}

  std::string getlhsName() { return this->lhs->name; }

  virtual std::vector<Node*> getChildren() {
    std::vector<Node*> list;
    list.push_back((Node*)lhs);
    list.push_back((Node*)rhs);
    return list;
  }
  virtual std::string toString() { return "Assignment"; }
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
};

class IfStmt : public Statement {
 public:
  Expression* condition;
  Statement* thenStmt;
  Statement* elseStmt;
  IfStmt(Expression* condition, Statement* thenStmt, Statement* elseStmt)
      : condition(condition), thenStmt(thenStmt), elseStmt(elseStmt){};
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
  virtual std::string toString() { return "If"; }
};
class WhileStmt : public Statement {
 public:
  Expression* condition;
  Statement* loopStmt;
  WhileStmt(Expression* condition, Statement* loopStmt)
      : condition(condition), loopStmt(loopStmt) {}
  virtual std::vector<std::string> CodeGen(CodeGenContext& context);
  virtual std::string toString() { return "while"; }
};

}  // namespace ast
#endif
