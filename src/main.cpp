#include <Memory.hpp>
#include <cpu.hpp>
#include <iostream>

int main() {
  Memory mem;
  mem.Load_ROM();
  CPU cpu(mem);

  cpu.run();
}