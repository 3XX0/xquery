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
%token <sval> EQUAL         "equality test"
%token <sval> LOGIC_JUNC    "logic junction"
%token <sval> LOGIC_NEG     "logic negation"
%token        TEXT          "text()"

%destructor { delete $$; } DOC TAG PATH_SEP PATH_GLOB EQUAL LOGIC_JUNC LOGIC_NEG

%type <node> query
%type <node> rp
%type <node> ap
%type <node> f

/*
 * Parser::parse() body definition
 */

%%

xquery  : END
        | query END
;

query   : ap                {
                                $$ = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::AP, {$1}));
                                SET_ROOT($$);
                            }
        | rp                {
                                $$ = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$1}));
                                SET_ROOT($$);
                            }
;

ap      : DOC PATH_SEP rp   {
                                auto doc = NEW_NODE(xquery::lang::Document(*$1));
                                auto rp = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$3}));
                                $$ = NEW_NODE(xquery::lang::PathSeparator(*$2, {doc, rp}));
                                delete $1;
                                delete $2;
                            }
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
                                auto rp1 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$1}));
                                auto rp2 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$3}));
                                $$ = NEW_NODE(xquery::lang::PathSeparator(*$2, {rp1, rp2}));
                                delete $2;
                            }
        | '(' rp ')'        {
                                auto rp = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$2}));
                                $$ = NEW_NODE(xquery::lang::Precedence({rp}));
                            }
        | rp ',' rp         {
                                auto rp1 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$1}));
                                auto rp2 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$3}));
                                $$ = NEW_NODE(xquery::lang::Concatenation({rp1, rp2}));
                            }
        | rp '[' f ']'      {
                                auto rp = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$1}));
                                auto f = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::F, {$3}));
                                $$ = NEW_NODE(xquery::lang::Filter({rp, f}));
                            }
;

f       : rp                {   $$ = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$1}));    }
        | rp EQUAL rp       {
                                auto rp1 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$1}));
                                auto rp2 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$3}));
                                $$ = NEW_NODE(xquery::lang::Equality(*$2, {rp1, rp2}));
                                delete $2;
                            }
        | '(' f ')'         {
                                auto f = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::F, {$2}));
                                $$ = NEW_NODE(xquery::lang::Precedence({f}));
                            }
        | f LOGIC_JUNC f    {
                                auto f1 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::F, {$1}));
                                auto f2 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::F, {$3}));
                                $$ = NEW_NODE(xquery::lang::LogicOperator(*$2, {f1, f2}));
                                delete $2;
                            }
        | LOGIC_NEG f       {
                                auto f = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::F, {$2}));
                                $$ = NEW_NODE(xquery::lang::LogicOperator(*$1, {f}));
                                delete $1;
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

