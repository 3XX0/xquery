#pragma once

#include <string>
#include <memory>
#include <iostream>

#include "xquery_misc.h"
#include "xquery_lexer.h"
#include "xquery_parser.tab.hh"
#include "xquery_ast.h"

namespace xquery
{

class Driver : public NonCopyable, public NonMoveable
{
    // Needed to access the AST from Bison
    friend class Parser;

    public:
        Driver() : parser_{nullptr}, lexer_{nullptr} {};
        virtual ~Driver() = default;

        int Parse(const char* filename);
        void Error(const std::string& msg) const
        {
            std::cerr << msg << std::endl;
        }
        void Error(const location& loc, const std::string& msg) const
        {
            std::cerr << loc << ": " << msg << std::endl;
        }

    private:
        // XXX: Non const to allow location access from the Bison parser
        std::string& filename()
        {
            return filename_;
        }
        void set_filename(const std::string& filename)
        {
            filename_ = filename;
        }

        Ast                     ast_;
        std::string             filename_ = "";
        std::unique_ptr<Parser> parser_;
        std::unique_ptr<Lexer>  lexer_;
};

}
