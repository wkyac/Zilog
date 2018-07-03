#include <iostream>
#include <fstream>
#include <string>

#include "Disassembler.hpp"

int counter;
uint16_t pc;

struct Flags {    
    uint8_t    z:1;     // Zero - Set if result is 0
    uint8_t    s:1;     // Sign - Set if result is negative
    uint8_t    pv:1;    // Parity/Overflow - Set if number of 1 bits in result is even. pv 1 = pe even, pv 0 = po odd.
    uint8_t    c:1;     // Carry - Set if last add/sub resulted in a carry/borrow
    uint8_t    hc:1;    // Half-Carry - Set if Binary-Coded Decimal arithmetic is used
    uint8_t    n:1;     // Add/Subtract - Used in DAA instruction. 0 = add, 1 = sub
};

struct State {
    // Main registers
    uint8_t     a;         
    struct      Flags flags;
    uint8_t     b;               
    uint8_t     c;
    uint8_t     d;
    uint8_t     e;
    uint8_t     h;
    uint8_t     l;
    uint16_t    bc; 
    uint16_t    de;
    uint16_t    hl;

    // Alternate Registers
    uint8_t     a_prime;
    struct      Flags flags_prime;
    uint8_t     b_prime;
    uint8_t     c_prime;
    uint8_t     d_prime;
    uint8_t     e_prime;
    uint8_t     h_prime;
    uint8_t     l_prime;
    uint16_t    bc_prime;
    uint16_t    de_prime;
    uint16_t    hl_prime;

    // Special-Purpose Registers
    uint16_t    sp;         // Stack pointer
    uint16_t    pc;         // Program counter
    uint8_t     ix;         // Index registers
    uint8_t     iy;         
    uint16_t    i;          // Interrupt register
    uint8_t     r;          // Memory-refresh register

    uint8_t     *memory;    // Loc of memory
    uint32_t    mem_size = 0x10000;
};

// z80 functions
int disassemble_file();
int load_file(State* state);
int emulate(State *state);
State* z80init(void);
void bad_inst(State *state);
void clearmem(State *state);
void printmem(State *state);
void helptext();
int reset(State *state);

// Auxilliary Emulator functions
uint8_t flagstoInt(State *state) {
    uint8_t a = 0;
    a |= state->flags.c << 0;
    a |= state->flags.n << 1;
    a |= state->flags.pv << 2;
    a |= state->flags.hc << 4;
    a |= state->flags.z << 6;
    a |= state->flags.s << 7;
    return a;
}

void inttoFlags(State *state, uint8_t f) {
    state->flags.c = (f >> 0);
    state->flags.n = (f >> 1);
    state->flags.pv = (f >> 2);
    state->flags.hc = (f >> 4);
    state->flags.z = (f >> 6);
    state->flags.s = (f >> 7);
}

// Z80 opcodes
// Load Group
uint8_t ld8(uint8_t r, unsigned char op1) { r = op1; return r; }
uint16_t ld16(uint16_t r, unsigned char op1, unsigned char op2) { return r = (op1 << 8) | op2; }
uint16_t ld16(uint16_t rmem, uint16_t r) { return rmem = r; }

void push(uint8_t &r1, uint8_t &r2, State *state) {
    state->sp -= 1;
    state->memory[state->sp] = r1;
    state->sp -= 1;
    state->memory[state->sp] = r2;
}

void push(uint8_t &r1, State *state) {
    state->sp -= 1;
    state->memory[state->sp] = r1;
    state->sp -= 1;
    state->memory[state->sp] = flagstoInt(state);
}

void pop(uint8_t &r1, uint8_t &r2, State *state) {
    r1 = state->memory[state->sp];
    state->sp += 1;
    r2 = state->memory[state->sp];
    state->sp += 1;
}

void pop(uint8_t &r1,  State *state) {
    r1 = state->memory[state->sp];
    state->sp += 1;
    uint8_t f = state->memory[state->sp];
    inttoFlags(state, f);
    state->sp += 1;
}

// Exchange, Block Transfer, and Search Group
void ex(uint16_t &r1, uint16_t &r2) {
    uint16_t t = r1;
    r1 = r2;
    r2 = t;
}

void ex(uint8_t &r1, uint16_t &r2) {
    uint8_t array[2];
    array[0]=r2 & 0xff;  // lsb
    array[1]=(r2 >> 8);  // msb
    
    r2 = ((r1 << 8) | (r1+1));
    r1 = array[0];
    r1++;
    r1 = array[1];
}

void ex(State *state) {
    Flags t = state->flags;
    state->flags = state->flags_prime;
    state->flags_prime = t;
}

void exx(State *state) {
    uint16_t temp_bc = state->bc;
    uint16_t temp_de = state->de;
    uint16_t temp_hl = state->hl;

    state->bc = state->bc_prime;
    state->de = state->de_prime;
    state->hl = state->hl_prime;

    state->bc_prime = temp_bc;
    state->de_prime = temp_de;
    state->hl_prime = temp_hl;
}

