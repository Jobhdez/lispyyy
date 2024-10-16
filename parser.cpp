#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

class Expression {
public:
  virtual ~Expression() = default;
  virtual std::string toString() const = 0;
};

class NumberExpression : public Expression {
  int value;

public:
  NumberExpression(int value) : value(value) {}
  std::string toString() const override { return std::to_string(value); }
};

class VariableExpression : public Expression {
  std::string name;

public:
  VariableExpression(const std::string &name) : name(name) {}
  std::string toString() const override { return name; }
};

class AdditionExpression : public Expression {
  std::shared_ptr<Expression> left;
  std::shared_ptr<Expression> right;

public:
  AdditionExpression(std::shared_ptr<Expression> left,
                     std::shared_ptr<Expression> right)
      : left(left), right(right) {}
  std::string toString() const override {
    return "(+ " + left->toString() + " " + right->toString() + ")";
  }
};

class LessExpression : public Expression {
  std::shared_ptr<Expression> left;
  std::shared_ptr<Expression> right;

public:
  LessExpression(std::shared_ptr<Expression> left,
                 std::shared_ptr<Expression> right)
      : left(left), right(right) {}
  std::string toString() const override {
    return "(< " + left->toString() + " " + right->toString() + ")";
  }
};

class LetExpression : public Expression {
  std::string variable;
  std::shared_ptr<Expression> value;
  std::shared_ptr<Expression> body;

public:
  LetExpression(const std::string &variable, std::shared_ptr<Expression> value,
                std::shared_ptr<Expression> body)
      : variable(variable), value(value), body(body) {}
  std::string toString() const override {
    return "(let ((" + variable + " " + value->toString() + ")) " +
           body->toString() + ")";
  }
};

class SetExpression : public Expression {
  std::string variable;
  std::shared_ptr<Expression> value;
public:
  SetExpression(const std::string &variable, std::shared_ptr<Expression> value)
    : variable(variable), value(value) {}
  std::string toString() const override {
    return "(set " + variable + " " + value->toString() + ")";
  }
};

class IfExpression : public Expression {
  std::shared_ptr<Expression> cnd;
  std::shared_ptr<Expression> thn;
  std::shared_ptr<Expression> els;

public:
  IfExpression(const std::shared_ptr<Expression> cnd,
               std::shared_ptr<Expression> thn, std::shared_ptr<Expression> els)
      : cnd(cnd), thn(thn), els(els) {}

  std::string toString() const override {
    return "(if " + cnd->toString() + " " + thn->toString() + " " +
           els->toString() + ")";
  }
};

class Parser {
public:
  static std::shared_ptr<Expression> parse(const std::string &program) {
    std::vector<std::string> tokens = tokenize(program);
    size_t index = 0;
    return parseExpression(tokens, index);
  }

private:
  static std::vector<std::string> tokenize(const std::string &program) {
    std::vector<std::string> tokens;
    std::string currentToken;
    for (char ch : program) {
      switch (ch) {
      case '(':
      case ')':
      case ' ':
        if (!currentToken.empty()) {
          tokens.push_back(currentToken);
          currentToken.clear();
        }
        if (ch != ' ')
          tokens.push_back(std::string(1, ch));
        break;
      default:
        currentToken += ch;
        break;
      }
    }
    if (!currentToken.empty()) {
      tokens.push_back(currentToken);
    }
    return tokens;
  }

  static std::shared_ptr<Expression>
  parseExpression(const std::vector<std::string> &tokens, size_t &index) {
    const std::string &token = tokens[index];

    if (token == "(") {
      ++index;
      const std::string &nextToken = tokens[index];

      if (nextToken == "+") {
        ++index;
        std::shared_ptr<Expression> left = parseExpression(tokens, index);
        ++index;
        std::shared_ptr<Expression> right = parseExpression(tokens, index);
        ++index; // Skip closing ')'
        return std::make_shared<AdditionExpression>(left, right);
      } else if (nextToken == "<") {
        ++index;
        std::shared_ptr<Expression> left = parseExpression(tokens, index);
        ++index;
        std::shared_ptr<Expression> right = parseExpression(tokens, index);
        ++index; // Skip closing ')'
        return std::make_shared<LessExpression>(left, right);
      } else if (nextToken == "if") {
        ++index; // Skip "if"
        std::shared_ptr<Expression> cnd = parseExpression(tokens, index);
        ++index;
        std::shared_ptr<Expression> thn = parseExpression(tokens, index);
        ++index;
        std::shared_ptr<Expression> els = parseExpression(tokens, index);
        ++index; // Skip closing ')'
        return std::make_shared<IfExpression>(cnd, thn, els);
      } else if (nextToken == "set") {
	++index;
        std::string variable = tokens[index++];
	std::shared_ptr<Expression> value = parseExpression(tokens, index);
	++index;
	return std::make_shared<SetExpression>(variable, value);
	
      } else if (nextToken == "let") {
        ++index; // Skip "let"
        ++index; // Skip first "("
        ++index;
        std::string variable = tokens[index++];
        std::shared_ptr<Expression> value = parseExpression(tokens, index);
        ++index; // Skip ")"
        ++index;
        ++index;
        std::shared_ptr<Expression> body = parseExpression(tokens, index);
        ++index; // Skip closing ')'
        return std::make_shared<LetExpression>(variable, value, body);
      }
    } else if (isNumber(token)) {
      return std::make_shared<NumberExpression>(std::stoi(token));

    } else {
      return std::make_shared<VariableExpression>(token);
    }

    return nullptr;
  }

  static bool isNumber(const std::string &token) {
    return !token.empty() && std::all_of(token.begin(), token.end(), ::isdigit);
  }
};

int main() {
  // Example input: (if (< 3 4) 3 4)
  // std::string input = "(if (< 3 4) 3 4)";
  // std::string input = "(if (< 2 4) 5 6)";
  // std::string input = "(let ((x 3)) (+ x 3))";
  //std::string input = "(let ((c 3)) (if (< c 4) 4 6))";
  //std::string input = "(set d 2)";
  std::string input = "(set d (if (< 3 4) 4 6))";

  std::shared_ptr<Expression> ast = Parser::parse(input);

  std::cout << "Parsed AST: " << std::endl;
  std::cout << ast->toString() << std::endl;

  return 0;
}
