#include "app.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() 
{
    vhl::HuiApp app;

    try 
    {
        app.run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}