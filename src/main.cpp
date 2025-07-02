#include <iostream>
#include <cpu.hpp>
#include <Memory.hpp>

int main()
{

    Memory memory;

    memory.Load_ROM();

    std::cout << "Frontend" << std::endl;
}