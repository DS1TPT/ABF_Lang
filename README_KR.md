ABF, 아스키 브레인퍽
===
ABF는 브레인퍽에 기반해 제작된 난해한 프로그래밍 언어입니다.

목차
---
- 명령어 목록
- 문법
- 예제 프로그램
- win32 인터프리터

명령어 목록
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

문법
---
인수 전달
- 모든 연산은 주소 X에 있는 값과 주소 Y에 있는 값을 피연산자로 합니다.
- 'w'명령어를 제외한 명령어의 인수는 모두 int 타입입니다. 부호의 유무는 포인터 타입에 의해 결정됩니다.
- 'w'명령어는 char, int 타입일 때 int를, double 타입일 때 int와 부동소수점 타입으로 인수를 전달해야 합니다. 부호의 유무는 포인터 타입에 의해 결정됩니다.
- 인수는 명령어 바로 뒤에 공백문자나 다른 명령어 없이 전달해야 합니다.
- 인수는 ','로 구분합니다.
```
예시: a0,5 ; *mem && *(mem + 5)
```
  
데이터와 포인터 처리
- 32비트, 64비트 시스템의 경우 char 타입은 1바이트, int 타입은 4바이트, double 타입은 8바이트입니다.
- 지정된 포인터 타입과 부호는 별도의 명령('k' 포함) 없이는 변화없이 유지됩니다.
- 연산결과는 메모리에 저장되지 않고 버퍼에 저장됩니다. 메모리에 저장하려면 연산 후 알맞은 타입을 지정 후 '=' 명령을 사용합니다.
```
예시: f0w100f1w250-1,0f2= ; buffer = *(mem + 1) - *mem; *(mem + 2) = buffer;
```
- 주소는 바이트 단위로 지정됩니다. 이는 포인터의 데이터 형식과 연관이 없습니다.
- 모든 데이터 읽기/쓰기는 현재 포인터가 가르키는 위치를 시점으로 합니다.
```
예시: #f5w255 ;주소 5부터 8까지 데이터를 기록. 주소 5부터 8의 값: 0x000000ff
```
- 메모리에 저장된 값의 타입 검사를 하지 않습니다.
```
예시: '#f0w2147483647$f2w0 ;메모리 0부터 3까지의 값: 0x7fff00ff
```
- '\\'명령은 포인터 타입에 해당하는 길이만큼 값을 읽어 출력합니다. double 포인터의 경우 8바이트 정수로 변환하여 출력합니다.
- '_'명령은 무조건 double 타입에 해당하는 길이만큼 값을 읽어 부동소수점으로 출력합니다
- '/'명령은 double 포인터가 아닌 경우 몫을 계산합니다.
- ':'명령은 signed char 타입으로 값을 버퍼에 저장합니다.
- if문 처리 시 조건문은 이전에 설정된 포인터 타입으로 메모리의 값을 읽어 연산합니다.
```
예시:'#c10,32,!(f0\) ;주소 10부터 13까지의 부호 있는 정수가 주소 32부터 35까지의 값과 같지 않은 경우 f0\ 실행
```
- double 포인터를 사용하는 경우 비트연산자와 나머지 연산자를 사용할 수 없습니다.
- '=' 명령은 현재 포인터 타입에 맞게 값을 저장합니다. 버퍼에 저장되는 연산결과와 크기는 명령어와 포인터 모드의 영향을 받으며, 버퍼에 저장된 연산값을 잘못된 포인터 타입으로 읽는 경우 예기치 못한 동작이 발생할 수 있습니다. ':' 명령은 연산결과를 버퍼의 첫 바이트에 signed char 형식으로 값을 기록하며, 그 이외의 명령은 연산결과를 포인터 타입의 크기만큼 버퍼의 첫 부분부터 값을 기록합니다.
```
z'#f0w15f4w31f8:0,4 명령을 실행했을 때 버퍼에 저장된 값:
ff 00 0f 00 01 f0 00 00 00...00
z'#f0w15f4w31f8+0,4 명령을 실행했을 때 버퍼에 저장된 값:
2e 00 0f 00 01 f0 00 00 00...00
z'$f0w15f4w31f8+0,4 명령을 실행했을 때 버퍼에 저장된 값:
2e 00 00 00...00
```
명령어 문법과 사용
- if문 이전에 반드시 'c'명령으로 조건문을 정의해야 합니다.
- 'c'명령은 3개의 인수가 필요합니다. X, Y, 비교/논리연산자 순서로 인수를 적어야 합니다. 단, ':' 연산자는 사용할 수 없습니다.
```
예시: c0,2,!(...) ;if (*mem != *(mem + 2)) {...}
```
- 'b'명령은 루프문 실행을 멈추고 바로 다음 ']'부터 명령을 실행하도록 합니다.
- 바이트, 포인터 증감 연산은 포인터 타입에 영향을 받지 않습니다.
- 'e'와 '!'는 비교연산자로, 조건이 참일 때 1을, 거짓일 때 0을 반환합니다.
- 'g'명령은 키보드에서 한 글자가 입력된 경우 즉시 그 값을 저장합니다.
- 'h'명령은 인터프리터에 오류코드(인수)를 반환하고 프로그램 실행을 정지합니다. 오류코드는 signed int 타입입니다.
```
예시, 실행할 파일에 z'$h100w65p 가 적혀있는 경우
READY(0,0000)>> s
Program handled error(100)
READY(0,0000)>> 
```
- 'j'명령은 파일 실행 시에만 가용합니다. 또한, BASIC 스타일로 줄번호를 명시하여야 사용할 수 있습니다.
```
;예제
10 f0w65
20 p
30 j20 ;주소 0의 값('A')을 무한히 출력
```
- 'k'는 포인터 타입, 내부 버퍼와 설정 등을 모두 초기화합니다. 단, 파일은 열린 상태로 유지합니다.
- 'm'명령은 포인터 타입의 영향을 받습니다.
- 'p'명령은 현재 바이트 값을 문자로 출력합니다.
- 'q'명령은 파일을 불러오지 않으면 사용할 수 없습니다.
- 't'명령은 프로그램을 정상적으로 종료시킬 때 사용하며, 오류코드를 반환하지 않습니다.
- 't'명령 이후의 모든 문자는 주석처리됩니다.
- 'v'명령은 파일 실행 시에만 사용할 수 있으며, 바로 다음 줄을 실행합니다.
- 중첩 if문과 중첩루프가 지원됩니다.
- 파일을 불러오는 경우 경로와 파일명 앞과 뒤에 '`'를 적어야 합니다.
```
예시: l`c:\abf\prg.abf`
```
- '/'명령은 주소 X의 값을 분자로, 주소 Y의 값을 분모로 합니다.
- ';'명령은 해당 줄의 실행을 즉시 중단합니다. 같은 줄이며 ';'뒤에 있는 문자는 모두 주석처리됩니다.
  - 주의: 'v'와 ';'는 내부적으로 같은 기능을 하지만, 사용 목적이 다르므로 구분하여 사용하는 것을 권장합니다.
