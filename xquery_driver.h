#pragma once

#include <string>
#include <memory>
#include <iostream>

#include "xquery_lexer.h"
#include "xquery_parser.tab.hh"
#include "xquery_ast.h"

namespace XQuery
{

class Driver
{
    // Needed to access the AST from Bison
    friend class Parser;

    public:
        Driver() : parser_(nullptr), lexer_(nullptr) {};
        virtual ~Driver() = default;

        void parse(const char* filename);
        void error(const std::string& msg) const
        {
            std::cerr << msg << std::endl;
        }
        void error(const location& loc, const std::string& msg) const
        {
            std::cerr << loc << ": " << msg << std::endl;
        }
        // XXX: Non const to allow location access from the Bison parser
        std::string& get_filename()
        {
            return filename_;
        }
        void set_filename(const std::string& filename)
        {
            filename_ = filename;
        }

    private:
        Ast                     ast_;
        std::string             filename_ = "";
        std::unique_ptr<Parser> parser_;
        std::unique_ptr<Lexer>  lexer_;
};

}