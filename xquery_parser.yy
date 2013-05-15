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
    #define BUFFERIZE(x) driver.ast_.MultiEdgesPush(x)
    #define UNBUFFERIZE() driver.ast_.MultiEdgesPop()
}

%union
{
    std::string* sval;
    const Node*  node;
}

%token        END       0   "EOF"
%token <sval> DOC           "doc()"
%token <sval> TAGNAME       "tagname"
%token <sval> PSEP          "path separator"
%token <sval> PGLOB         "path globbing"
%token <sval> EQUAL         "equality operator"
%token <sval> LJUNC         "logic junction"
%token <sval> LNEG          "not"
%token <sval> VAR           "$var"
%token <sval> CSTR          "constant string"
%token <sval> OTAG          "<tag>"
%token <sval> CTAG          "</tag>"
%token        TEXT          "text()"
%token        FOR           "for"
%token        IN            "in"
%token        LET           "let"
%token        AFFECT        ":="
%token        WHERE         "where"
%token        RET           "return"
%token        EMPTY         "empty()"
%token        SOME          "some"
%token        SATISFY       "satisfies"

%destructor { delete $$; } DOC TAGNAME PSEP PGLOB EQUAL LJUNC LNEG OTAG CTAG VAR CSTR

%type <node> query
%type <node> rp
%type <node> ap
%type <node> f
%type <node> xq
%type <node> for
%type <node> let
%type <node> where
%type <node> return
%type <node> cond

/*
 * Parser::parse() body definition
 */

/* Precedence (lower to higher priority) */
%nonassoc RP_PREC
%left ','
%nonassoc LET_PREC
%nonassoc RET_PREC
%nonassoc TUP_PREC
%left '[' ']'
%left PSEP
%left EQUAL
%left LJUNC
%left LNEG
%left SATISFY
%left '(' ')'

%%

xquery  : END
        | query END
;

query   : xq                {
                                $$ = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::XQ, {$1}));
                                SET_ROOT($$);
                            }
;

xq      : VAR               {
                                $$ = NEW_NODE(xquery::lang::Variable(*$1));
                                delete $1;
                            }
        | CSTR              {
                                $$ = NEW_NODE(xquery::lang::ConstantString(*$1));
                                delete $1;
                            }
        | ap                {   $$ = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::AP, {$1}));   }
        | '(' xq ')'        {   $$ = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::XQ, {$2}));   }
        | xq ',' xq         {
                                auto xq1 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::XQ, {$1}));
                                auto xq2 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::XQ, {$3}));
                                $$ = NEW_NODE(xquery::lang::Concatenation({xq1, xq2}));
                            }
        | xq PSEP rp        {
                                auto xq = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::XQ, {$1}));
                                auto rp = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$3}));
                                $$ = NEW_NODE(xquery::lang::PathSeparator(*$2, {xq, rp}));
                                delete $2;
                            }
        | OTAG xq CTAG      {
                                auto xq = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::XQ, {$2}));
                                $$ = NEW_NODE(xquery::lang::Tag(*$1, *$3, {xq}));
                                delete $1;
                                delete $3;
                            }
        | for let where
          return            {   $$ = NEW_NODE(xquery::lang::ForExpression({$1, $2, $3, $4}));   }
        | for where
          return            {   $$ = NEW_NODE(xquery::lang::ForExpression({$1, $2, $3}));   }
        | for let return    {   $$ = NEW_NODE(xquery::lang::ForExpression({$1, $2, $3}));   }
        | for return        {   $$ = NEW_NODE(xquery::lang::ForExpression({$1, $2}));   }
        | let xq
          %prec LET_PREC    {   $$ = NEW_NODE(xquery::lang::LetClause({$1}));   }
;

fstuple : VAR IN xq
          %prec TUP_PREC    {
                                auto var = NEW_NODE(xquery::lang::Variable(*$1));
                                auto xq = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::XQ, {$3}));
                                BUFFERIZE(NEW_NODE(xquery::lang::Tuple({var, xq})));
                                delete $1;
                            }
        | fstuple ','
          fstuple           {}
;

ltuple  : VAR AFFECT xq
          %prec TUP_PREC    {
                                auto var = NEW_NODE(xquery::lang::Variable(*$1));
                                auto xq = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::XQ, {$3}));
                                BUFFERIZE(NEW_NODE(xquery::lang::Tuple({var, xq})));
                                delete $1;
                            }
        | ltuple ',' ltuple {}
;

for     : FOR fstuple       {   $$ = NEW_NODE(xquery::lang::ForClause(UNBUFFERIZE()));   }
;

let     : LET ltuple        {   $$ = NEW_NODE(xquery::lang::LetClause(UNBUFFERIZE()));   }
;

where   : WHERE cond        {   $$ = NEW_NODE(xquery::lang::WhereClause({$2}));   }
;

return  : RET xq
          %prec RET_PREC    {   $$ = NEW_NODE(xquery::lang::ReturnClause({$2}));   }
;

cond    : xq EQUAL xq       {
                                auto xq1 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::XQ, {$1}));
                                auto xq2 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::XQ, {$3}));
                                $$ = NEW_NODE(xquery::lang::Equality(*$2, {xq1, xq2}));
                                delete $2;
                            }
        | '(' cond ')'      {
                                auto cond = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::COND, {$2}));
                                $$ = NEW_NODE(xquery::lang::Precedence({cond}));
                            }
        | cond LJUNC
          cond              {
                                auto cond1 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::COND, {$1}));
                                auto cond2 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::COND, {$3}));
                                $$ = NEW_NODE(xquery::lang::LogicOperator(*$2, {cond1, cond2}));
                                delete $2;
                            }
        | LNEG cond         {
                                auto cond = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::COND, {$2}));
                                $$ = NEW_NODE(xquery::lang::LogicOperator(*$1, {cond}));
                                delete $1;
                            }
        | EMPTY '(' xq ')'  {   $$ = NEW_NODE(xquery::lang::Empty({$3}));   }
        | SOME fstuple
          SATISFY cond      {
                                BUFFERIZE(NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::COND, {$4})));
                                $$ = NEW_NODE(xquery::lang::SomeClause(UNBUFFERIZE()));
                            }
;

ap      : DOC PSEP rp       {
                                auto doc = NEW_NODE(xquery::lang::Document(*$1));
                                auto rp = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$3}));
                                $$ = NEW_NODE(xquery::lang::PathSeparator(*$2, {doc, rp}));
                                delete $1;
                                delete $2;
                            }
;

rp      : TAGNAME           {
                                $$ = NEW_NODE(xquery::lang::TagName(*$1));
                                delete $1;
                            }
        | PGLOB             {
                                $$ = NEW_NODE(xquery::lang::PathGlobbing(*$1));
                                delete $1;
                            }
        | TEXT              {   $$ = NEW_NODE(xquery::lang::Text());   }
        | rp PSEP rp        {
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

f       : rp %prec RP_PREC  {   $$ = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::RP, {$1}));   }
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
        | f LJUNC f         {
                                auto f1 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::F, {$1}));
                                auto f2 = NEW_NODE(xquery::lang::NonTerminalNode(xquery::lang::F, {$3}));
                                $$ = NEW_NODE(xquery::lang::LogicOperator(*$2, {f1, f2}));
                                delete $2;
                            }
        | LNEG f            {
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

