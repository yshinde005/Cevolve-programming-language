#include<cctype>
#include<cstdio>
#include<cstdlib>
#include<map>
#include<memory>
#include<string>
#include<utility>
#include<vector>

using namespace std;

// The lexer returns tokens [0-255]{i.e ASCII} for characters not defined  , otherwise one
// of these for known things.
enum Token {
  token_eof = -1,

  // commands
  token_let = -2, 
  token_fn = -3,

  // primary
  token_identifier = -4,
  token_number = -5,
};

static string IdentifierStr; // Filled in if token_identifier
// static int SmNumVal;         // Filled in if token_number  
static double NumVal;        // Filled in if token_number
// although NumVal and IdentifierStr are global variables and it's not good practise
// for a basic language like Cevo it is acceptable

static int gettoken(){
  static int LastChar  = ' ';

  // Skip any whitespace.To make sure that we don't care about wide spaces like in python
  while (isspace(LastChar))
    LastChar = getchar();

if (isalpha(LastChar)) { // identifier: [a-zA-Z] [a-zA-Z0-9]*
  IdentifierStr = LastChar;
  while (isalnum((LastChar = getchar())))
    IdentifierStr += LastChar;

  if (IdentifierStr == "let")
    return token_let;
  if (IdentifierStr == "fn")
    return token_fn;
  return token_identifier;
}

if (isdigit(LastChar) || LastChar == '.') {   // Number: [0-9.]+
  string NumStr;
  do {
    NumStr += LastChar;
    LastChar = getchar();
  } while (isdigit(LastChar) || LastChar == '.');

  NumVal = strtod(NumStr.c_str(), 0);//we  use strod to convert Numerical value we store in NumVal
  return token_number;
}

if (LastChar == '#') {
  // Comment until end of line.
  do
    LastChar = getchar();
  while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
//EOF will check whether the end of line is reached or not
  if (LastChar != EOF)
    return gettoken();
}
  // Check for end of file.  Don't eat the EOF.
  if (LastChar == EOF)
    return token_eof;

  // Otherwise, just return the character as its ascii value.
  int ThisChar = LastChar;
  LastChar = getchar();
  return ThisChar;
}


//Now let's define our AST (Abstract Syntax tree)

//ExpressionAST will be our base class for all express nodes.
class ExpressionAST{
  public: virtual ~ExpressionAST(){}// we use destructor to basically free up the memory
};


//now we create an expression subclass for numeric literals
class NumExpressionAST : public ExpressionAST{
  double val;

  public : NumExpressionAST(double val): val(val){}
};

//exression subclass for referencing varibles
class VarExpressionAST : public ExpressionAST{
  string Name;
  public:
  VarExpressionAST(const string&Name): Name(Name){}
};

//exression subclass for binary operators 
class BinaryExpressionAST: public ExpressionAST{
  char Op;
  unique_ptr<ExpressionAST> LHS,RHS;
  //the unique_ptr<object> will take sole ownership of the object and destroy it will it gets out of scpoe

  public:
  BinaryExpressionAST(char op,unique_ptr<ExpressionAST> LHS, unique_ptr<ExpressionAST> RHS  ):
  Op(op), LHS(move(LHS)),RHS(move(RHS)){}
};

//move will transfer the value of element to element pointed by result
//expression subclass for function calls
class CallExpressionAST: public ExpressionAST{
  string Callee;
  vector<unique_ptr<ExpressionAST>>Args;  
  public:
  CallExpressionAST(const string &Callee, vector<unique_ptr<ExpressionAST>> Args)
  :Callee(Callee) , Args(move(Args)){}
};


//Prototype for funcions, it'll capture name and arguments 
class PrototypeAST{
  string Name;
  vector<string> Args;

  public :
  PrototypeAST(const string &name, vector<string> Args)
  :Name(name),  Args(move(Args)){}

  const string &getName() const {return Name;}
  };

  //
  class FunctionAST {
    unique_ptr<PrototypeAST> Prototype;
    unique_ptr<ExpressionAST> Body; 
    
    public:
    FunctionAST(unique_ptr<PrototypeAST> Prototype,
    unique_ptr<ExpressionAST> Body)
    : Prototype(move(Prototype)), Body(move(Body)) {}
  };

