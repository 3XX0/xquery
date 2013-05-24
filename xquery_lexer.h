#pragma once

#undef  YY_DECL
#define YY_DECL int xquery::Lexer::yylex()

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "xquery_misc.h"
#include "xquery_driver.h"
#include "xquery_parser.tab.hh"

namespace xquery
{

class Lexer : public yyFlexLexer, public NonCopyable, public NonMoveable
{
    public:
        Lexer(const Driver& driver, std::istream& input)
          : yyFlexLexer{&input},
            driver_{driver},
            yylval_{nullptr},
            yylloc_{nullptr} {}
        ~Lexer() = default;

        // Wrapper arround yylex called by Bison
        int Yylex(Parser::semantic_type* lval, Parser::location_type *lloc)
        {
            yylval_ = lval;
            yylloc_ = lloc;
            return yylex();
        }

    private:
        int yylex() override; // Generated by Flex

        const Driver&          driver_;
        Parser::semantic_type* yylval_;
        Parser::location_type* yylloc_;
};

}