- ':'명령은 주소 X의 값이 주소 Y의 값보다 작은 경우 -1(0xff)을, 같은 경우 0을, 큰 경우 1(0x01)을 버퍼에 기록합니다.
- '{'과 '}'는 명령어가 아닙니다. if문의 조건문에서만 사용할 수 있습니다.

예제 프로그램
---
Hello, world 출력
```
z'$f0w72>w101>w108m2,3f4w111>w44>w32>w119m4,8f9w114m3,10f11w100>w10>w13f0
;"Hello, world"를 메모리 주소 0부터 기록 후 주소를 0으로 설정
[p>]zt ;메모리 현 주소의 값이 0이 될 때 까지 문자를 출력 후 주소 증가, 이후 메모리를 0으로 초기화 후 프로그램 종료
```
Press Enter(Return) to continue 예제
z'$f0w13>w7[gc0,1,e(b)]w88pzf0t
; 주소 0에 13(CR) 저장, 주소 1에 7을 저장(루프문 실행을 위함)
; 문자를 입력받고, 주소 0의 값과 같은 경우(CR, 엔터키) break
; 루프 탈출 후 88(X)를 주소 1에 저장하고 출력 후 메모리 제로필, 주소 0으로 초기화, 프로그램 종료


win32 인터프리터
---
win32 인터프리터는 C언어로 작성되었습니다. 버그가 있는 경우 알려주시기 바랍니다.

