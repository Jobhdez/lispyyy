#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class Expression {
public:
  virtual ~Expression() = default;
  virtual string toString() const = 0;
};

class NumberExpression : public Expression {
  int value;

public:
  NumberExpression(int value) : value(value) {}
  string toString() const override { return to_string(value); }
  int get_value() { return value; }
};

class VariableExpression : public Expression {
  string name;

public:
  VariableExpression(const string &name) : name(name) {}
  string toString() const override { return name; }
  string get_name() { return name; }
};

class AdditionExpression : public Expression {
  shared_ptr<Expression> left;
  shared_ptr<Expression> right;

public:
  AdditionExpression(shared_ptr<Expression> left, shared_ptr<Expression> right)
      : left(left), right(right) {}
  string toString() const override {
    return "(+ " + left->toString() + " " + right->toString() + ")";
  }
  shared_ptr<Expression> get_left() {
    return left;
  }
  shared_ptr<Expression> get_right() {
    return right;
  }
};

class LessExpression : public Expression {
  shared_ptr<Expression> left;
  shared_ptr<Expression> right;

public:
  LessExpression(shared_ptr<Expression> left, shared_ptr<Expression> right)
      : left(left), right(right) {}
  string toString() const override {
    return "(< " + left->toString() + " " + right->toString() + ")";
  }
  shared_ptr<Expression> get_left() { return left; }
  shared_ptr<Expression> get_right() { return right; }
};

class LetExpression : public Expression {
  string variable;
  shared_ptr<Expression> value;
  shared_ptr<Expression> body;

public:
  LetExpression(const string &variable, shared_ptr<Expression> value,
                shared_ptr<Expression> body)
      : variable(variable), value(value), body(body) {}
  string toString() const override {
    return "(let ((" + variable + " " + value->toString() + ")) " +
           body->toString() + ")";
  }
  string get_variable() { return variable; }
  shared_ptr<Expression> get_value() { return value; }
  shared_ptr<Expression> get_body() { return body; }
};

class SetExpression : public Expression {
  string variable;
  shared_ptr<Expression> value;

public:
  SetExpression(const string &variable, shared_ptr<Expression> value)
      : variable(variable), value(value) {}
  string toString() const override {
    return "(set " + variable + " " + value->toString() + ")";
  }
  string get_variable() { return variable; }
  shared_ptr<Expression> get_value() { return value; }
};

class IfExpression : public Expression {
  shared_ptr<Expression> cnd;
  shared_ptr<Expression> thn;
  shared_ptr<Expression> els;

public:
  IfExpression(const shared_ptr<Expression> cnd, shared_ptr<Expression> thn,
               shared_ptr<Expression> els)
      : cnd(cnd), thn(thn), els(els) {}

  string toString() const override {
    return "(if " + cnd->toString() + " " + thn->toString() + " " +
           els->toString() + ")";
  }
  shared_ptr<Expression> get_cnd() { return cnd; }
  shared_ptr<Expression> get_thn() { return thn; }
  shared_ptr<Expression> get_els() { return els; }
};

class WhileExpression : public Expression {
  shared_ptr<Expression> cnd;
  shared_ptr<Expression> body;

public:
  WhileExpression(const shared_ptr<Expression> cnd,
                  const shared_ptr<Expression> body)
      : cnd(cnd), body(body) {}
  string toString() const override {
    return "(while " + cnd->toString() + " " + body->toString() + ")";
  }
  shared_ptr<Expression> get_cnd() { return cnd; }
  shared_ptr<Expression> get_body() { return body; }
};

class BeginExpression : public Expression {
  vector<shared_ptr<Expression>> expressions;

public:
  BeginExpression(const vector<shared_ptr<Expression>> &expressions)
      : expressions(expressions) {}

  string toString() const override {
    string result = "(begin";
    for (const auto &expr : expressions) {
      result += " " + expr->toString();
    }
    result += ")";
    return result;
  }
  vector<shared_ptr<Expression>> get_expressions() { return expressions; }
};

class Parser {
public:
  static shared_ptr<Expression> parse(const string &program) {
    vector<string> tokens = tokenize(program);
    size_t index = 0;
    return parseExpression(tokens, index);
  }

private:
  static vector<string> tokenize(const string &program) {
    vector<string> tokens;
    string currentToken;
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
          tokens.push_back(string(1, ch));
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

  static shared_ptr<Expression> parseExpression(const vector<string> &tokens,
                                                size_t &index) {
    const string &token = tokens[index];

    if (token == "(") {
      ++index;
      const string &nextToken = tokens[index];

      if (nextToken == "+") {
        ++index;
        shared_ptr<Expression> left = parseExpression(tokens, index);
        ++index;
        shared_ptr<Expression> right = parseExpression(tokens, index);
        ++index; // Skip closing ')'
        return make_shared<AdditionExpression>(left, right);
      } else if (nextToken == "<") {
        ++index;
        shared_ptr<Expression> left = parseExpression(tokens, index);
        ++index;
        shared_ptr<Expression> right = parseExpression(tokens, index);
        ++index; // Skip closing ')'
        return make_shared<LessExpression>(left, right);
      } else if (nextToken == "if") {
        ++index; // Skip "if"
        shared_ptr<Expression> cnd = parseExpression(tokens, index);
        ++index;
        shared_ptr<Expression> thn = parseExpression(tokens, index);
        ++index;
        shared_ptr<Expression> els = parseExpression(tokens, index);
        ++index; // Skip closing ')'
        return make_shared<IfExpression>(cnd, thn, els);
      } else if (nextToken == "begin") {
        ++index;
        vector<shared_ptr<Expression>> expressions;
        while (tokens[index] != ")") {
          expressions.push_back(parseExpression(tokens, index));
          ++index;
        }
	
	
         // Skip the closing ')'
        return make_shared<BeginExpression>(expressions);
      } else if (nextToken == "set") {
        ++index;
        string variable = tokens[index++];
        shared_ptr<Expression> value = parseExpression(tokens, index);
        ++index;
        return make_shared<SetExpression>(variable, value);
      } else if (nextToken == "let") {
        ++index; // Skip "let"
        ++index; // Skip first "("
        ++index;
        string variable = tokens[index++];
        shared_ptr<Expression> value = parseExpression(tokens, index);
        ++index; // Skip ")"
        ++index;
        ++index;
        shared_ptr<Expression> body = parseExpression(tokens, index);
        ++index; // Skip closing ')'
        return make_shared<LetExpression>(variable, value, body);
      } else if (nextToken == "while") {
        ++index;
        shared_ptr<Expression> cnd = parseExpression(tokens, index);
        ++index;
        shared_ptr<Expression> body = parseExpression(tokens, index);
        ++index; // Skip closing ')'
        return make_shared<WhileExpression>(cnd, body);
      }
    } else if (isNumber(token)) {
      return make_shared<NumberExpression>(stoi(token));
    } else {
      return make_shared<VariableExpression>(token);
    }

    return nullptr;
  }

  static bool isNumber(const string &token) {
    return !token.empty() && all_of(token.begin(), token.end(), ::isdigit);
  }
};

int main() {
  //string program = "(let ((sum 0)) (let ((i 0)) (begin (while (< i 5) (begin (set sum (+ sum 2)) (set i (+ i 1)))) sum)))";
  string program = "(let ((i 3)) (begin (set i (+ i 1)) (set i (+ i 1)) i))";
  shared_ptr<Expression> expression = Parser::parse(program);
  cout << expression->toString() << endl;

  return 0;
}

