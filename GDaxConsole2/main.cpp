#include <iostream>

#include "gdaxlib.h"

int main()
{
    std::cout << "GDax" << std::endl;
    GDaxLib g(GDL::defaultStreamUrl);
    return 0;
}
