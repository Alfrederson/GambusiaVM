#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#ifndef ARDUINO
typedef unsigned char u8;
typedef unsigned int u32;
#endif

typedef unsigned short u16;

typedef char i8;
typedef short i16;
typedef int i32;

#ifdef ARDUINO
typedef double f32;
#else
typedef float f32;
#endif



#define HEAP_SIZE  1024
#define STACK_SIZE 256
#define MAX_STACK_DEPTH 32

struct VMType{
  const char* name;
  u8 size;
  u8 is_signed;
};


#define t_u8 0
#define t_u16 1
#define t_u32 2
#define t_i8 3
#define t_i16 4
#define t_i32 5

struct VMState;

typedef u8 (*VMFunction)(VMState*);


// instructions
enum Op{
  EXIT     = 0, // ends
  PUSH_U8  = 1, // puts a value on the stack
  PUSH_U16 = 2,
  PUSH_U32 = 3,
  PUSH_I8  = 4,
  PUSH_I16 = 5,
  PUSH_I32 = 6,
  PUSH_F32 = 7,
  SET_U8   = 8, // sets a value on the heap
  SET_U16  = 9,
  SET_U32  = 10,
  SET_I8   = 11,
  SET_I16  = 12,
  SET_I32  = 13,
  SET_F32  = 14,
  LOAD     = 15, // x = pop
  LOAD_OFF = 16, // x[y] = pop
  ADD      = 17, // a + b
  SUB      = 18, // a - b
  DIV      = 19, // a / b
  MUL      = 20, // a * b
  MOD      = 21, // a % b
  LSH      = 22, // a << b
  RSH      = 23, // a >> b

  NEG      = 24, // -a

  LNOT     = 25, // !a
  LAND     = 26, // a && b
  LOR      = 27, // a || b

  NOT      = 28, // ~a

  AND      = 29, // a & b
  OR       = 30, // a | b
  XOR      = 31, // a ^ b

  EQL      = 32, // a == b
  DIFF     = 33, // a != b
  LT       = 34, // a < b
  GT       = 35, // a > b
  LTE      = 36, // a <= b
  GTE      = 37, // a >= b

  CALL     = 38, // push call frame, call a
  RET      = 39, // pop call frame
  POP      = 40, // pops the stack into a
  JMP      = 41, // unconditional jump
  JZ       = 42, // jump if zero
  JNZ      = 43, // jump if not zero
  OUT      = 44  // print
};

u8 vm_print(VMState*);
u8 vm_print2(VMState*);



// builtin functions
VMFunction functions[] = {
  vm_print,
  vm_print2,
};


// name, size
VMType types[] = {
  {"U8 ",1   ,false}, 
  {"U16",2  ,false},
  {"U32",4  ,false},
  {"I8 ",1   ,true},
  {"I16",2  ,true},
  {"I32",4  ,true},
  {"F32",4  ,true}
};

union VMLido32{
  i32 i;
  u32 u;
  f32 f;
  u8  pacote[4] ;
};

struct VMRegister{
  int type;
  union{
    i32 i;
    u32 u;
    f32 f;
    u8 bytes[4];
  } val ;
};

struct VMFrame{
  u16 program_counter;
  u16 stack_top;
  u16 heap_top;
};

struct VMState {
  u8 finished = 0;
  u8* program=NULL;
  // program counter
  // combined stack/heap
  u8 stack[STACK_SIZE+HEAP_SIZE];
  u16 stack_top=0;

  // registers for operations
  VMRegister regA, regB;
  // result type and signedness
  u8 r_type=0;
  u8 r_signed=0;
  
  u8 call_depth;
  VMFrame call_stack[MAX_STACK_DEPTH];


  
  void init(){
    call_depth=0;
    memset(call_stack,0,sizeof(VMFrame)*MAX_STACK_DEPTH);
    memset(stack,0,STACK_SIZE);
    stack_top=0;
  }

  // push stack frame

  // pop stack frame

  u16 program_counter(){
    return call_stack[call_depth].program_counter;
  }
  void program_advance(u8 steps){
    call_stack[call_depth].program_counter+=steps;
  }
  void program_jump   (u16 to){
    call_stack[call_depth].program_counter=to;
  }


  // pop top of stack into register
  void pop(VMRegister* reg){
    u8 step=types[ reg->type=stack[stack_top-1] ].size;
    reg->val.u = 0;
    for(u8 p = 0 ; p < step  ; p++)
      reg->val.bytes[p] = stack[stack_top-1 - step + p];
    stack_top -= (1+step);
  }

  void popA(){
    pop(&regA);
  }

  void push(VMRegister* reg){
    if(types[reg->type].is_signed){
      push_i(reg->val.i, reg->type);
    }else{
      push_u(reg->val.u, reg->type);
    }
  }

  void pop_operands(){
    pop(&regB);
    pop(&regA);
    r_type = types[regA.type].size > types[regB.type].size ? regA.type : regB.type;
    r_signed = types[regA.type].is_signed || types[regB.type].is_signed;
    if(r_signed && !types[r_type].is_signed )
      r_type += 3;    
  }
  
