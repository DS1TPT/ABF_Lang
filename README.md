ABF, Ascii BrainFuck
===
ABF is an esoteric programming language based on BrainFuck.

List of commands
---
```
a: AND                                []: while(*ptr != 0) loop bracket
b: Break                              `: name marker
c: define Condition                   !: NEQ
d: Decrement byte                     @: set double pointer
e: EQU                                #: set integer pointer
f: Forward(set ptr to X)              $: set char pointer
g: Getchar                            %: modulus
h: Handle Error                       ^: bitwise XOR
i: Increment byte                     &: bitwise AND
j: Jump                               *: multiply
k: Knockdown(initialize interpreter)  (): if parentheses
l: Load file                          -: subtract
m: Match(copy value at X to Y)        _: print as double
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
Handling Argument(s)
- Every calculations' operand(s) is/are value at address X and/or adresss Y.
- Argument type of commands except for 'w' are int(unsigned/signed type depends on pointer mode).
- 'w'command's argument type is int when pointer type is char or int, and signed int or double when pointer type is double. Unsigned/signed type depends on pointer mode.
- Arguments must be written right after a command without space or anything else.
- Arguments must be separated using ','.
```
Example: a0,5 ; *mem && *(mem + 5)
```
  
Data and pointer processing
- size of char is 1B, size of int is 4B, and size of double is 8B For 32bit and 64bit systems.
- Pointer type and signed/unsigned will be maintained without specific commands including 'k'.
- Product of calculation is stored in buffer. To save the product to memory, you must set correct pointer type and use '=' command.
```
Example: f0w100f1w250-1,0f2= ;buffer = *(mem + 1) - *mem; *(mem + 2) = buffer;
```
- Address is set byte-wise. pointer type has no effect.
- Every data read/write starts from the byte at the current pointer/address, and it's the Left-Most Byte.
```
Example: #f5w255 ;write int type data from address 5 to 8. Data from 5 to 8: 0x000000ff
```
- The type check of the value stored in the memory will NOT performed.
```
Example: '#f0w2147483647$f2w0 ;value from 0 to 3: 0x7fff00ff
```
- '\\' command reads and outputs the value as much as the length corresponding to the pointer type. In the case of a double pointer, the value is converted to an integer and output.
- '_' command reads and outputs the value in floating-point type as much as the length of double type.
- '/' command calculates quotient if pointer type is not double.
- ':' command saves product to buffer in signed char type.
- When checking conditions for if statement, interpreter reads and compares data in previously set pointer type.
```
Example:'#c10,32,!(f0\) ;if int value at address 10 to 13 is not equal to value at address 32 to 35 then f0\
```
Command usage and syntax
- Before if statement, conditions must be defined using 'c' command.
- 'c' command needs 3 arguments. Argument order is: X, Y, comparative/logical operation command.
```
Example: c0,2,!(...) ;if (*mem != *(mem + 2)) {...}
```
- 'b' command stops command execution and starts it from next ']'.
- byte, pointer increment/decrement will not be affected by the pointer type.
- 'e' and '!' are comparative operator. They return 1 if condition is true, and 0 if condition is false.
- 'g' command uses stdin.
- 'h' command returns error code and halts program execution. Error code's type is unsigned int.
```
Example, z'h100w65p is written in the file which is being executed.
READY>> s
Program handled error(100)
READY>> 
```
- 'j' can only be used in file executions. To use this command you must define line number in BASIC style.
```
;Example
10 f0w65
20 p
30 j20 ;jumps to 20 and create infinite loop, printing 'A'.
```
- 'k' command initializes pointer type, internal buffers, settings, etc. However, the file remains open.
- 'm' command is affected by pointer type.
- 'p' command prints current byte value as char.
- 'q' command cannot be used without loading a file.
- 't' command is used for terminating file execution without error. It does not return error code.
- Any characters after 't' will be considered as remarks/comments.
- 'v' command can only be used in file execution. 'v' executes commands at the next line.
- Nested if and loop are supported.
- When loading a file, you must put '`' before and after the directory and name of the file.
```
Example: l`c:\abf\prg.abf`
```
- '/' command uses the value at X as numerator, and the value at Y as denominator.
- ';' command stops execution of the line immediately. Any characters after ';' and in the same line will be considered as remarks/comments.
  - Caution: 'v' and ';' has same function internally, but since the purpose of use of the two is different, it is recommended to use them separately.
- ':' command stores -1(0xff) if the value at X is less than the value at Y, 0 if it is equal, 1(0x01) if it is greater.
- '{' and '}' are not commands. They can only be used for condition of if statement.

Example Programs
---
print Hello, world
```
z'$f0w72>w101>w108m2,3f4w111>w44>w32>w119m4,8f9w114m3,10f11w100>w10>w13f0
;write "Hello, world" from address 0 and then set address to 0
[p>]zt ;print and increment pointer(address) while byte is not 0, fill memory with 0 and terminate after loop
```
