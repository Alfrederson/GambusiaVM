#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EMPTY_STRING 0


typedef unsigned char u8;
typedef unsigned long u32;

#ifdef ARDUINO

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
#define t_f32 6
#define t_str 7

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
  PUSH_STR = 8,

  SET_8    = 9, // sets a value on the heap
  SET_16   = 10,
  SET_32   = 11,
  SET_F32  = 12,
  SET_STR  = 13,

  GET_U8   = 14, // pushes an u8 value from the heap into the stack
  GET_U16  = 15, // same but u16, etc..
  GET_U32  = 16,
  GET_I8   = 17,
  GET_I16  = 18,
  GET_I32  = 19,
  GET_F32  = 20,
  GET_STR  = 21,

  SGC      = 22, // sets the global pointer
  RES      = 23, // reserves X bytes in the heap
  FREE     = 24, // frees X bytes from the heap
  OFF      = 25, // defines the offset for GET or SET (used for arrays)

  ADD      = 26, // a + b
  SUB      = 27, // a - b
  DIV      = 28, // a / b
  MUL      = 29, // a * b
  MOD      = 30, // a % b
  LSH      = 31, // a << b
  RSH      = 32, // a >> b

  NEG      = 33, // -a

  LNOT     = 34, // !a
  LAND     = 35, // a && b
  LOR      = 36, // a || b

  NOT      = 37, // ~a

  AND      = 38, // a & b
  OR       = 39, // a | b
  XOR      = 40, // a ^ b

  EQL      = 41, // a == b
  DIFF     = 42, // a != b
  LT       = 43, // a < b
  GT       = 44, // a > b
  LTE      = 45, // a <= b
  GTE      = 46, // a >= b
  CMP      = 47, // a > b ? 1 : -1 : 0

  CALL     = 48, // push call frame, call a
  RET      = 49, // pop call frame
  POP      = 50, // pops the stack into a
  JMP      = 51, // unconditional jump
  JZ       = 52, // jump if zero
  JNZ      = 53, // jump if not zero
  OUT      = 54,  // print
  CRASH    = 255  // compiler generated crash
};

#ifdef ARDUINO
u8 pinos[] = {
  PA1  
};
#endif

// built in functions
u8 vm_print(VMState*);
u8 vm_println(VMState*);
u8 vm_delay(VMState*);
u8 vm_pin_mode(VMState*);
u8 vm_out(VMState*);
u8 vm_pin(VMState*);


// fazer isso automaticamente?
VMFunction functions[] = {
  vm_print,
  vm_println,               
  vm_delay,               
  vm_pin_mode,            
  vm_out,
  vm_pin
};



enum VMFunctions {
    VM_PRINT = 0,
    VM_PRINTLN = 1,
    VM_DELAY = 2,
    VM_PIN_MODE = 3,
    VM_OUT = 4,
    VM_PIN  = 5
};


// name, size
VMType types[] = {
  {"U8 ",1   ,false}, 
  {"U16",2  ,false},
  {"U32",4  ,false},
  {"I8 ",1   ,true},
  {"I16",2  ,true},
  {"I32",4  ,true},
  {"F32",4  ,true},
  {"STR",2  ,false}
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
  u16 heap_bottom;
  u16 heap_size;
};


struct VMState {
  u8 finished = 0;
  u8* program=NULL;

  u8 stack[STACK_SIZE];
  u8 heap [HEAP_SIZE];
  // let the compiler deal with this shit
  u16 stack_top=0;
  u16 data_size;

  // registers for operations
  VMRegister regA, regB;
  // result type and signedness
  u8 r_type=0;
  u8 r_signed=0;
  u8 r_float=0;
  
  u8 call_depth;
  u16 heap_bottom;
  u16 globals_depth;
  VMFrame call_stack[MAX_STACK_DEPTH];

