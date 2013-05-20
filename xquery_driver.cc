#include <fstream>
#include <cassert>

#include "xquery_driver.h"

int xquery::Driver::Parse(const char* filename)
{
    assert(filename != nullptr);
    set_filename(filename);

    std::ifstream fs(filename_);

    if ( !fs.good())
        throw std::ios_base::failure("Could not open " + filename_);

    lexer_ = std::unique_ptr<Lexer>(new Lexer(*this, fs));
    parser_ = std::unique_ptr<Parser>(new Parser(*lexer_, *this));

    if ( parser_->parse()) {
        Error("Parsing failed");
        return 1;
    }

    ast_.PlotGraph();
    return 0;
}
