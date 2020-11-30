ABF, The Alphabetical BrainFuck Language
===
The Alphabetical Brainfuck is a esoteric programming language based on the BF.

Commands
---
```
a: AND                                []: while(*ptr != 0) loop bracket
b: Break                              `: name marker
c: define Condition                   !: NEQ
d: Decrement byte                     @: set double pointer
e: EQU                                #: set integer pointer
f: Forward(set ptr to X)              $: set char pointer
g: Getchar                            %: remainder(mod)
h: Handle Error                       ^: bitwise XOR
i: Increment byte                     &: bitwise AND
j: Jump                               *: multiply
k: Knockdown(initialize interpreter)  (): if parentheses
l: Load file                          -: subtract
m: Match(copy current value to Y)     _: print as double
n: NOT                                =: save product to current address
o: OR                                 +: add
p: Print as char                      .: decimal, for double
q: Quine                              ,: separator
r: Remove condition(s)                <: decrement pointer
s: Start file execution               >: increment pointer
t: Terminate file execution           /: divide(*(p+X)/*(p+Y))
u: Unload file                        ?: help(command list)
v: jump to next line(↓)               ;: REM(comment)
w: Write X                            :: compare
x: close interpreter                  ': set signed type
y: whY(toggle debugging mode)         ": set unsigned type
z: Zero all bytes                     \: print as integer
|: bitwise OR                         {: condition *(p+X) < *(p+Y)
~: bitwise NOT                        }: condition *(p+X) > *(p+Y)
```

Syntax
---