// General-Purpose Arithmetic and CPU Control Groups
void daa();
void cpl();
void ccf();
void scf();
void di();
void ei();

// Arithmetical and Logical
uint8_t add8(uint8_t r1, uint8_t r2) { return r1 += r2; }
uint16_t add16(uint16_t r1, uint16_t r2) { return r1 += r2; }
uint8_t adc(uint8_t r1, uint8_t r2, uint8_t c) { return r1 + r2 + c; }
uint8_t sub(uint8_t r1, uint8_t r2) { return r1 - r2; }
uint8_t sbc(uint8_t r1, uint8_t r2, uint8_t c) { return r1 - r2 - c; }
uint8_t _and(uint8_t r1, uint8_t r2) { return r1 & r2; }
uint8_t _xor(uint8_t r1, uint8_t r2) { return r1 ^ r2; }
uint8_t _or(uint8_t r1, uint8_t r2) { return r1 | r2; }
void cp(uint8_t r1, uint8_t r2, State *state) { if (r1 == r2 ) { state->flags.z=1; } }
uint8_t inc8(uint8_t r1) { return r1 += 1; }
uint16_t inc16(uint16_t r1) { return r1 += 1; }
uint8_t dec8(uint8_t r1) { return r1 -= 1; }
uint16_t dec16(uint16_t r1) { return r1 -= 1; }


// Rotate and Shift
void rrca(State *state) { uint8_t tempc = state->a & 1; state->a = ((state->a >> 1 ) | (tempc << 7)); }
void rra(State *state) { }
void rla(State *state) { }
void rlca(State *state) { uint8_t tempc = state->a >> 7; state->a = ((state->a << 1 ) | tempc); }
void srl();

// Jump, Call, and Return
void jrc(uint8_t flag, unsigned char op1, State *state) {
    if(flag == 1)
        state->pc += (signed char)op1;
}
void jrnc(uint8_t flag, unsigned char op1, State *state) {
    if(flag == 0)
        state->pc += (signed char)op1;
}

void jrz(uint8_t flag, unsigned char op1, State *state) {
    if(flag == 1)
        state->pc += (signed char)op1;
}

void jrnz(uint8_t flag, unsigned char op1, State *state) {
    if(flag == 0)
        state->pc += (signed char)op1;
}

void jr(unsigned char op1, State *state) {
    state->pc += (signed char)op1;
}

uint16_t jp(unsigned char op1, unsigned char op2) {
    return (uint16_t)((op1 << 8) | op2);
    
   
}

void jp_cc(unsigned char opcode, unsigned char op1, unsigned char op2, State *state) {
    switch (opcode) {
        case 0xC2: if(state->flags.z == 0) state->pc = ((op2 << 8) | op1); break;   // nz    
        case 0xCA: if(state->flags.z == 1) state->pc = ((op2 << 8) | op1); break;   // z
        case 0xD2: if(state->flags.c == 0) state->pc = ((op2 << 8) | op1); break;   // nc , nc is 0
        case 0xDA: if(state->flags.c == 1) state->pc = ((op2 << 8) | op1); break;   // c  , c is 1
        case 0xE2: if(state->flags.pv == 0) state->pc = ((op2 << 8) | op1); break;   // po , po is odd  0
        case 0xEA: if(state->flags.pv == 0) state->pc = ((op2 << 8) | op1); break;   // pe , pe is even 1
        case 0xF2: if(state->flags.s == 0) state->pc = ((op2 << 8) | op1);break;   // p 
        case 0xFA: if(state->flags.s == 0) state->pc = ((op2 << 8) | op1);break;   // m
    }
}

void call(unsigned char opcode, unsigned char op1, unsigned char op2, State *state) {
    uint16_t value = state->pc;
    uint8_t array[2];
    uint8_t  pcl = array[0]=value & 0xff;  // lsb
    uint8_t  pch = array[1]=(value >> 8);  // msb
    switch (opcode) {   
        case 0xC4: if (state->flags.z == 0) { 
                       state->memory[(state->sp-1)] = pch; 
                       state->memory[(state->sp-2)] = pcl; 
                       state->pc = ((op1 << 8) | op2); } break; // nz
        case 0xCC: if (state->flags.z == 1) { 
                       state->memory[(state->sp-1)] = pch;
                       state->memory[(state->sp-2)] = pcl; 
                       state->pc = ((op1 << 8) | op2); } break; // z
        case 0xD4: if (state->flags.c == 0) { 
                       state->memory[(state->sp-1)] = pch;
                       state->memory[(state->sp-2)] = pcl; 
                       state->pc = ((op1 << 8) | op2); } break; // nc
        case 0xDC: if (state->flags.c == 1) { 
                       state->memory[(state->sp-1)] = pch;
                       state->memory[(state->sp-2)] = pcl; 
                       state->pc = ((op1 << 8) | op2); } break; // c
        case 0xE4: if (state->flags.pv == 0) {
                       state->memory[(state->sp-1)] = pch;
                       state->memory[(state->sp-2)] = pcl; 
                       state->pc = ((op1 << 8) | op2); } break; // po
        case 0xEC: if (state->flags.pv == 1) {
                       state->memory[(state->sp-1)] = pch;
                       state->memory[(state->sp-2)] = pcl; 
                       state->pc = ((op1 << 8) | op2); } break; // pe
        case 0xF4: if (state->flags.s == 0) {
                       state->memory[(state->sp-1)] = pch;
                       state->memory[(state->sp-2)] = pcl; 
                       state->pc = ((op1 << 8) | op2); } break; // p
        case 0xFC: if (state->flags.s == 1) {
                       state->memory[(state->sp-1)] = pch;
                       state->memory[(state->sp-2)] = pcl; 
                       state->pc = ((op1 << 8) | op2); } break; // m
        case 0xCD:     state->memory[(state->sp-1)] = pch;
                       state->memory[(state->sp-2)] = pcl;
                       state->pc = ((op1 << 8) | op2); break;   // unconditional 
    }
}
void djnz(State *state, signed char jump) {
    state->b -= 1;
    if (state->b != 0)
        state->pc += jump;
}