ABF와 프로그램을 제작한 사람은 단순히 취미활동으로서 프로그래밍을 하는 사람으로, 소스코드가 읽기 복잡하거나 버그가 있을 수 있습니다.

- 사양

가용 메모리: 4096 바이트

명령어 버퍼: 널 문자 포함 1024 바이트

int 자료형 크기: 4 바이트

double 자료형 크기: 8 바이트

지원 자료형: char, unsigned char, int, unsigned int, double

- 소개

win32 인터프리터는 x86 시스템에서 구동할 수 있도록 빌드되어 있습니다. x86, x86_64 환경이 아닌 경우 MSVC를 이용해 소스코드를 빌드해야 합니다.

win32 인터프리터를 시작하면 다음과 같은 화면이 출력됩니다.
```
Ascii BrainFuck Language Interpreter Prompt
int size: 4 Bytes, double size: 8 Bytes
int range: -2147483648 to 2147483647, double range: (+-) 2.22507385851e-308 to 1.79769313486e+308
Memory size: 4096 Bytes
Maximum command length per line: 1024 Bytes with NULL
***Type ? for commands***

READY(0,0000)>>
```
인터프리터 구동 시 위의 화면과 같이 int, double 자료형의 크기와 범위를 출력합니다. 메모리의 크기와 한 줄당 입력할 수 있는 명령어의 크기 또한 출력합니다.
인터프리터가 명령을 처리하고 다음 명령을 기다리는 경우 다음이 출력됩니다.
```
READY(x,yyyy)>>
```
x는 현재 포인터 모드를 나타냅니다. 포인터 모드는 0: char, 1: int, 2: double, 3: unsigned char, 4: unsigned int의 5가지입니다.
yyyy는 현재 메모리 주소를 10진수로 나타냅니다.

인터프리터에서 디버그 모드를 활성화한 경우 매 명령 수행 시마다 다음 정보를 출력합니다.
```
READY(0,0000)>> y
Debug mode on
File Loaded: FALSE, ptrMode: 0, ptr: 0, cmdPos: 0
Cond present: FALSE, x: 0, y: 0, g_cmdCond: , name buffer:
cmd: y, dx: 0, byte: 0
buf: 0x0000000000000000000000000000000000000000000000000000000000000000
READY(0,0000)>>
```
y 명령을 입력받으면 디버그 모드의 활성화/비활성화 상태를 표시합니다. 디버그 모드가 활성화된 경우 표시되는 정보는 아래와 같습니다.
```
File Loaded: 파일이 열려있는지의 여부
ptrMode: 포인터 모드
ptr: 현재 주소
cmdPos: 실행한/건너뛴 명령의 가로방향 위치(0부터 시작)
Cond present: 조건식 정의 여부
x, y: 조건식 또는 인수 X 및 Y
g_cmdCond: 조건식의 비교/논리연산자. 공란인 경우 정의되지 않음
name buffer: 네임 마커('`')로 둘러싸인 문자열의 내용
cmd: 실행한/건너뛴 명령어
dx: double 타입 사용 시 인수 X
byte: 현재 바이트 값(16진수)
buf: 버퍼 데이터 값(16진수)
```
디버그 모드에서 명령어를 실행한 경우 위와 같은 메시지를 무조건 출력하며, 건너뛴 경우라도 출력될 수 있습니다.
디버그 모드가 활성화된 경우 명령어는 1초에 한 개씩 실행됩니다. 또한, 디버그 모드가 활성화된 경우 'w'명령으로 입력한 값과 if문이 종료된 경우 if문 종료 안내 메시지를 출력합니다.

파일을 불러오려는 경우 반드시 경로를 지정해야만 합니다. 파일 불러오기와 실행을 한줄에 모두 처리하는 경우와 파일을 불러온 후 다음 줄에서 실행시키는 경우의 출력값은 다릅니다. 예시로 hw.abf 파일을 불러와 실행시키면 다음과 같이 출력합니다.
```
READY(0,0000)>> l`c:\abf\hw.abf`
File Loaded c:\abf\hw.abf
READY(0,0000)>> s
Hello, world

