#ifndef SCANNER_H
#define SCANNER_H

#if !defined(yyFlexLexerOnce)
#include<FlexLexer.h>
#endif

#include"../out/parse.hpp"

//Scanning class
class Scanner : public yyFlexLexer
{
public:
  //Scanner on the stream in
  Scanner(std::istream* in,const std::string& filename ) : yyFlexLexer(in),name(filename) {}

  //read a token and store its value in yyval
  virtual int yylex (yy::parser::semantic_type* yyval, yy::parser::location_type*loc);

  std::string * getName()
  {
	return &name;
  }


private :

  std::string name;
};
// YY_DECL contain the signature of the function where flex puts its code
#undef YY_DECL
#define YY_DECL int Scanner::yylex (yy::parser::semantic_type* yyval, yy::parser::location_type*loc)


//error function (implemented in asm.y)
void error (const yy::location& loc,const std::string& st);

#endif