uint16_t ret(uint16_t pc, State *state) {
    uint8_t pcl = state->memory[state->sp];
    uint8_t pch = state->memory[state->sp+1];
    pc = ((pcl << 8) | pch);
    return pc;
}
uint16_t ret_nz(uint8_t z, State *state) {
    if (z == 0) {
        uint8_t pcl = state->memory[state->sp];
        uint8_t pch = state->memory[state->sp+1];
        state->pc = ((pcl << 8) | pch);
        state->pc += 1;
    }
    return state->pc;
}
uint16_t ret_z(uint8_t z, State *state) {
    if (z == 1) {
        uint8_t pcl = state->memory[state->sp];
        uint8_t pch = state->memory[state->sp+1];
        state->pc = ((pcl << 8) | pch);
        state->pc += 1;
    }
    return state->pc;
}
uint16_t ret_nc(uint8_t c, State *state) {
    if (c == 0) {
        uint8_t pcl = state->memory[state->sp];
        uint8_t pch = state->memory[state->sp+1];
        state->pc = ((pcl << 8) | pch);
        state->pc += 1;
    }
    return state->pc;
}
uint16_t ret_c(uint8_t c, State *state) {
    if (c == 1) {
        uint8_t pcl = state->memory[state->sp];
        uint8_t pch = state->memory[state->sp+1];
        state->pc = ((pcl << 8) | pch);
        state->pc += 1;
    }
    return state->pc;
}
uint16_t ret_po(uint8_t pv, State *state) {
    if (pv == 0) {
        uint8_t pcl = state->memory[state->sp];
        uint8_t pch = state->memory[state->sp+1];
        state->pc = ((pcl << 8) | pch);
        state->pc += 1;
    }
    return state->pc;
}
uint16_t ret_pe(uint8_t pv, State *state) {
    if (pv == 1) {
        uint8_t pcl = state->memory[state->sp];
        uint8_t pch = state->memory[state->sp+1];
        state->pc = ((pcl << 8) | pch);
        state->pc += 1;
    }
    return state->pc;
}
uint16_t ret_p(uint8_t s, State *state) {
    if (s == 0) {
        uint8_t pcl = state->memory[state->sp];
        uint8_t pch = state->memory[state->sp+1];
        state->pc = ((pcl << 8) | pch);
        state->pc += 1;
    }
    return state->pc;
}
uint16_t ret_m(uint8_t s, State *state) {
    if (s == 1) {
        uint8_t pcl = state->memory[state->sp];
        uint8_t pch = state->memory[state->sp+1];
        state->pc = ((pcl << 8) | pch);
        state->pc += 1;
    }
    return state->pc;
}


void rst(unsigned char opcode, State *state) {
    uint16_t value = state->pc;
    uint8_t  array[2];
    uint8_t  pcl = array[0]=value & 0xff;  // lsb
    uint8_t  pch = array[1]=(value >> 8);  // msb

    switch(opcode) {
        case 0xC7: state->memory[(state->sp-1)] = pch;
                   state->memory[(state->sp-2)] = pcl; 
                   pch = 0x00;
                   pcl = 0x00;
                   break;
        case 0xCF: state->memory[(state->sp-1)] = pch;
                   state->memory[(state->sp-2)] = pcl;
                   pch = 0x00;
                   pcl = 0x08;
                   break;
        case 0xD7: state->memory[(state->sp-1)] = pch;
                   state->memory[(state->sp-2)] = pcl;
                   pch = 0x00;
                   pcl = 0x10;
                   break;
        case 0xDF: state->memory[(state->sp-1)] = pch;
                   state->memory[(state->sp-2)] = pcl;
                   pch = 0x00;
                   pcl = 0x18;
                   break;
        case 0xE7: state->memory[(state->sp-1)] = pch;
                   state->memory[(state->sp-2)] = pcl;
                   pch = 0x00;
                   pcl = 0x20;
                   break;
        case 0xEF: state->memory[(state->sp-1)] = pch;
                   state->memory[(state->sp-2)] = pcl;
                   pch = 0x00;
                   pcl = 0x28;
                   break;
        case 0xF7: state->memory[(state->sp-1)] = pch;
                   state->memory[(state->sp-2)] = pcl;
                   pch = 0x00;
                   pcl = 0x30;
                   break;
        case 0xFF: state->memory[(state->sp-1)] = pch;
                   state->memory[(state->sp-2)] = pcl;
                   pch = 0x00;
                   pcl = 0x38;
                   break;
    }
    state->pc = ((pcl << 8) | pch);
}

