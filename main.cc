#include <iostream>

#include "xquery_driver.h"

int main(const int argc, const char* argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " filename" << std::endl;
        return 1;
    }

    try {
        xquery::Driver driver;

        return driver.Parse(argv[1]);
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
