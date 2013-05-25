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
        class Processor;
        class Lexer;
        class Node;
    }
}

%lex-param   { Lexer &lexer }
%lex-param   { Processor &process }

%parse-param { Lexer &lexer }
%parse-param { Processor &process }

%locations
%initial-action
{
    /* Initialize the initial location */
    @$.begin.filename = @$.end.filename = &process.filename();
}

%code
{
    #include <iostream>
    #include <cstdlib>
    #include <fstream>

    #include "xquery_processor.h"
    #include "xquery_nodes.h"

    static int yylex(xquery::Parser::semantic_type *yylval,
                     xquery::Parser::location_type *yylloc,
                     xquery::Lexer &lexer,
                     xquery::Processor &process);

    #define NEW_NODE(...) process.ast_.AddNode(new __VA_ARGS__)
    #define SET_ROOT(x) process.ast_.set_root(x)
    #define BUFFERIZE(x) process.ast_.BufferizeEdge(x)
    #define UNBUFFERIZE() process.ast_.UnbufferizeEdges()

    namespace xql = xquery::lang;
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
%type <node> some
%type <node> cond

/*
 * Parser::parse() body definition
 */

/* Precedences */
%nonassoc RP
%nonassoc '(' ')'
%nonassoc LNEG
%nonassoc SATISFY
%left LJUNC
%nonassoc RET
%nonassoc LET
%nonassoc '['
%left ','
%right PSEP
%nonassoc TUPLE

%%

xquery  : END
        | query END
;

query   : xq                {
                                $$ = NEW_NODE(xql::NonTerminalNode{xql::XQ, {$1}});
                                SET_ROOT($$);
                            }
;

xq      : VAR               {
                                $$ = NEW_NODE(xql::Variable{*$1});
                                delete $1;
                            }
        | CSTR              {
                                $$ = NEW_NODE(xql::ConstantString{*$1});
                                delete $1;
                            }
        | ap                {   $$ = NEW_NODE(xql::NonTerminalNode{xql::AP, {$1}});   }
        | '(' xq ')'        {   $$ = NEW_NODE(xql::NonTerminalNode{xql::XQ, {$2}});   }
        | xq ',' xq         {
                                auto xq1 = NEW_NODE(xql::NonTerminalNode{xql::XQ, {$1}});
                                auto xq2 = NEW_NODE(xql::NonTerminalNode{xql::XQ, {$3}});
                                $$ = NEW_NODE(xql::Concatenation{{xq1, xq2}});
                            }
        | xq PSEP rp        {
                                auto xq = NEW_NODE(xql::NonTerminalNode{xql::XQ, {$1}});
                                auto rp = NEW_NODE(xql::NonTerminalNode{xql::RP, {$3}});
                                $$ = NEW_NODE(xql::PathSeparator{*$2, {xq, rp}});
                                delete $2;
                            }
        | OTAG xq CTAG      {
                                auto xq = NEW_NODE(xql::NonTerminalNode{xql::XQ, {$2}});
                                $$ = NEW_NODE(xql::Tag{*$1, *$3, {xq}});
                                delete $1;
                                delete $3;
                            }
        | for let where
          return            {   $$ = NEW_NODE(xql::FLWRExpression{{$1, $2, $3, $4}});   }
        | for where return  {   $$ = NEW_NODE(xql::FLWRExpression{{$1, $2, $3}});   }
        | for let return    {   $$ = NEW_NODE(xql::FLWRExpression{{$1, $2, $3}});   }
        | for return        {   $$ = NEW_NODE(xql::FLWRExpression{{$1, $2}});   }
        | let xq %prec LET  {
                                auto xq = NEW_NODE(xql::NonTerminalNode{xql::XQ, {$2}});
                                $$ = NEW_NODE(xql::LetExpression{{$1, xq}});
                            }
;

fstuple : VAR IN xq
          %prec TUPLE       {
                                auto var = NEW_NODE(xql::Variable{*$1});
                                auto xq = NEW_NODE(xql::NonTerminalNode{xql::XQ, {$3}});
                                BUFFERIZE(NEW_NODE(xql::Tuple{{var, xq}}));
                                delete $1;
                            }
        | fstuple ','
          fstuple           {}
;

ltuple  : VAR AFFECT xq
          %prec TUPLE       {
                                auto var = NEW_NODE(xql::Variable{*$1});
                                auto xq = NEW_NODE(xql::NonTerminalNode{xql::XQ, {$3}});
                                BUFFERIZE(NEW_NODE(xql::Tuple{{var, xq}}));
                                delete $1;
                            }
        | ltuple ',' ltuple {}
;

for     : FOR fstuple       {   $$ = NEW_NODE(xql::ForClause{UNBUFFERIZE()});   }
;

let     : LET ltuple        {   $$ = NEW_NODE(xql::LetClause{UNBUFFERIZE()});   }
;

where   : WHERE cond        {   $$ = NEW_NODE(xql::WhereClause{{$2}});   }
;

return  : RET xq            {   $$ = NEW_NODE(xql::ReturnClause{{$2}});   }
;

some    : SOME fstuple      {   $$ = NEW_NODE(xql::SomeClause{UNBUFFERIZE()});   }
;

cond    : xq EQUAL xq       {
                                auto xq1 = NEW_NODE(xql::NonTerminalNode{xql::XQ, {$1}});
                                auto xq2 = NEW_NODE(xql::NonTerminalNode{xql::XQ, {$3}});
                                $$ = NEW_NODE(xql::Equality{*$2, {xq1, xq2}});
                                delete $2;
                            }
        | '(' cond ')'      {
                                auto cond = NEW_NODE(xql::NonTerminalNode{xql::COND, {$2}});
                                $$ = NEW_NODE(xql::Precedence{{cond}});
                            }
        | cond LJUNC
          cond              {
                                auto cond1 = NEW_NODE(xql::NonTerminalNode{xql::COND, {$1}});
                                auto cond2 = NEW_NODE(xql::NonTerminalNode{xql::COND, {$3}});
                                $$ = NEW_NODE(xql::LogicOperator{*$2, {cond1, cond2}});
                                delete $2;
                            }
        | LNEG cond         {
                                auto cond = NEW_NODE(xql::NonTerminalNode{xql::COND, {$2}});
                                $$ = NEW_NODE(xql::LogicOperator{*$1, {cond}});
                                delete $1;
                            }
        | EMPTY '(' xq ')'  {   $$ = NEW_NODE(xql::Empty{{$3}});   }
        | some SATISFY cond {
                                auto cond = NEW_NODE(xql::NonTerminalNode{xql::COND, {$3}});
                                $1->AddEdge(cond);
                                $$ = $1;
                            }
;

ap      : DOC PSEP rp       {
                                auto doc = NEW_NODE(xql::Document{*$1});
                                auto rp = NEW_NODE(xql::NonTerminalNode{xql::RP, {$3}});
                                $$ = NEW_NODE(xql::PathSeparator{*$2, {doc, rp}});
                                delete $1;
                                delete $2;
                            }
;

rp      : TAGNAME           {
                                $$ = NEW_NODE(xql::TagName{*$1});
                                delete $1;
                            }
        | PGLOB             {
                                $$ = NEW_NODE(xql::PathGlobbing{*$1});
                                delete $1;
                            }
        | TEXT              {   $$ = NEW_NODE(xql::Text{});   }
        | rp PSEP rp        {
                                auto rp1 = NEW_NODE(xql::NonTerminalNode{xql::RP, {$1}});
                                auto rp2 = NEW_NODE(xql::NonTerminalNode{xql::RP, {$3}});
                                $$ = NEW_NODE(xql::PathSeparator{*$2, {rp1, rp2}});
                                delete $2;
                            }
        | '(' rp ')'        {
                                auto rp = NEW_NODE(xql::NonTerminalNode{xql::RP, {$2}});
                                $$ = NEW_NODE(xql::Precedence{{rp}});
                            }
        | rp ',' rp         {
                                auto rp1 = NEW_NODE(xql::NonTerminalNode{xql::RP, {$1}});
                                auto rp2 = NEW_NODE(xql::NonTerminalNode{xql::RP, {$3}});
                                $$ = NEW_NODE(xql::Concatenation{{rp1, rp2}});
                            }
        | rp '[' f ']'      {
                                auto rp = NEW_NODE(xql::NonTerminalNode{xql::RP, {$1}});
                                auto f = NEW_NODE(xql::NonTerminalNode{xql::F, {$3}});
                                $$ = NEW_NODE(xql::Filter{{rp, f}});
                            }
;

f       : rp %prec RP       {   $$ = NEW_NODE(xql::NonTerminalNode{xql::RP, {$1}});   }
        | rp EQUAL rp       {
                                auto rp1 = NEW_NODE(xql::NonTerminalNode{xql::RP, {$1}});
                                auto rp2 = NEW_NODE(xql::NonTerminalNode{xql::RP, {$3}});
                                $$ = NEW_NODE(xql::Equality{*$2, {rp1, rp2}});
                                delete $2;
                            }
        | '(' f ')'         {
                                auto f = NEW_NODE(xql::NonTerminalNode{xql::F, {$2}});
                                $$ = NEW_NODE(xql::Precedence{{f}});
                            }
        | f LJUNC f         {
                                auto f1 = NEW_NODE(xql::NonTerminalNode{xql::F, {$1}});
                                auto f2 = NEW_NODE(xql::NonTerminalNode{xql::F, {$3}});
                                $$ = NEW_NODE(xql::LogicOperator{*$2, {f1, f2}});
                                delete $2;
                            }
        | LNEG f            {
                                auto f = NEW_NODE(xql::NonTerminalNode{xql::F, {$2}});
                                $$ = NEW_NODE(xql::LogicOperator{*$1, {f}});
                                delete $1;
                            }
;

%%

// Defined by Bison
void xquery::Parser::error(const xquery::Parser::location_type& loc,
                           const std::string& msg)
{
    process.Error(loc, msg);
}

#include "xquery_lexer.h"

// Define an entry point for Bison to call our Lexer wrapper
static int yylex(xquery::Parser::semantic_type* yylval,
                 xquery::Parser::location_type *yylloc,
                 xquery::Lexer& lexer,
                 xquery::Processor&)
{
    return lexer.Yylex(yylval, yylloc);
}