// CPU Control
void nop();
void halt();


// User prompts
enum Actions {
    EXIT,
    HELP,
    DISASSEMBLE,
    LOAD_PGRM,
    CLEAR_MEM,
    PRINT_MEM,
    RUN,
    RESET
};

int main(int argc, char* argv[]) {
  
    int done = 0;
    State* state = z80init();
    std::cout << "Z80 State Initialized" << std::endl;
    std::cout << state->mem_size << "KB Available" << std::endl;
    std::cout << "Welcome. For help, enter \"help\"." << std::endl;
    Actions a; 
    
    state->memory[0x00100] = 0x76;

    // Prompt Loop
    do {  
        std::cout << ">";
        std::string prompt;
        std::cin >> prompt;

        if(prompt == "exit") {a = EXIT;}
        else if (prompt == "help") {a = HELP;}
        else if (prompt == "disassemble") {a = DISASSEMBLE;}
        else if (prompt == "load") {a = LOAD_PGRM;}
        else if (prompt == "printmem") {a = PRINT_MEM;}
        else if (prompt == "clearmem") {a = CLEAR_MEM;}
        else if (prompt == "run") {a = RUN;}
        else if (prompt == "reset") {a = RESET;}
        else { std::cout << "Enter \"help\" for commands." << std::endl; }
    
        switch(a) {
            case EXIT: break;
            case HELP: helptext(); break;
            case DISASSEMBLE: disassemble_file(); break;
            case LOAD_PGRM: load_file(state); break;
            case PRINT_MEM: std::cout << "print_mem" << std::endl; break;
            case CLEAR_MEM: clearmem(state); break;
            case RESET: done = reset(state); break;
            case RUN: 
                        while(done == 0)
                            done = emulate(state);
            default: break;
        }
        
    } while (a != EXIT);
    
    /*
    // Insert a program into memory
    //readfiletomem(state, filename, 0x00000);
    // Insert a value at an address in memory
    //state->memory[0x00100] = 0x76;
    while (done == 0)
        done = emulate(state);
    }
    */
    return 0;
}

