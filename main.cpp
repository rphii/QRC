
#include <stdio.h>
#include <iostream>

#include "QRC.hpp"

int main(int argc, char **argv)
{
    QRC qr;
    
    qr.Encode("Hello World!", 3);
    qr.ExportAsBMP("qr.bmp");
    
    std::cout << "Done" << std::endl;
    
    return 0;
}
