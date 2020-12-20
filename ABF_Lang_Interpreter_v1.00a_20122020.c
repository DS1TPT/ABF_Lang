/*
* Copyright 2020. Lee Geon-goo
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

/* version 1.00a (20122020) */
/* DEV Environment: MSVC 2019, x86 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h> /* atof */
#include <limits.h>
#include <float.h>

#if defined _MSC_VER && ( defined _WIN32 || defined _WIN64 )
#pragma message ("Windows system, Using _getche() for 'g'.")
#include <conio.h> /* for _getch() */
#elif defined __linux__ || defined unix || defined __unix || defined __unix__
#pragma message ("*nix system, using termios to implement _getche(). for 'g'")
#include <termios.h> /* for implementation of _getch() for *nix */
int nixGetche() {
    int ch;
    struct termios old;
    struct termios current;

    tcgetattr(0, &old);
    current = old;
    current.c_lflag &= ~ICANON;
    current.c_lflag |= ECHO;
    tcsetattr(0, TCSANOW, &current);
    ch = getchar();
    tcsetattr(0, TCSANOW, &old);

    return ch;
}
#define _getche() nixGetche()
#else
#define _getche() getchar()
#pragma message ("[WARNING] OS is not Windows or *nix. _getche() equivalent function is not declared. Using getchar() for 'g' instead.")
#pragma message ("Please implement _getche() equivalent function if available.")
#endif

#define FALSE 0
#define TRUE 1
#define RUN_FILE 100
#define USE_CURR_ADDR -1
#define null 0 /* int null */

/* SIZE AND VALUE LIMIT */
#define INT_SIZE sizeof(int)
#define DBL_SIZE sizeof(double)
#define MEM_SIZE 65535
#define BUF_SIZE 4096
#define NAME_SIZE 1024

typedef _Bool BOOL; /* C99 required */
typedef unsigned char byte;
/* typedef byte BOOL */ /* Use this instead of _Bool if C99 is not supported */

/* BUFFERS */
char cmdBuf[BUF_SIZE], fBuf[BUF_SIZE], nameBuf[NAME_SIZE];
char g_cmdCond = 0;
byte g_buf[32]; /* reduced size */
int g_iBuf = 0;
double g_dBuf = 0.0;

/* POINTERS & ARRAYS */
byte mem[MEM_SIZE];
char* pc = (char*)mem;
double* pd = (double*)mem;
int* pi = (int*)mem;
unsigned* pu = (unsigned*)mem;
FILE* pFile = NULL;

/* STATUS MARKERS */
byte ptrMode = 0; /* ptrMode 0: char, 1: int, 2: double, 3: uchar(byte), 4: uint */
BOOL isUnsigned = FALSE, isFileLoaded = FALSE, isCondPresent = FALSE, termination = FALSE, isExec = FALSE;
unsigned uLineCnt = 0, basicPos = 0, basicPosBuf = 0;

/* MISCELLANEOUS */
int x = 0, y = 0;
double dx = 0.0;
unsigned addr = 0;

void help(); /* print help message(cmd list) */
void memFill(char* dst, char dta, int size);  /* fill mem with dta */
BOOL searchCh(char* sz, char* cp); /* search if there's *cp in sz */
unsigned szLen(char* sz); /* calculate the length of string */
void szCpy(char* dst, int size, char* src); /* copy src's dta to dst, writes null */
void memCpy(char* dst, int size, char* src); /* copy src's dta to dst, does not write null */
unsigned atou(char* sz); /* ASCII string to unsinged */
long long roundftoll(double* pDb); /* round double and cast it to int, then return int value */
void clrscr(); /* clears screen(console) */
BOOL chkEndian(); /* check endianness, returns TRUE if system is little endian */

