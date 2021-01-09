/*
* Copyright 2020-2021. Lee Geon-goo
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/* version 1.10 (03-JAN-2021) */
/* DEV Environment: MSVC 2019, x86 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <time.h> /* for random number */

#define FALSE 0
#define TRUE 1
#define RUN_FILE 100
#define null 0 /* int null */

/* SIZE AND VALUE LIMIT */
#define INT_SIZE sizeof(int)
#define DBL_SIZE sizeof(double)
#define MEM_SIZE 65535
#define BUF_SIZE 4096
#define NAME_SIZE 1024

typedef unsigned char byte;

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#if defined _MSC_VER && ( defined _WIN32 || defined _WIN64 )
#pragma message ("Windows system, Using _getche() for 'g'.")
#include <conio.h> /* for _getch() and _getche() */
#include <Windows.h>
#define suspend(x) Sleep((x) * 1000)
#elif defined __linux__ || defined unix || defined __unix || defined __unix__ || TARGET_OS_OSX
#pragma message ("Unix or Linux system, using termios to implement _getch() and _getche().")
#include <termios.h>
#include <unistd.h>
int niGetch(_Bool echo) {
    int ch;
    struct termios old;
    struct termios current;

    tcgetattr(0, &old);
    current = old;
    current.c_lflag &= ~ICANON;
    if (echo) current.c_lflag |= ECHO;
    else current.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &current);
    ch = getchar();
    tcsetattr(0, TCSANOW, &old);

    return ch;
}
#define _getche() niGetch(TRUE)
#define _getch() niGetch(FALSE)
#define suspend(x) sleep(x)
#else
#error ?Unknown platform. 'g' and 'k' are not implemented for your OS. Compilation terminated.
#endif

/* BUFFERS */
char cmdBuf[BUF_SIZE], fBuf[BUF_SIZE], nameBuf[NAME_SIZE];
char g_cmdCond = 0;
byte g_buf[32]; /* reduced size */
int g_iBuf = 0;
double g_dBuf = 0.0;

/* POINTERS & ARRAYS */
byte mem[MEM_SIZE];
signed char* pc = (signed char*)mem;
double* pd = (double*)mem;
int* pi = (int*)mem;
unsigned* pu = (unsigned*)mem;
FILE* pFile = NULL;
FILE* pRecordFile = NULL;
FILE* pLib = NULL;
FILE* pRec = NULL;

/* STATUS MARKERS */
unsigned ptrMode = 0; /* ptrMode 0: char, 1: int, 2: double, 3: uchar(byte), 4: uint */
_Bool isUnsigned = FALSE, termination = FALSE, isExec = FALSE, isRecording = FALSE;
unsigned uLineCnt = 0, basicPos = 0, basicPosBuf = 0;

/* MISCELLANEOUS */
int x = 0, y = 0;
double dx = 0.0;
unsigned addr = 0;

void help(void); /* print help message(cmd list) */
void memFill(char* dst, char dta, unsigned size);  /* fill mem with dta */
_Bool searchCh(char* sz, char* cp); /* search if there's *cp in sz */
unsigned szLen(char* sz); /* calculate the length of string */
void szCpy(char* dst, unsigned size, char* src); /* copy src's dta to dst, writes null */
char* szParse(char* sz, const char* delim); /* Parse string */
int szCmp(char* sz1, char* sz2); /* compare strings */
void memCpy(char* dst, unsigned size, char* src); /* copy src's dta to dst, does not write null */
unsigned atou(char* sz); /* ASCII string to unsinged */
long long roundftoll(double* pDb); /* round double and cast it to ll, then return ll value */
void clrscr(void); /* clears screen(console) */
_Bool chkEndian(void); /* check endianness, returns TRUE if system is little endian */

void initGlobalVar(void) {
    if (!isExec) {
        memFill(cmdBuf, 0, BUF_SIZE);
        basicPos = 0, basicPosBuf = 0;
    }
    memFill(mem, 0, MEM_SIZE);
    memFill(fBuf, 0, BUF_SIZE);
    memFill(nameBuf, 0, NAME_SIZE);
    memFill(g_buf, 0, sizeof(g_buf));
    g_cmdCond = 0;
    g_iBuf = 0;
    g_dBuf = 0.0;
    x = 0, y = 0;
    dx = 0.0;
    addr = 0;
    ptrMode = 0;
    isUnsigned = FALSE, termination = FALSE;
    pc = (signed char*)mem;
    pi = (int*)mem;
    pd = (double*)mem;
    pu = (unsigned*)mem;
}

_Bool chkAddrRange(int cmdPos, _Bool isXY) {
    if (isXY) {
        if (ptrMode == 0 || ptrMode == 3) {
            if (x > MEM_SIZE - 1 || x < 0 || y > MEM_SIZE - 1 || y < 0) {
                printf("?Error address out of range. cmd at %d, line %u", cmdPos, uLineCnt);
                return TRUE;
            }
        }
        else if (ptrMode == 1 || ptrMode == 4) {
            if (x > MEM_SIZE - INT_SIZE || x < 0 || y > MEM_SIZE - INT_SIZE || y < 0) {
                printf("?Error address out of range. cmd at %d, line %u", cmdPos, uLineCnt);
                return TRUE;
            }
        }
        else if (ptrMode == 2) {
            if (x > MEM_SIZE - DBL_SIZE || x < 0 || y > MEM_SIZE - DBL_SIZE || y < 0) {
                printf("?Error address out of range. cmd at %d, line %u", cmdPos, uLineCnt);
                return TRUE;
            }
        }
    }
    else {
        if (ptrMode == 1 || ptrMode == 4) {
            if (addr > (MEM_SIZE - INT_SIZE)) {
                printf("?Error address out of range. cmd at %d, line %u", cmdPos, uLineCnt);
                return TRUE;
            }
        }
        else if (ptrMode == 2) {
            if (addr > (MEM_SIZE - DBL_SIZE)) {
                printf("?Error address out of range. cmd at %d, line %u", cmdPos, uLineCnt);
                return TRUE;
            }
        }
    }
    return FALSE;
}

int strProc(int cmdPos) { /* get string and store it to memory */
    char* cp = cmdBuf + cmdPos;
    char strBuf[BUF_SIZE];
    char* tmp = strBuf;
    memFill(strBuf, 0, BUF_SIZE);
    _Bool firstMarker = FALSE;
    int pos = 0;

    while (*cp) {
        if (*cp == '`' && firstMarker == TRUE) {
            firstMarker = FALSE;
            *tmp = null;
            tmp = strBuf;
            while (addr < MEM_SIZE - 1 && *tmp) {
                *(mem + addr) = (byte)(*tmp++);
                addr++;
            }
            *(mem + addr) = null;
            if (*tmp) {
                printf("?Exceeded max memory address. String did not stored completely");
                addr = MEM_SIZE - 1;
            }
            return pos;
        }
        else if (*cp != '`' && firstMarker == TRUE) *tmp++ = *cp;
        else if (*cp == '`' && firstMarker == FALSE) firstMarker = TRUE;
        cp++;
        pos++;
    }
    return 0;
}

