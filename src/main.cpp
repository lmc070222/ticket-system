#include "/home/lmc123456/Ticket-System-2026/include/paser.hpp" 
#include <iostream>

int main() {
   
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    paser cmd_parser;
    while (cmd_parser.readcommand()) {
        cmd_parser.exucute(); 
    }

    return 0;
}