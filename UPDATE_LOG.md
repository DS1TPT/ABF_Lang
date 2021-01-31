- v1.12
  - Added feature: Interpreter can run a file directly from command line, without using interpreter prompt
  - Fixed: Interpreter reading subroutines inside of subroutine list
- v1.11
  - Added BF to ABF compiler(single line, command line input only)
  - Errors will be recorded to record file
  - Fixed recording file with incorrect newline character(s). On UNIX-based systems the interpreter will use '\n' for newline, and '\r\n' for Windows
  - Fixed exceptions when using some interpreter specific commands without argument
  - Fixed printing parentheses/brackets balance error when interpreter specific command is input
  - Fixed incorrect error messages
  - Fixed typos
  - Shortened unnecessarily long codes
  - Added license notice in prompt
  - Miscellaneous fixes
- v1.10
  - Some commands and syntax have been changed due to unnecessary command assignments. No more commands/syntax change will be done, so this is the first and last update of command and syntax. Only missing syntax description(s) will be updated. Here's the list of commands that changed, with previous functions and current functions.
  ```
  k: initialize interpreter -> get keyboard input without echo
  l: load file -> increase/decrease value by X
  q: print source code -> increase/decrease pointer by X
  s: start file execution -> write string
  u: unload(close) file -> suspend for X second(s)
  x: close interpreter -> exchange value at X with value at Y
  ?: print help -> write random number
  `: name marker -> string start/end marker
  ```
  - Added command line input recording feature
  - Added subroutine library feature
  - Added clear screen feature
  - Added internal variables display feature
  - "--help" command prints help message('?' previously)
  - Fixed some codes that can potentially cause problems
  - Removed unnecessary variables
- v1.01
  - Removed unused variable(s)
  - Fixed signed char related problems on ARM platforms
  - Some other Minor changes
- v1.00: First interpreter version