  void init(){
    call_depth=0;
    stack_top=0;
    heap_bottom=0;

    globals_depth=0;
    memset(call_stack,0,sizeof(VMFrame)*MAX_STACK_DEPTH);
    memset(stack,0,STACK_SIZE);
    memset(heap ,0,HEAP_SIZE);
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
    call_stack[call_depth].program_counter=(to + 2);
  }
  void push_stack(){
    // guarda o fundo
    call_stack[call_depth].heap_bottom = heap_bottom;
    // soma a altura anterior
    heap_bottom += call_stack[call_depth].heap_size;
    call_depth++;
  }
  void pop_stack(){
    call_depth--;
    // lembra o fundo
    heap_bottom = call_stack[call_depth].heap_bottom;
  }
  void heap_grow(u16 how_much){
    call_stack[call_depth].heap_size += how_much;
  }
  void heap_shrink(u16 how_much){
    call_stack[call_depth].heap_size -= how_much;
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
    if(r_type == t_f32)
      r_float = 1;
  }
  f32 reg_to_float(VMRegister* reg){
    switch(reg->type){
      case t_i8:
      case t_i16:
      case t_i32:
        return (f32) reg->val.i;
      case t_u8:
      case t_u16:
      case t_u32:
        return (f32) reg->val.u;
      case t_f32: return reg->val.f;
    }
    return 0;
  }
  i32 reg_to_int(VMRegister* reg){
    switch(reg->type){
      case t_i8:
      case t_i16:
      case t_i32:
        return reg->val.i;
      case t_u8:
      case t_u16:
      case t_u32:
        return reg->val.u;
      case t_f32:
        return (i32) reg->val.f;
    }
    return 0;
  }
  u32 reg_to_uint(VMRegister* reg){
    switch(reg->type){
      case t_i8:
      case t_i16:
      case t_i32:
        return reg->val.i;
      case t_u8:
      case t_u16:
      case t_u32:
        return reg->val.u;
      case t_f32:
        return (u32) reg->val.f;
    }
    return 0;    
  }
  u8 _ADD(){
    pop_operands();
    // 1- picks the bigger type:
    // 2- if either is signed, the result is signed:
    // result type should be signed
    // let the compiler handle this shit
    if(r_float){
      push_f(reg_to_float(&regA) + reg_to_float(&regB));
      return 1;
    }
    if(r_signed){
      push_i( reg_to_int(&regA) + reg_to_int(&regB),r_type);
    }else{
      push_u( reg_to_uint(&regA) + reg_to_uint(&regB),r_type);
    }
    return 1;
  }

  u8 _SUB(){
    pop_operands();
    if(r_float){
      push_f(reg_to_float(&regA) - reg_to_float(&regB));
      return 1;      
    }
    if(r_signed){
      i32 r = reg_to_int(&regA) - reg_to_int(&regB);
      push_i( r,r_type);
    }else{
      push_u( reg_to_uint(&regA) - reg_to_uint(&regB),r_type);
    }
    return 1;
  }

  u8 _DIV(){
    pop_operands();
    if(r_float){
      push_f(reg_to_float(&regA) / reg_to_float(&regB));
      return 1;      
    }    
    if(r_signed){
      push_i( reg_to_int(&regA) / reg_to_int(&regB),r_type);
    }else{
      push_u( reg_to_uint(&regA) / reg_to_uint(&regB),r_type);
    }
    return 1;
  }  

  u8 _MUL(){
    pop_operands();
    if(r_float){
      push_f(reg_to_float(&regA) * reg_to_float(&regB));
      return 1;      
    }
    if(r_signed){
      push_i(reg_to_int(&regA) * reg_to_int(&regB),r_type);
    }else{
      push_u( reg_to_uint(&regA) * reg_to_uint(&regB),r_type);     
    }
    return 1;
  }

