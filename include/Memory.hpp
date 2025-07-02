#pragma once
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <sstream>
#include <fstream>

class Memory
{
public:
    uint8_t *mem;
    uint16_t ROM_END = 0x1FFF;
    uint16_t STACK_START = 0xFF00;

    Memory()
    {
        mem = new uint8_t[64 * 1024];
    };

    uint8_t read(uint16_t address)
    {
        if (address > 0xffff)
        {
            std::cout << "Memory Out of Bounds" << std::endl;
        }
        else
        {
            std::cout << std::bitset<8>(mem[address]) << std::endl;
            return mem[address];
        }
    };

    void write(uint16_t address, uint16_t value)
    {
        if (address <= ROM_END)
        {
            std::cout << "Can't write here" << std::endl;
        }
        else
        {
            mem[address] = value;
        }
    }

    void Load_ROM()
    {

        std::ifstream file("../8080EXM.COM", std::ios::binary);

        if (!file)
        {
            std::cout << "Can't Open File" << std::endl;
        }

        file.seekg(0, std::ios::end);
        const auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        uint16_t address = 0x00;
        uint8_t byte;

        if (size > ROM_END)
        {
            std::cout << "File Size too Big Get a Smaller ROM" << std::endl;
        }
        else
        {
            while (file.read(reinterpret_cast<char *>(&byte), 1))
            {
                mem[address] = byte;
                address++;
            }

            std::cout << "ROM Loading Done" << std::endl;
        }
    }
};