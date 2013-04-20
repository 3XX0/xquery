%skeleton "lalr1.cc"
%require "2.5"
%error-verbose
%debug
%defines
%define namespace "XQuery"
%define parser_class_name "Parser"

%code requires
{
    namespace XQuery
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
    @$.begin.filename = @$.end.filename = &driver.get_filename();
}

%code
{
    #include <iostream>
    #include <cstdlib>
    #include <fstream>

    #include "xquery_driver.h"
    #include "xquery_nodes.h"

    static int yylex(XQuery::Parser::semantic_type *yylval,
                     XQuery::Parser::location_type *yylloc,
                     XQuery::Lexer &lexer,
                     XQuery::Driver &driver);

    #define NEW_NODE(x) driver.ast_.addNode(new x)
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

query   : ap                {   driver.ast_.setRoot($1);    }
        | rp                {   driver.ast_.setRoot($1);    }
;

rp      : TAG               {
                                $$ = NEW_NODE(XQuery::Lang::TagName(*$1));
                                delete $1;
                            }
        | PATH_GLOB         {
                                $$ = NEW_NODE(XQuery::Lang::PathGlobbing(*$1));
                                delete $1;
                            }
        | TEXT              {   $$ = NEW_NODE(XQuery::Lang::Text());    }
        | rp PATH_SEP rp    {
                                auto psep = NEW_NODE(XQuery::Lang::PathSeparator(*$2));
                                $$ = NEW_NODE(XQuery::Lang::NonTerminalNode(
                                                XQuery::Lang::TLabel::RP,
                                                {$1, psep, $3}));
                                delete $2;
                            }
        | '(' rp ')'        {}
        | rp '[' ']'        {}
        | rp ',' rp         {}
;

ap      : DOC PATH_SEP rp   {
                                auto doc = NEW_NODE(XQuery::Lang::Document(*$1));
                                auto psep = NEW_NODE(XQuery::Lang::PathSeparator(*$2));
                                $$ = NEW_NODE(XQuery::Lang::NonTerminalNode(
                                                XQuery::Lang::TLabel::AP,
                                                {doc, psep, $3}));
                                delete $1;
                                delete $2;
                            }
;

%%

void XQuery::Parser::error(const XQuery::Parser::location_type& loc,
                           const std::string& msg)
{
    driver.error(loc, msg);
}

#include "xquery_lexer.h"

// Define an entry point for Bison to call our Lexer wrapper
static int yylex(XQuery::Parser::semantic_type* yylval,
                 XQuery::Parser::location_type *yylloc,
                 XQuery::Lexer& lexer,
                 XQuery::Driver&)
{
    return lexer.yylex(yylval, yylloc);
}

