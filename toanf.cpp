#include "parser.cpp"
#include <iostream>
#include <memory>

using namespace std;

class ToAnf {
public:
  static shared_ptr<Expression> to_anf(const shared_ptr<Expression> &ast) {
    size_t counter = 0;
    return ast_to_anf(ast, counter);
  }

private:
  static shared_ptr<Expression> ast_to_anf(const shared_ptr<Expression> &ast,
                                           size_t &counter) {
    if (shared_ptr<IfExpression> ast_if =
            dynamic_pointer_cast<IfExpression>(ast)) {
      string tmp = "temp_" + to_string(counter);
      shared_ptr<VariableExpression> temp_var =
          make_shared<VariableExpression>(tmp);
      ++counter;
      shared_ptr<Expression> thn = ast_to_anf(ast_if->get_thn(), counter);
      shared_ptr<Expression> els = ast_to_anf(ast_if->get_els(), counter);

      shared_ptr<IfExpression> if_exp =
          make_shared<IfExpression>(temp_var, thn, els);
      return make_shared<LetExpression>(temp_var->get_name(), ast_if->get_cnd(),
                                        if_exp);
    } else if (shared_ptr<LetExpression> ast_let =
                   dynamic_pointer_cast<LetExpression>(ast)) {
      shared_ptr<Expression> body = ast_to_anf(ast_let->get_body(), counter);
      return make_shared<LetExpression>(ast_let->get_variable(),
                                        ast_let->get_value(), body);
    } else if (shared_ptr<BeginExpression> ast_begin =
                   dynamic_pointer_cast<BeginExpression>(ast)) {
      vector<shared_ptr<Expression>> expressions;
      vector<shared_ptr<Expression>> ast_expressions =
          ast_begin->get_expressions();
      for (int i = 0; i < ast_expressions.size(); i++) {
        expressions.push_back(ast_to_anf(ast_expressions[i], counter));
      }
      return make_shared<BeginExpression>(expressions);
    } else if (shared_ptr<SetExpression> ast_set =
                   dynamic_pointer_cast<SetExpression>(ast)) {
      return make_shared<SetExpression>(
          ast_set->get_variable(), ast_to_anf(ast_set->get_value(), counter));
    } else if (shared_ptr<WhileExpression> ast_while =
                   dynamic_pointer_cast<WhileExpression>(ast)) {
      return make_shared<WhileExpression>(
          ast_to_anf(ast_while->get_cnd(), counter),
          ast_to_anf(ast_while->get_body(), counter));
    }

    return ast;
  }
};

/*
int main() {
  //string input = "(let ((x 4)) (let ((y 3)) (if (< x y) (let ((z 5)) (if (< z
3)
  //2 4)) 1)))";
  string input = "(let ((x 1)) (while (< x 5) (begin (set x (+ x 1)) x)))";
  shared_ptr<Expression> ast = Parser::parse(input);
  shared_ptr<Expression> anf = ToAnf::to_anf(ast);
  cout << anf->toString() << endl;
  return 0;
}
*/
