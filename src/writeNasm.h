#ifndef __writeNasm__
#define __writeNasm__ 

#include "typeSymboleTable.h"

#define PATHENASM "nasm/nasmFile.asm"



typedef enum { 
  rdi, rsi, rdx, rcx, r8, r9
}Register;


int writeNasmFile(Hachage tabSymbole, Node* tree, char *file_name);

char* reverseName(char* file_name);

char* nasmFile(char* file_name, char*f);


#endif