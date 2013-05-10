#include <iostream>
#include <cstdlib>

#include "xquery_driver.h"

int main(const int argc, const char* argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " filename" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        xquery::Driver driver;

        driver.Parse(argv[1]);
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
