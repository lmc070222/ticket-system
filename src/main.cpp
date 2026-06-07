#include "paser.hpp" 
#include <iostream>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    paser* cmd_parser = new paser();
    while (cmd_parser->readcommand()) {
        cmd_parser->exucute(); 
    }
    delete cmd_parser;
    
    return 0;
}