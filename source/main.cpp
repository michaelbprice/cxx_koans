#include "server/CxxKoanServer.hpp"

#include <iostream>

using namespace std;

int main ()
{
    cxxkoans::Server server("http://172.18.83.21:8180", "../www", "../koans");

    server.start();

    std::string line;
    std::getline(std::cin, line);

    server.stop();

    return 0;
}

