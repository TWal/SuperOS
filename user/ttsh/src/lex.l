

%{
#include<iostream>

#include"Scanner.h"

    using namespace std;
    using namespace yy;
    typedef yy::parser::token token;



#define yyerror error


// This code is executed at each action : update position in code.
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);
%}

%option c++
%option yyclass="Scanner"
%option yylineno
%option noyywrap

%s str
%%
[ \t]
"\n" { assert(loc);loc->lines(); return 0;}
"<"  {return token::IN;}
">>"  {return token::OUTA;}
">"  {return token::OUT;}
"("    {return token::LP;}
")"    {return token::RP;}
"|"  {return token::PIPE;}
";"  {return token::SEQ;}
"&&"  {return token::AND;}
"||"  {return token::OR;}
"&"  {return token::BG;}

"if" {return token::IF;}
"else" {return token::ELSE;}
"then" {return token::THEN;}
"fi" {return token::FI;}

[^ \t\n<>()|;&]+ { yyval->build(std::string(yytext)); return token::STRING;}

  /*. { error(*loc,"unknown token");}*/



%%
