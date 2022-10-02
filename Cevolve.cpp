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
