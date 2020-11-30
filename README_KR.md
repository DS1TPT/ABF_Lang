ABF, 알파벳 브레인퍽
===
ABF는 브레인퍽에 기반해 제작된 난해한 프로그래밍 언어입니다.

명령어 목록
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
인수 전달
- 모든 연산은 주소 X에 있는 값과 주소 Y에 있는 값을 피연산자로 합니다.
- 'w'명령어를 제외한 명령어의 인수는 모두 unsinged int 타입입니다.
- 'w'명령어는 char, int 타입일 때 unsigned int를, double 타입일 때 unsigned int와 부동소수점 타입으로 인수를 전달해야 합니다.
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
예시: #f5w127 ;주소 5부터 8까지 데이터를 기록
```
- 메모리에 저장된 값의 타입 검사를 하지 않습니다.
```
예시: '#f0w2147483647$f2w0 ;메모리 0부터 3까지의 값: 0x7fff00ff
```
- '\'명령은 포인터 타입에 해당하는 길이만큼 값을 읽어 출력합니다. double 포인터의 경우 정수로 변환하여 출력합니다.
- '_'명령은 무조건 double 타입에 해당하는 길이만큼 값을 읽어 부동소수점으로 출력합니다
- '/'명령은 double 포인터가 아닌 경우 몫을 계산합니다.
- ':'명령은 signed char 타입으로 값을 버퍼에 저장합니다.
명령어 문법과 사용
- if문 이전에 반드시 'c'명령으로 조건문을 정의해야 합니다.
- 'c'명령은 3개의 인수가 필요합니다. X, Y, 비교연산자 순서로 인수를 적어야 합니다.
```
예시: c0,2,!(...) ;if (*mem != *(mem + 2)) {...}
```
- 'b'명령은 루프문 실행을 멈추고 바로 다음 ']'부터 명령을 실행하도록 합니다.
- 바이트, 포인터 증감 연산은 포인터 타입에 영향을 받지 않습니다.
- 'e'와 '!'는 비교연산자로, 조건이 참일 때 1을, 거짓일 때 0을 반환합니다.
- 'g'명령은 stdin을 이용합니다.
- 'h'명령은 인터프리터에 오류코드(인수)를 반환하고 프로그램 실행을 정지합니다.
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
zf0w72>w101>w108m3,2f4w111>w44>w32>w119m8,4f9w114m10,3f11w100>w10>w13f0
;"Hello, world"를 메모리 주소 0부터 기록
[p>]zt ;메모리를 0으로 초기화 후 프로그램 종료
```
