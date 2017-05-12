%code requires{
#include <string>
#include <vector>
#include <map>
class Scanner;
#include "Commands.h"
}

%{
#include <iostream>
#include "Scanner.h"
#include <string>
#include <vector>
#include <map>

#include"parse.hpp"

using namespace std;



//using namespace std;

void error (const yy::location& loc,const std::string& st){
    //cout << "hey" << endl;
         std::cerr << "Syntax error " << loc.begin.column
              << "-" << loc.end.column <<": " << st<< std::endl;
     exit(EXIT_FAILURE);
}

void yy::parser::error(const yy::location& loc,const std::string& st)
{
    ::error(loc,st); // calling global error function and not this one.
}

%}

// C++ declaration
%skeleton "lalr1.cc"
%language "c++"
%define api.value.type variant
%define lr.type ielr

// keywords
%token IF THEN ELSE FI

//operator declaration
%token PIPE AND OR SEQ LP RP IN OUT OUTA NL BG

//typed tokens
%token  <std::string>   STRING

%parse-param {Scanner& scan}
%parse-param {Command*& res}

%code{
    // declare the parser fonction to call :
#define yylex scan.yylex
 }

// Better error explanation than "syntax error"
%define parse.error verbose

 //Location tracking for errors
%locations

 //Location initialisation
%initial-action
{
    @$.initialize (scan.getName());
};


%start line

%type   <Command*> command
%type   <Atomic*> atomic_command
%type   <std::vector<std::string>> strlist
%type   <std::vector<Redirection*>> redirlist
%type   <Redirection*> redir

%left SEQ
%left AND
%left OR
%left PIPE

%%

line:
       command
        {
            res = $1;
        }
    ;

command:
       atomic_command
        {
            $$ = $1;
        }
    |   command SEQ command
        {
            $$ = new Seq($1,$3);
        }
    |   command AND command
        {
            $$ = new And($1,$3);
        }
    |   command OR command
        {
            $$ = new Or($1,$3);
        }
    |   command PIPE command
        {
            $$ = new Pipe($1,$3);
        }
    |   LP command RP
        {
            $$ = $2;
        }
    |   IF command THEN command ELSE command FI
        {
            $$ = new If($2,$4,$6);
        }
    ;

atomic_command:
        strlist redirlist
        {
            $$ = new Atomic($1,$2,false);
        }
    ;


redirlist:
        {
            $$ = {};
        }
    |   redirlist redir
        {
            $1.push_back($2);
            $$ = $1;
        }
    ;

redir:
        IN STRING
        {
            $$ = new Input($2);
        }
    |   OUT STRING
        {
            $$ = new Output($2);
        }
    |   OUTA STRING
        {
            $$ = new OutputAppend($2);
        }
    ;

strlist:
       STRING
        {
            $$ = vector<string>{$1};
        }
    |   strlist STRING
        {
            $1.push_back($2);
            $$ = $1;
        }
    ;

%%
