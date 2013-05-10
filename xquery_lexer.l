%{

    #include <string>

    #include "xquery_lexer.h"

    typedef xquery::Parser::token token;

    // Define yyterminate as a token instead of NULL
    #ifndef yyterminate
    #define yyterminate() return (token::END)
    #endif

    #define STOKEN(...) (new std::string(__VA_ARGS__))

%}

%option debug
/* %option nodefault */
%option yyclass="Lexer"
%option noyywrap
%option c++

DELIMITER       [\(\)\[\],\/]
BLANK           [:blank:]+
ENDL            \n
EQUALITY        =|eq|==|is
LOGIC_OP        or|and|not
DOC_KEYWORD     doc\({FILENAME}\)
TEXT_KEYWORD    text\(\)
PATH_SEPARATOR  \/\/?
PATH_GLOBBING   \*|\.\.?
TAGNAME         [[:alnum:]_]+
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
{TAGNAME}           {
                        yylval_->sval = STOKEN(yytext);
                        return token::TAG;
                    }
{PATH_SEPARATOR}    {
                        yylval_->sval = STOKEN(yytext);
                        return token::PATH_SEP;
                    }

{PATH_GLOBBING}     {
                        yylval_->sval = STOKEN(yytext);
                        return token::PATH_GLOB;
                    }

{TEXT_KEYWORD}          return token::TEXT;

                        /* Convert delimiters in tokens */
{DELIMITER}             return xquery::Parser::token_type(yytext[0]);

{BLANK}                 yylloc_->step();
{ENDL}              {
                        yylloc_->lines(yyleng);
                        yylloc_->step();
                    }
.                   {
                        driver_.Error(*yylloc_, "invalid character");
                        return 1;
                    }

%%