// Emulation loop
int emulate(State* state) {
    int halt = 0;
    unsigned char *opcode = &state->memory[state->pc];
    printf("%02x %02x \n", state->pc, *opcode);
    switch(*opcode) {
        case 0x00: break;                          
        case 0x01: state->bc = ld16(state->bc, opcode[1], opcode[2]); state->pc += 2; break;        
        case 0x02: state->memory[state->bc] = ld8(state->memory[state->bc], state->a); break; 
        case 0x03: state->bc = inc16(state->bc); break;                           
        case 0x04: state->b = inc8(state->b); break;                          
        case 0x05: state->b = dec8(state->b); break;                           
        case 0x06: state->b = ld8(state->b, opcode[1]); break;
        case 0x07: rlca(state); break;
        case 0x08: ex(state); break;         
        case 0x09: state->hl = add16(state->hl, state->bc); break;                          
        case 0x0A: state->a = ld16(state->a, state->memory[state->bc]); break;    
        case 0x0B: state->bc = dec16(state->bc); break;                           
        case 0x0C: state-> c = inc8(state->c); break;                          
        case 0x0D: state->c = dec8(state->c); break;                          
        case 0x0E: state->c = ld8(state->c, opcode[1]); break;                           
        case 0x0F: rrca(state); break;   // rrca
   
        case 0x10: djnz(state, opcode[1]); break;                     // djnz *
        case 0x11: state->de = ld16(state->de, opcode[1], opcode[2]); state->pc += 2; break; 
        case 0x12: state->memory[state->de] = ld8(state->memory[state->de], state->a); 
        case 0x13: state->de = inc16(state->de); break;
        case 0x14: state->d = inc8(state->d); break;
        case 0x15: state->d = dec8(state->d); break;
        case 0x16: state->d = ld8(state->d, opcode[1]); break;
        case 0x17: rla(state); break;                                // rla
        case 0x18: jr(opcode[1], state); break;                          
        case 0x19: state->hl = add16(state->hl, state->de); break;        
        case 0x1A: state->a = ld8(state->a, state->memory[state->de]); break;                   
        case 0x1B: state->de = dec16(state->de); break;
        case 0x1C: state->e = inc8(state->e); break;
        case 0x1D: state->e = dec8(state->e); break;
        case 0x1E: state->e = ld8(state->e, opcode[1]); break;
        case 0x1F: rra(state); break;          //rra
        
        case 0x20: jrnz(state->flags.z, opcode[1], state); break;
        case 0x21: state->hl = ld16(state->hl, opcode[1], opcode[2]); state->pc += 2; break;
        case 0x22: state->memory[((opcode[1] << 8) |opcode[2])] = ld16(state->memory[((opcode[2] << 8) |opcode[1])], state->hl); break;
        case 0x23: state->hl = inc16(state->hl); break;
        case 0x24: state->h = inc8(state->h); break;
        case 0x25: state->h = dec8(state->h); break;
        case 0x26: state->h = ld8(state->h, opcode[1]); break;
        case 0x27: bad_inst(state); break;                                // daa
        case 0x28: jrz(state->flags.z, opcode[1], state); break;                          
        case 0x29: state->hl = add16(state->hl, state->hl); break;                           
        case 0x2A: state->hl = ld16(state->hl, state->memory[((opcode[2] << 8) |opcode[1])]); break;
        case 0x2B: state->hl = dec16(state->hl); break;
        case 0x2C: state->l = inc8(state->l); break;
        case 0x2D: state->l = dec8(state->l); break;
        case 0x2E: state->l = ld8(state->l, opcode[1]); break;
        case 0x2F: bad_inst(state); break;                                // cpl
        
        case 0x30: jrnc(state->flags.c, opcode[1], state);  break;
        case 0x31: state->sp = ld16(state->sp, opcode[1], opcode[2]); break;
        case 0x32: state->memory[((opcode[2] << 8) |opcode[1])] = ld8(state->memory[((opcode[2] << 8) |opcode[1])], state->a);
        case 0x33: state->sp = inc16(state->sp); break;                           
        case 0x34: state->hl = inc16(state->memory[state->hl]); break;
        case 0x35: state->hl = dec16(state->memory[state->hl]); break;
        case 0x36: state->hl = ld8(state->h, opcode[1]); break;          
        case 0x37: bad_inst(state);  break;                               // scf
        case 0x38: jrc(state->pc, opcode[1], state); break;
        case 0x39: state->hl = add16(state->hl, state->sp); break;
        case 0x3A: state->a = ld8(state->a, state->memory[((opcode[2] << 8) |opcode[1])]); break;
        case 0x3B: state->sp = dec16(state->sp); break;
        case 0x3C: state->a = inc8(state->a); break;
        case 0x3D: state->a = dec8(state->a); break;
        case 0x3E: state->a = ld8(state->a, opcode[1]); break;
        case 0x3F: bad_inst(state);  break;                               // ccf
        
        case 0x40: state->b = ld8(state->b, state->b); break;
        case 0x41: state->b = ld8(state->b, state->c); break;
        case 0x42: state->b = ld8(state->b, state->d); break;
        case 0x43: state->b = ld8(state->b, state->e); break;
        case 0x44: state->b = ld8(state->b, state->h); break;
        case 0x45: state->b = ld8(state->b, state->l); break;
        case 0x46: state->b = ld8(state->b, state->memory[state->hl]); break;
        case 0x47: state->b = ld8(state->b, state->a); break;
        case 0x48: state->c = ld8(state->c, state->b); break;
        case 0x49: state->c = ld8(state->c, state->c); break;
        case 0x4A: state->c = ld8(state->c, state->d); break;
        case 0x4B: state->c = ld8(state->c, state->e); break;
        case 0x4C: state->c = ld8(state->c, state->h); break;
        case 0x4D: state->c = ld8(state->c, state->l); break;
        case 0x4E: state->c = ld8(state->b, state->memory[state->hl]); break;
        case 0x4F: state->c = ld8(state->c, state->a); break;
        
        case 0x50: state->d = ld8(state->d, state->b); break;
        case 0x51: state->d = ld8(state->d, state->c); break;
        case 0x52: state->d = ld8(state->d, state->d); break;
        case 0x53: state->d = ld8(state->d, state->e); break;
        case 0x54: state->d = ld8(state->d, state->h); break;
        case 0x55: state->d = ld8(state->d, state->l); break;
        case 0x56: state->d = ld8(state->d, state->memory[state->hl]); break;
        case 0x57: state->d = ld8(state->d, state->a); break;
        case 0x58: state->e = ld8(state->e, state->b); break;
        case 0x59: state->e = ld8(state->e, state->c); break;
        case 0x5A: state->e = ld8(state->e, state->d); break;
        case 0x5B: state->e = ld8(state->e, state->e); break;
        case 0x5C: state->e = ld8(state->e, state->h); break;
        case 0x5D: state->e = ld8(state->e, state->l); break;
        case 0x5E: state->e = ld8(state->e, state->memory[state->hl]); break;
        case 0x5F: state->e = ld8(state->e, state->a); break;
        
        case 0x60: state->h = ld8(state->h, state->b); break;
        case 0x61: state->h = ld8(state->h, state->c); break;
        case 0x62: state->h = ld8(state->h, state->d); break;
        case 0x63: state->h = ld8(state->h, state->e); break;
        case 0x64: state->h = ld8(state->h, state->h); break;
        case 0x65: state->h = ld8(state->h, state->l); break;
        case 0x66: state->h = ld8(state->h, state->memory[state->hl]); break;
        case 0x67: state->h = ld8(state->h, state->a); break;
        case 0x68: state->l = ld8(state->l, state->b); break;
        case 0x69: state->l = ld8(state->l, state->c); break;
        case 0x6A: state->l = ld8(state->l, state->d); break;
        case 0x6B: state->l = ld8(state->l, state->e); break;
        case 0x6C: state->l = ld8(state->l, state->h); break;
        case 0x6D: state->l = ld8(state->l, state->l); break;
        case 0x6E: state->l = ld8(state->l, state->memory[state->hl]); break;
        case 0x6F: state->l = ld8(state->l, state->a); break;
        
        case 0x70: state->memory[state->hl] = ld8(state->memory[state->hl], state->b); break;
        case 0x71: state->memory[state->hl] = ld8(state->memory[state->hl], state->c); break;
        case 0x72: state->memory[state->hl] = ld8(state->memory[state->hl], state->d); break;
        case 0x73: state->memory[state->hl] = ld8(state->memory[state->hl], state->e); break;
        case 0x74: state->memory[state->hl] = ld8(state->memory[state->hl], state->h); break;
        case 0x75: state->memory[state->hl] = ld8(state->memory[state->hl], state->l); break;
        case 0x76: halt = 1; break;
        case 0x77: state->memory[state->hl] = ld8(state->memory[state->hl], state->a); break;
        case 0x78: state->a = ld8(state->a, state->b); break;
        case 0x79: state->a = ld8(state->a, state->c); break;
        case 0x7A: state->a = ld8(state->a, state->d); break;
        case 0x7B: state->a = ld8(state->a, state->e); break;
        case 0x7C: state->a = ld8(state->a, state->h); break;
        case 0x7D: state->a = ld8(state->a, state->l); break;
        case 0x7E: state->a = ld8(state->a, state->memory[state->hl]); break;
        case 0x7F: state->a = ld8(state->a, state->a); break;
                   
        case 0x80: state->a = add8(state->a,state->b); break;
        case 0x81: state->a = add8(state->a,state->c); break;
        case 0x82: state->a = add8(state->a,state->d); break;
        case 0x83: state->a = add8(state->a,state->e); break;
        case 0x84: state->a = add8(state->a,state->h); break;
        case 0x85: state->a = add8(state->a,state->l); break;
        case 0x86: state->a = add8(state->a, state->memory[state->hl]); break;
        case 0x87: state->a = add8(state->a,state->a); break;
        case 0x88: state->a = adc(state->a, state->b, state->flags.c); break;
        case 0x89: state->a = adc(state->a, state->c, state->flags.c); break;
        case 0x8A: state->a = adc(state->a, state->d, state->flags.c); break;
        case 0x8B: state->a = adc(state->a, state->e, state->flags.c); break;
        case 0x8C: state->a = adc(state->a, state->h, state->flags.c); break;
        case 0x8D: state->a = adc(state->a, state->l, state->flags.c); break;
        case 0x8E: state->a = adc(state->a, state->memory[state->hl], state->flags.c); break;
        case 0x8F: state->a = adc(state->a, state->a, state->flags.c); break;
        
        case 0x90: state->a = sub(state->a, state->b); break;
        case 0x91: state->a = sub(state->a, state->c); break;
        case 0x92: state->a = sub(state->a, state->d); break;
        case 0x93: state->a = sub(state->a, state->e); break;
        case 0x94: state->a = sub(state->a, state->h); break;
        case 0x95: state->a = sub(state->a, state->l); break;
        case 0x96: state->a = sub(state->a, state->memory[state->hl]); break;       
        case 0x97: state->a = sub(state->a, state->a); break;
        case 0x98: state->a = sbc(state->a, state->b, state->flags.c); break;
        case 0x99: state->a = sbc(state->a, state->c, state->flags.c); break;
        case 0x9A: state->a = sbc(state->a, state->d, state->flags.c); break;
        case 0x9B: state->a = sbc(state->a, state->e, state->flags.c); break;
        case 0x9C: state->a = sbc(state->a, state->h, state->flags.c); break;
        case 0x9D: state->a = sbc(state->a, state->l, state->flags.c); break;
        case 0x9E: state->a = sbc(state->a, state->memory[state->hl], state->flags.c); break;         
        case 0x9F: state->a = sbc(state->a, state->a, state->flags.c); break;
        
        case 0xA0: state->a = _and(state->a, state->b); break;
        case 0xA1: state->a = _and(state->a, state->c); break;
        case 0xA2: state->a = _and(state->a, state->d); break;
        case 0xA3: state->a = _and(state->a, state->e); break;
        case 0xA4: state->a = _and(state->a, state->h); break;
        case 0xA5: state->a = _and(state->a, state->l); break;
        case 0xA6: state->a = _and(state->a, state->memory[state->hl]); break;
        case 0xA7: state->a = _and(state->a, state->a); break;
        case 0xA8: state->a = _xor(state->a, state->b); break;
        case 0xA9: state->a = _xor(state->a, state->c); break;
        case 0xAA: state->a = _xor(state->a, state->d); break;
        case 0xAB: state->a = _xor(state->a, state->e); break;
        case 0xAC: state->a = _xor(state->a, state->h); break;
        case 0xAD: state->a = _xor(state->a, state->l); break;
        case 0xAE: state->a = _xor(state->a, state->memory[state->hl]); break;      
        case 0xAF: state->a = _xor(state->a, state->a); break;
        
        case 0xB0: state->a = _or(state->a, state->b); break;
        case 0xB1: state->a = _or(state->a, state->c); break;
        case 0xB2: state->a = _or(state->a, state->d); break;
        case 0xB3: state->a = _or(state->a, state->e); break;
        case 0xB4: state->a = _or(state->a, state->h); break;
        case 0xB5: state->a = _or(state->a, state->l); break;
        case 0xB6: state->a = _or(state->a, state->memory[state->hl]); break;
        case 0xB7: state->a = _or(state->a, state->a); break;
        case 0xB8: cp(state->a, state->b, state); break;
        case 0xB9: cp(state->a, state->c, state); break;
        case 0xBA: cp(state->a, state->d, state); break;
        case 0xBB: cp(state->a, state->e, state); break;
        case 0xBC: cp(state->a, state->h, state); break;
        case 0xBD: cp(state->a, state->l, state); break;
        case 0xBE: cp(state->a, state->memory[state->hl], state); break;
        case 0xBF: cp(state->a, state->a, state); break;
       
        case 0xC0: state->pc = ret_nz(state->flags.z, state); break;
        case 0xC1: pop(state->b, state->c, state); break;
        case 0xC2: jp_cc(*opcode, opcode[1], opcode[2], state); break;
        case 0xC3: state->pc = jp(opcode[1], opcode[2]); break;
        case 0xC4: call(*opcode, opcode[1], opcode[2], state); break;      
        case 0xC5: push(state->b, state->c, state); break;
        case 0xC6: state->a = add8(state->a, opcode[1]); break;
        case 0xC7: rst(*opcode, state); break;                         
        case 0xC8: state->pc = ret_z(state->flags.z, state); break;
        case 0xC9: state->pc = ret(state->pc, state); break;
        case 0xCA: jp_cc(*opcode, opcode[1], opcode[2], state); break;
        case 0xCB: std::cout << "BITS"; break;
        case 0xCC: call(*opcode, opcode[1], opcode[2], state); break;
        case 0xCD: call(*opcode, opcode[1], opcode[2], state); break;
        case 0xCE: state->a = adc(state->a, opcode[1], state->flags.c); break;
        case 0xCF: rst(*opcode, state); break;          
        
        case 0xD0: state->pc = ret_nc(state->flags.c, state); break;
        case 0xD1: pop(state->d, state->e, state); break;
        case 0xD2: jp_cc(*opcode, opcode[1], opcode[2], state); break;
        //case 0xD3: bad_inst(state); break;            // out (*) a
        case 0xD4: call(*opcode, opcode[1], opcode[2], state); break;            
        case 0xD5: push(state->d, state->e, state); break;
        case 0xD6: state->a = sub(state->a, opcode[1]); break;
        case 0xD7: rst(*opcode, state); break;            
        case 0xD8: state->pc = ret_c(state->flags.c, state); break;
        case 0xD9: exx(state); break;            //exx
        case 0xDA: jp_cc(*opcode, opcode[1], opcode[2], state); break;
        //case 0xDB: bad_inst(state); break;            // in a (*)
        case 0xDC: call(*opcode, opcode[1], opcode[2], state); break;
        case 0xDD: std::cout << "IX"; break;
        case 0xDE: state->a = sbc(state->a, opcode[1], state->flags.c); break;
        case 0xDF: rst(*opcode, state); break;            

        case 0xE0: state->pc = ret_po(state->flags.pv, state); break;
        case 0xE1: pop(state->h, state->l, state); break;
        case 0xE2: jp_cc(*opcode, opcode[1], opcode[2], state); break;
        case 0xE3: ex(state->memory[state->sp], state->hl); break;
        case 0xE4: call(*opcode, opcode[1], opcode[2], state); break;
        case 0xE5: push(state->h, state->l, state); break;
        case 0xE6: state->a = _and(state->a, opcode[1]); break;
        case 0xE7: rst(*opcode, state); break;            
        case 0xE8: state->pc = ret_po(state->flags.pv, state); break;
        case 0xE9: bad_inst(state); break;
        case 0xEA: jp_cc(*opcode, opcode[1], opcode[2], state); break;
        case 0xEB: ex(state->de, state->hl); break;
        case 0xEC: call(*opcode, opcode[1], opcode[2], state); break;
        case 0xED: std::cout << "EXTD"; break;
        case 0xEE: state->a = _xor(state->a, opcode[1]); break;
        case 0xEF: rst(*opcode, state); break;            
        
        case 0xF0: state->pc = ret_p(state->flags.s, state); break;
        case 0xF1: pop(state->a, state); break;
        case 0xF2: jp_cc(*opcode, opcode[1], opcode[2], state); break;
        //case 0xF3: bad_inst(state); break;            // di
        case 0xF4: call(*opcode, opcode[1], opcode[2], state); break;
        case 0xF5: push(state->a, state); break;
        case 0xF6: state->a = _or(state->a, opcode[1]); break;
        case 0xF7: rst(*opcode, state); break;            
        case 0xF8: state->pc = ret_m(state->flags.s, state); break;
        case 0xF9: state->sp = ld16(state->sp, state->hl); break;
        case 0xFA: jp_cc(*opcode, opcode[1], opcode[2], state); break;
        //case 0xFB: bad_inst(state); break;            // ei
        case 0xFC: call(*opcode, opcode[1], opcode[2], state); break;
        case 0xFD: std::cout << "IY"; break;
        case 0xFE: cp(state->a, opcode[1], state); break;
        case 0xFF: rst(*opcode, state); break;            
    }
    state->pc+=1;
    return halt;
}

