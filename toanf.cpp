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
    }
    return ast;
  }
};

int main() {

  // std::string input = "(if (< 2 3) 3 4)";
  // std::string input = "(let ((x 3)) (if (< x 3) 1 2))";
  // std::string input = "(let ((x 3)) (let ((y 5)) (if (< x y) 1 2)))";
  std::string input = "(let ((x 4)) (let ((y 3)) (if (< x y) (let ((z 5)) (if "
                      "(< z 3) 2 4)) 1)))";
  std::shared_ptr<Expression> ast = Parser::parse(input);
  std::shared_ptr<Expression> anf = ToAnf::to_anf(ast);
  std::cout << anf->toString() << std::endl;
  return 0;
}