  u8 _ADD(){
    pop_operands();
    // 1- picks the bigger type:
    // 2- if either is signed, the result is signed:
    // result type should be signed
    // let the compiler handle this shit
    if(r_signed){
      push_i(
        (types[regA.type].is_signed ? regA.val.i : regA.val.u) +
        (types[regB.type].is_signed ? regB.val.i : regB.val.u)
      ,r_type);
    }else{
      push_u(
        (types[regA.type].is_signed ? regA.val.i : regA.val.u) +
        (types[regB.type].is_signed ? regB.val.i : regB.val.u)
      ,r_type);
    }
    return 1;
  }

  u8 _SUB(){
    pop_operands();
    if(r_signed){
      push_i(
        (types[regA.type].is_signed ? regA.val.i : regA.val.u) -
        (types[regB.type].is_signed ? regB.val.i : regB.val.u)
      ,r_type);
    }else{
      push_u(
        (types[regA.type].is_signed ? regA.val.i : regA.val.u) -
        (types[regB.type].is_signed ? regB.val.i : regB.val.u)
      ,r_type);      
    }
    return 1;
  }

  u8 _DIV(){
    pop_operands();
    if(r_signed){
      push_i(
        (types[regA.type].is_signed ? regA.val.i : regA.val.u) /
        (types[regB.type].is_signed ? regB.val.i : regB.val.u)
      ,r_type);
    }else{
      push_u(
        (types[regA.type].is_signed ? regA.val.i : regA.val.u) /
        (types[regB.type].is_signed ? regB.val.i : regB.val.u)
      ,r_type);
    }
    return 1;
  }  

  u8 _MUL(){
    pop_operands();
    if(r_signed){
      push_i(
        (types[regA.type].is_signed ? regA.val.i : regA.val.u) *
        (types[regB.type].is_signed ? regB.val.i : regB.val.u)
      ,r_type);
    }else{
      push_u(
        (types[regA.type].is_signed ? regA.val.i : regA.val.u) *
        (types[regB.type].is_signed ? regB.val.i : regB.val.u)
      ,r_type);      
    }
    return 1;
  }

  u8 _MOD(){
    pop_operands();
    if(r_signed){
      push_i(
        (types[regA.type].is_signed ? regA.val.i : regA.val.u) %
        (types[regB.type].is_signed ? regB.val.i : regB.val.u)
      ,r_type);
    }else{
      push_u(
        (types[regA.type].is_signed ? regA.val.i : regA.val.u) %
        (types[regB.type].is_signed ? regB.val.i : regB.val.u)
      ,r_type);
    }
    return 1;    
  }

  u8 _LSH(){
    pop_operands();
    if(r_signed)
      push_i( regA.val.u << regB.val.u ,r_type);
    else
      push_u( regA.val.u << regB.val.u ,r_type); 
    return 1;
  }
  u8 _RSH(){
    pop_operands();
    if(r_signed)
      push_i( regA.val.u >> regB.val.u ,r_type);
    else
      push_u( regA.val.u >> regB.val.u ,r_type);
    return 1;
  }  

  u8 _LNOT(){
    pop(&regA);
    push_u( regA.val.u == 0 ? 1 : 0, t_u8);
    return 1;
  }
  u8 _LAND(){
    pop_operands();
    push_u( regA.val.u && regB.val.u , t_u8);
    return 1;
  }
  u8 _LOR(){
    pop_operands();
    push_u( regA.val.u || regB.val.u , t_u8);
    return 1;
  }

  u8 _AND(){
    pop_operands();
    if(r_signed)
      push_i( regA.val.u & regB.val.u ,r_type);
    else
      push_u( regA.val.u & regB.val.u ,r_type);
    return 1;    
  }
  u8 _OR(){
    pop_operands();
    if(r_signed)
      push_i( regA.val.u | regB.val.u ,r_type);
    else
      push_u( regA.val.u | regB.val.u ,r_type);
    return 1;    
  }
  u8 _XOR(){
    if(r_signed)
      push_i( regA.val.u ^ regB.val.u ,r_type);
    else
      push_u( regA.val.u ^ regB.val.u ,r_type);
    return 1;    
  }
  u8 _NOT(){
    pop(&regA);
    if(types[regA.type].is_signed){
      push_i( ~regA.val.u, regA.type);
    }else{
      push_u( ~regA.val.u, regA.type);
    }
    return 1;    
  }

  u8 _JMP(){
    u16 to = read_u16();
    program_jump(to);
    return 0;
  }

  u8 _JZ(){
    pop(&regA);
    if(regA.val.u == 0){
      u16 to = read_u16();
      program_jump(to);
      return 0;
    }else{
      return 3;
    }
  }

  u8 _JNZ(){
    pop(&regA);
    if(regA.val.u != 0){
      u16 to = read_u16();
      program_jump(to);
      return 0;
    }else{
      return 3;
    }
  }

