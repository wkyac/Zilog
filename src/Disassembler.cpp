#include "Disassembler.hpp"
#include <iostream>

Disassembler::Disassembler() {
    std::cout << "Disassembler Loaded" << std::endl;
}

/*
void Disassembler::foo() {
    std::cout << "kek" << std::endl;
}
*/

// Take byte data from a buffer and translate it into an opcode + data
int Disassembler::disassemble(unsigned char *buffer, int pc) {
    // buffer is a pointer to machine code in .h file
    // pc is current offset
    unsigned char *code = &buffer[pc];
    int opbytes = 1;
    printf("%04x ", pc);  
    printf("%x ",*code);
    switch(*code) {
        case 0x00: printf("nop"); break;    
        case 0x01: printf("ld    bc,#$%02x%02x", code[2], code[1]); opbytes=3; break;
        case 0x02: printf("ld    (bc),a"); break;
        case 0x03: printf("inc   bc"); break;
        case 0x04: printf("inc   b"); break;
        case 0x05: printf("dec   b"); break; 
        case 0x06: printf("ld    b,%02x", code[1]); opbytes=2; break; 
        case 0x07: printf("rlca"); break; 
        case 0x08: printf("ex    af,af'"); break; 
        case 0x09: printf("add   hl,bc"); break; 
        case 0x0A: printf("ld    a,(bc)"); break; 
        case 0x0B: printf("dec   bc"); break; 
        case 0x0C: printf("inc   c"); break; 
        case 0x0D: printf("dec   c"); break; 
        case 0x0E: printf("ld    c,%02x", code[1]); opbytes=2; break; 
        case 0x0F: printf("rrca"); break; 
        
        case 0x10: printf("djnz  %02x", code[1]); opbytes=2; break; 
        case 0x11: printf("ld    de,#$%02x%02x",code[2], code[1]); opbytes=3; break;
        case 0x12: printf("ld    (de),a"); break; 
        case 0x13: printf("inc   de"); break; 
        case 0x14: printf("inc   d"); break; 
        case 0x15: printf("dec   d"); break; 
        case 0x16: printf("ld    d,%02x", code[1]); opbytes=2; break; 
        case 0x17: printf("rla"); break; 
        case 0x18: printf("jr    %02x", code[1]); opbytes=2; break; 
        case 0x19: printf("add   hl,de"); break; 
        case 0x1A: printf("ld    a,(de)"); break; 
        case 0x1B: printf("dec   de"); break; 
        case 0x1C: printf("inc   e"); break; 
        case 0x1D: printf("inc   e"); break; 
        case 0x1E: printf("ld    e,%02x", code[1]); opbytes=2; break; 
        case 0x1F: printf("rra"); break; 
        
        case 0x20: printf("jr    nz,%02x", code[1]); opbytes=2; break; 
        case 0x21: printf("ld    hl,#$%02x%02x",code[2], code[1]); opbytes=3; break; 
        case 0x22: printf("id    (#$%02x%02x),hl",code[2],code[1]); opbytes=3; break; 
        case 0x23: printf("inc   hl"); break; 
        case 0x24: printf("inc   h"); break; 
        case 0x25: printf("dec   h"); break; 
        case 0x26: printf("ld    h,%02x", code[1]); opbytes=2; break; 
        case 0x27: printf("daa"); break; 
        case 0x28: printf("jr    z,%02x", code[1]); opbytes=2; break; 
        case 0x29: printf("add   hl,hl"); break; 
        case 0x2A: printf("ld    hl,(#$%02x%02x)", code[2], code[1]); opbytes=3; break; 
        case 0x2B: printf("dec   hl"); break; 
        case 0x2C: printf("inc   l"); break; 
        case 0x2D: printf("dec   l"); break; 
        case 0x2E: printf("ld    l,%02x", code[1]); opbytes=2; break; 
        case 0x2F: printf("cpl"); break; 
        
        case 0x30: printf("jr    nc,%02x", code[1]); opbytes=2; break;           
        case 0x31: printf("ld    sp, #$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0x32: printf("ld    (#$%02x%02x),a", code[2], code[1]); opbytes=3; break; 
        case 0x33: printf("inc   sp"); break; 
        case 0x34: printf("inc   (hl)"); break; 
        case 0x35: printf("dec   (hl)"); break; 
        case 0x36: printf("ld    (hl),%02x", code[1]); opbytes=2; break; 
        case 0x37: printf("scf"); break; 
        case 0x38: printf("jr    c,%02x", code[1]); opbytes=2; break; 
        case 0x39: printf("add   hl,sp"); break; 
        case 0x3A: printf("ld    a,(#$%02x%02x)", code[2], code[1]); opbytes=3; break; 
        case 0x3B: printf("dec   sp"); break; 
        case 0x3C: printf("inc   a"); break; 
        case 0x3D: printf("dec   a"); break; 
        case 0x3E: printf("ld    a,%02x", code[1]); opbytes=2; break; 
        case 0x3F: printf("ccf"); break; 
        
        case 0x40: printf("ld    b,b"); break; 
        case 0x41: printf("ld    b,c"); break; 
        case 0x42: printf("ld    b,d"); break; 
        case 0x43: printf("ld    b,e"); break; 
        case 0x44: printf("ld    b,h"); break; 
        case 0x45: printf("ld    b,l"); break; 
        case 0x46: printf("ld    b,(hl)"); break; 
        case 0x47: printf("ld    b,a"); break; 
        case 0x48: printf("ld    c,b"); break; 
        case 0x49: printf("ld    c,c"); break; 
        case 0x4A: printf("ld    c,d"); break; 
        case 0x4B: printf("ld    c,e"); break; 
        case 0x4C: printf("ld    c,h"); break; 
        case 0x4D: printf("ld    c,l"); break; 
        case 0x4E: printf("ld    c,(hl)"); break; 
        case 0x4F: printf("ld    c,a"); break; 
        
        case 0x50: printf("ld    d,b"); break; 
        case 0x51: printf("ld    d,c"); break; 
        case 0x52: printf("ld    d,d"); break; 
        case 0x53: printf("ld    d,e"); break; 
        case 0x54: printf("ld    d,h"); break; 
        case 0x55: printf("ld    d,l"); break; 
        case 0x56: printf("ld    d,(hl)"); break; 
        case 0x57: printf("ld    d,a"); break; 
        case 0x58: printf("ld    e,b"); break; 
        case 0x59: printf("ld    e,c"); break; 
        case 0x5A: printf("ld    e,d"); break; 
        case 0x5B: printf("ld    e,e"); break; 
        case 0x5C: printf("ld    e,h"); break; 
        case 0x5D: printf("ld    e,l"); break; 
        case 0x5E: printf("ld    e,(hl)"); break; 
        case 0x5F: printf("ld    e,a"); break; 
        
        case 0x60: printf("ld    h,b"); break; 
        case 0x61: printf("ld    h,c"); break; 
        case 0x62: printf("ld    h,d"); break; 
        case 0x63: printf("ld    h,e"); break; 
        case 0x64: printf("ld    h,h"); break; 
        case 0x65: printf("ld    h,l"); break; 
        case 0x66: printf("ld    h,(hl)"); break; 
        case 0x67: printf("ld    h,a"); break; 
        case 0x68: printf("ld    l,b"); break; 
        case 0x69: printf("ld    l,c"); break; 
        case 0x6A: printf("ld    l,d"); break; 
        case 0x6B: printf("ld    l,e"); break; 
        case 0x6C: printf("ld    l,h"); break; 
        case 0x6D: printf("ld    l,l"); break; 
        case 0x6E: printf("ld    l,(hl)"); break; 
        case 0x6F: printf("ld    l,a"); break; 
        
        case 0x70: printf("ld    (hl),b"); break; 
        case 0x71: printf("ld    (hl),c"); break; 
        case 0x72: printf("ld    (hl),d"); break; 
        case 0x73: printf("ld    (hl),e"); break; 
        case 0x74: printf("ld    (hl),h"); break; 
        case 0x75: printf("ld    (hl),l"); break; 
        case 0x76: printf("halt"); break; 
        case 0x77: printf("ld    (hl),a"); break; 
        case 0x78: printf("ld    a,b"); break; 
        case 0x79: printf("ld    a,c"); break; 
        case 0x7A: printf("ld    a,d"); break; 
        case 0x7B: printf("ld    a,e"); break; 
        case 0x7C: printf("ld    a,h"); break; 
        case 0x7D: printf("ld    a,l"); break; 
        case 0x7E: printf("ld    a,(hl)"); break; 
        case 0x7F: printf("ld    a,a"); break; 
        
        case 0x80: printf("add   a,b"); break; 
        case 0x81: printf("add   a,c"); break; 
        case 0x82: printf("add   a,d"); break; 
        case 0x83: printf("add   a,e"); break; 
        case 0x84: printf("add   a,h"); break; 
        case 0x85: printf("add   a,l"); break; 
        case 0x86: printf("add   a,(hl)"); break; 
        case 0x87: printf("add   a,a"); break; 
        case 0x88: printf("adc   a,b"); break; 
        case 0x89: printf("adc   a,c"); break; 
        case 0x8A: printf("adc   a,d"); break; 
        case 0x8B: printf("adc   a,e"); break; 
        case 0x8C: printf("adc   a,h"); break; 
        case 0x8D: printf("adc   a,l"); break; 
        case 0x8E: printf("adc   a,(hl)"); break; 
        case 0x8F: printf("adc   a,a"); break; 
        
        case 0x90: printf("sub   b"); break; 
        case 0x91: printf("sub   c"); break; 
        case 0x92: printf("sub   d"); break; 
        case 0x93: printf("sub   e"); break; 
        case 0x94: printf("sub   h"); break; 
        case 0x95: printf("sub   l"); break; 
        case 0x96: printf("sub   (hl)"); break; 
        case 0x97: printf("sub   a"); break; 
        case 0x98: printf("sbc   a,b"); break; 
        case 0x99: printf("sbc   a,c"); break; 
        case 0x9A: printf("sbc   a,d"); break; 
        case 0x9B: printf("sbc   a,e"); break; 
        case 0x9C: printf("sbc   a,h"); break; 
        case 0x9D: printf("sbc   a,l"); break; 
        case 0x9E: printf("sbc   a,(hl)"); break; 
        case 0x9F: printf("sbc   a,a"); break; 
        
        case 0xA0: printf("and   b"); break; 
        case 0xA1: printf("and   c"); break; 
        case 0xA2: printf("and   d"); break; 
        case 0xA3: printf("and   e"); break; 
        case 0xA4: printf("and   h"); break; 
        case 0xA5: printf("and   l"); break; 
        case 0xA6: printf("and   (hl)"); break; 
        case 0xA7: printf("and   a"); break; 
        case 0xA8: printf("xor   b"); break; 
        case 0xA9: printf("xor   c"); break; 
        case 0xAA: printf("xor   d"); break; 
        case 0xAB: printf("xor   e"); break; 
        case 0xAC: printf("xor   h"); break; 
        case 0xAD: printf("xor   l"); break; 
        case 0xAE: printf("xor   (hl)"); break; 
        case 0xAF: printf("xor   a"); break; 
        
        case 0xB0: printf("or    b"); break; 
        case 0xB1: printf("or    c"); break; 
        case 0xB2: printf("or    d"); break; 
        case 0xB3: printf("or    e"); break; 
        case 0xB4: printf("or    h"); break; 
        case 0xB5: printf("or    l"); break; 
        case 0xB6: printf("or    (hl)"); break; 
        case 0xB7: printf("or    a"); break; 
        case 0xB8: printf("cp    b"); break; 
        case 0xB9: printf("cp    c"); break; 
        case 0xBA: printf("cp    d"); break; 
        case 0xBB: printf("cp    e"); break; 
        case 0xBC: printf("cp    h"); break; 
        case 0xBD: printf("cp    l"); break; 
        case 0xBE: printf("cp    (hl)"); break; 
        case 0xBF: printf("cp    a"); break; 
        
        case 0xC0: printf("ret   nz"); break; 
        case 0xC1: printf("pop   bc"); break; 
        case 0xC2: printf("jp    nz,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xC3: printf("jp    #$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xC4: printf("call  nz,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xC5: printf("push  bc"); break; 
        case 0xC6: printf("add   a,%02x", code[1]); opbytes=2; break; 
        case 0xC7: printf("rst   00h"); break; 
        case 0xC8: printf("ret   z"); break; 
        case 0xC9: printf("ret"); break; 
        case 0xCA: printf("jp    z,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xCB: printf("BITS"); break; 
        case 0xCC: printf("call  z,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xCD: printf("call  #$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xCE: printf("adc   a,%02x", code[1]); opbytes=2; break; 
        case 0xCF: printf("rst   08h"); break; 
        
        case 0xD0: printf("ret   nc"); break; 
        case 0xD1: printf("pop   de"); break; 
        case 0xD2: printf("jp    nc,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xD3: printf("out   (%02x),a", code[1]); opbytes=2; break; 
        case 0xD4: printf("call  nc,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xD5: printf("push  de"); break; 
        case 0xD6: printf("sub   %02x", code[1]); opbytes=2; break; 
        case 0xD7: printf("rst   10h"); break; 
        case 0xD8: printf("ret   c"); break; 
        case 0xD9: printf("exx"); break; 
        case 0xDA: printf("jp    c,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xDB: printf("in    a,(%02x)", code[1]); opbytes=2; break; 
        case 0xDC: printf("call  c,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xDD: printf("IX"); break; 
        case 0xDE: printf("sbc   a,%02x", code[1]); opbytes=2; break; 
        case 0xDF: printf("rst   18h"); break; 
        
        case 0xE0: printf("ret   po"); break; 
        case 0xE1: printf("pop   hl"); break; 
        case 0xE2: printf("jp    po,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xE3: printf("ex    (sp),hl"); break; 
        case 0xE4: printf("call  po,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xE5: printf("push  hl"); break; 
        case 0xE6: printf("and   %02x", code[1]); opbytes=2; break; 
        case 0xE7: printf("ret   20h"); break; 
        case 0xE8: printf("ret   pe"); break; 
        case 0xE9: printf("jp    (hl)"); break; 
        case 0xEA: printf("jp    pe,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xEB: printf("ex    de,hl"); break; 
        case 0xEC: printf("call  pe,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xED: printf("EXTD"); break; 
        case 0xEE: printf("xor   %02x", code[1]); opbytes=2; break; 
        case 0xEF: printf("rst   28h"); break; 
        
        case 0xF0: printf("ret   p"); break; 
        case 0xF1: printf("pop   af"); break; 
        case 0xF2: printf("jp    p,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xF3: printf("di"); break; 
        case 0xF4: printf("call  p,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xF5: printf("push  af"); break; 
        case 0xF6: printf("or    %02x", code[1]); opbytes=2; break; 
        case 0xF7: printf("rst   30h"); break; 
        case 0xF8: printf("ret   m"); break; 
        case 0xF9: printf("ld    sp,hl"); break; 
        case 0xFA: printf("jp    m,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xFB: printf("ei"); break; 
        case 0xFC: printf("call  m,#$%02x%02x", code[2], code[1]); opbytes=3; break; 
        case 0xFD: printf("IY"); break; 
        case 0xFE: printf("cp    %02x", code[1]); opbytes=2; break; 
        case 0xFF: printf("rst   38h"); break;
    }
    printf("\n");
    return opbytes;
}
