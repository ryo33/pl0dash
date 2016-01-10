/* codegen.c */
#include <stdio.h>
#include <stdlib.h>
#include "codegen.h"
#include "symbol_table.h"

#define MAX_OP_LINE  20
#define MAX_CODE    800

int code_ptr = 0;

typedef struct {
  char op_line[MAX_OP_LINE];
  int address; /* JMP/JPC 用 */
} Asm_Code;

static Asm_Code code[MAX_CODE];

int gencode_no_arg(Opr o); /* 引数なし */
int gencode_arg_ST(Opr o, int ptr); /* 引数として記号表番号 */
int gencode_arg_v(Opr o, int value); /* 引数として値そのもの */
int gencode_arg_v_ST(Opr o, int value, int ptr); /* 引数として値と記号表番号 */
int next_code();
void list_code();
void backpatch(int code_lineno);
  
/* 以下は本ファイルでのみ使用 */
int add_code(char *opline);
int add_code_val(char *fmt, int value);
int add_code_addr(char *opr, int address);

void list_code(){ /* リストを出力 */
  int i = 1;
  while (i <= code_ptr) {
    if (code[i].address < 0) 
      fprintf(stdout,"%4d %s\n", i, code[i].op_line);
    else
      fprintf(stdout,"%4d %s%d\n", i, code[i].op_line,code[i].address);
    i++;
  }
}

int gencode_no_arg(Opr o) {
  int cpt;
  if (o == wrl) {
    return add_code("PRINTLN");
  }

  if (o == end) {
    return add_code("END");
  }

  if (o == wrt) {
    add_code("POP A");
    return add_code("PRINT A");
  }

  if (o == odd) {
    add_code("POP A");
  } else { /* スタックから2つPOPするもの */
    add_code("POP B");
    add_code("POP A");
  }
  switch (o) {
  case pls:
    add_code("PLUS");
    break;
  case min:
    add_code("MINUS");
    break;
  case mul:
    add_code("MULTI");
    break;
  case divi:
    add_code("DIV");
    break;
  case odd:
    add_code("CMPODD");
    break;
  case eq:
    add_code("CMPEQ");
    break;
  case neq:
    add_code("CMPNOTEQ");
    break;
  case lt:
    add_code("CMPLT");
    break;
  case gt:
    add_code("CMPGT");
    break;
  case le:
    add_code("CMPLE");
    break;
  case ge:
    add_code("CMPGE");
    break;
  default:
    break;
  }
  return add_code("PUSH C");
}

int gencode_arg_V_ST(Opr o, int value, int ptr) {
  int cpt = 0;
  switch(o) {
  case str:
    cpt = add_code_val("LOAD A,%d", value);
    cpt = add_code_val("STORE A,#(%d)", get_symbol_address(ptr));
    break;
  default:
    break;
  }
  return cpt;
}

int gencode_arg_V(Opr o, int value) {
  int cpt = 0;
  switch(o) {
  case lod:
    cpt = add_code_val("LOAD A,%d", value);
    cpt = add_code("PUSH A");
    break;
  case jmp:
    cpt = add_code_addr("JMP ", value);
    break;
  case jpc:
    cpt = add_code_addr("JPC ", value);
    break;
  default:
    break;
  }
  return cpt;
}


int gencode_arg_ST(Opr o, int ptr) {
  int cpt = 0;
  switch(o) {
  case str:
    cpt = add_code("POP A");
    cpt = add_code_val("STORE A,#(%d)", get_symbol_address(ptr));
    break;
  case lod:
    cpt = add_code_val("LOAD A,#(%d)", get_symbol_address(ptr));
    cpt = add_code("PUSH A");
    break;
  default:
    break;
  }
  return cpt;
}

int add_code(char *opline) {
  code_ptr++;
  strcpy(code[code_ptr].op_line, opline);
  code[code_ptr].address = -1; /* address を使用しない */
  return code_ptr;
}

int add_code_val(char *fmt, int value) {
  code_ptr++;
  sprintf(code[code_ptr].op_line, fmt, value);
  code[code_ptr].address = -1; /* address を使用しない */
  return code_ptr;
}

int add_code_addr(char *opr, int address) {
  code_ptr++;
  strcpy(code[code_ptr].op_line, opr);
  code[code_ptr].address = address; 
  return code_ptr;
}

void backpatch(int code_lineno) { /* 現在のコード行の次の行を入れる */
  code[code_lineno].address = code_ptr+1;
}

int next_code() {
  return code_ptr+1;
}