// Initialize space for z80 states and memory
State* z80init(void) {
    State* state = (State*)calloc(1,sizeof(State));
    state->mem_size = 0x10000;
    state->memory = (uint8_t*)calloc(state->mem_size, sizeof(uint8_t));  //64kb
    return state;
}

void bad_inst(State* state) {
    state->pc--; 
    printf("Bad instruction\n");
    exit(1);
}

// Display the helptext

void helptext() {
    std::cout << "Available commands are: \n";
    std::cout << "\thelp\t\t -- Displays this message.\n";
    std::cout << "\tdisassemble\t -- Starts a prompt for disassembling a file.\n";
    std::cout << "\tload\t\t -- Loads a file into memory starting at the given position.\n";
    std::cout << "\tprintmem\t -- Displays an ncurses window of the current memory of the machine.\n";
    std::cout << "\tclearmem\t -- Zeroes out memory.\n";
    std::cout << "\trun\t\t -- Runs whatever is currently loaded into memory.\n";
    std::cout << "\treset\t\t -- Resets the program counter.\n";
    std::cout << "\texit\t\t -- Exits the program.\n";
}

// Dissassemble file
int disassemble_file() {
    Disassembler d;
    std::cout << "Please enter the name of the file you wish to disassemble: \n";
    std::string file;
    std::cin >> file;

    std::streampos begin,end, size;
    char * memblock;

    std::ifstream input(""+ file, std::ios::binary);
    begin = input.tellg();
    input.seekg(0,std::ios::end);
    end = input.tellg();
    size = end - begin;
    memblock = new char [size];
    input.seekg(0, std::ios::beg);
    input.read(memblock, size);
    
    std::cout << "Filesize is " + (end-begin) << std::endl;

    if (input.is_open()) {
    std::cout << file+" opened successfully."  << std::endl;
        pc = 0;
        while(pc < size) {
            pc += d.disassemble((unsigned char *)memblock,pc);
        }
    }
    input.close();
    std::cout << file+" closed successfully."  << std::endl;
    return 0;
}

