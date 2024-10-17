#include "parser.cpp"
#include <iostream>
#include <memory>

class ToAnf {
public:
  static std::shared_ptr<Expression>
  to_anf(const std::shared_ptr<Expression> &ast) {
    size_t counter = 0;
    return ast_to_anf(ast, counter);
  }

private:
  static std::shared_ptr<Expression>
  ast_to_anf(const std::shared_ptr<Expression> &ast, size_t &counter) {
    if (std::shared_ptr<IfExpression> ast_if =
            std::dynamic_pointer_cast<IfExpression>(ast)) {
      std::string tmp = "temp_" + std::to_string(counter);
      std::shared_ptr<VariableExpression> temp_var =
          std::make_shared<VariableExpression>(tmp);
      ++counter;
      std::shared_ptr<Expression> thn = ast_to_anf(ast_if->get_thn(), counter);
      std::shared_ptr<Expression> els = ast_to_anf(ast_if->get_els(), counter);

      std::shared_ptr<IfExpression> if_exp =
          std::make_shared<IfExpression>(temp_var, thn, els);
      return std::make_shared<LetExpression>(temp_var->get_name(),
                                             ast_if->get_cnd(), if_exp);
    } else if (std::shared_ptr<LetExpression> ast_let =
                   std::dynamic_pointer_cast<LetExpression>(ast)) {
      std::shared_ptr<Expression> body =
          ast_to_anf(ast_let->get_body(), counter);
      return std::make_shared<LetExpression>(ast_let->get_variable(),
                                             ast_let->get_value(), body);
    } else if (std::shared_ptr<BeginExpression> ast_begin =
                   std::dynamic_pointer_cast<BeginExpression>(ast)) {
      std::vector<std::shared_ptr<Expression>> expressions;
      std::vector<std::shared_ptr<Expression>> ast_expressions =
          ast_begin->get_expressions();
      for (int i = 0; i < ast_expressions.size(); i++) {
        expressions.push_back(ast_to_anf(ast_expressions[i], counter));
      }
      return std::make_shared<BeginExpression>(expressions);
    } else if (std::shared_ptr<SetExpression> ast_set =
                   std::dynamic_pointer_cast<SetExpression>(ast)) {
      return std::make_shared<SetExpression>(
          ast_set->get_variable(), ast_to_anf(ast_set->get_value(), counter));
    } else if (std::shared_ptr<WhileExpression> ast_while =
                   std::dynamic_pointer_cast<WhileExpression>(ast)) {
      return std::make_shared<WhileExpression>(
          ast_to_anf(ast_while->get_cnd(), counter),
          ast_to_anf(ast_while->get_body(), counter));
    }

    return ast;
  }
};

int main() {

  // std::string input = "(if (< 2 3) 3 4)";
  // std::string input = "(let ((x 3)) (if (< x 3) 1 2))";
  // std::string input = "(let ((x 3)) (let ((y 5)) (if (< x y) 1 2)))";
  // std::string input = "(let ((x 4)) (let ((y 3)) (if (< x y) (let ((z 5)) (if
  // "
  // "(< z 3) 2 4)) 1)))";
  std::string input = "(let ((sum 0)) (let ((i 0)) (begin (while (< i 5) "
                      "(begin (set sum (+ i sum)) (set i (+ i 1)))) sum)))";
  std::shared_ptr<Expression> ast = Parser::parse(input);
  std::shared_ptr<Expression> anf = ToAnf::to_anf(ast);
  std::cout << anf->toString() << std::endl;
  return 0;
}
