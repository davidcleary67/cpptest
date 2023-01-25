#include <iostream>

using namespace std;

#include "ctest.h"

int main(int argc, char* argv[])
{
    cout << "Command line arguments\n";
    
    for (int i = 0; i < argc; i ++)
    {
        cout << "Argument[" << i << "]: " << argv[i] << "\n";
    }
    
    cout << "Name: " << name << "\n";
}