int load_file(State *state) {
    std::string filename;
    std::string path = "../ROMS/";
    std::cout << "Please enter the name of the file you wish to load: ";
    std::cin >> filename;
    filename.insert(0, path);
    const char* cfilename = filename.c_str();

    std::string stroffset;
    std::cout << "Please enter in hex (e.g. 0x10) where you would like to load the file: ";
    std::cin >> stroffset;

    //str to uint32
    uint32_t offset = std::stoul(stroffset,nullptr,0);
    std::cout << offset;

    FILE *f= fopen(cfilename, "rb");
	if (f==NULL) {
		printf("error: Couldn't open %s\n", cfilename);
		return 1;
	}
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);

	uint8_t *buffer = &state->memory[offset];
	fread(buffer, fsize, 1, f);
	fclose(f);
    return 0;
}

void printmem(State *state) {
    for(int i = 0; i < 0x104; i++) {
        unsigned char c = ((char*)state->memory)[i];
        if(i % 40 == 0)
            std::cout << std::endl;
        printf("%02x ", c);
    }

    std::cout << std::endl;
}

void clearmem(State *state) {
    state->memory = (uint8_t*)calloc(state->mem_size, sizeof(uint8_t));
}

int reset(State *state) {
    state->pc = 0;
    return 0;
}
