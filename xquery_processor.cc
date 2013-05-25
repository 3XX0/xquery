#include <fstream>
#include <cassert>

#include "xquery_misc.h"
#include "xquery_processor.h"

int xquery::Processor::Run(const char* filename)
{
    assert(filename != nullptr);
    set_filename(filename);

    std::ifstream fs{filename_};

    try {
        if ( !fs.good())
            throw std::ios_base::failure{"Could not open " + filename_};

        lexer_ = std::unique_ptr<Lexer>{new Lexer{*this, fs}};
        parser_ = std::unique_ptr<Parser>{new Parser{*lexer_, *this}};

        if ( parser_->parse()) {
            Error("Parsing failed"_red);
            return 1;
        }

        ast_.PlotGraph(); // Throws
        ast_.Eval();      // Throws
    }
    catch (const std::ios_base::failure& e) {
        Error(e.what());
        return 1;
    }
    catch (const std::runtime_error& e) {
        Error(e.what());
        Error("Evaluation failed"_red);
        return 1;
    }

    std::cout << "Evaluation done"_green << std::endl;
    return 0;
}
