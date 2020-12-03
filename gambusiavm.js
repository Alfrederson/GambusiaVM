const
stack_size = 256,
push = valor => stack[--stack_top] = valor,
pop = () => stack[++stack_top-1],
peek = () => stack[stack_top],
set  = val => stack[stack_top] = val,
heap = p => stack[p],
store = (p,v) => stack[p]=v,
opcodes = {
      NOP  : 0,
			PUSH : 1,
      ADD  : 2,
      SUB  : 3,
      DIV  : 4,
      MUL  : 5,
      OUT  : 6,
      NEG  : 7,
      NOT  : 8,
      SET  : 9,
      LOAD : 10,
      CALL : 11
},
OPCODE = 0,
ARG    = 1
functions = {
	print : () => document.write( `${pop()} <br/>` )
}

let stack = new Array(stack_size),
    stack_top = 256,
asm = 
`PUSH 90
PUSH 30
ADD
CALL print
PUSH Hello
CALL print
PUSH World
CALL print
`,
program = []

asm.split("\n").forEach( l => {
	l = l.trim().split(";")[0].split(" ")
  l[0] = opcodes[l[0]]
  if(l.length == 2){
  	let num = parseInt(l[1])
    if(!isNaN(num)) l[1] = num
  } 
	program.push(l)
})

let program_counter = 0,
    program_size    = program.length

console.log(`Program has ${program.length} instructions`)

const instructions = {
	[opcodes.PUSH]: () => push(program[program_counter][ARG]),
  [opcodes.ADD ]: () => push ( pop() + pop() ),
  [opcodes.SUB ]: () => push ( pop() - pop() ),
  [opcodes.MUL ]: () => push ( pop() * pop() ),
  [opcodes.DIV ]: () => push ( pop() / pop() ),
  [opcodes.NEG ]: () => set  ( -peek() ),
  [opcodes.NOT ]: () => set  ( peek() === 0 ? 1 : 0 ),
  [opcodes.LOAD]: () => push ( heap(program[program_counter][ARG]) ),
  [opcodes.SET ]: () => store(program[program_counter][ARG], pop() ),
  [opcodes.CALL]: () => functions[program[program_counter][ARG]]() 
}

while(program_counter < program_size-1){
  instructions[ program[program_counter][OPCODE] ]()
	program_counter++
}
