let stack = [0,0,0,0,0,0,0,0,0,0];
let stack_top = -1;

const push = valor => {
	stack_top ++;
  stack[stack_top] = valor;
}

const pop = () =>{
	stack_top --;
  return stack[stack_top+1];
}

const PUSH = 1,
      ADD  = 2,
      SUB  = 3,
      DIV  = 4,
      MUL  = 5,
      OUT  = 6;
      
const OPCODE = 0,
      ARG    = 1;
/*      
// programa gerado pelo compilador.
Não sei como fazer isso funcionar ainda.
2
4
5
MUL
ADD
3
ADD
STORE x
*/

let program = [
	[PUSH,2],
  [PUSH,4],
  [PUSH,5],
  [MUL],
  [ADD],
  [PUSH,3],
  [ADD],
  [OUT]
]

let program_counter = 0;
let program_size    = program.length;

console.log("Programa tem "+program_size+" instruções");

let instrucoes_executadas = 0;

while(program_counter < program_size){
	let a,b;
	console.log(stack);
	switch( program[program_counter][OPCODE] ){
    // empurra o argumento na pilha
  	case PUSH:
    	push(program[program_counter][ARG]);
    break;
    case ADD:
    	// pega os dois do alto da pilha, soma, coloca no lugar do segundo e faz um pop.
      a = pop();
      b = pop();
      push ( a + b);
    break;
    case SUB:
    	a = pop()
      b = pop()
      push ( a - b);
    case MUL:
      a = pop();
      b = pop();
      push ( a * b);    
    break;
    case DIV:
    	a = pop()
      b = pop()
      push ( a / b);
    break;
      
    case OUT:
    	// Exibe o que estiver no alto da pilha e faz um pop.
      console.log("OUT-- "+pop());
    break;
  }
	program_counter++;
	instrucoes_executadas++;
}

console.log(instrucoes_executadas + " instruções executadas.");
