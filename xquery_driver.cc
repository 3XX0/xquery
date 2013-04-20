#include <fstream>
#include <cassert>

#include "xquery_driver.h"

void XQuery::Driver::parse(const char* filename)
{
    assert(filename != nullptr);
    set_filename(filename);

    std::ifstream fs(filename_);

    if ( !fs.good())
        throw std::ios_base::failure("Could not open " + filename_);

    lexer_ = std::unique_ptr<Lexer>(new XQuery::Lexer(*this, fs));
    parser_ = std::unique_ptr<Parser>(new XQuery::Parser(*lexer_, *this));

    if ( parser_->parse())
        error("Parsing failed");
    else
        ast_.print();
}
