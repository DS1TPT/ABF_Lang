ABF, Ascii BrainFuck
===
ABF is an esoteric programming language inspired by BrainFuck. ABF can be considered as an dialect of BF.
- 8 basic BF commands(half of them are assigned to different characters)
- Various data types(schar, int, uchar, uint, double)
- Logical/comparative/four fundamental calculations/modulus/bit-wise/bit-shift operators
- Error code handling (to interpreter, flimsy)
- Jumps
- Subroutine support(using jump)
- integrated memory zerofill command
- Significantly more complex(?) syntax
- Spaghettification ability with jumps
- Library support(On my interpreter)
- Pseudo-random number support
- Supports writing strings without messing with ASCII code

Current version is: 1.12. Check UPDATE_LOG file to read update log.

Contents
---
- Commands of ABF
- Interpreter specific commands
- Syntax
- Example programs
- Interpreter
- Probably useful codes

Commands of ABF
---
```
The ABF language commands
a: AND                                       []: while(*ptr) loop bracket
b: break                                     `: string start/end marker
c: define condition                          !: NEQ
d: decrement value                           @: set double pointer
e: EQU                                       #: set integer pointer
f: forward(set ptr to X)                     $: set char pointer
g: get keyboard input and save as uchar      %: modulus
h: handle Error code                         ^: bitwise XOR
i: increment value                           &: bitwise AND
j: jump                                      *: multiply
k: key(get keyboard input without echo)      (): if parentheses
l: increase/decrease value by X              -: subtract
m: match(copy value at X to Y)               _: print as double
n: NOT                                       =: save product to current address
o: OR                                        +: add
p: print as char                             .: decimal, for double
q: increase/decrease pointer by X            ,: separator
r: return                                    <: decrement pointer
s: write string                              >: increment pointer
t: terminate file execution                  /: divide(*(p+X)/*(p+Y))
u: suspend for X second(s)                   ?: write random number
v: jump to next line(↓)                      ;: REM(comment)
w: write X                                   :: compare
x: exchange value at X with value at Y       ': set signed type
y: save current line num to line num buffer  ": set unsigned type
z: zerofill memory                           \: print as integer
|: bitwise OR                                {: bit shift to left
~: bitwise NOT                               }: bit shift to right
```
Note: 4 commands of BF('+', '-', '.', ',') are changed to 'i', 'd', 'p', 'g'. But 'i' and 'd' work a little bit different because they can also be used to increment/decrement the value of 32b integer and 64b double. Here's the example of BF Hello, world program translated into ABF.
```
++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++. ;BF
$iiiiiiii[>iiii[>ii>iii>iii>i<<<<d]>i>i>d>>i[<]<d]>>p>dddpiiiiiiippiiip>>p<dp<piiipddddddpddddddddp>>ip>iip ;direct-translation, '$' is being put to do byte-wise increment/decrement operations
$l8[>l4[>ii>l3>l3>iq-4d]>i>i>d>>i[<]<d]>>p>l-3pl7ppl3p>>p<dp<pl3pl-6pl-8p>>ip>iip ;simplified translation.
```

Interpreter specific commands
---
```
--help: print help message
--license: print license notice
--load name: load program file
--run: run program
--print-code: print source code of program file
--close: close program file
--init: initialize interpreter except for file pointers and command buffer
--compile-direct code: compile BF code to ABF. 'q' and 'l' won't be used
--compile code: compile BF code to ABF. 'q' and 'l' will be used
--exit: exit interpreter
--clrscr: clear screen
--import name: import a library file
--close-lib: close library file
++name: library only, subroutine [name] start point marker
--srend: library only, subroutine end point marker
--list-begin: library only, beginning of subroutine list
--list-end: library only, end of subroutin list
--record-start: start recording command line input to file
--record-stop: stop recording command line input
--record-del: delete record file
--record-print: print record file
--call name: call subroutine defined in the imported library
--disp-internal-vars: display values of internal variables
--disp-subroutine-list: display list of subroutines in library
```
Note1: name is literally the name of file or subroutine. name must not be wrapped with string marker command '`'.

Note2: Compiled code will be saved to record file if recording feature is active.

Syntax
---
*** Note ***
In the descriptions of the example codes below, "mem" is a representation of memory in a form of unsigned char array.

Handling Argument(s)
- Every calculations' operand(s) is/are value at address X and/or adresss Y.
- Argument type of commands except for 'w' are int(unsigned/signed type depends on pointer mode).
- 'w'command's argument type is int when pointer type is char or int, and signed int or double when pointer type is double. Unsigned/signed type depends on pointer mode.
- Arguments must be written right after a command without space or anything else.
- Size of arguments are 4 Bytes(typically) when pointer type is int/char, and 8 bytes(typically) when pointer type is double.
- Arguments must be separated using ','.
```
Example: a0,5 ; *mem && *(mem + 5)
```
- If non-last argument is not given, argument will set to current address.
```
Example: $+,1000 ; *(mem + addr) + *(mem + 1000)
```
- If last argument is ',', argument will set to current address.
```
Example: $m1000,, ; *(mem + addr) = *(mem + 1000)
```
- for 'h', 'l', and 'q', the argument is forced to signed type regardless of pointer mode.
  
Data and pointer processing
- size of char is 1B, but the size of int and double is system-dependant.
- Pointer type and signed/unsigned will be maintained without specific commands.
- Product of calculations except for shift calculations('{' and '}') will be stored in buffer. To save the product to memory, you must set correct pointer type and use '=' command.
```
Example: f0w100f1w250-1,0f2= ;buffer = *(mem + 1) - *mem; *(mem + 2) = buffer;
```
- Address is set byte-wise. pointer type has no effect.
- Every data read/write starts from the byte at the current pointer/address.
```
Example: #f5w255 ;write int type data from address 5 to 8. Byte value from 5 to 8: 0xff000000(little-endian)
```
- The type check of the value stored in the memory will NOT performed.
```
Example: '#f0w2147483647$f2w0 ;value from 0 to 3: 0xffff007f(little-endian)
```
- '\\' command reads and outputs the value as much as the length corresponding to the pointer type. In the case of a double pointer, the value is converted to an integer and output.
- '_' command reads and outputs the value in floating-point type as much as the length of double type.
- '/' command calculates quotient if pointer type is not double.
- ':' command ALWAYS saves product to buffer in signed char type.
- 'x' command uses buffer. The value at X may remain in the buffer.
- When checking conditions for if statement, interpreter reads and compares data in previously set pointer type.
```
Example:'#c10,32,!(f0\) ;if int value at address 10 to 13 is not equal to value at address 32 to 35 then f0\
```
- Bit-wise operations(except for shift) and modulus are prohibited when double pointer is active.
- '=' command saves the value at buffer to current pointer according to the pointer type. The operation result and size stored in the buffer are affected by the command and pointer mode, and unexpected behavior may occur if the operation value stored in the buffer is read with the wrong pointer type. The':' command writes the operation result to the first byte of the buffer in signed char format, while other commands write the operation result as much as the size of data type from the first part of the buffer.
```
Note: these data examples are in little-endian.
Data stored in buffer after z'#f0w15f4w31f8:0,4 :
ff 00 00 00 0f 00 00 00 1f 00 00 00 ... 00
Data stored in buffer after z'#f0w15f4w31f8+0,4 :
2e 00 00 00 0f 00 00 00 1f 00 00 00 ... 00
Date stored in buffer after z'$f0w15f4w31f8+0,4 :
2e 00 00 00 00 00 00 00 00 00 00 00 ... 00
```
Command usage and syntax
- Before if statement, conditions must be defined using 'c' command.
- 'c' command needs 3 arguments. Argument order is: X, Y, comparative/logical operation command.
```
Example: c0,2,!(...) ;if (*mem != *(mem + 2)) {...}
```
- Among the operators passed as arguments to the 'c' command, '~' and 'n' require only one argument for calculation. Internally, they use the value of address X, so argument Y will be ignored. You can pass arguments in two ways.
```
Method 1: c1,1,n(...) ;if (! *(mem + 1)) {...}
Method 2: c1,,n(...)
```
- 'b' command stops command execution and starts it from next ']'.
- pointer(address) incrementation and decrementation will not be affected by the pointer type.
- 'i' and 'd' increments/decrements data value. These commands are affected by the pointer type. 
- 'e' and '!' are comparative operator. They return 1 if condition is true, and 0 if condition is false.
- 'h' command returns error code and halts program execution. Error code's type is signed int.
```
Example, z'h100w65p is written in the file which is being executed.
READY(0,0000)>> s
Program handled error(100)
READY(0,0000)>> 
```
- 'j' can only be used in file executions. To use this command you must define line number in BASIC style.
```
;Example
10 f0w65
20 p
30 j20 ;jumps to 20 and create infinite loop, printing 'A'.
```
- line number is not essential. You can skip writing line number if jump to that line won't be happend.
```
;Example
10 z'$
20 j50
30 v
f0w88pzt
50 f0w65pw0j30
;execution order: 10 -> 20 -> 50 -> 30 -> "f0w88pzt"
```
- line number, ABF commands and remarks(comments) must NOT be written with interpreter specific commands in the same line.
```
10 --import basic.abl ;incorrect
10 v
--import basic.abl
;correct example above
```
- 'l' command increase value by X. If argument is negative, it decreases value.
- 'm' and 'x' are affected by pointer type.
- 'p' command prints current byte value as char.
- 'q' command increase pointer by X. If argument is negative, it decreases pointer.
- 'r' command is return. codes in the next line of the line stored in line number buffer by 'y' command will be executed.
```
;Example
10 z'$
20 yj100 ;save line number "20" to line number buffer, then jump to 100
25 c,,e() ;starts from here after 'r' at 110
30 f0zt

100 f0w90>w64[ipc0,1,e(b)] ;print ABCDE...XYZ
110 r ;return to the next line of "20"
```
- 's' command writes string wrapped by '`' to current address. It also writes null to the end of string.
```
f100s`Hello, world!` ;writes Hello, world!\0 from 100
```
- 't' command is used for terminating file execution without error. It does not return error code.
- Any characters after 't' will be considered as remarks/comments.
- 'u' suspends(sleep) program execution for X second(s).
- 'v' executes commands at the next line.
- 'x' command exchanges value at X with value at Y. This command is affected by pointer type.
```
;Example
$f0w88>w120 ;X at 0, x at 1
x0,1 ;x at 0, X at 1
```
- 'y' command saves current BASIC style position to position buffer.
- Nested if and loop are supported.
- Subroutine can be implemented in file(s) using 'j', 'r', 'y' commands. Please refer to Example programs.
- '/' command uses the value at X as numerator, and the value at Y as denominator.
- Any characters after ';' and in the same line will be considered as remarks/comments.
- ':' command stores -1(0xff) if the value at X is less than the value at Y, 0 if it is equal, 1(0x01) if it is greater.
- '{' and '}' are shift operators. They shift bit value to left or right. These operators are affected by pointer type. Unlike the other bit-wise operators, these operators can be used when the double pointer is active.
- Corresponding brackets and parentheses must be in the same line
```
z'$f0w100>[ip<d>] ;Correct
z'$f0w100>[ip<
d>]               ;Wrong!
```
- In libraries, jump-related commands are not blocked by interpreter, but they must not be used. Also it's not a good idea to use 'z' unless totally necessary.
- Upper-case letters and space are not commands. Use them to insert remarks/comments between codes.
```
;Example code
z"$f0 INIT s`Hello, world!`f0[p>] PRINT STR f0[w0>]t CLEAR ;prints Hello, world! then terminate
;The code above is equivalent to:
z"$f0s`Hello, world!`f0[p>]f0[w0>]t
```

Example Programs
---
print Hello, world using 's'
```
z'$f0s`Hello, world`f0[p>]zf0t
```
print Hello, world without using 's'
```
z'$f0w72>w101>w108m2,3f4w111>w44>w32>w119m4,8f9w114m3,10f11w100>w10>w13f0
;write "Hello, world" from address 0 and then set address to 0
[p>]zt ;print and increment pointer(address) while byte is not 0, fill memory with 0 and terminate after loop
```
Big endian to little endian(32b int), subroutine implementation example(File exe only)
```
;Write uint data 0x12345678 in Big endian(on little endian system)
1 f1000"#
10 w2018915346\ ;uint 0x12345678 in Big endian
15 yj160
20 yj100
30 \ ;305419896 should be printed
40 yj160
50 f1000yj150 ;185286120 should be printed
60 yj160
70 f1004yj150 ;120865218 should be printed
80 zf0'$t

;INT32 big endian to little endian
100 "$f1000
110 m1003,1004m1002,1005m1001,1006m1000,1007
120 "#f1004
130 r

;print 4B integer byte-wise in integer type
150 "$\>\>\>\<<<r
;print line feed and CR
160 "$f0w10pw13pw0r

;memory map
;address 0: char to print
;address 1000 to 1003: big-endian int32
;address 1004 to 1007: little-endian int32
```

Interpreter
---
Interpreter is written in C language. Please let me know if you find any bug(s).

Guide based on current interpreter version(1.12).

I am just a hobbyist, therefore the source code may be complicated to read and/or have bugs. The descriptions below are based on tests performed on MS Windows(x86) Environment. Note: Interpreter built by gcc and tested on Debian linux.

- Specifications

Memory available: 65535 Bytes

Command buffer: 4096 Bytes including null

File name length limit: 1024 Bytes including null

Size of int: 4 Bytes(system dependant)

Size of double: 8 Bytes(system dependant)

Data types supported: signed char, unsigned char, int, unsigned int, double

- Introduction

The win32 interpreter is built to run on Windows x86 systems. If you are not using an x86 or x86_64 environment, or if you are using Unix-based OS or Linux, you must build the source code yourself.

In order to start interpretation of a file without using prompt, you must give correct location and name of the file as argument to interpreter.
```
Windows example: abf_lang_intrpr.exe c:\abf\pi.abf
*nix example: ./abf_lang_intrpr /abf/pi.abf

Result: 3.14159265359
```

If you start the interpreter without an argument, the following screen is displayed.
```
Ascii BrainFuck Language Interpreter Prompt v1.12
int size: 4 Bytes, double size: 8 Bytes
int range: -2147483648 to 2147483647, double range: (+-) 2.22507385851e-308 to 1.79769313486e+308
Memory size: 65535 Bytes
Maximum command length per line: 4096 Bytes including null
Maximum file name length: 1024 Bytes including null
Byte order is Little endian.
***Type --help for commands, --license for license notice***

READY(0,00000)>>
```
If the interpreter is started in prompt mode, the size and range of the int and double data types are displayed as shown in the screen above. It also prints the size of memory, size of commands that can be entered per line, and endianness.
When the interpreter processed command(s) and waits for the next command, the following is output:
```
READY(x,yyyyy)>>
```
x represents the current pointer mode. There are 5 pointer modes: 0: char, 1: int, 2: double, 3: unsigned char, and 4: unsigned int.
yyyyy represents the current memory address in decimal.

When loading a file, you must specify the extension. Path is not required when the file is in the same directory.
```
;Windows example
--import \lib\basic.abl
--load c:\abf\helloworld.abf
;Linux example
--import /lib/basic.abl
--load /abf/helloworld.abf
```
If an error occurs during command execution, an error message is displayed and the command execution is stopped. If the file is running, the file will stop being executed. An example error message is as follows:
```
READY(0,00000)>> `
?Error string marker without command. cmd at 0, line 0
READY(0,00000)>>
```
The error message specifies where the error occurred in the command, such as "cmd at 0, line 0". If line is 0, an error occurred while entering and executing a command in the interpreter without loading a file. Error messages are output in the format "?[error message]".

Before executing the command, the interpreter checks whether the number of opening and closing parts of the parentheses () of the if statement and the brackets [] of the loop statement are the same. If the number of open parts and the number of closed parts do not match, the following error message is displayed.
```
READY(0,00000)>> []]
?Error parentheses and/or brackets are not balanced
READY(0,00000)>>
```
In the interpreter, the 'g' and 'k' commands perform the same function as MSVC's non-standard function _getch() and _getche(). In other words, as soon as a character is entered on the keyboard, the value is stored in the memory. For POSIX-compliant systems, those functions have been implemented using termios. However, if the operating system is not MS Windows, Unix-based operating systems or Linux, the 'g' command may not be executed normally. In this case, the user must modify the source code and implement it.

When writing a value to memory as an int or double type, the interpreter checks that the memory range is not exceeded. The maximum value of the memory address is 65534. If an address exceeding 65531 for int and 65527 for double is set, the maximum value of memory address is exceeded and an error message is displayed.
```
READY(1,65531)>> w5647

READY(1,65531)>> >w5647
?Error address out of range. cmd at 1, line 0
READY(1,65532)>> @f65527w2.2

READY(2,65527)>> >w2.2
?Error address out of range. cmd at 1, line 0
READY(2,65528)>>
```
If the pointer mode is double, an error message is output if the bitwise operator and modulus operator are used.
```
READY(0,00000)>> @%1,10
?Error modulus operation in floating point format. cmd at 1, line 0
READY(2,00000)>> @|1,10
?Error bitwise operation in floating point format(|). cmd at 1, line 0
READY(2,00000)>>
```
If you use the 'w' command while using an int or double pointer, the interpreter does not check the size of the input value.

If the interpreter receives an error from the program, it displays an error code and then stops executing the program.
```
READY(0,00000)>> --run

Program handled error(-1)... Program execution halted.
READY(0,00000)>>
```
The operation result stored in the buffer is retained unless a command that changes the value of the buffer is executed. After performing an operation using this behavior, you can save the operation result in multiple addresses in the memory.
```
READY(0,00000)>> w1>w2:0,1>=\>=\
-1-1
READY(0,00003)>>
```
Recording saves command line input, error messages, compiled code(BF -> ABF) to "abf_record" file. Record file is in the directory where the interpreter is located. If record file does not exist, interpreter will make one. If not, the interpreter will start saving from the new line. Record file contains when the recording is started.
```
[Interpreter input]
READY(0,00000)>> --record-start

READY(0,00000)>> w123"$f1w16>%0,1=>>/0,1=<%4,1=f4w48>w48>w58f2[df5ic5,6,e(w65)f2]f3[df4ic4,6,e(w65)f3]f4p>pf0z
7B
READY(3,00000)>> --compile ++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.
l8[>l4[>ii>l3>l3>iq-4d]>i>i>d>>i[<]<d]>>p>l-3pl7ppl3p>>p<dp<pl3pl-6pl-8p>>ip>iip
READY(3,00000)>> --compile-direct ++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.
iiiiiiii[>iiii[>ii>iii>iii>i<<<<d]>i>i>d>>i[<]<d]>>p>dddpiiiiiiippiiip>>p<dp<piiipddddddpddddddddp>>ip>iip
READY(3,00000)>> --record-stop

READY(3,00000)>> --record-start

READY(3,00000)>> s`Second time recording`f0[p>]z
Second time recording
READY(3,00021)>> [[]
?Error parentheses and/or brackets are not balanced
READY(3,00021)>> --record-stop

READY(3,00021)>>
```
```
[Record file output]
Recording started at: [DATE&TIME]
w123"$f1w16>%0,1=>>/0,1=<%4,1=f4w48>w48>w58f2[df5ic5,6,e(w65)f2]f3[df4ic4,6,e(w65)f3]f4p>pf0z
--compile ++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.
Compiled code: l8[>l4[>ii>l3>l3>iq-4d]>i>i>d>>i[<]<d]>>p>l-3pl7ppl3p>>p<dp<pl3pl-6pl-8p>>ip>iip
--compile-direct ++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.
Compiled code: iiiiiiii[>iiii[>ii>iii>iii>i<<<<d]>i>i>d>>i[<]<d]>>p>dddpiiiiiiippiiip>>p<dp<piiipddddddpddddddddp>>ip>iip
--record-stop
Recording ended at: [DATE&TIME]
Recording started at: [DATE&TIME]
s`Second time recording`f0[p>]z
[[]
Error: parentheses and/or brackets are not balanced
--record-stop
Recording ended at: [DATE&TIME]

```

Probably useful codes
---
Press any key to continue, uses address 0(temporary)
```
$f0w10pw13pw32pw80pw114pw101pw115ppw32pw97pw110pw121pw32pw107pw101pw121p
w32pw116pw111pw32pw99pw111pw110pw116pw105pw110pw117pw101pw46pppw32p
gw8pw32pw0r
```
Print byte value in hex, argument at 0(byte val), temporary 1 - 6
```
"$f1w16>%0,1=>>/0,1=<%4,1=f4w48>w48>w58f2[df5ic5,6,e(w65)f2]f3[df4ic4,6,e(w65)f3]f4p>p
>w0<w0<w0<w0<w0<w0<r ;clear temporary data
```
Print byte value in bin, argument at 0(byte val), temporary 1 - 5
```
"$f4w0>w1<\\\\\\\f4w8f3w2</0,3=<%0,3=\f4ppf2[<%2,3=\>/2,3=f4ppf2c2,5,e(b)]\
f1w0>w0>w0>w0>w0r ;clear temporary data
```
Get string while CR or LF, writes from 0, temporary 65533 - 65534, 65532 is always null
```
$f65531w0>w0>w10>w13f0w1[gc,65533,e(w0b)c,65534,e(w0b)c65531,65532,!(b)>w1]f65531w0>w0f0r
```
