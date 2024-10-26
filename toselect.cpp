#include "toanf.cpp"  
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>

class Instruction {
  std::vector<std::string> instruction;
public:
  Instruction(const std::vector<std::string> &instruction) : instruction(instruction) {}
  std::vector<std::string> get_instruction() const {
    return instruction;
  }
};

class Instructions {
  std::vector<std::shared_ptr<Instruction>> instructions;

public:
  Instructions(const std::vector<std::shared_ptr<Instruction>> &instructions) : instructions(instructions) {}
  
  std::vector<std::shared_ptr<Instruction>> get_instructions() const {
    return instructions;
  }

  void print_instructions() const {
    std::cout << "{";
    
    for (size_t i = 0; i < instructions.size(); ++i) {
      std::cout << "{";
      
      const auto &instr = instructions[i]->get_instruction();
      for (size_t j = 0; j < instr.size(); ++j) {
        std::cout << instr[j];
        if (j < instr.size() - 1) {
          std::cout << ", ";
        }
      }
      
      std::cout << "}";
      if (i < instructions.size() - 1) {
        std::cout << ", ";
      }
    }
    
    std::cout << "}" << std::endl;
  }
};

class InstructionSelector {
public:
  static Instructions to_select(const std::shared_ptr<Expression> &anf) {
    size_t counter = 0;
    std::unordered_map<std::string, std::string> stack;
    auto instructions = anf_to_select(anf, counter, stack);
    return Instructions(instructions); 
  }

private:
  static std::vector<std::shared_ptr<Instruction>> anf_to_select(
      const std::shared_ptr<Expression> &anf,
      size_t &counter,
      std::unordered_map<std::string, std::string> &stack) {
    
    std::vector<std::shared_ptr<Instruction>> instructions;

    if (std::shared_ptr<LetExpression> anf_let = std::dynamic_pointer_cast<LetExpression>(anf)) {
      std::string let_var = anf_let->get_variable();

      if (stack.find(let_var) == stack.end()) {
        counter += 8;
        std::string number_string = std::to_string(counter);
        std::string stack_location = "-" + number_string + "(%rbp)";
        stack[let_var] = stack_location;
        std::shared_ptr<Expression> value = anf_let->get_value();
        if (std::shared_ptr<NumberExpression> number_expr = std::dynamic_pointer_cast<NumberExpression>(value)) {
          int val = number_expr->get_value();
          std::string immediate = std::to_string(val);
          std::vector<std::string> mov = {"movq", immediate, stack_location};
          instructions.push_back(std::make_shared<Instruction>(mov));
        } else if (std::shared_ptr<LessExpression> less_expr = std::dynamic_pointer_cast<LessExpression>(value)) {
          std::shared_ptr<Expression> left = less_expr->get_left();
          std::shared_ptr<VariableExpression> var_expr = std::dynamic_pointer_cast<VariableExpression>(left);
          std::shared_ptr<Expression> right = less_expr->get_right();
          std::shared_ptr<NumberExpression> num_expr = std::dynamic_pointer_cast<NumberExpression>(right);
          std::string var = var_expr->get_name();
          if (stack.find(var) != stack.end()) {
            std::string stack_location_ = stack[var];
            std::string immediate_ = std::to_string(num_expr->get_value());
            std::vector<std::string> cmp = {"cmpq", immediate_, stack_location_};
            std::vector<std::string> setl = {"setl", "%al"};
            std::vector<std::string> movz = {"movzbq", "%al", "%rsi"};
            instructions.push_back(std::make_shared<Instruction>(cmp));
            instructions.push_back(std::make_shared<Instruction>(setl));
            instructions.push_back(std::make_shared<Instruction>(movz));
	            }
        }
	 auto body_instructions = anf_to_select(anf_let->get_body(), counter, stack);
        instructions.insert(instructions.end(), body_instructions.begin(), body_instructions.end());
      } else {
        std::string stack_location = stack[let_var];
        std::shared_ptr<Expression> value = anf_let->get_value();
        auto number_expr = std::dynamic_pointer_cast<NumberExpression>(value);
        int val = number_expr->get_value();
        std::string immediate = std::to_string(val);

        std::vector<std::string> mov = {"movq", immediate, stack_location};
        instructions.push_back(std::make_shared<Instruction>(mov));

        auto body_instructions = anf_to_select(anf_let->get_body(), counter, stack);
        instructions.insert(instructions.end(), body_instructions.begin(), body_instructions.end());
      }
    } else if (std::shared_ptr<IfExpression> if_expr = std::dynamic_pointer_cast<IfExpression>(anf)) {
      std::shared_ptr<Expression> cnd = if_expr->get_cnd();
      std::shared_ptr<VariableExpression> var_cnd = std::dynamic_pointer_cast<VariableExpression>(cnd);
      std::string stack_location__ = stack[var_cnd->get_name()];
      
        std::vector<std::string> cmp = {"cmpq", "$1", "%rsi"};
        std::string block = "block_" + std::to_string(counter);
        size_t temp_counter = counter;
        ++temp_counter;
        std::string block2 = "block_" + std::to_string(temp_counter);
        instructions.push_back(std::make_shared<Instruction>(cmp));
        std::vector<std::string> je = {"je", block};
        std::vector<std::string> jmp = {"jmp", block2};
        instructions.push_back(std::make_shared<Instruction>(je));
        instructions.push_back(std::make_shared<Instruction>(jmp));
        std::vector<std::string> label = {"label", block};
        instructions.push_back(std::make_shared<Instruction>(label));
        auto thn_instructions = anf_to_select(if_expr->get_thn(), counter, stack);
        instructions.insert(instructions.end(), thn_instructions.begin(), thn_instructions.end());
        std::vector<std::string> label2 = {"label", block2};
        instructions.push_back(std::make_shared<Instruction>(label2));
        auto els_instructions = anf_to_select(if_expr->get_els(), counter, stack);
        instructions.insert(instructions.end(), els_instructions.begin(), els_instructions.end());
      
      
    } else if (std::shared_ptr<NumberExpression> num_expr = std::dynamic_pointer_cast<NumberExpression>(anf)) {
      int val = num_expr->get_value();
      std::string immediate = std::to_string(val);
      std::vector<std::string> mov = {"movq", immediate, "%rdi"};
      instructions.push_back(std::make_shared<Instruction>(mov));
      std::vector<std::string> print = {"callq", "print_int"};
      instructions.push_back(std::make_shared<Instruction>(print));
    } else {
      auto var_expr = std::dynamic_pointer_cast<VariableExpression>(anf);
      std::string var = var_expr->get_name();
      std::string stack_location = stack[var];
      std::vector<std::string> movq = {"movq", stack_location, "%rdi"};
      instructions.push_back(std::make_shared<Instruction>(movq));
      
      std::vector<std::string> print = {"callq", "print_int"};
      instructions.push_back(std::make_shared<Instruction>(print));
    }

    return instructions;
  }
};

int main() {
  std::string input = "(let ((x 3)) (if (< x 3) 1 2))";
  std::shared_ptr<Expression> ast = Parser::parse(input);
  std::shared_ptr<Expression> anf = ToAnf::to_anf(ast);
  std::cout << anf->toString() << std::endl;
  Instructions ins = InstructionSelector::to_select(anf);
  ins.print_instructions();
}
