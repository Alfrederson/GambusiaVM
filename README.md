# GambusiaVM
Máquina virtual e testes relacionados.


gambusiavm.js : análogo da máquina virtual
outro arquivo.html: para analisar .hex gerado pelo AVR-gcc.

O objetivo é:

- Fazer a máquina virtual que rode num Arduino qualquer
- Ser capaz de introduzir o código de VM dentro do arduino.

# Instruções da GambusiaVM
NOP    = Nada

PUSH x = Coloca X na pilha

ADD, SUB, DIV, MUL = Tira 2 valores da pilha, realiza a operação correspondente e coloca o resultado nela

EQL, DIFF = Tira 2 valores da pilha. Se forem iguais/diferentes coloca 1, senão 0.

OR = Tira 2 valores da pilha. Se um dos dois for diferente de 0, coloca 1.

AND = Tira 2 valores da pilha. Se os dois forem diferentes de 0, coloca 1

LT, GT, LTE, GTE = Tira 2 valores da pilha. Se o primeiro for respectivamente < , >, <=, >= o segundo, coloca 1.
 
NEG = Inverte o sinal do topo da pilha.

NOT = Se o topo da pilha for diferente de zero, vira zero.

SET x = Tira 1 valor da pilha e coloca na posição (tamanho da pilha - X)

LOAD x = Pega o valor da posição (tamanho da pilha - X) e coloca no alto da pilha

CALL x = Chama função em x

JMP x = Pula para a posição X

JZ x = Tira um valor do alto da pilha e pula para a posição X se o valor for zero

END = termina a execução.
