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

//Parser
// CurrentToken is current token the parser is looking at and
//getNextToken is another token from the lexer which updates the CurrentToken with its result.
static int CurrentToken;
static int getNextToken(){
  return CurrentToken = gettoken();
}

//an error handling function LogError 
unique_ptr<ExpressionAST> LogError(const char *Str){
  fprintf(stderr,"LogError: %s\n",Str);
  return nullptr;
}

unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}

//numbexpr ::= number
static unique_ptr<ExpressionAST> ParseNumExpr(){
  auto Result = make_unique<NumExpressionAST>(NumVal);
  getNextToken();//consume the number
  return move(Result);
}

/// parenexpr ::= '(' expression ')'
static unique_ptr<ExpressionAST> ParseParenExpr() {
  getNextToken(); // eat (.
  auto V = ParseExpression();//we will define ParseExpression later , this will mainly do recursion. 
  if (!V)
    return nullptr;

  if (CurrentToken != ')')
    return LogError("expected ')'");
  getNextToken(); // eat ).
  return V;
}

//idexpr 
//      ::identifier
//      ::identifier '('expression*')'
static unique_ptr<ExpressionAST> ParseIdExpr(){
  string IdName = IdentifierStr; 

  getNextToken(); //eat identifier

  if(CurrentToken !='(') //simple variable reference
  return make_unique<VarExpressionAST>(IdName);

  //call
  getNextToken(); //eat (
     std::vector<std::unique_ptr<ExpressionAST>> Args;
  if (CurrentToken != ')') {
    while (1) {
      if (auto Arg = ParseExpression()) //again, defined later
        Args.push_back(std::move(Arg));
      else
        return nullptr;

      if (CurrentToken == ')')
        break;

      if (CurrentToken != ',')
        return LogError("Expected ')' or ',' in argument list");
      getNextToken();
    }
  }

  // Eat the ')'.
  getNextToken();
  
  return make_unique<CallExpressionAST>(IdName, move(Args));
}
//the look-ahesd part of above routine checks whether the token is stannd alone or a 
//function call reference

static unique_ptr<ExpressionAST> ParsePrimary(){
  switch(CurrentToken){
    default:
    return LogError("unknown token when expecting an expression");
  case token_identifier:
    return ParseIdExpr();
  case token_number:
    return ParseNumExpr();
  case '(':
    return ParseParenExpr();

  }
}

//for binary expression parsing, we will have to use Operator-Precedence Parsing 
//We do this to make sure that the mathematical logic is followed 

//BinPrecedence - this will hold the precedence of all binary operators
static map<char,int> BinopPrecedence;

//GetTokenPrecedence - Get the precedence of the pending binary operator token.
static  int GetTokenPrecedence(){
  if(!isascii(CurrentToken))
  return -1;
  
  //check whether it is a declared binary operator
  int TokenPrecedence =  BinopPrecedence[CurrentToken];
  if(TokenPrecedence <=0) 
  return-1;
  return TokenPrecedence;
}

//expression
//          ::= primary binoprhs

// static unique_ptr<ExpressionAST> ParseExpression(){
//   auto LHS = ParsePrimary;
//   if(!LHS)
//   return nullptr;

//   return ParseBinopRHS(0, move(LHS));
// }

static unique_ptr<ExpressionAST> ParseExpression() {
  auto LHS = ParsePrimary();
  if (!LHS)
    return nullptr;

  return ParseBinopRHS(0, move(LHS));
}


static unique_ptr<ExpressionAST> ParseBinopRHS(int ExprPrecedence, unique_ptr<ExpressionAST> LHS){
  //find its precedence if it is a binary operator
  while (1)
  {
 int TokenPrecedence = GetTokenPrecedence();

 // If this is a binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done.
    if (TokenPrecedence<ExprPrecedence)
    return LHS;

//now as we know that this is Binpo,
int Binop = CurrentToken;
getNextToken();//eat Binop

//parse the primary expression after the binary operator
auto RHS = ParsePrimary();
if(!RHS)
return nullptr;

// If BinOp binds less tightly with RHS than the operator after RHS, let
// the pending operator take RHS as its LHS.
int NextPrecedence = GetTokenPrecedence();
if(TokenPrecedence< NextPrecedence){
  RHS = ParseBinopRHS(TokenPrecedence + 1, move(RHS));
      if (!RHS)
        return nullptr;
}

//Merge LHS/RHS
LHS= make_unique<BinaryExpressionAST>(Binop, move(LHS),move(RHS));

  
  }
}

//prototype
//        ::= id '(' id* ')'
static unique_ptr<PrototypeAST> ParsePrototype(){
  if(CurrentToken != token_identifier)
  return LogErrorP("Expected function in prototype");

  string FunctionName = IdentifierStr;
  getNextToken();

  if(CurrentToken != '(')
  return LogErrorP("Expected '(' in prototype ");

  //read the list of argument names.
  vector<string> ArgNames;
  while (getNextToken() == token_identifier)
  {
    ArgNames.push_back(IdentifierStr);
    if(CurrentToken != ')')
    return LogErrorP("Expected ')' in  prototype");

    //success
    getNextToken(); //eat ')'

    return make_unique<PrototypeAST>(FunctionName, move(ArgNames));
  }
  
}

//defining let expression
static unique_ptr<FunctionAST> ParseDefinitionLet() {
  getNextToken();  // eat def.
  auto Proto = ParsePrototype();
  if (!Proto) return nullptr;

  if (auto E = ParseExpression())
    return make_unique<FunctionAST>(move(Proto), move(E));
  return nullptr;
}


//fn :: = function prototype
static unique_ptr<PrototypeAST> ParseFn() {
  getNextToken();  // eat extern.
  return ParsePrototype();
}

/// toplevelexpr ::= expression
static unique_ptr<FunctionAST> ParseTopLevelExpr() {
  if (auto E = ParseExpression()) {
    // Make an anonymous proto.
    auto Proto = make_unique<PrototypeAST>("", vector<string>());
    return make_unique<FunctionAST>(move(Proto), move(E));
  }
  return nullptr;
}

static void HandleDefination(){
  if(ParseDefinitionLet()){
    fprintf(stderr, " Parsed a function defination.\n");
  }else{
    //skip token for error recovery
    getNextToken();
  }
}

static void HandleFn(){
  if(ParseFn()){
    fprintf(stderr, " Parsed a function defination.\n");
  }else{
    //skip token for error recovery
    getNextToken();
  }
}

static void HandleTopLevelExpression(){
  //evaluate a top-level expression into an anonymous function.
  if(ParseTopLevelExpr()){
    fprintf(stderr, " Parsed a top level expression.\n");
  }else{
    //skip token for error recovery
    getNextToken();
  }
}



///top ::= defination | external | expression | ';'
static void MainLoop(){
  while (1)
  {
    fprintf(stderr, "ready> ");
    switch(CurrentToken){
      case token_eof:
      return;
      case ';': //ignore top level semicolon
      getNextToken();
      break;
      case token_let:
      HandleDefination();
      break;
      case token_fn:
      HandleFn();
      break;
      default:
      HandleTopLevelExpression();
      break;
    }
  }
  
}



int main() {
  // Install standard binary operators.
  // 1 is lowest precedence.
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;//as + and -  has same precedece
  BinopPrecedence['*'] = 40;  

  //prime 1st token
  fprintf(stderr, "readyg> ");
  getNextToken();

  MainLoop();

  return 0;
}