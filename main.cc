#include <iostream>

#include "xquery_processor.h"

int main(const int argc, const char* argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " filename" << std::endl;
        return 1;
    }

    xquery::Processor process;

    return process.Run(argv[1]);
}