char* findNextCond(char* pSz) { /* find next condition using ','. returns pos of ',' if it's present */
    while (*pSz) {
        if (*pSz >= '0' && *pSz <= '9') pSz++;
        else if (*pSz == ',') return ++pSz; /* found separator */
        else {
            if (*pSz == '.' && ptrMode != 2) {
                printf("?Detected Decimal('.') in integer conditions.");
                return NULL;
            }
            return pSz; /* returns pSz if *pSz is not between '0' to '9' and neq to ',' */
        }
    }
    return pSz;
}

void getDblCond(int cmdPos) { /* set double type condition, returns FALSE(err) or TRUE(norm)*/
    char* cp = cmdBuf + cmdPos + 1;
    char strBuf[64];
    memFill(strBuf, 0, 64);

    szCpy(strBuf, 64, cp);
    dx = atof(strBuf);
    memCpy(g_buf, DBL_SIZE, (char*)&dx);
}

int condProc(int cmdPos, int condNum, _Bool forceSigned) { /* set int type conditions, returns condition count */
    char* cp = cmdBuf + cmdPos + 1;
    int condCnt = 0, i = 0;
    _Bool isCmpCmd = FALSE;

    x = 0;
    y = 0;

    for (i = 0; i < condNum; i++) {
        if (!(*cp)) return i;
        else if (i == 0) {
            if (*cp == ',') x = addr;
            else if ((forceSigned || !isUnsigned) && *cp != ',') x = atoi(cp);
            else if (isUnsigned && *cp != ',' && !forceSigned) x = atou(cp);
            condCnt++;
        }
        else if (i == 1) {
            if (*cp == ',') y = addr;
            else if ((forceSigned || !isUnsigned) && *cp != ',') y = atoi(cp);
            else if (isUnsigned && *cp != ',' && !forceSigned) y = atou(cp);
            condCnt++;
        }
        else if (i == 2) {
            isCmpCmd = searchCh("aeno|~!^&", cp);
            if (isCmpCmd == FALSE)
                return condCnt;
            else {
                g_cmdCond = *cp;
                return ++condCnt;
            }
        }
        if (i == 0 || i == 1) {
            cp = findNextCond(cp);
            if (cp == NULL) return -1;
        }
    }
    return condCnt;
}

void prepIntCalc(_Bool isY) { /* store int value to g_buf for calculation */
    unsigned u = 0;
    if (isY) {
        for (u = 0; u < INT_SIZE; u++) {
            g_buf[INT_SIZE + u] = mem[x + u];
            g_buf[INT_SIZE * 2 + u] = mem[y + u];
        }
    }
    else {
        for (u = 0; u < INT_SIZE; u++)
            g_buf[INT_SIZE + u] = mem[x + u];
    }
    if (ptrMode == 1) pi = (int*)g_buf;
    else if (ptrMode == 4) pu = (unsigned*)g_buf;
}

void prepDblCalc(_Bool isY) { /* store double value to g_buf for calculation */
    unsigned u = 0;
    if (isY) {
        for (u = 0; u < DBL_SIZE; u++) {
            g_buf[DBL_SIZE + u] = mem[x + u];
            g_buf[DBL_SIZE * 2 + u] = mem[y + u];
        }
    }
    else {
        for (u = 0; u < DBL_SIZE; u++)
            g_buf[DBL_SIZE + u] = mem[x + u];
    }
    pd = (double*)g_buf;
}

void pAddrSet(void) { /* set pointers except for char types to mem + addr */
    if (ptrMode == 0) pc = (signed char*)(mem + addr);
    else if (ptrMode == 1) pi = (int*)(mem + addr);
    else if (ptrMode == 2) pd = (double*)(mem + addr);
    else if (ptrMode == 4) pu = (unsigned*)(mem + addr);
}

void pAddrInit(void) { /* reset pointers except for char types to mem */
    if (ptrMode == 0) pc = (signed char*)mem;
    else if (ptrMode == 1) pi = (int*)mem;
    else if (ptrMode == 2) pd = (double*)mem;
    else if (ptrMode == 4) pu = (unsigned*)mem;
}

void writeDta(void) { /* write data to mem */
    if (ptrMode == 0) {
        pc = (signed char*)mem;
        *(pc + addr) = (signed char)x;
        return;
    }
    else if (ptrMode == 1) {
        pi = (int*)g_buf;
        *pi = x;
        memCpy((mem + addr), INT_SIZE, g_buf);
    }
    else if (ptrMode == 2) {
        memCpy((mem + addr), DBL_SIZE, g_buf);
    }
    else if (ptrMode == 3) {
        mem[addr] = (byte)x;
        return;
    }
    else if (ptrMode == 4) {
        pu = (unsigned*)g_buf;
        *pu = x;
        memCpy((mem + addr), INT_SIZE, g_buf);
    }
    pAddrInit();
}

int cmpProc(char cmd) { /* process compare commands except for ':' */
    if (ptrMode == 0 || ptrMode == 3) {
        switch (cmd) {
        case 'a':
            g_buf[0] = mem[x] && mem[y];
            break;
        case 'e':
            g_buf[0] = mem[x] == mem[y];
            break;
        case 'n':
            g_buf[0] = !mem[x];
            break;
        case 'o':
            g_buf[0] = mem[x] || mem[y];
            break;
        case '|':
            g_buf[0] = mem[x] | mem[y];
            break;
        case '~':
            g_buf[0] = ~mem[x];
            break;
        case '!':
            g_buf[0] = mem[x] != mem[y];
            break;
        case '^':
            g_buf[0] = mem[x] ^ mem[y];
            break;
        case '&':
            g_buf[0] = mem[x] & mem[y];
            break;
        }
    }
    else if (ptrMode == 1 || ptrMode == 4) {
        if (searchCh("n~", &cmd)) prepIntCalc(FALSE);
        else prepIntCalc(TRUE);

        switch (cmd) {
        case 'a':
            g_iBuf = *(pi + 1) && *(pi + 2);
            break;
        case 'e':
            g_iBuf = *(pi + 1) == *(pi + 2);
            break;
        case 'n':
            g_iBuf = !*(pi + 1);
            break;
        case 'o':
            g_iBuf = *(pi + 1) || *(pi + 2);
            break;
        case '|':
            g_iBuf = *(pi + 1) | *(pi + 2);
            break;
        case '~':
            g_iBuf = ~*(pi + 1);
            break;
        case '!':
            g_iBuf = *(pi + 1) != *(pi + 2);
            break;
        case '^':
            g_iBuf = *(pi + 1) ^ *(pi + 2);
            break;
        case '&':
            g_iBuf = *(pi + 1) & *(pi + 2);
            break;
        }
        *pi = g_iBuf;
        pAddrInit();
    }
    else if (ptrMode == 2) {
        if (searchCh("|~^&", &cmd)) {
            printf("?Error bitwise operation in floating point format(%c).", cmd);
            return TRUE;
        }
        if (cmd == 'n') prepDblCalc(FALSE);
        else prepDblCalc(TRUE);

        switch (cmd) {
        case 'a':
            g_dBuf = *(pd + 1) && *(pd + 2);
            break;
        case 'e':
            g_dBuf = *(pd + 1) == *(pd + 2);
            break;
        case 'n':
            g_dBuf = !*(pd + 1);
            break;
        case 'o':
            g_dBuf = *(pd + 1) || *(pd + 2);
            break;
        case '!':
            g_dBuf = *(pd + 1) != *(pd + 2);
            break;
        }
        *pd = g_dBuf;
        pAddrInit();
    }
    return FALSE;
}

