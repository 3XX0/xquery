%skeleton "lalr1.cc"
%require "2.5"
%error-verbose
%debug
%defines
%define namespace "xquery"
%define parser_class_name "Parser"

%code requires
{
    namespace xquery
    {
        class Driver;
        class Lexer;
        class Node;
    }
}

%lex-param   { Lexer &lexer }
%lex-param   { Driver &driver }

%parse-param { Lexer &lexer }
%parse-param { Driver &driver }

%locations
%initial-action
{
    /* Initialize the initial location */
    @$.begin.filename = @$.end.filename = &driver.filename();
}

%code
{
    #include <iostream>
    #include <cstdlib>
    #include <fstream>

    #include "xquery_driver.h"
    #include "xquery_nodes.h"

    static int yylex(xquery::Parser::semantic_type *yylval,
                     xquery::Parser::location_type *yylloc,
                     xquery::Lexer &lexer,
                     xquery::Driver &driver);

    #define NEW_NODE(x) driver.ast_.AddNode(new x)
    #define SET_ROOT(x) driver.ast_.set_root(x)
}

%union
{
    std::string* sval;
    const Node*  node;
}

%token        END       0   "end of file"
%token <sval> DOC           "doc()"
%token <sval> TAG           "tagname"
%token <sval> PATH_SEP      "path separator"
%token <sval> PATH_GLOB     "path globbing"
%token        TEXT          "text()"

%destructor { delete $$; } DOC TAG PATH_SEP PATH_GLOB

%type <node> rp
%type <node> ap

/*
 * Parser::parse() body definition
 */

%%

xquery  : END
        | query END
;

query   : ap                {   SET_ROOT($1);    }
        | rp                {   SET_ROOT($1);    }
;

rp      : TAG               {
                                $$ = NEW_NODE(xquery::lang::TagName(*$1));
                                delete $1;
                            }
        | PATH_GLOB         {
                                $$ = NEW_NODE(xquery::lang::PathGlobbing(*$1));
                                delete $1;
                            }
        | TEXT              {   $$ = NEW_NODE(xquery::lang::Text());    }
        | rp PATH_SEP rp    {
                                auto psep = NEW_NODE(xquery::lang::PathSeparator(*$2));
                                $$ = NEW_NODE(xquery::lang::NonTerminalNode(
                                                xquery::lang::RP,
                                                {$1, psep, $3}));
                                delete $2;
                            }
        | '(' rp ')'        {}
        | rp '[' ']'        {}
        | rp ',' rp         {}
;

ap      : DOC PATH_SEP rp   {
                                auto doc = NEW_NODE(xquery::lang::Document(*$1));
                                auto psep = NEW_NODE(xquery::lang::PathSeparator(*$2));
                                $$ = NEW_NODE(xquery::lang::NonTerminalNode(
                                                xquery::lang::AP,
                                                {doc, psep, $3}));
                                delete $1;
                                delete $2;
                            }
;

%%

// Defined by Bison
void xquery::Parser::error(const xquery::Parser::location_type& loc,
                           const std::string& msg)
{
    driver.Error(loc, msg);
}

#include "xquery_lexer.h"

// Define an entry point for Bison to call our Lexer wrapper
static int yylex(xquery::Parser::semantic_type* yylval,
                 xquery::Parser::location_type *yylloc,
                 xquery::Lexer& lexer,
                 xquery::Driver&)
{
    return lexer.Yylex(yylval, yylloc);
}

