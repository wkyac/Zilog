#ifndef DISASSEMBLER_HPP
#define DISASSEMBLER_HPP

class Disassembler {
    public:
      Disassembler(); 
      int disassemble(unsigned char* buffer, int pc);
      void foo();
    private:
      int test = 1;  
};
#endif
