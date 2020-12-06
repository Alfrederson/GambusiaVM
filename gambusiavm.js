const opnames = 
"NOP,PUSH,ADD,SUB,DIV,MUL,EQL,DIFF,OR,AND,LT,LTE,GT,GTE,NEG,NOT,SET,LOAD,CALL,JMP,JZ,END";

var counter = 0,
    oc = {}    
opnames.split(",").forEach( v => oc[v] = counter++ )

const
stack_size = 256,
push = valor => stack[--stack_top] = valor,
pop = () => stack[stack_top++],
peek = () => stack[stack_top],
set  = val => stack[stack_top] = val,
heap = p => stack[p],
store = (p,v) => stack[p]=v,
OPCODE = 0,
ARG    = 1,
functions = {
	print : () => document.write( `${pop()} <br/>` )
}

let stack = new Array(stack_size),
    stack_top = 256,
asm = 
`
0:PUSH 0
1:SET 14    ;   x
2:LOAD 14 ; loop_start_1
3:PUSH 13
4:LT
5:JZ 13 ; loop_end_1
6:LOAD 14
7:CALL print
8:LOAD 14
9:PUSH 3
10:ADD
11:SET 14    ;   x
12:JMP 2 ; loop_start_1
13:END ; loop_end_1
`,
program = []

asm.split("\n").forEach( l => {
	if(l === "") return
	l = l.split(":")[1].split(";")[0].trim().split(" ")
  console.log(l)
  l[0] = oc[l[0]]
  if(l.length == 2){
  	let num = parseInt(l[1])
    if(!isNaN(num)) l[1] = num
  } 
	program.push(l)
})

let program_counter = 0,
    program_size    = program.length,
    program_done = false

console.log(`Program has ${program.length} instructions`)
const arg = qual => program[program_counter][ARG + qual]

const instructions = {
	[oc.PUSH]: () => push(program[program_counter][ARG]),
  [oc.ADD ]: () => push ( pop() + pop() ),
  [oc.SUB ]: () => push ( pop() - pop() ),
  [oc.MUL ]: () => push ( pop() * pop() ),
  [oc.DIV ]: () => push ( pop() / pop() ),
  [oc.NEG ]: () => set  ( -peek() ),
  [oc.NOT ]: () => set  ( peek() === 0 ? 1 : 0 ),
  [oc.LOAD]: () => push ( heap(arg(0)) ),
  [oc.SET ]: () => store( arg(0), pop() ),
  [oc.CALL]: () => functions[arg(0)]()  ,
  [oc.LT  ]: () => push ( pop() < pop() ? 0 : 1),
  [oc.GT]  : () => push ( pop() > pop() ? 0 : 1),
  [oc.LTE] : () => push ( pop() <= pop() ? 0 : 1),
  [oc.GTE] : () => push ( pop() >= pop() ? 0 : 1),
  [oc.JZ  ]: () => {
  	if ( pop() === 0 ) program_counter = arg(0)-1
  },
  [oc.JMP]: () => {program_counter = arg(0)-1 },
  [oc.END]: () =>{
  	program_done = true
  }
}

while(program_counter < program_size){
  instructions[ program[program_counter][OPCODE] ]()
	program_counter++
  if(program_done) break
}

document.write("Done!</br>")