  u8 _CALL(){
    pop(&regA);
    if(regA.type == t_u8){
      functions[regA.val.u](this);
    }else{
      printf("fun esterna\n");
    }
    return 1;
  }

  u8 _RET(){
    return 0;
  }

  void push_u(u32 val, u8 type){
    u8 size = types[type].size;
    for(u8 p = 0 ; p < size ; p++)
      stack[stack_top+p] = ((u8*) &val)[p];
    stack[stack_top+size] = type;
    stack_top += 1 + size;
  }
  void push_i(i32 val, u8 type){
    u8 size = types[type].size;
    for(u8 p = 0 ; p < size ; p++)
      stack[stack_top+p] = ((u8*) &val)[p];
    stack[stack_top+size] = type;
    stack_top += 1 + size;
  }

  u8 read_u8(){
    return program[program_counter()+1];
  }
  u16 read_u16(){
    return program[program_counter()+2] | (program[program_counter()+1]<<8);
  }
  u32 read_u32(){
    VMLido32 lido;
    lido.pacote[3] = program[program_counter()+1];
    lido.pacote[2] = program[program_counter()+2];
    lido.pacote[1] = program[program_counter()+3];
    lido.pacote[0] = program[program_counter()+4];
    return lido.u;
  }

  u8 _EXIT(){
    #ifdef ARDUINO
     Serial.println("done");
    #else
     printf("done\n");
    #endif
    
    finished=1;
    return 1;
  }

  u8 _PUSH_U8 () { push_u(read_u8() , t_u8 ); return 2; }
  u8 _PUSH_U16() { push_u(read_u16(), t_u16); return 3; }
  u8 _PUSH_U32() { push_u(read_u32(), t_u32); return 5; }
  u8 _PUSH_I8 () { push_i((i8)  read_u8() , t_i8 ); return 2; }
  u8 _PUSH_I16() { push_i((i16) read_u16(), t_i16); return 3; }
  u8 _PUSH_I32() { push_i((i32) read_u32(), t_i32); return 5; }


  void step(){
    // fetch
    u8 op = program[ program_counter() ];
    u8 ss=0;
    // no AVR o programa é lido da memória flash, então isso não
    // ocupa memória. no STM32 é diferente...
    switch(op){
      case EXIT     : ss = _EXIT()    ; break;
      case PUSH_U8 : ss = _PUSH_U8(); break;
      case PUSH_U16: ss = _PUSH_U16(); break;
      case PUSH_U32: ss = _PUSH_U32(); break;
      case PUSH_I8 : ss = _PUSH_I8(); break;
      case PUSH_I16: ss = _PUSH_I16(); break;
      case PUSH_I32: ss = _PUSH_I32(); break;

      case ADD     : ss = _ADD()    ; break;
      case SUB     : ss = _SUB()    ; break;
      case MUL     : ss = _MUL()    ; break;
      case DIV     : ss = _DIV()    ; break;
      case MOD     : ss = _MOD()    ; break;

      case LSH     : ss = _LSH()    ; break;
      case RSH     : ss = _RSH()    ; break;

      //case NEG     : ss = _NEG()    ; break;

      case LNOT : ss = _LNOT() ; break;
      case LAND : ss = _LAND() ; break;
      case LOR  : ss = _LOR () ; break;

      case AND  : ss = _AND()  ; break;
      case OR   : ss = _OR()   ; break;
      case XOR  : ss = _XOR()  ; break;

      case JMP  : ss = _JMP() ; break;

      case CALL : ss = _CALL(); break;
      case RET  : ss = _RET() ; break;
    }
    if(ss) program_advance(ss);
  }

  void run(u8 *what){
    program = what;
    while(!finished)
      step();
  }
};

u8 prog[] = {
  //PUSH_I16,0x01,0x00,
  PUSH_U8,0x01,
  CALL,
  PUSH_U8,0x00,
  CALL,
  EXIT
};

VMState state;
int _main(){
    state.init();
    state.run(prog);    
    return 0;
}

#ifdef ARDUINO
void setup(){
  Serial.begin(115200);
  _main();
}
void loop(){

}
#else
int main(){
  _main();
}
#endif


// BUILT-IN FUNCTIONS

u8 vm_print(VMState* state){
  printf("print 0\n");
  return 0;
  /*
  state->popA();
  switch(state->regA.type){
    case t_u8:
    case t_u16:
    case t_u32:
    #ifdef ARDUINO
      Serial.print(regA.val.u);
    #else
      printf("%s %d\n", types[state->regA.type].name, state->regA.val.u);
    #endif
    break;
    case t_i8:
    case t_i16:
    case t_i32:
    #ifdef ARDUINO
      Serial.print(regA.val.i);
    #else
      printf("%s %d\n",types[state->regA.type].name, state->regA.val.i);
    #endif
    break;    
  }
  return 0;*/
}

u8 vm_print2(VMState *state){
  printf("print 2\n");
  return 0;
}