int exeCond(void) {
    if (cmpProc(g_cmdCond)) return -1;
    if (ptrMode == 0 || ptrMode == 3) {
        if (!!g_buf[0]) return 1;
        else return 0;
    }
    else if (ptrMode == 1 || ptrMode == 4) {
        if (!!g_iBuf) return 1;
        else return 0;
    }
    else if (ptrMode == 2) {
        if (g_dBuf != 0.0) return 1;
        else return 0;
    }
    return 0;
}

int cmdProc(void) { /* Interpret commands */
    int bracketCnt = 0, parenthesesCnt = 0, iRet = 0, i = 0, j = 0;
    char currCmd;

    /*
    return value 1 is syntax error, 2 is memory/value error, 3 is Internal/External error,
    4 is Miscellaneous error, -1 is Exit interpreter, -2 is terminate execution(with error).
    */

    for (i = 0; cmdBuf[i]; i++) {
        currCmd = cmdBuf[i];
        if (cmdBuf[i] == '-' && cmdBuf[i + 1] == '-') break;
        else if (cmdBuf[i] == '+' && cmdBuf[i + 1] == '+') break;
        switch (currCmd) {
        case 'a':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else cmpProc(currCmd);
            break;
        case 'b':
            while (cmdBuf[i] != ']') i++;
            break;
        case 'c':
            iRet = condProc(i, 3, FALSE);
            if (iRet != 3) {
                g_cmdCond = null;
                x = 0;
                y = 0;
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            else while (cmdBuf[i] != g_cmdCond) i++;
            break;
        case 'd':
            if (ptrMode == 0 || ptrMode == 3) mem[addr]--;
            else if (ptrMode == 1) {
                pAddrSet();
                (*pi)--;
                pAddrInit();
            }
            else if (ptrMode == 2) {
                pAddrSet();
                *pd -= 1.0;
                pAddrInit();
            }
            else if (ptrMode == 4) {
                pAddrSet();
                (*pu)--;
                pAddrInit();
            }
            break;
        case 'e':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else cmpProc(currCmd);
            break;
        case 'f':
            iRet = condProc(i, 1, FALSE);
            if (iRet != 1) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            else {
                if (x > MEM_SIZE - 1 || x < 0) {
                    printf("?Error address out of range. cmd at %d, line %u", i, uLineCnt);
                    return 2;
                }
                addr = x;
                x = 0;
            }
            break;
        case 'g':
            mem[addr] = (byte)_getche();
            break;
        case 'h':
            if (!isExec) {
                printf("?This command can be used ONLY in file(s). cmd at %d", i);
                return 1;
            }
            else {
                iRet = condProc(i, 1, TRUE);
                if (iRet != 1) {
                    printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                    return 1;
                }
                else return -2;
            }
            break;
        case 'i':
            if (ptrMode == 0 || ptrMode == 3) mem[addr]++;
            else if (ptrMode == 1) {
                pAddrSet();
                (*pi)++;
                pAddrInit();
            }
            else if (ptrMode == 2) {
                pAddrSet();
                *pd += 1.0;
                pAddrInit();
            }
            else if (ptrMode == 4) {
                pAddrSet();
                (*pu)++;
                pAddrInit();
            }
            break;
        case 'j':
            if (!isExec) {
                printf("?This command can be used ONLY in file(s). cmd at %d", i);
                return 1;
            }
            else {
                rewind(pFile);
                uLineCnt = 1;
                if (!isUnsigned) dx = 1.0;
                else dx = 0.0;
                isUnsigned = TRUE;
                iRet = condProc(i, 1, FALSE);
                if (iRet != 1) {
                    printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                    return 1;
                }
                if (dx == 1.0) {
                    isUnsigned = FALSE;
                    dx = 0.0;
                }
                j = 1;
                while (!feof(pFile)) {
                    fgets(cmdBuf, BUF_SIZE, pFile);
                    if (!feof(pFile) && cmdBuf[szLen(cmdBuf)]) cmdBuf[szLen(cmdBuf) - 1] = null;
                    uLineCnt++;
                    iRet = atou(cmdBuf);
                    if (iRet == x) {
                        j = 0;
                        basicPos = iRet;
                        i = 0;
                        break;
                    }
                }
                if (j) {
                    printf("?Error label %d not found, cmd at %d, line %u", x, i, uLineCnt);
                    return 1;
                }
            }
            break;
        case 'k':
            mem[addr] = (byte)_getch();
            break;
        case 'l':
            if (ptrMode != 2) {
                iRet = condProc(i, 1, TRUE);
                if (iRet != 1) {
                    printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                    return 1;
                }
            }
            if (chkAddrRange(i, FALSE)) return 2;
            else getDblCond(i);
            if (ptrMode == 0) {
                pAddrSet();
                *pc += (signed char)x;
                pAddrInit();
            }
            else if (ptrMode == 1) {
                pAddrSet();
                *pi += x;
                pAddrInit();
            }
            else if (ptrMode == 2) {
                pAddrSet();
                *pd += dx;
                pAddrInit();
            }
            else if (ptrMode == 3) mem[addr] += (byte)x;
            else if (ptrMode == 4) {
                pAddrSet();
                *pu += (unsigned)x;
                pAddrInit();
            }
            i++;
            if (cmdBuf[i] == '-') {
                i++;
                while ((cmdBuf[i] >= '0' && cmdBuf[i] <= '9') || cmdBuf[i] == 'e' || cmdBuf[i] == '.') {
                    if (cmdBuf[i] == '-' && j) break;
                    else if (cmdBuf[i] == '-' && !j) j = 1;
                    i++;
                }
                i--;
            }
            break;
        case 'm':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            else {
                if (chkAddrRange(i, TRUE)) return 2;
                if (ptrMode == 0 || ptrMode == 3) mem[y] = mem[x];
                else if (ptrMode == 1 || ptrMode == 4) {
                    pi = (int*)(mem + x);
                    g_iBuf = *pi;
                    pi = (int*)(mem + y);
                    *pi = g_iBuf;
                    pAddrInit();
                }
                else if (ptrMode == 2) {
                    pd = (double*)(mem + x);
                    g_dBuf = *pd;
                    pd = (double*)(mem + y);
                    *pd = g_dBuf;
                    pAddrInit();
                }
            }
            break;
        case 'n':
            iRet = condProc(i, 1, FALSE);
            if (iRet != 1) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else cmpProc(currCmd);
            break;
        case 'o':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else cmpProc(currCmd);
            break;
        case 'p':
            printf("%c", mem[addr]);
            break;
        case 'q':
            iRet = condProc(i, 1, TRUE);
            if (iRet != 1) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (MEM_SIZE - 1 < addr + x || 0 > addr + x) {
                printf("?Error address out of range(q). cmd at %d, line %u", i, uLineCnt);
                return 2;
            }
            addr += x;
            i++;
            while (cmdBuf[i] == '-') i++;
            break;
        case 'r':
            if (!isExec) {
                printf("?This command can be used ONLY in file(s). cmd at %d", i);
                return 1;
            }
            rewind(pFile);
            uLineCnt = 1;
            j = 1;
            while (!feof(pFile)) {
                fgets(cmdBuf, BUF_SIZE, pFile);
                if (!feof(pFile) && cmdBuf[szLen(cmdBuf)]) cmdBuf[szLen(cmdBuf) - 1] = null;
                uLineCnt++;
                iRet = atou(cmdBuf);
                if ((unsigned)iRet == basicPosBuf) {
                    basicPos = iRet;
                    j = 0;
                    return 0;
                }
            }
            if (j) {
                printf("?Error label %u not found, cmd at %d, line %u", basicPosBuf, i, uLineCnt);
                return 1;
            }
            break;
        case 's':
            iRet = strProc(i);
            if (!iRet) {
                printf("?String marker error, cmd at %d, line %u", i, uLineCnt);
                return 4;
            }
            i += iRet;
            break;
        case 't':
            if (isExec) termination = TRUE;
            return 0;
            break;
        case 'u':
            iRet = condProc(i, 1, FALSE);
            if (iRet != 1) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            suspend(x);
            break;
        case 'v': return 0;
        case 'w':
            if (ptrMode != 2) {
                iRet = condProc(i, 1, FALSE);
                if (iRet != 1) {
                    printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                    return 1;
                }
                if ((x > SCHAR_MAX || x < SCHAR_MIN) && ptrMode == 0) {
                    printf("?Error value X is out of range. cmd at %d, line %u", i, uLineCnt);
                    return 2;
                }
                else if (x > UCHAR_MAX && ptrMode == 3) {
                    printf("?Error value X is out of range. cmd at %d, line %u", i, uLineCnt);
                    return 2;
                }
            }
            else getDblCond(i);
            /* Does not check value range for int, uint, and double */
            if (chkAddrRange(i, FALSE)) return 2;
            writeDta();
            i++;
            while ((cmdBuf[i] >= '0' && cmdBuf[i] <= '9') || cmdBuf[i] == '-' || cmdBuf[i] == 'e' || cmdBuf[i] == '.') i++;
            i--;
            break;
        case 'x':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            if (ptrMode == 0 || ptrMode == 3) {
                g_buf[0] = mem[x];
                mem[x] = mem[y];
                mem[y] = g_buf[0];
            }
            else if (ptrMode == 1 || ptrMode == 4) {
                int tmp;
                pi = (int*)(mem + x);
                g_iBuf = *pi;
                pi = (int*)(mem + y);
                tmp = *pi;
                *pi = g_iBuf;
                pi = (int*)(mem + x);
                *pi = tmp;
                pi = (int*)mem;
            }
            else if (ptrMode == 2) {
                double tmp;
                pd = (double*)(mem + x);
                g_dBuf = *pd;
                pd = (double*)(mem + y);
                tmp = *pd;
                *pd = g_dBuf;
                pd = (double*)(mem + x);
                *pd = tmp;
                pd = (double*)mem;
            }
            break;
        case 'y':
            if (!pFile || !isExec) {
                printf("?Error this command can be used ONLY in file(s). cmd at %d", i);
                return 1;
            }
            else basicPosBuf = basicPos;
            break;
        case 'z':
            memFill(mem, 0, MEM_SIZE);
            break;
        case '!':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else cmpProc(currCmd);
            break;
        case '@':
            ptrMode = 2;
            break;
        case '#':
            if (isUnsigned) ptrMode = 4;
            else ptrMode = 1;
            break;
        case '$':
            if (isUnsigned) ptrMode = 3;
            else ptrMode = 0;
            break;
        case '%':
            if (ptrMode == 2) {
                printf("?Error modulus operation in floating point format. cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else {
                if (ptrMode == 0) {
                    pc = g_buf;
                    *pc = (signed char)mem[x] % (signed char)mem[y];
                }
                else if (ptrMode == 1) {
                    prepIntCalc(TRUE);
                    pi = (int*)(g_buf + INT_SIZE);
                    g_iBuf = *pi % *(pi + 1);
                    *(pi - 1) = g_iBuf;
                    pAddrInit();
                }
                else if (ptrMode == 3) g_buf[0] = mem[x] % mem[y];
                else if (ptrMode == 4) {
                    prepIntCalc(TRUE);
                    pu = (unsigned*)(g_buf + INT_SIZE);
                    g_iBuf = *pu % *(pu + 1);
                    *(pu - 1) = g_iBuf;
                    pAddrInit();
                }
            }
            break;
        case '^':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            else {
                if (chkAddrRange(i, TRUE)) return 2;
                if (cmpProc(currCmd)) {
                    printf(" cmd at %d, line %u", i, uLineCnt);
                    return 1;
                }
            }
            break;
        case '&':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            else {
                if (chkAddrRange(i, TRUE)) return 2;
                if (cmpProc(currCmd)) {
                    printf(" cmd at %d, line %u", i, uLineCnt);
                    return 1;
                }
            }
            break;
        case '*':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else {
                if (ptrMode == 0) {
                    pc = g_buf;
                    *pc = (signed char)mem[x] * (signed char)mem[y];
                    pAddrInit();
                }
                else if (ptrMode == 1) {
                    prepIntCalc(TRUE);
                    pi = (int*)(g_buf + INT_SIZE);
                    g_iBuf = (*pi) * (*(pi + 1));
                    *(pi - 1) = g_iBuf;
                    pAddrInit();
                }
                else if (ptrMode == 2) {
                    prepDblCalc(TRUE);
                    pd = (double*)(g_buf + DBL_SIZE);
                    g_dBuf = (*pd) * (*(pd + 1));
                    *(pd - 1) = g_dBuf;
                    pAddrInit();
                }
                else if (ptrMode == 3) g_buf[0] = mem[x] * mem[y];
                else if (ptrMode == 4) {
                    prepIntCalc(TRUE);
                    pu = (unsigned*)(g_buf + INT_SIZE);
                    g_iBuf = (*pu) * (*(pu + 1));
                    *(pu - 1) = g_iBuf;
                    pAddrInit();
                }
            }
            break;
        case '-':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else {
                if (ptrMode == 0) {
                    pc = (signed char*)g_buf;
                    *pc = (signed char)mem[x] - (signed char)mem[y];
                    pAddrInit();
                }
                else if (ptrMode == 1) {
                    prepIntCalc(TRUE);
                    pi = (int*)(g_buf + INT_SIZE);
                    g_iBuf = *pi - *(pi + 1);
                    *(pi - 1) = g_iBuf;
                    pAddrInit();
                }
                else if (ptrMode == 2) {
                    prepDblCalc(TRUE);
                    pd = (double*)(g_buf + DBL_SIZE);
                    g_dBuf = *pd - *(pd + 1);
                    *(pd - 1) = g_dBuf;
                    pAddrInit();
                }
                else if (ptrMode == 3) g_buf[0] = mem[x] - mem[y];
                else if (ptrMode == 4) {
                    prepIntCalc(TRUE);
                    pu = (unsigned*)(g_buf + INT_SIZE);
                    g_iBuf = *pu - *(pu + 1);
                    *(pu - 1) = g_iBuf;
                    pAddrInit();
                }
            }
            break;
        case '_':
            if (chkAddrRange(i, FALSE)) return 2;
            pd = (double*)(mem + addr);
            printf("%.12g", *pd);
            pAddrInit();
            break;
        case '+':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else {
                if (ptrMode == 0) {
                    pc = g_buf;
                    *pc = (signed char)mem[x] + (signed char)mem[y];
                    pAddrInit();
                }
                else if (ptrMode == 1) {
                    prepIntCalc(TRUE);
                    pi = (int*)(g_buf + INT_SIZE);
                    g_iBuf = *pi + *(pi + 1);
                    *(pi - 1) = g_iBuf;
                    pAddrInit();
                }
                else if (ptrMode == 2) {
                    prepDblCalc(TRUE);
                    pd = (double*)(g_buf + DBL_SIZE);
                    g_dBuf = *pd + *(pd + 1);
                    *(pd - 1) = g_dBuf;
                    pAddrInit();
                }
                else if (ptrMode == 3) g_buf[0] = mem[x] + mem[y];
                else if (ptrMode == 4) {
                    prepIntCalc(TRUE);
                    pu = (unsigned*)(g_buf + INT_SIZE);
                    g_iBuf = *pu + *(pu + 1);
                    *(pu - 1) = g_iBuf;
                    pAddrInit();
                }
            }
            break;
        case '=':
            if (chkAddrRange(i, FALSE)) return 2;
            if (ptrMode == 0) {
                pc = mem;
                *(pc + addr) = g_buf[0];
            }
            else if (ptrMode == 1 || ptrMode == 4) memCpy((mem + addr), INT_SIZE, g_buf);
            else if (ptrMode == 3) mem[addr] = g_buf[0];
            else if (ptrMode == 2) memCpy((mem + addr), DBL_SIZE, g_buf);
            break;
        case '\\':
            if (chkAddrRange(i, FALSE)) return 2;
            if (ptrMode == 3) printf("%u", mem[addr]);
            else {
                pAddrSet();
                if (ptrMode == 0) printf("%d", *pc);
                else if (ptrMode == 1) printf("%d", *pi);
                else if (ptrMode == 2) printf("%lld", roundftoll(pd));
                else if (ptrMode == 4) printf("%u", *pu);
                pAddrInit();
            }
            break;
        case '|':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else {
                if (cmpProc(currCmd)) {
                    printf(" cmd at %d, line %u", i, uLineCnt);
                    return 1;
                }
            }
            break;
        case ';': return 0;
        case ':':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else {
                if (ptrMode == 0) {
                    pAddrInit();
                    if (*(pc + x) > *(pc + y)) g_buf[0] = 1;
                    else if (*(pc + x) == *(pc + y)) g_buf[0] = 0;
                    else if (*(pc + x) < *(pc + y)) g_buf[0] = (byte)-1;
                }
                else if (ptrMode == 3) {
                    if (mem[x] > mem[y]) g_buf[0] = 1;
                    else if (mem[x] == mem[y]) g_buf[0] = 0;
                    else if (mem[x] < mem[y]) g_buf[0] = (byte)-1;
                }
                else if (ptrMode == 1) {
                    prepIntCalc(TRUE);
                    pi = (int*)g_buf;
                    if (*(pi + 1) > *(pi + 2)) g_buf[0] = 1;
                    else if (*(pi + 1) == *(pi + 2)) g_buf[0] = 0;
                    else if (*(pi + 1) < *(pi + 2)) g_buf[0] = (byte)-1;
                    pAddrInit();
                }
                else if (ptrMode == 4) {
                    prepIntCalc(TRUE);
                    pu = (unsigned*)g_buf;
                    if (*(pu + 1) > *(pu + 2)) g_buf[0] = 1;
                    else if (*(pu + 1) == *(pu + 2)) g_buf[0] = 0;
                    else if (*(pu + 1) < *(pu + 2)) g_buf[0] = (byte)-1;
                    pAddrInit();
                }
                else if (ptrMode == 2) {
                    prepDblCalc(TRUE);
                    pd = (double*)g_buf;
                    if (*(pd + 1) > *(pd + 2)) g_buf[0] = 1;
                    else if (*(pd + 1) == *(pd + 2)) g_buf[0] = 0;
                    else if (*(pd + 1) < *(pd + 2)) g_buf[0] = (byte)-1;
                    pAddrInit();
                }
            }
            break;
        case '\'':
            isUnsigned = FALSE;
            if (ptrMode == 3) ptrMode = 0;
            else if (ptrMode == 4) ptrMode = 1;
            break;
        case '\"':
            isUnsigned = TRUE;
            if (ptrMode == 0) ptrMode = 3;
            else if (ptrMode == 1) ptrMode = 4;
            break;
        case '<':
            if (addr > 0) --addr;
            else {
                printf("?Error address lower limit. cmd at %d, line %u", i, uLineCnt);
                return 2;
            }
            break;
        case '>':
            if (addr < MEM_SIZE - 1) ++addr;
            else {
                printf("?Error address upper limit. cmd at %d, line %u", i, uLineCnt);
                return 2;
            }
            break;
        case '/':
            iRet = condProc(i, 2, FALSE);
            if (iRet != 2) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            if (ptrMode == 0) {
                if (!mem[y]) {
                    printf("?Division by 0. cmd at %d, line %u", i, uLineCnt);
                    return 2;
                }
                pc = (signed char*)g_buf;
                *pc = (signed char)mem[x] / (signed char)mem[y];
                pAddrInit();
            }
            else if (ptrMode == 1) {
                prepIntCalc(TRUE);
                if (!*(pi + 1)) {
                    printf("?Division by 0. cmd at %d, line %u", i, uLineCnt);
                    pAddrInit();
                    return 2;
                }
                pi = (int*)(g_buf + INT_SIZE);
                g_iBuf = *pi / *(pi + 1);
                *(pi - 1) = g_iBuf;
                pAddrInit();
            }
            else if (ptrMode == 2) {
                prepDblCalc(TRUE);
                if (!*(pd + 1)) {
                    printf("?Division by 0. cmd at %d, line %u", i, uLineCnt);
                    pAddrInit();
                    return 2;
                }
                pd = (double*)(g_buf + DBL_SIZE);
                g_dBuf = *pd / *(pd + 1);
                *(pd - 1) = g_dBuf;
                pAddrInit();
            }
            else if (ptrMode == 3) {
                if (!mem[y]) {
                    printf("?Division by 0. cmd at %d, line %u", i, uLineCnt);
                    return 2;
                }
                g_buf[0] = mem[x] / mem[y];
            }
            else if (ptrMode == 4) {
                prepIntCalc(TRUE);
                if (!*(pi + 1)) {
                    printf("?Division by 0. cmd at %d, line %u", i, uLineCnt);
                    pAddrInit();
                    return 2;
                }
                pu = (unsigned*)(g_buf + INT_SIZE);
                g_iBuf = *pu / *(pu + 1);
                *(pu - 1) = g_iBuf;
                pAddrInit();
            }
            break;
        case '?':
            if (ptrMode == 0 || ptrMode == 3) {
                mem[addr] = (byte)((rand() % 255) + 1);
            }
            else if (ptrMode == 1 || ptrMode == 4) {
                pu = (unsigned*)(mem + addr);
                *pu = (unsigned)(rand() % RAND_MAX + 1);
                pAddrInit();
            }
            else if (ptrMode == 2) {
                pd = (double*)(mem + addr);
                *pd = (double)(rand() / (double)RAND_MAX * 1000.0);
            }
            break;
        case '`':
            printf("?Error string marker without command. cmd at %d, line %u", i, uLineCnt);
            return 1;
        case '~':
            iRet = condProc(i, 1, FALSE);
            if (iRet != 1) {
                printf("?Condition error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            else {
                if (chkAddrRange(i, TRUE)) return 2;
                if (cmpProc(currCmd)) {
                    printf(" cmd at %d, line %u", i, uLineCnt);
                    return 1;
                }
            }
            break;
        case '[':
            bracketCnt = 0;
            if (!mem[addr]) {
                while (1) {
                    if (cmdBuf[i] == '[') bracketCnt++;
                    if (cmdBuf[i] == ']') bracketCnt--;
                    if (cmdBuf[i] == ']' && !bracketCnt) break;
                    i++;
                }
            }
            break;
        case ']':
            bracketCnt = 0;
            if (mem[addr]) {
                while (1) {
                    if (cmdBuf[i] == ']') bracketCnt--;
                    if (cmdBuf[i] == '[') bracketCnt++;
                    if (cmdBuf[i] == '[' && !bracketCnt) break;
                    i--;
                }
            }
            break;
        case '(':
            if (chkAddrRange(i, TRUE)) return 2;
            iRet = exeCond();
            if (iRet == 0) {
                while (1) {
                    if (cmdBuf[i] == ';') return 0;
                    else if (cmdBuf[i] == '(') parenthesesCnt++;
                    else if (cmdBuf[i] == ')' && parenthesesCnt) parenthesesCnt--;
                    if (cmdBuf[i] == ')' && !parenthesesCnt) break;
                    i++;
                }
            }
            else if (iRet == 1) break;
            else if (iRet == -1) {
                printf(" cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            break;
        case '{':
            if (chkAddrRange(i, FALSE)) return 2;
            else {
                if (ptrMode == 0 || ptrMode == 3) mem[addr] = mem[addr] << 1;
                else if (ptrMode == 1 || ptrMode == 4) {
                    pu = (unsigned*)(mem + addr);
                    *pu = (*pu) << 1;
                    pu = (unsigned*)mem;
                }
                else if (ptrMode == 2) {
                    long long* pll = (long long*)(mem + addr);
                    *pll = (*pll) << 1;
                }
            }
            break;
        case '}':
            if (chkAddrRange(i, FALSE)) return 2;
            else {
                if (ptrMode == 0 || ptrMode == 3) mem[addr] = mem[addr] >> 1;
                else if (ptrMode == 1 || ptrMode == 4) {
                    pu = (unsigned*)(mem + addr);
                    *pu = (*pu) >> 1;
                    pu = (unsigned*)mem;
                }
                else if (ptrMode == 2) {
                    long long* pll = (long long*)(mem + addr);
                    *pll = (*pll) << 1;
                }
            }
            break;
        }
    }
    return 0;
}

_Bool balanceChk(void) {
    unsigned u = 0;
    int parenthesesBalance = 0, bracketBalance = 0;

    while (cmdBuf[u]) {
        if (cmdBuf[u] == '(') parenthesesBalance++;
        else if (cmdBuf[u] == ')') parenthesesBalance--;
        else if (cmdBuf[u] == '[') bracketBalance++;
        else if (cmdBuf[u] == ']') bracketBalance--;
        else if (cmdBuf[u] == ';') break;
        u++;
    }
    if (parenthesesBalance == 0 && bracketBalance == 0) return FALSE;
    else return TRUE;
}

int chkIntrprCmd(void) {
    char* p;
    unsigned u = 0;
    _Bool stat = FALSE;
    p = szParse(cmdBuf, " \n\r");
    if (!szCmp(cmdBuf, "--import")) {
        szCpy(nameBuf, NAME_SIZE, p);
        (void)szParse(nameBuf, " \n\r");
        pLib = fopen(nameBuf, "r");
        if (pLib == NULL) {
            printf("?Error importing library %s", p);
            if (isExec) isExec = FALSE;
            return 1;
        }
    }
    else if (!szCmp(cmdBuf, "--close-lib")) {
        if (!pLib) {
            printf("?Error library not loaded");
            return 1;
        }
        fclose(pLib);
        pLib = NULL;
    }
    else if (!szCmp(cmdBuf, "--record-start")) {
        if (pRec) {
            printf("?Recording already started");
            return 1;
        }
        else {
            time_t t = time(NULL);
            struct tm* timeinfo = localtime(&t);
            pRec = fopen("abf_record", "a+");
            fputs("Recording started at: ", pRec);
            fputs(asctime(timeinfo), pRec);
            isRecording = TRUE;
        }
    }
    else if (!szCmp(cmdBuf, "--record-stop")) {
        if (!pRec && !isRecording) {
            printf("?Recording not started");
            return 1;
        }
        else {
            fclose(pRec);
            pRec = NULL;
            isRecording = FALSE;
        }
    }
    else if (!szCmp(cmdBuf, "--record-del")) {
        if (pRec && isRecording) {
            printf("?Stop recording using \"--record-stop\" before deleting the file");
            return 1;
        }
        else remove("abf_record");
    }
    else if (!szCmp(cmdBuf, "--record-print")) {
        if (!pRec) pRec = fopen("abf_record", "a+");
        rewind(pRec);
        char strBuf[BUF_SIZE];
        memFill(strBuf, 0, BUF_SIZE);
        while (!feof(pRec)) {
            fgets(strBuf, BUF_SIZE, pRec);
            if (feof(pRec)) break;
            printf("%s", strBuf);
        }
        if (!isRecording) {
            fclose(pRec);
            pRec = NULL;
        }
        else fseek(pRec, 0, SEEK_END);
    }
    else if (!szCmp(cmdBuf, "--call")) {
        if (!pLib) {
            printf("?Error library not loaded");
            return 1;
        }
        rewind(pLib);
        szCpy(nameBuf, NAME_SIZE, p);
        (void)szParse(nameBuf, " \n\r");
        while (!feof(pLib) && !stat) {
            fgets(fBuf, BUF_SIZE, pLib);
            if (!feof(pLib) && fBuf[szLen(fBuf)]) fBuf[szLen(fBuf) - 1] = null;
            if (fBuf[0] == '+' && fBuf[1] == '+') p = szParse(fBuf, " \n\r");
            if (!szCmp((fBuf + 2), nameBuf)) while (!feof(pLib) && !stat) {
                fgets(fBuf, BUF_SIZE, pLib);
                if (!feof(pLib) && fBuf[szLen(fBuf)]) fBuf[szLen(fBuf) - 1] = null;
                if (fBuf[0] == '-' && fBuf[1] == '-') p = szParse(fBuf, " \n\r");
                if (!szCmp(fBuf, "--srend")) {
                    stat = TRUE;
                    break;
                }
                szCpy(cmdBuf, BUF_SIZE, fBuf);
                if (!!cmdProc()) {
                    stat = TRUE;
                    break;
                }
            }
        }
        if (!stat) {
            printf("?Subroutine \"%s\" not found", nameBuf);
            return 1;
        }
    }
    else if (!szCmp(cmdBuf, "--disp-internal-vars")) {
        printf("File loaded: %s, Library loaded: %s, ", pFile ? "TRUE" : "FALSE", pLib ? "TRUE" : "FALSE");
        printf("x: %#x, y: %#x, dx: %.12g\n", x, y, dx);
        printf("line: %u, position label: %u, position buffer: %u\n", uLineCnt, basicPos, basicPosBuf);
        printf("int buffer: %#x, dbl buffer: %.12g, condition cmd: %c\n", g_iBuf, g_dBuf, g_cmdCond ? g_cmdCond : '0');
        printf("buffer: 0x");
        for (u = 0; u < 32; u++) {
            printf("%x", g_buf[u]);
        }
        printf("\nname buffer: %s", nameBuf);
    }
    else if (!szCmp(cmdBuf, "--disp-subroutine-list")) {
        if (!pLib) {
            printf("?Error library not loaded");
            return 1;
        }
        rewind(pLib);
        while (!feof(pLib) && !stat) {
            fgets(fBuf, BUF_SIZE, pLib);
            if (!feof(pLib) && fBuf[szLen(fBuf)]) fBuf[szLen(fBuf) - 1] = null;
            (void)szParse(fBuf, " \n\r");
            if (!szCmp(fBuf, "--list-begin")) {
                while (!feof(pLib)) {
                    fgets(fBuf, BUF_SIZE, pLib);
                    if (fBuf[0] == '-' && fBuf[1] == '-') (void)szParse(fBuf, " \n\r");
                    if (!szCmp(fBuf, "--list-end")) {
                        stat = TRUE;
                        break;
                    }
                    printf("%s", fBuf);
                }
            }
        }
    }
    else if (!szCmp(cmdBuf, "--srend")) {
        printf("?\"--srend\" can only be used in library files");
        return 1;
    }
    else if (!szCmp(cmdBuf, "--help")) help();
    else if (!szCmp(cmdBuf, "--clrscr")) clrscr();
    else if (!szCmp(cmdBuf, "--load")) {
        if (pFile) {
            printf("?Error file not loaded");
            return 1;
        }
        else {
            szCpy(nameBuf, NAME_SIZE, p);
            (void)szParse(nameBuf, " \n\r");
            pFile = fopen(nameBuf, "r");
            if (pFile == NULL) {
                printf("?Error loading file %s", p);
                if (isExec) isExec = FALSE;
                return 1;
            }
        }
    }
    else if (!szCmp(cmdBuf, "--run")) {
        if (!pFile) {
            printf("?Error File not loaded");
            return 1;
        }
        else if (isExec) {
            printf("?Error \"--run\" cannot be used in file");
            return 1;
        }
        else return RUN_FILE;
    }
    else if (!szCmp(cmdBuf, "--print-code")) {
        if (!pFile) {
            printf("?Error file not loaded");
            return 1;
        }
        else if (isExec) {
            printf("?Error \"--print-code\" cannot be used in file");
            return 1;
        }
        else {
            while (!feof(pFile)) {
                fgets(fBuf, BUF_SIZE, pFile);
                printf("%s", fBuf);
            }
            rewind(pFile);
            uLineCnt = 0;
        }
    }
    else if (!szCmp(cmdBuf, "--close")) {
        if (!pFile) {
            printf("?Error file not loaded");
            return 1;
        }
        else {
            if (!!fclose(pFile)) printf("?Error closing file");
            else pFile = NULL;
        }
    }
    else if (!szCmp(cmdBuf, "--init")) initGlobalVar();
    else if (!szCmp(cmdBuf, "--exit")) {
        if (pFile) fclose(pFile);
        if (pLib) fclose(pLib);
        if (pRec) fclose(pRec);
        exit(0);
    }
    return FALSE;
}

int exeFile(void) {
    int rtnVal = 0;
    uLineCnt = 1;
    isExec = TRUE;
    rewind(pFile);

    while (!feof(pFile)) {
        fgets(cmdBuf, BUF_SIZE, pFile);
        if (!feof(pFile) && cmdBuf[szLen(cmdBuf)]) cmdBuf[szLen(cmdBuf) - 1] = null;
        basicPos = atou(cmdBuf);
        uLineCnt++;
        if (balanceChk()) {
            printf("?Error parentheses and/or brackets are not balanced");
            uLineCnt = 0;
            isExec = FALSE;
            basicPos = 0;
            return 1;
        }
        rtnVal = cmdProc();
        if (rtnVal) {
            if (rtnVal == -2) {
                printf("\nProgram handled error(%d)", x);
                initGlobalVar();
                uLineCnt = 0;
                rewind(pFile);
                isExec = FALSE;
                basicPos = 0;
                return -1;
            }
            uLineCnt = 0;
            rewind(pFile);
            isExec = FALSE;
            basicPos = 0;
            return 1;
        }
        if (termination) {
            termination = FALSE;
            rewind(pFile);
            uLineCnt = 0;
            isExec = FALSE;
            basicPos = 0;
            return 0;
        }
        if (chkIntrprCmd()) {
            uLineCnt = 0;
            isExec = FALSE;
            basicPos = 0;
            return -1;
        }
    }
    uLineCnt = 0;
    basicPos = 0;
    isExec = FALSE;
    return 0;
}

int main(void) {
    int rtnVal = 0;
    clrscr();
    srand((unsigned)time(NULL));

    initGlobalVar();
    puts("Ascii BrainFuck Language Interpreter Prompt v1.10");
    printf("int size: %d Bytes, double size: %d Bytes\n", INT_SIZE, DBL_SIZE);
    printf("int range: %d to %d, double range: (+-) %.12g to %.12g\n", INT_MIN, INT_MAX, DBL_MIN, DBL_MAX);
    printf("Memory size: %d Bytes\nMaximum command length per line: %d Bytes including null\n", MEM_SIZE, BUF_SIZE);
    printf("Maximum file name length: %d Bytes including null\n", NAME_SIZE);
    printf("Byte order is %s.\n", chkEndian() ? "Little endian" : "NOT little endian.");
    puts("***Type --help for commands***");

    while (1) {
        printf("\nREADY(%d,%05d)>> ", ptrMode, addr);
        fgets(cmdBuf, BUF_SIZE, stdin);
        if (isRecording) fputs(cmdBuf, pRec);
        if (szLen(cmdBuf) < BUF_SIZE) cmdBuf[szLen(cmdBuf) - 1] = null;
        if (balanceChk()) printf("?Error parentheses and/or brackets are not balanced");
        else {
            (void)cmdProc();
            rtnVal = chkIntrprCmd();
            if (rtnVal == RUN_FILE) {
                rtnVal = exeFile();
                if (rtnVal == 1) printf("... Program execution halted.");
                continue;
            }
        }
    }
    if (pFile) fclose(pFile);
    if (pLib) fclose(pLib);
    if (pRec) fclose(pRec);
    exit(1);
    return 1;
}

void help(void) {
    puts("The Ascii BrainFuck commands");
    puts("a: AND                                       []: while(*ptr) loop bracket");
    puts("b: break                                     `: string start/end marker");
    puts("c: define condition                          !: NEQ");
    puts("d: decrement value                           @: set float pointer");
    puts("e: EQU                                       #: set int pointer");
    puts("f: forward(set ptr to X)                     $: set char pointer");
    puts("g: get keyboard input with echo              %: modulus");
    puts("h: handle Error code                         ^: bitwise XOR");
    puts("i: increment value                           &: bitwise AND");
    puts("j: jump                                      *: multiply");
    puts("k: key(get keyboard input without echo)      (): if parentheses");
    puts("l: increase/decrease value by X              -: subtract");
    puts("m: match(copy value at X to Y)               _: print as double");
    puts("n: NOT                                       =: save product to ptr");
    puts("o: OR                                        +: add");
    puts("p: print as char                             .: decimal, for double");
    puts("q: increase/decrease pointer by X            ,: separator");
    puts("r: return                                    <: decrement pointer");
    puts("s: write string                              >: increment pointer");
    puts("t: terminate file execution                  /: divide(*(p+X) / *(p+Y))");
    puts("u: suspend for X second(s)                   ?: write random number");
    puts("v: jump to next line                         ;: REM(comment)");
    puts("w: write X                                   :: compare");
    puts("x: exchange value at X with value at Y       ': set signed type");
    puts("y: save current line num to line num buffer  \": set unsigned type");
    puts("z: zerofill memory                           \\: print as int");
    puts("|: bitwise OR                                {: bit shift to left");
    puts("~: bitwise NOT                               }: bit shift to right");
    puts("Interpreter specific commands");
    puts("--help: print this help message");
    puts("--load name: load program file");
    puts("--run: run program");
    puts("--print-code: print source code of program file");
    puts("--close: close program file");
    puts("--init: initialize interpreter except for file ptr and command buffer");
    puts("--exit: exit interpreter");
    puts("--clrscr: clear screen");
    puts("--import name: import a library file");
    puts("--close-lib: close library file");
    puts("++name: library only, subroutine [name] start point marker");
    puts("--srend: library only, subroutine end point marker");
    puts("--list-begin: library only, beginning of subroutine list");
    puts("--list-end: library only, end of subroutine list");
    puts("--record-start: start recording command line input to file");
    puts("--record-stop: stop recording command line input");
    puts("--record-del: delete record file");
    puts("--record-print: print record file");
    puts("--call name: call subroutine defined in the imported library");
    puts("--disp-internal-vars: display values of internal vars");
    puts("--disp-subroutine-list: display list of subroutines in library");
    puts("For syntax & command usage, please refer to Github or README.");
    puts("Github: https://github.com/DS1TPT/ABF_Lang");
    puts("This program is licensed under the Apache License, Version 2.0");
    printf("Copyright 2020-2021. Lee Geon-goo");
}

void memFill(char* dst, char dta, unsigned size) {
    unsigned u = 0;
    for (u = 0; u < size; u++) *dst++ = dta;
}

_Bool searchCh(char* sz, char* cp) {
    while (*sz) if (*sz++ == *cp) return TRUE;
    return FALSE;
}

unsigned szLen(char* sz) {
    unsigned u = 0;
    while (sz[u]) u++;
    return u;
}

void szCpy(char* dst, unsigned size, char* src) {
    unsigned u = 0;
    for (u = 0; u < size - 1; u++) *dst++ = *src++;
    *dst = null;
}

char* szParse(char* sz, const char* delim) {
    char* pDelimCh;
    while (*sz) {
        pDelimCh = (char*)delim;
        while (*pDelimCh) {
            if (*sz == *pDelimCh++) *sz = null;
        }
        if (!*sz) return ++sz;
        sz++;
    }
    return NULL;
}

int szCmp(char* sz1, char* sz2) {
    while (*sz1 == *sz2) {
        if (!(*sz1)) return *sz1 - *sz2;
        sz1++;
        sz2++;
    }
    return *sz1 - *sz2;
}

void memCpy(char* dst, unsigned size, char* src) {
    unsigned u = 0;
    for (u = 0; u < size; u++) *dst++ = *src++;
}

unsigned atou(char* sz) {
    unsigned n = 0;
    while (*sz >= '0' && *sz <= '9') n = n * 10 + *sz++ - '0';
    return n;
}

long long roundftoll(double* pDb) {
    return ((*pDb > 0.0) ? (long long)(*pDb + 0.5) : (long long)(*pDb - 0.5));
}

void clrscr(void) {
    system("@cls||clear");
}

_Bool chkEndian(void) {
    unsigned u = 1;
    byte* ptr = (byte*)&u;
    if (*ptr) return TRUE;
    else return FALSE;
}