READY(0,0014)>> ku

READY(0,0000)>> l`c:\abf\hw.abf`s
File Loaded c:\abf\hw.abfHello, world
```
예시와 같이 한 줄에 불러오기와 실행을 모두 처리하는 경우 프로그램의 출력은 줄바꿈이 되지 않고 출력됩니다. 반면 불러오기 후 다음 줄에 파일을 실행시키는 경우 정상적으로 "Hello, world"가 출력되었음을 알 수 있습니다.

명령 실행 중 오류가 발생하면 오류메시지가 출력되고 명령 실행을 중단합니다. 파일을 실행 중인 경우 파일의 실행이 중단됩니다. 오류 메시지 예시는 다음과 같습니다.
```
READY(0,0000)>> `
?Error name marker without command. cmd at 0, line 0
Command execution halted(Syntax Error)
READY(0,0000)>>
```
오류 메시지는 "cmd at 0, line 0"와 같이 명령어의 어느 부분에서 오류가 발생했는지를 명시합니다. line이 0인 경우 파일을 불러오지 않고 인터프리터에 명령을 입력하여 실행하던 중 오류가 발생한 것입니다. 오류 메시지는 "?Error ..." 형식으로 출력됩니다. 이후 다음 줄에 "Command execution halted(...)"에서 ...는 어떠한 오류가 발생하였는지를 명시합니다. 예시의 경우 구문 오류입니다.

인터프리터는 명령을 실행하기 전 if문의 괄호 ()와 루프문의 대괄호 []의 여는 부분과 닫는 부분의 개수가 같은지를 검사합니다. 만약 여는 부분의 개수와 닫는 부분의 개수가 일치하지 않으면 다음과 같은 오류 메시지가 출력됩니다.
```
READY(0,0000)>> []]
?Error parentheses and/or brackets are not balanced
READY(0,0000)>>
```
인터프리터에서 'g' 명령은 MSVC의 비표준 함수인 _getch()와 같은 기능을 수행합니다. 즉, 키보드에서 한 글자가 입력된 즉시 그 값을 메모리에 저장합니다.

인터프리터는 int 또는 double 타입으로 메모리에 값을 기록하려는 경우 메모리 범위를 초과하지 않는지 검사합니다. 메모리 주소의 최대값은 4095이며, int의 경우 4092, double의 경우 4088을 초과하는 주소가 설정되어 있는 경우 메모리 주소의 최대값을 넘어가게 되어 오류 메시지가 출력됩니다.
```
READY(0,4092)>> #w5647

READY(1,4092)>> >w5647
?Error address out of range. cmd at 1
Command execution halted(Memory/Value Range Error)
READY(1,4093)>> @f4088w2.2

READY(2,4088)>> >w2.2
?Error address out of range. cmd at 1
Command execution halted(Memory/Value Range Error)
READY(2,4089)>>
```
포인터 모드가 double 인 경우 비트 연산자와 나머지 연산자를 사용하는 경우 오류 메시지가 출력됩니다.
```
READY(0,0000)>> @%1,10
?Error modulus operation in floating point format. cmd at 1, line 0
Command execution halted(Syntax Error)
READY(2,0000)>> @|1,10
?Error bitwise operation in floating point format(|). cmd at 1, line 0
Command execution halted(Syntax Error)
READY(2,0000)>>
```
int 또는 double 포인터를 사용 중 w명령을 사용하는 경우 인터프리터는 입력된 값의 크기를 검사하지 않습니다.

인터프리터가 프로그램에서 오류를 핸들받은 경우 오류코드를 표시한 후 프로그램 실행을 중단합니다.
```
READY(0,0000)>> s

Program handled error(-1)
READY(0,0000)>>
```
버퍼에 저장된 연산결과는 버퍼의 값을 변경하는 명령어를 수행하지 않는 이상 그대로 유지됩니다. 이를 이용해 연산을 한 후 메모리의 여러 주소에 연산 결과를 저장할 수 있습니다.
```
READY(0,0000)>> w1>w2:0,1>=\>=\
-1-1
READY(0,0003)>>
```
