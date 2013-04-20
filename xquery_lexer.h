#pragma once

#undef  YY_DECL
#define YY_DECL int XQuery::Lexer::yylex()

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "xquery_driver.h"
#include "xquery_parser.tab.hh"

namespace XQuery
{

class Lexer : public yyFlexLexer
{
    public:
        Lexer(const Driver& driver, std::istream& input)
          : yyFlexLexer(&input),
            driver_(driver),
            yylval_(nullptr),
            yylloc_(nullptr) {}
        ~Lexer() = default;

        // Wrapper arround yylex called by Bison
        int yylex(Parser::semantic_type* lval, Parser::location_type *lloc)
        {
            yylval_ = lval;
            yylloc_ = lloc;
            return yylex();
        }

    private:
        virtual int yylex() override; // Generated by Flex

        const Driver&          driver_;
        Parser::semantic_type* yylval_;
        Parser::location_type* yylloc_;
};

}