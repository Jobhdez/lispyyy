#include "toanf.cpp"
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using std::cout;
using std::dynamic_pointer_cast;
using std::endl;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::to_string;
using std::unordered_map;
using std::vector;

class Instruction {
  vector<string> instruction;

public:
  Instruction(const vector<string> &instruction) : instruction(instruction) {}
  vector<string> get_instruction() const { return instruction; }
};

class Instructions {
  vector<shared_ptr<Instruction>> instructions;

public:
  Instructions(const vector<shared_ptr<Instruction>> &instructions)
      : instructions(instructions) {}

  vector<shared_ptr<Instruction>> get_instructions() const {
    return instructions;
  }

  void print_instructions() const {
    cout << "{";

    for (size_t i = 0; i < instructions.size(); ++i) {
      cout << "{";

      const auto &instr = instructions[i]->get_instruction();
      for (size_t j = 0; j < instr.size(); ++j) {
        cout << instr[j];
        if (j < instr.size() - 1) {
          cout << ", ";
        }
      }

      cout << "}";
      if (i < instructions.size() - 1) {
        cout << ", ";
      }
    }

    cout << "}" << endl;
  }
};

class InstructionSelector {
public:
  static Instructions to_select(const shared_ptr<Expression> &anf) {
    size_t counter = 0;
    unordered_map<string, string> stack;
    auto instructions = anf_to_select(anf, counter, stack);
    return Instructions(instructions);
  }

private:
  static vector<shared_ptr<Instruction>>
  anf_to_select(const shared_ptr<Expression> &anf, size_t &counter,
                unordered_map<string, string> &stack) {

    vector<shared_ptr<Instruction>> instructions;

    if (shared_ptr<LetExpression> anf_let =
            dynamic_pointer_cast<LetExpression>(anf)) {
      string let_var = anf_let->get_variable();

      if (stack.find(let_var) == stack.end()) {
        counter += 8;
        string number_string = to_string(counter);
        string stack_location = "-" + number_string + "(%rbp)";
        stack[let_var] = stack_location;
        shared_ptr<Expression> value = anf_let->get_value();

        if (shared_ptr<NumberExpression> number_expr =
                dynamic_pointer_cast<NumberExpression>(value)) {
          int val = number_expr->get_value();
          string immediate = to_string(val);
          vector<string> mov = {"movq", immediate, stack_location};
          instructions.push_back(make_shared<Instruction>(mov));

        } else if (shared_ptr<LessExpression> less_expr =
                       dynamic_pointer_cast<LessExpression>(value)) {
          shared_ptr<Expression> left = less_expr->get_left();
          shared_ptr<VariableExpression> var_expr =
              dynamic_pointer_cast<VariableExpression>(left);
          shared_ptr<Expression> right = less_expr->get_right();
          shared_ptr<NumberExpression> num_expr =
              dynamic_pointer_cast<NumberExpression>(right);
          string var = var_expr->get_name();

          if (stack.find(var) != stack.end()) {
            string stack_location_ = stack[var];
            string immediate_ = to_string(num_expr->get_value());
            vector<string> cmp = {"cmpq", immediate_, stack_location_};
            vector<string> setl = {"setl", "%al"};
            vector<string> movz = {"movzbq", "%al", "%rsi"};
            instructions.push_back(make_shared<Instruction>(cmp));
            instructions.push_back(make_shared<Instruction>(setl));
            instructions.push_back(make_shared<Instruction>(movz));
          }
        }
        auto body_instructions =
            anf_to_select(anf_let->get_body(), counter, stack);
        instructions.insert(instructions.end(), body_instructions.begin(),
                            body_instructions.end());

      } else {
        string stack_location = stack[let_var];
        shared_ptr<Expression> value = anf_let->get_value();
        auto number_expr = dynamic_pointer_cast<NumberExpression>(value);
        int val = number_expr->get_value();
        string immediate = to_string(val);

        vector<string> mov = {"movq", immediate, stack_location};
        instructions.push_back(make_shared<Instruction>(mov));

        auto body_instructions =
            anf_to_select(anf_let->get_body(), counter, stack);
        instructions.insert(instructions.end(), body_instructions.begin(),
                            body_instructions.end());
      }

    } else if (shared_ptr<IfExpression> if_expr =
                   dynamic_pointer_cast<IfExpression>(anf)) {
      shared_ptr<Expression> cnd = if_expr->get_cnd();
      shared_ptr<VariableExpression> var_cnd =
          dynamic_pointer_cast<VariableExpression>(cnd);
      string stack_location__ = stack[var_cnd->get_name()];

      vector<string> cmp = {"cmpq", "$1", "%rsi"};
      string block = "block_" + to_string(counter);
      size_t temp_counter = counter;
      ++temp_counter;
      string block2 = "block_" + to_string(temp_counter);
      instructions.push_back(make_shared<Instruction>(cmp));
      vector<string> je = {"je", block};
      vector<string> jmp = {"jmp", block2};
      instructions.push_back(make_shared<Instruction>(je));
      instructions.push_back(make_shared<Instruction>(jmp));
      vector<string> label = {"label", block};
      instructions.push_back(make_shared<Instruction>(label));
      auto thn_instructions = anf_to_select(if_expr->get_thn(), counter, stack);
      instructions.insert(instructions.end(), thn_instructions.begin(),
                          thn_instructions.end());
      vector<string> label2 = {"label", block2};
      instructions.push_back(make_shared<Instruction>(label2));
      auto els_instructions = anf_to_select(if_expr->get_els(), counter, stack);
      instructions.insert(instructions.end(), els_instructions.begin(),
                          els_instructions.end());

    } else if (shared_ptr<NumberExpression> num_expr =
                   dynamic_pointer_cast<NumberExpression>(anf)) {
      int val = num_expr->get_value();
      string immediate = to_string(val);
      vector<string> mov = {"movq", immediate, "%rdi"};
      instructions.push_back(make_shared<Instruction>(mov));
      vector<string> print = {"callq", "print_int"};
      instructions.push_back(make_shared<Instruction>(print));

    } else {
      auto var_expr = dynamic_pointer_cast<VariableExpression>(anf);
      string var = var_expr->get_name();
      string stack_location = stack[var];
      vector<string> movq = {"movq", stack_location, "%rdi"};
      instructions.push_back(make_shared<Instruction>(movq));

      vector<string> print = {"callq", "print_int"};
      instructions.push_back(make_shared<Instruction>(print));
    }

    return instructions;
  }
};

int main() {
  string input = "(let ((x 3)) (if (< x 3) 1 2))";
  shared_ptr<Expression> ast = Parser::parse(input);
  shared_ptr<Expression> anf = ToAnf::to_anf(ast);
  cout << anf->toString() << endl;
  Instructions ins = InstructionSelector::to_select(anf);
  ins.print_instructions();
}
