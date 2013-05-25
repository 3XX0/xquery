%{

    #include <string>

    #include "xquery_lexer.h"

    using token = xquery::Parser::token;

    // Define yyterminate as a token instead of NULL
    #ifndef yyterminate
    #define yyterminate() return (token::END)
    #endif

    #define STOKEN(...) new std::string{__VA_ARGS__}

%}

%option debug
/* %option nodefault */
%option yyclass="Lexer"
%option noyywrap
%option c++

DELIMITER       [\(\)\[\],\/]
BLANK           [ \t]+
ENDL            \n
EQUALITY        =|eq|==|is
LOGIC_JUNCTION  or|and
LOGIC_NEGATION  not
DOC_KEYWORD     doc\({FILENAME}\)
TEXT_KEYWORD    text\(\)
PATH_SEPARATOR  \/\/?
PATH_GLOBBING   \*|\.\.?
TAGNAME         [[:alnum:]_]+
OPEN_TAG        <{TAGNAME}>\{
CLOSE_TAG       \}<\/{TAGNAME}>
FOR             for
IN              in
LET             let
AFFECTATION     :=
WHERE           where
RETURN          return
EMPTY           empty
SOME            some
SATISFIES       satisfies
FILENAME        [[:alnum:]_]+(\.[[:alnum:]_]+)?
VARIABLE        \$[[:alnum:]_]+
CSTRING         \"[^\"]*\"

%{
    /* Track location accurately (columns num) */
    #define YY_USER_ACTION  yylloc_->columns(yyleng);
%}

/*
 * Lexer::yylex() body definition
 */

%%

%{
    yylloc_->step();
%}

{DOC_KEYWORD}       {
                        yylval_->sval = STOKEN(yytext+4, yyleng-5);
                        return token::DOC;
                    }
{PATH_SEPARATOR}    {
                        yylval_->sval = STOKEN(yytext);
                        return token::PSEP;
                    }
{PATH_GLOBBING}     {
                        yylval_->sval = STOKEN(yytext);
                        return token::PGLOB;
                    }
{EQUALITY}          {
                        yylval_->sval = STOKEN(yytext);
                        return token::EQUAL;
                    }
{LOGIC_JUNCTION}    {
                        yylval_->sval = STOKEN(yytext);
                        return token::LJUNC;
                    }
{LOGIC_NEGATION}    {
                        yylval_->sval = STOKEN(yytext);
                        return token::LNEG;
                    }
{VARIABLE}          {
                        yylval_->sval = STOKEN(yytext+1, yyleng-1);
                        return token::VAR;
                    }
{CSTRING}           {
                        yylval_->sval = STOKEN(yytext+1, yyleng-2);
                        return token::CSTR;
                    }
{OPEN_TAG}          {
                        yylval_->sval = STOKEN(yytext+1, yyleng-3);
                        return token::OTAG;
                    }
{CLOSE_TAG}         {
                        yylval_->sval = STOKEN(yytext+3, yyleng-4);
                        return token::CTAG;
                    }
                        /* Convert delimiters in tokens */
{DELIMITER}             return xquery::Parser::token_type(yytext[0]);
{TEXT_KEYWORD}          return token::TEXT;
{FOR}                   return token::FOR;
{IN}                    return token::IN;
{LET}                   return token::LET;
{AFFECTATION}           return token::AFFECT;
{WHERE}                 return token::WHERE;
{RETURN}                return token::RET;
{EMPTY}                 return token::EMPTY;
{SOME}                  return token::SOME;
{SATISFIES}             return token::SATISFY;
{TAGNAME}           {
                        yylval_->sval = STOKEN(yytext);
                        return token::TAGNAME;
                    }

{BLANK}                 yylloc_->step();
{ENDL}              {
                        yylloc_->lines(yyleng);
                        yylloc_->step();
                    }
.                   {
                        process_.Error(*yylloc_, "invalid character");
                        return 1;
                    }

%%

