
push fx
push ex
add
pop ex

push 2

push fx
pop [ex]
pop fx

call :main
hlt


sum:

push [ex+1]
push [ex+2]
add
push [ex]
push ex
push [ex]
sub
pop ex
pop fx
pop ax
ret


some_fnc:

while_1:
push [ex+1]
push 5
jl :while_exec_1
jmp :while_exit_1
while_exec_1:
push 10128
push 25152
strout
push [ex+1]
push 1
add
pop [ex+1]
jmp :while_1
while_exec_1:

push 0
push [ex]
push ex
push [ex]
sub
pop ex
pop fx
pop ax
ret


main:
push 1
pop [ex+1]

while_2:
push [ex+1]
push 5
jl :while_exec_2
jmp :while_exit_2
while_exec_2:
push 12088
push 25152
strout
push [ex+1]
push 1
add
pop [ex+1]
jmp :while_2
while_exec_2:

push fx
push ex
add
pop ex

push 3

push fx
pop [ex]
pop fx

push 2
push 1
pop [ex+1]
pop [ex+2]
call :sum

push fx
push ex
add
pop ex

push 2

push fx
pop [ex]
pop fx

push -5
pop [ex+1]
call :some_fnc

push [ex+1]
push 1
je :if_cond_1
jmp :next_cond_1_1
if_cond_1:
push 123
out
jmp :exit_cond_1

next_cond_1_1:
push [ex+1]
push 1
jg :elif_cond_1_1
jmp :next_cond_1_2
elif_cond_1_1:
push 456
out
jmp :exit_cond_1

next_cond_1_2:
push 789
out

exit_cond_1:

push 0
push [ex]
push ex
push [ex]
sub
pop ex
pop fx
pop ax
ret