void initGlobalVar() {
    memFill(mem, 0, MEM_SIZE);
    memFill(cmdBuf, 0, BUF_SIZE);
    memFill(fBuf, 0, BUF_SIZE);
    if (!isFileLoaded) memFill(nameBuf, 0, NAME_SIZE);
    memFill(g_buf, 0, sizeof(g_buf));
    g_cmdCond = 0;
    g_iBuf = 0;
    g_dBuf = 0.0;
    x = 0, y = 0;
    dx = 0.0;
    addr = 0;
    ptrMode = 0;
    isUnsigned = FALSE, isCondPresent = FALSE, termination = FALSE;
    basicPos = 0, basicPosBuf = 0;
    pc = (char*)mem;
    pi = (int*)mem;
    pd = (double*)mem;
    pu = (unsigned*)mem;
}

BOOL chkAddrRange(int cmdPos, BOOL isXY) {
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

int nameProc(int cmdPos) { /* get name and store it to nameBuf */
    char* cp = cmdBuf + cmdPos;
    char strBuf[NAME_SIZE];
    memFill(strBuf, 0, NAME_SIZE);
    char* pArr = strBuf;
    BOOL firstNameMarker = FALSE;
    int pos = 0;

    while (*cp) {
        if (*cp == '`' && firstNameMarker == TRUE) {
            firstNameMarker = FALSE;
            *pArr = 0;
            szCpy(nameBuf, NAME_SIZE, strBuf);
            return pos;
        }
        else if (*cp != '`' && firstNameMarker == TRUE) *pArr++ = *cp;
        else if (*cp == '`' && firstNameMarker == FALSE) firstNameMarker = TRUE;
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

int condProc(int cmdPos, int condNum) { /* set int type conditions, returns condition count */
    char* cp = cmdBuf + cmdPos + 1;
    char strBuf[34];
    memFill(strBuf, 0, 34);
    int condCnt = 0, i = 0;
    BOOL isCmpCmd = FALSE;

    x = 0;
    y = 0;

    szCpy(strBuf, 34, cp);
    for (i = 0; i < condNum; i++) {
        if (!(*cp))
            return i;
        else if (i == 0) {
            if (!isUnsigned) x = atoi(cp);
            else x = atou(cp);
            condCnt++;
        }
        else if (i == 1) {
            if (!isUnsigned) y = atoi(cp);
            else y = atou(cp);
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

void condErr(int condCnt) { /* prints condition count error */
    printf("?Error condition count is not %d.", condCnt);
}

void prepIntCalc(BOOL isY) { /* store int value to g_buf for calculation */
    int i = 0;
    if (isY) {
        for (i = 0; i < INT_SIZE; i++) {
            g_buf[INT_SIZE + i] = mem[x + i];
            g_buf[INT_SIZE * 2 + i] = mem[y + i];
        }
    }
    else {
        for (i = 0; i < INT_SIZE; i++)
            g_buf[INT_SIZE + i] = mem[x + i];
    }
    if (ptrMode == 1) pi = (int*)g_buf;
    else if (ptrMode == 4) pu = (unsigned*)g_buf;
}

void prepDblCalc(BOOL isY) { /* store double value to g_buf for calculation */
    int i = 0;
    if (isY) {
        for (i = 0; i < DBL_SIZE; i++) {
            g_buf[DBL_SIZE + i] = mem[x + i];
            g_buf[DBL_SIZE * 2 + i] = mem[y + i];
        }
    }
    else {
        for (i = 0; i < DBL_SIZE; i++)
            g_buf[DBL_SIZE + i] = mem[x + i];
    }
    pd = (double*)g_buf;
}

void pAddrSet(int pos) { /* set pointers except for char types to mem + addr */
    if (pos == USE_CURR_ADDR) {
        if (ptrMode == 0) pc = (char*)(mem + addr);
        else if (ptrMode == 1) pi = (int*)(mem + addr);
        else if (ptrMode == 2) pd = (double*)(mem + addr);
        else if (ptrMode == 4) pu = (unsigned*)(mem + addr);
    }
    else {
        if (ptrMode == 0) pc = (char*)(mem + pos);
        else if (ptrMode == 1) pi = (int*)(mem + pos);
        else if (ptrMode == 2) pd = (double*)(mem + pos);
        else if (ptrMode == 4) pu = (unsigned*)(mem + pos);
    }
}

void pAddrInit() { /* reset pointers except for char types to mem */
    if (ptrMode == 0) pc = (char*)mem;
    else if (ptrMode == 1) pi = (int*)mem;
    else if (ptrMode == 2) pd = (double*)mem;
    else if (ptrMode == 4) pu = (unsigned*)mem;
}

void writeDta() { /* write data to mem */
    if (ptrMode == 0) {
        pc = (char*)mem;
        *(pc + addr) = (char)x;
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

int exeCond() {
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

int cmdProc() { /* Interpret commands */
    int bracketCnt = 0, parenthesesCnt = 0, iRet = 0, i = 0, j = 0;
    char currCmd;

    /*
    return value 1 is syntax error, 2 is memory/value range error, 3 is Internal/External error,
    4 is Miscellaneous error, -1 is Exit interpreter, -2 is terminate execution(with error).
    */

    for (i = 0; cmdBuf[i]; i++) {
        currCmd = cmdBuf[i];
        switch (currCmd) {
        case 'a':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else cmpProc(currCmd);
            break;
        case 'b':
            while (cmdBuf[i] != ']') i++;
            break;
        case 'c':
            iRet = condProc(i, 3);
            if (iRet != 3) {
                isCondPresent = FALSE;
                g_cmdCond = null;
                if (iRet == -1) return 1;
                condErr(3);
                return 1;
            }
            else {
                isCondPresent = TRUE;
                while (cmdBuf[i] != g_cmdCond) {
                    i++;
                }
            }
            break;
        case 'd':
            if (ptrMode == 0 || ptrMode == 3) mem[addr]--;
            else if (ptrMode == 1) {
                pAddrSet(USE_CURR_ADDR);
                (*pi)--;
                pAddrInit();
            }
            else if (ptrMode == 2) {
                pAddrSet(USE_CURR_ADDR);
                *pd -= 1.0;
                pAddrInit();
            }
            else if (ptrMode == 4) {
                pAddrSet(USE_CURR_ADDR);
                (*pu)--;
                pAddrInit();
            }
            break;
        case 'e':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else cmpProc(currCmd);
            break;
        case 'f':
            iRet = condProc(i, 1);
            if (iRet != 1) {
                if (iRet == -1) return 1;
                condErr(1);
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
                iRet = condProc(i, 1);
                if (iRet != 1) {
                    if (iRet == -1) return 1;
                    condErr(1);
                    return 1;
                }
                else return -2;
            }
            break;
        case 'i':
            if (ptrMode == 0 || ptrMode == 3) mem[addr]++;
            else if (ptrMode == 1) {
                pAddrSet(USE_CURR_ADDR);
                (*pi)++;
                pAddrInit();
            }
            else if (ptrMode == 2) {
                pAddrSet(USE_CURR_ADDR);
                *pd += 1.0;
                pAddrInit();
            }
            else if (ptrMode == 4) {
                pAddrSet(USE_CURR_ADDR);
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
                iRet = condProc(i, 1);
                if (iRet != 1) {
                    if (iRet == -1) return 1;
                    condErr(1);
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
            initGlobalVar();
            break;
        case 'l':
            if (isFileLoaded) {
                printf("?Error a file has already been loaded. Unload file first. cmd at %d", i);
                return 4;
            }
            iRet = nameProc(i);
            if (iRet == 0) {
                printf("?Name marker error, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            else {
                i += iRet;
                pFile = fopen(nameBuf, "r");
                if (pFile == NULL) {
                    printf("?Error Loading file %s", nameBuf);
                    return 3;
                }
                else {
                    printf("File Loaded %s", nameBuf);
                    isFileLoaded = TRUE;
                }
            }
            break;
        case 'm':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
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
            iRet = condProc(i, 1);
            if (iRet != 1) {
                if (iRet == -1) return 1;
                condErr(1);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else cmpProc(currCmd);
            break;
        case 'o':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else cmpProc(currCmd);
            break;
        case 'p':
            printf("%c", mem[addr]);
            break;
        case 'q':
            if (!isFileLoaded) {
                printf("?Error 'q' requires loading file. cmd at %d", i);
                return 4;
            }
            else if (isExec) {
                printf("?Error 'q' is used in file. cmd at %d, line %u", i, uLineCnt);
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
            if (!isFileLoaded) {
                printf("?Error File not loaded");
                return 3;
            }
            else if (isExec) {
                printf("?Error file already running, cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            else return RUN_FILE;
            break;
        case 't':
            if (isExec) termination = TRUE;
            return 0;
            break;
        case 'u':
            if (!isFileLoaded) printf("?Error file not loaded");
            else {
                if (!!fclose(pFile)) printf("?Error closing file");
                else {
                    pFile = NULL;
                    isFileLoaded = FALSE;
                }
            }
            break;
        case 'v': return 0;
        case 'w':
            iRet = condProc(i, 1);
            if (iRet != 1) {
                if (iRet == -1) return 1;
                condErr(1);
                return 1;
            }
            if ((x > CHAR_MAX || x < CHAR_MIN) && ptrMode == 0) {
                printf("?Error value X is out of range. cmd at %d, line %u", i, uLineCnt);
                return 2;
            }
            else if (x > UCHAR_MAX&& ptrMode == 3) {
                printf("?Error value X is out of range. cmd at %d, line %u", i, uLineCnt);
                return 2;
            }
            /* Does not check value range for int, uint, and double */
            if (chkAddrRange(i, FALSE)) return 2;
            if (ptrMode == 2) getDblCond(i);
            writeDta();
            i++;
            while ((cmdBuf[i] >= '0' && cmdBuf[i] <= '9') || cmdBuf[i] == '-' || cmdBuf[i] == 'e' || cmdBuf[i] == '.') i++;
            i--;
            break;
        case 'x':
            return -1;
        case 'y':
            if (!isFileLoaded || !isExec) {
                printf("?Error this command can be used ONLY in file(s). cmd at %d", i);
                return 1;
            }
            else basicPosBuf = basicPos;
            break;
        case 'z':
            memFill(mem, 0, MEM_SIZE);
            break;
        case '!':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
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
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else {
                if (ptrMode == 0) {
                    pc = g_buf;
                    *pc = (char)mem[x] % (char)mem[y];
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
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
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
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
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
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else {
                if (ptrMode == 0) {
                    pc = g_buf;
                    *pc = (char)mem[x] * (char)mem[y];
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
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            else {
                if (ptrMode == 0) {
                    pc = (char*)g_buf;
                    *pc = (char)mem[x] - (char)mem[y];
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
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            else {
                if (ptrMode == 0) {
                    pc = g_buf;
                    *pc = (char)mem[x] + (char)mem[y];
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
                pAddrSet(USE_CURR_ADDR);
                if (ptrMode == 0) printf("%d", *pc);
                else if (ptrMode == 1) printf("%d", *pi);
                else if (ptrMode == 2) printf("%lld", roundftoll(pd));
                else if (ptrMode == 4) printf("%u", *pu);
                pAddrInit();
            }
            break;
        case '|':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                condErr(2);
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
            iRet = condProc(i, 2);
            if (iRet != 2) {
                condErr(2);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else {
                if (ptrMode == 0) {
                    pAddrInit();
                    if (*(pc + x) > * (pc + y)) g_buf[0] = 1;
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
                    if (*(pi + 1) > * (pi + 2)) g_buf[0] = 1;
                    else if (*(pi + 1) == *(pi + 2)) g_buf[0] = 0;
                    else if (*(pi + 1) < *(pi + 2)) g_buf[0] = (byte)-1;
                    pAddrInit();
                }
                else if (ptrMode == 4) {
                    prepIntCalc(TRUE);
                    pu = (unsigned*)g_buf;
                    if (*(pu + 1) > * (pu + 2)) g_buf[0] = 1;
                    else if (*(pu + 1) == *(pu + 2)) g_buf[0] = 0;
                    else if (*(pu + 1) < *(pu + 2)) g_buf[0] = (byte)-1;
                    pAddrInit();
                }
                else if (ptrMode == 2) {
                    prepDblCalc(TRUE);
                    pd = (double*)g_buf;
                    if (*(pd + 1) > * (pd + 2)) g_buf[0] = 1;
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
            iRet = condProc(i, 2);
            if (iRet != 2) {
                condErr(2);
                return 1;
            }
            if (chkAddrRange(i, TRUE)) return 2;
            else {
                if (ptrMode == 0) {
                    pc = (char*)g_buf;
                    *pc = (unsigned char)mem[x] / (unsigned char)mem[y];
                    pAddrInit();
                }
                else if (ptrMode == 1) {
                    prepIntCalc(TRUE);
                    pi = (int*)(g_buf + INT_SIZE);
                    g_iBuf = *pi / *(pi + 1);
                    *(pi - 1) = g_iBuf;
                    pAddrInit();
                }
                else if (ptrMode == 2) {
                    prepDblCalc(TRUE);
                    pd = (double*)(g_buf + DBL_SIZE);
                    g_dBuf = *pd / *(pd + 1);
                    *(pd - 1) = g_dBuf;
                    pAddrInit();
                }
                else if (ptrMode == 3) g_buf[0] = mem[x] / mem[y];
                else if (ptrMode == 4) {
                    prepIntCalc(TRUE);
                    pu = (unsigned*)(g_buf + INT_SIZE);
                    g_iBuf = *pu / *(pu + 1);
                    *(pu - 1) = g_iBuf;
                    pAddrInit();
                }
            }
            break;
        case '?':
            help();
            break;
        case '`':
            printf("?Error name marker without command. cmd at %d, line %u", i, uLineCnt);
            return 1;
        case '~':
            iRet = condProc(i, 1);
            if (iRet != 1) {
                condErr(1);
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

BOOL balanceChk() {
    int i = 0, parenthesesBalance = 0, bracketBalance = 0;

    while (cmdBuf[i]) {
        if (cmdBuf[i] == '(') parenthesesBalance++;
        else if (cmdBuf[i] == ')') parenthesesBalance--;
        else if (cmdBuf[i] == '[') bracketBalance++;
        else if (cmdBuf[i] == ']') bracketBalance--;
        else if (cmdBuf[i] == ';') break;
        i++;
    }
    if (parenthesesBalance == 0 && bracketBalance == 0) return FALSE;
    else return TRUE;
}

int exeFile() {
    int rtnVal = 0;
    uLineCnt = 1;
    isExec = TRUE;

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
    }
    rewind(pFile);
    uLineCnt = 0;
    basicPos = 0;
    return 0;
}

int main() {
    int rtnVal = 0;
    clrscr();

    initGlobalVar();
    puts("Ascii BrainFuck Language Interpreter Prompt v1.00a");
    printf("int size: %d Bytes, double size: %d Bytes\n", INT_SIZE, DBL_SIZE);
    printf("int range: %d to %d, double range: (+-) %.12g to %.12g\n", INT_MIN, INT_MAX, DBL_MIN, DBL_MAX);
    printf("Memory size: %d Bytes\nMaximum command length per line: %d Bytes including null\n", MEM_SIZE, BUF_SIZE);
    printf("Maximum file name length: %d Bytes including null\n", NAME_SIZE);
    printf("Byte order is %s.\n", chkEndian() ? "Little endian" : "NOT little endian.");
    puts("***Type ? for commands***");

    while (1) {
        printf("\nREADY(%d,%05d)>> ", ptrMode, addr);
        fgets(cmdBuf, BUF_SIZE, stdin);
        if (szLen(cmdBuf) < BUF_SIZE) cmdBuf[szLen(cmdBuf) - 1] = null;
        if (balanceChk()) printf("?Error parentheses and/or brackets are not balanced");
        else {
            rtnVal = cmdProc();
            if (rtnVal == RUN_FILE) {
                rtnVal = exeFile();
                if (rtnVal == 1) printf("... Program execution halted.");
                rtnVal = 0;
                continue;
            }
            else if (rtnVal == -1) break;
            rtnVal = 0;
        }
    }
    return 0;
}

void help() {
    puts("The Ascii BrainFuck commands");
    puts("a: AND                                       []: while(*ptr) loop bracket");
    puts("b: break                                     `: name marker");
    puts("c: define condition                          !: NEQ");
    puts("d: decrement value                            @: set float pointer");
    puts("e: EQU                                       #: set int pointer");
    puts("f: forward(set ptr to X)                     $: set char pointer");
    puts("g: get keyboard input and save as uchar      %: modulus");
    puts("h: handle Error code                         ^: bitwise XOR");
    puts("i: increment value                           &: bitwise AND");
    puts("j: jump                                      *: multiply");
    puts("k: Initialize interpreter                    (): if parentheses");
    puts("l: load file                                 -: subtract");
    puts("m: match(copy value at X to Y)               _: print as double");
    puts("n: NOT                                       =: save product to ptr");
    puts("o: OR                                        +: add");
    puts("p: print as char                             .: decimal, for double");
    puts("q: print source code                         ,: separator");
    puts("r: return                                    <: decrement pointer");
    puts("s: start file execution                      >: increment pointer");
    puts("t: terminate file execution                  /: divide(*(p+X) / *(p+Y))");
    puts("u: unload file                               ?: help(command list)");
    puts("v: jump to next line                         ;: REM(comment)");
    puts("w: write X                                   :: compare");
    puts("x: exit interpreter                          ': set signed type");
    puts("y: save current line num to line num buffer  \": set unsigned type");
    puts("z: zerofill memory                           \\: print as int");
    puts("|: bitwise OR                                {: bit shift to left");
    puts("~: bitwise NOT                               }: bit shift to right");
    puts("For syntax & command usage, please refer to Github page or README.");
    puts("Github page: https://github.com/DS1TPT/ABF_Lang");
    printf("Note: This program is licensed under Apache License Version 2.0");
}

void memFill(char* dst, char dta, int size) {
    int i = 0;
    for (i = 0; i < size; i++) *dst++ = dta;
}

BOOL searchCh(char* sz, char* cp) {
    while (*sz) if (*sz++ == *cp) return TRUE;
    return FALSE;
}

unsigned szLen(char* sz) {
    unsigned i = 0;
    while (sz[i]) i++;
    return i;
}

void szCpy(char* dst, int size, char* src) {
    int i = 0;
    for (i = 0; i < size - 1; i++) *dst++ = *src++;
    *dst = null;
}

void memCpy(char* dst, int size, char* src) {
    int i = 0;
    for (i = 0; i < size; i++) *dst++ = *src++;
}

unsigned atou(char* sz) {
    unsigned n = 0;
    while (*sz >= '0' && *sz <= '9') n = n * 10 + *sz++ - '0';
    return n;
}

long long roundftoll(double* pDb) {
    return ((*pDb > 0.0) ? (long long)(*pDb + 0.5) : (long long)(*pDb - 0.5));
}

void clrscr() {
    system("@cls||clear");
}

BOOL chkEndian() {
    unsigned u = 1;
    byte* ptr = (byte*)&u;
    if (*ptr) return TRUE;
    else return FALSE;
}