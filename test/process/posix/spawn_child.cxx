#include <iostream>
#include <string>

int main()
{
    std::cerr << "READY" << std::endl;
    std::string input;
    std::getline(std::cin, input);
    std::cout << input << "BAR" << std::endl;
    return 0;
}