  u8 _MOD(){
    pop_operands(); 
    if(r_float){
      push_f(reg_to_int(&regA) % reg_to_int(&regB));
      return 1;
    } 
    if(r_signed){
      push_i(reg_to_int(&regA) % reg_to_int(&regB),r_type);
    }else{
      push_u(reg_to_int(&regA) % reg_to_int(&regB),r_type);
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


  u8 _NEG(){
    pop(&regA);
    if(regA.type == t_f32){
      regA.val.f = - regA.val.f;  
      push_u(regA.val.u,t_f32);
    }else{
      if(types[regA.type].is_signed){
        push_i(-regA.val.i, regA.type);
      }else{
        push_i(-regA.val.u, regA.type+3);
      }
    }
    return 1;
  }

  u8 _JMP(){
    u16 to = read_u16();
    program_jump(to);
    return 0;
  }

  u8 _LT(){
    pop_operands();
    if(r_float){
      push_u( reg_to_float(&regA) < reg_to_float(&regB), t_u8);
    }else{
      if(r_signed){
        push_u( reg_to_int(&regA) < reg_to_int(&regB), t_u8);
      }else{
        push_u( reg_to_uint(&regA) < reg_to_uint(&regB), t_u8);
      }
    }
    return 1;
  }

  u8 _GT(){
    pop_operands();
    if(r_float){
      push_u( reg_to_float(&regA) > reg_to_float(&regB), t_u8);
    }else{
      if(r_signed){
        push_u( reg_to_int(&regA) > reg_to_int(&regB), t_u8);
      }else{
        push_u( reg_to_uint(&regA) > reg_to_uint(&regB), t_u8);
      }
    }
    return 1;
  }

  u8 _GTE(){
    pop_operands();
    if(r_float){
      push_u( reg_to_float(&regA) >= reg_to_float(&regB), t_u8);
    }else{
      if(r_signed){
        push_u( reg_to_int(&regA) >= reg_to_int(&regB), t_u8);
      }else{
        push_u( reg_to_uint(&regA) >= reg_to_uint(&regB), t_u8);
      }
    }
    return 1;
  }

  u8 _CMP(){
    
    pop_operands();
    i8 r = 0;
    if(r_float){
      f32 d = reg_to_float(&regA) - reg_to_float(&regB);
      if(d > 0) r = 1;
      if(d < 0) r = -1; 
    }else{
      i32 d;
      d = (types[regA.type].is_signed ? regA.val.i : regA.val.u) - (types[regB.type].is_signed ? regB.val.i : regB.val.u);
      if(d > 0) r = 1;
      if(d < 0) r = -1;
    }
    push_i(r,t_i8);
    return 1;
  }


  u8 _EQL(){
    pop_operands();
    push_u( regA.val.u == regB.val.u , t_u8);
    return 1;
  }

  u8 _JZ(){
    popA();
    if(!regA.val.u){
      program_jump(read_u16());
      return 0;
    }else{
      return 3;
    }
  }

  u8 _JNZ(){
    popA();
    if(regA.val.u){
      program_jump(read_u16());
      return 0;
    }else{
      return 3;
    }
  }

  u8 _CALL(){
    popA();
    // se o label for de 8 bits, a função é built-in
    if(regA.type == t_u8){
      functions[regA.val.u](this);
      return 1;
    }else{
    // se o label for de 16 bits, é uma das funções do usuário
      // salva a posição atual e pula para lá
      push_stack();
      program_jump(regA.val.u);
      return 0;
    }
  }

  u8 _RES(){
    u16 bytes = read_u16();
    heap_grow(bytes);
    return 3;
  }

  u8 _FREE(){
    u16 bytes = read_u16();
    heap_shrink(bytes);
    return 3;
  }

  
  u8 _RET(){
    pop_stack();
    return 1;
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
  void push_f(f32 val){
    u8 size = types[t_f32].size;
    for(u8 p=0;p<size;p++)
      stack[stack_top+p] = ((u8*)&val)[p];
    stack[stack_top+size] = t_f32;
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
     Serial.println("fim");
    #else
     printf("fim\n");
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
  u8 _PUSH_F32() { push_u(read_u32(), t_f32); return 5; }

  // tem que ver se o endereço não se refere a uma variável global.
  u8 _PUSH_STR(){
    u16 endereco = (u16) read_u16();
    push_u( endereco, t_str );
    return 3;
  }


  u8 _SET_8(){
    popA();
    // verificar a profundidade da stack?
    // verificar o endereço que quer ser acessado?
    u16 endereco = (u16) read_u16() + heap_bottom; // + call_stack[call_depth].heap_depth ;
    heap[endereco] = regA.val.u & 0xFF;
    return 3;
  }
  u8 _SET_16(){
    popA();
    // heap starts at HEAP_POS and goes back
    u16 endereco = (u16) read_u16()+ heap_bottom; // + call_stack[call_depth].heap_depth ;
    heap[endereco]   = regA.val.u & 0xFF;
    heap[endereco+1] = (regA.val.u >> 8) & 0xFF ;
    return 3;
  }
  u8 _SET_32(){
    popA();
    // heap starts at HEAP_POS and goes back
    u16 endereco = (u16) read_u16()+ heap_bottom;// + call_stack[call_depth].heap_depth ;
    heap[endereco]   =  regA.val.u & 0xFF;
    heap[endereco+1] = (regA.val.u >> 8 ) & 0xFF ;
    heap[endereco+2] = (regA.val.u >> 16 ) & 0xFF;
    heap[endereco+3] = (regA.val.u >> 24) & 0xFF;

    return 3;
  }

  u8 _SET_F32(){
    return _SET_32();
  }
  u8 _SET_STR(){
    return _SET_16();
  }

  u8 _GET_U8(){
    popA();
    u16 endereco = (u16) read_u16()+ heap_bottom; // + call_stack[call_depth].heap_depth;
    u8  val = heap[endereco];
    push_u(val, t_u8);
    return 3;
  }
  u8 _GET_U16(){
    u16 endereco = (u16) read_u16()+ heap_bottom;// + call_stack[call_depth].heap_depth;
    u16 val = heap[endereco] << 8| heap[endereco+1] ;
    push_u(val, t_u16);
    return 3;
  }
  u8 _GET_U32(){
    popA();
    return 3;
  }
  u8 _GET_I8(){
    popA();
    return 3;
  }
  u8 _GET_I16(){
    u16 endereco = (u16) read_u16()+ heap_bottom;
    i16 val = heap[endereco]  | heap[endereco+1]<<8 ;
    push_i(val, t_i16);
    return 3;
  }
  u8 _GET_I32(){
    popA();
    return 3;
  }
  u8 _GET_F32(){
    popA();
    return 3;
  }
  u8 _GET_STR(){
    popA();
    return 3;
  }

  u8 _CRASH(){
    printf("AI MINHA VOAIDA!");
    finished=true;
    return 1;
  }

  u8 _SGC(){
    u16 gc = read_u16();
    globals_depth = gc;
    return 3;
  }

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
      case PUSH_F32: ss = _PUSH_F32(); break;
      case PUSH_STR: ss = _PUSH_STR(); break;

      case SET_8   : ss = _SET_8() ; break;
      case SET_16  : ss = _SET_16(); break;
      case SET_32  : ss = _SET_32(); break;

      case SET_F32  : ss = _SET_F32(); break;
      case SET_STR  : ss = _SET_STR(); break;

      case GET_U8   : ss = _GET_U8() ; break;
      case GET_U16  : ss = _GET_U16(); break;
      case GET_U32  : ss = _GET_U32(); break;
      case GET_I8   : ss = _GET_I8() ; break;
      case GET_I16  : ss = _GET_I16(); break;
      case GET_I32  : ss = _GET_I32(); break;
      case GET_F32  : ss = _GET_F32(); break;
      case GET_STR  : ss = _GET_STR(); break;

      case SGC      : ss = _SGC(); break;
      case RES      : ss = _RES(); break;
      case FREE     : ss = _FREE(); break;

      case ADD     : ss = _ADD()    ; break;
      case SUB     : ss = _SUB()    ; break;
      case MUL     : ss = _MUL()    ; break;
      case DIV     : ss = _DIV()    ; break;
      case MOD     : ss = _MOD()    ; break;

      case CMP     : ss = _CMP()    ; break;
      case LT      : ss = _LT()     ; break;
      case GT      : ss = _GT()     ; break;
      case GTE     : ss = _GTE()    ; break;
      case EQL     : ss = _EQL()    ; break;

      case LSH     : ss = _LSH()    ; break;
      case RSH     : ss = _RSH()    ; break;

      case NEG     : ss = _NEG()    ; break;

      case LNOT : ss = _LNOT() ; break;
      case LAND : ss = _LAND() ; break;
      case LOR  : ss = _LOR () ; break;

      case AND  : ss = _AND()  ; break;
      case OR   : ss = _OR()   ; break;
      case XOR  : ss = _XOR()  ; break;

      case JMP  : ss = _JMP() ; break;
      case JZ   : ss = _JZ()  ; break;

      case CALL : ss = _CALL(); break;
      case RET  : ss = _RET() ; break;
      case CRASH : ss = _CRASH(); break;
      default:
        printf("CRASH %d\n",op);
        finished = true;
    }
    program_advance(ss);
  }

  void run(u8 *what){
    
    program = what;
    data_size = what[0] << 8 | what[1]; // início do programa é isso + 2
    program_jump(data_size);
    while(!finished)
      step();
  }
};

#define _8(x) x
#define _16(x) ((x & 0xFF00)>>8),(x & 0xff)
#define _32(x) ((x & 0xff000000)>>24),((x & 0xff0000)>>16),((x & 0xff00)>>8),(x & 0xff)


// PROGRAMA
u8 prog[] = {
#include "prog.txt"
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
  /*
  for(u16 i = 0 ; i < sizeof(prog) ; i++){
    printf("%d = %d\n", i, prog[i]);
  }*/
  _main();
}
#endif


// BUILT-IN FUNCTIONS

void _PRINT(VMState* state){
  state->popA();
  switch(state->regA.type){
    case t_u8:
    case t_u16:
    case t_u32:
    #ifdef ARDUINO
      Serial.print(state->regA.val.u);
    #else
      printf("%d",state->regA.val.u);
    #endif
    break;
    case t_i8:
      #ifdef ARDUINO
        Serial.print( state->regA.val.i);
      #else
        printf("%d",(i8) state->regA.val.i);
      #endif
    break;
    case t_i16:
    case t_i32:
    #ifdef ARDUINO
      Serial.print( state->regA.val.i);
    #else
      printf("%d",state->regA.val.i);
    #endif
    break;   
    case t_f32:
      printf("%f",state->regA.val.f);
    break; 
    case t_str:{
      printf("%s", (char*) (state->program + state->regA.val.u + 2) );
    }
    break;
  }
}

u8 vm_print(VMState* state){
  _PRINT(state);
  return 0;
}

u8 vm_println(VMState* state){
  _PRINT(state);
  #ifdef ARDUINO
    Serial.print("\n");
  #else
    printf("\n");
  #endif
  return 0;
}

u8 vm_delay(VMState* state){
  state->popA();
  #ifdef ARDUINO
  delay(state->regA.val.u);
  #else
  printf("delay %d ms \n",state->regA.val.u);
  #endif
  return 0;
}

u8 vm_pin_mode(VMState* state){
  state->pop_operands();
  #ifdef ARDUINO
  pinMode(state->regA.val.u,state->regB.val.u);
  #else
  printf("pin %d mode %d\n",state->regA.val.u,state->regB.val.u);
  #endif
  return 0;
}

u8 vm_out(VMState* state){
  state->pop_operands();
  #ifdef ARDUINO
  digitalWrite(state->regA.val.u,state->regB.val.u);
  #else
  printf("write %d to %d\n",state->regB.val.u,state->regA.val.u);
  #endif
  return 0;
}

u8 vm_pin(VMState* state){
  state->popA(); // takes 1 value
  #ifdef ARDUINO
  u8 pin_num = state->regA.val.u;
  state->push_u( digitalRead(pin_num), t_u8);
  #else
  state->push_u(0,t_u8);
  printf("read from %d\n",state->regA.val.u);
  #endif
  return 1; // returns 1 value
}