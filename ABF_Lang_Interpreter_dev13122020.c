/*
* Copyright 2020 Lee Geon-goo
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

/* DEV 13-12-2020 */
/* DEV Environment: MSVC 2019, x86 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h> /* atof */
#include <limits.h>
#include <float.h>

#if defined _MSC_VER || defined _WIN32 || defined _WIN64
#include <conio.h>
#elif defined __linux__
#include <termios.h>
/* static struct termios old, current; */
linuxGetche() {
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
#define _getche() linuxGetche()
#endif

/*
To-do list
1. implement unsigned calculations
2. use pc for ptrMode 0
3. find logic errors
?. debug
???. simplify after finishing implementation and debugging
*/

#define FALSE 0
#define TRUE 1
#define RUN_FILE 100
#define USE_CURR_ADDR -1

/* SIZE AND VALUE LIMIT */
#define INT_SIZE sizeof(int)
#define DBL_SIZE sizeof(double)
#define MEM_SIZE 4096
#define BUF_SIZE 1024
#define NAME_SIZE 512

typedef _Bool BOOL; /* C99 required */
typedef unsigned char byte;
/* typedef byte BOOL */ /* Use this instead of _Bool if C99 is not supported */

/* BUFFERS */
char cmdBuf[BUF_SIZE], fBuf[BUF_SIZE], nameBuf[NAME_SIZE];
char g_cmdCond = 0;
byte g_buf[64];
int g_iBuf = 0;
double g_dBuf = 0.0;

/* POINTERS & ARRAYS */
byte mem[MEM_SIZE];
char* pc = mem;
double* pd = mem;
int* pi = mem;
unsigned* pu = mem;
FILE* pFile = NULL;

/* STATUS MARKERS */
char ptrMode = 0; /* ptrMode 0: char, 1: int, 2: double, 3: uchar(byte), 4: uint */
BOOL isUnsigned = FALSE, isFileLoaded = FALSE, isCondPresent = FALSE, termination = FALSE, isDebugMode = FALSE, isExec = FALSE;
unsigned uLineCnt = 0;

/* MISCELLANEOUS */
int x = 0, y = 0;
double dx = 0.0;
int addr = 0;

void help(); /* print help message(cmd list) */
void memFill(char* dst, char dta, int size);  /* fill mem with dta */
searchCh(char* sz, char* cp); /* search if there's *cp in sz */
unsigned szLen(char* sz); /* calculate the length of string */
void szCpy(char* dst, int size, char* src); /* copy src's dta to dst, writes null */
void memCpy(char* dst, int size, char* src); /* copy src's dta to dst, does not write null */
unsigned atou(char* sz); /* ASCII string to unsinged */
long long roundftoi(double* pDb); /* round double and cast it to int, then return int value */
void clrscr();

void initGlobalVar() {
    memFill(mem, 0, MEM_SIZE);
    memFill(cmdBuf, 0, BUF_SIZE);
    memFill(fBuf, 0, BUF_SIZE);
    if (isFileLoaded == FALSE) memFill(nameBuf, 0, NAME_SIZE);
    memFill(g_buf, 0, 64);
    g_cmdCond = 0;
    g_iBuf = 0;
    g_dBuf = 0.0;
    x = 0, y = 0;
    dx = 0.0;
    addr = 0;
    ptrMode = 0;
    isUnsigned = FALSE, isCondPresent = FALSE, termination = FALSE, isDebugMode = FALSE;
    pc = mem;
    pi = mem;
    pd = mem;
    pu = mem;
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

nameProc(int cmdPos) { /* get name and store it to nameBuf */
    char* cp = cmdBuf + cmdPos;
    char strBuf[BUF_SIZE];
    memFill(strBuf, 0, BUF_SIZE);
    char* pArr = strBuf;
    BOOL firstNameMarker = FALSE;
    int pos = 0;

    while (*cp != NULL) {
        if (*cp == '`' && firstNameMarker == TRUE) {
            firstNameMarker = FALSE;
            *pArr = NULL;
            szCpy(nameBuf, BUF_SIZE, strBuf);
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
    while (*pSz != NULL) {
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
    int i = 0;

    szCpy(strBuf, 64, cp);
    dx = atof(strBuf);
    memCpy(g_buf, DBL_SIZE, &dx);
}

condProc(int cmdPos, int condNum) { /* set int type conditions, returns condition count */
    char* cp = cmdBuf + cmdPos + 1;
    char* strBuf[34];
    memFill(strBuf, 0, 34);
    int condCnt = 0, i = 0, j = 0;
    BOOL isCmpCmd = FALSE;

    x = 0;
    y = 0;

    szCpy(strBuf, 34, cp);
    for (i = 0; i < condNum; i++) {
        /* printf("DEBUG: %s\n", cp); */
        if (*cp == NULL)
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
            isCmpCmd = searchCh("aeno|~!^&{}", cp);
            if (isCmpCmd == FALSE)
                return condCnt;
            else {
                g_cmdCond = *cp;
                condCnt++;
                return condCnt;
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
    if (ptrMode == 1) pi = g_buf;
    else if (ptrMode == 4) pu = g_buf;
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
    pd = g_buf;
}

void pAddrSet(int pos) { /* set pointers except for char types to mem + addr */
    if (pos == USE_CURR_ADDR) {
        if (ptrMode == 0) pc = (mem + addr);
        else if (ptrMode == 1) pi = (mem + addr);
        else if (ptrMode == 2) pd = (mem + addr);
        else if (ptrMode == 4) pu = (mem + addr);
    }
    else {
        if (ptrMode == 0) pc = (mem + pos);
        else if (ptrMode == 1) pi = (mem + pos);
        else if (ptrMode == 2) pd = (mem + pos);
        else if (ptrMode == 4) pu = (mem + pos);
    }
}

void pAddrInit() { /* reset pointers except for char types to mem */
    if (ptrMode == 0) pc = mem;
    else if (ptrMode == 1) pi = mem;
    else if (ptrMode == 2) pd = mem;
    else if (ptrMode == 4) pu = mem;
}

void writeDta() { /* write data to mem */
    if (ptrMode == 0) {
        pc = mem;
        *(pc + addr) = x;
        return;
    }
    else if (ptrMode == 1) {
        pi = g_buf;
        *pi = x;
        memCpy((mem + addr), INT_SIZE, g_buf);
        if (isDebugMode) {
            pAddrSet(USE_CURR_ADDR);
            printf("%d\n", *pi);
        }
    }
    else if (ptrMode == 2) {
        memCpy((mem + addr), DBL_SIZE, g_buf);
        if (isDebugMode) {
            pAddrSet(USE_CURR_ADDR);
            printf("%.12g\n", *pd);
        }
    }
    else if (ptrMode == 3) {
        mem[addr] = x;
        return;
    }
    else if (ptrMode == 4) {
        pu = g_buf;
        *pu = x;
        memCpy((mem + addr), INT_SIZE, g_buf);
        if (isDebugMode) {
            pAddrSet(USE_CURR_ADDR);
            printf("%u\n", *pu);
        }
    }
    pAddrInit();
}

cmpProc(char cmd) { /* process compare commands except for ':' */
    BOOL isSnglCond = FALSE;
    BOOL isIllegal = FALSE;

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
        isSnglCond = searchCh("n~", &cmd);
        if (isSnglCond == TRUE) prepIntCalc(FALSE);
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
        isIllegal = searchCh("|~^&", &cmd);
        if (isIllegal == TRUE) {
            printf("?Error bitwise operation in floating point format(%c).", cmd);
            return TRUE;
        }
        if (cmd == 'n') isSnglCond = TRUE;
        else isSnglCond = FALSE;
        if (isSnglCond == TRUE) prepDblCalc(FALSE);
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

exeCond() {
    BOOL err = FALSE;
    err = cmpProc(g_cmdCond);
    if (err) return -1;
    if (ptrMode == 0 || ptrMode == 3) {
        if (g_buf[0] == 1) return 1;
        else return 0;
    }
    else if (ptrMode == 1 || ptrMode == 4) {
        if (g_iBuf == 1) return 1;
        else return 0;
    }
    else if (ptrMode == 2) {
        if (g_dBuf != 0.0) return 1;
        else return 0;
    }
}

cmdProc() { /* Interpret commands */
    BOOL err = FALSE;
    int bracketCnt = 0, parenthesesCnt = 0, iRet = 0, i = 0, j = 0;
    char currCmd;

    for (i = 0; cmdBuf[i] != NULL; i++) {
        currCmd = cmdBuf[i];
        switch (currCmd) {
        case 'a':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            err = chkAddrRange(i, TRUE);
            if (err) return 2;
            else cmpProc(currCmd);
            break;
        case 'b':
            while (cmdBuf[i] != ']') i++;
            break;
        case 'c':
            iRet = condProc(i, 3);
            if (iRet != 3) {
                isCondPresent = FALSE;
                g_cmdCond = NULL;
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
            if (mem[addr] > CHAR_MIN || isUnsigned == FALSE) ++mem[addr];
            else if (mem[addr] > 0 || isUnsigned == TRUE) ++mem[addr];
            else printf("?Error lower limit of byte value. cmd at %d, line %u", i, uLineCnt);
            break;
        case 'e':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            err = chkAddrRange(i, TRUE);
            if (err) return 2;
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
            /* mem[addr] = getchar(); */
            mem[addr] = _getche();
            if (mem[addr] == 13) printf("\n");
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
            if (mem[addr] < CHAR_MAX || isUnsigned == FALSE) ++mem[addr];
            else if (mem[addr] < UCHAR_MAX || isUnsigned == TRUE) ++mem[addr];
            else printf("?Error upper limit of byte value, cmd at %d, line %u", i, uLineCnt);
            break;
        case 'j':
            if (!isExec) {
                printf("?This command can be used ONLY in file(s). cmd at %d", i);
                return 1;
            }
            else {
                rewind(pFile);
                uLineCnt = 0;
                iRet = condProc(i, 1);
                if (iRet != 1) {
                    if (iRet == -1) return 1;
                    condErr(1);
                    return 1;
                }
                else {
                    j = 1;
                    while (!feof(pFile)) {
                        fgets(cmdBuf, BUF_SIZE, pFile);
                        cmdBuf[szLen(cmdBuf) - 1] = NULL;
                        iRet = atoi(cmdBuf);
                        if (iRet == x) {
                            j = 0;
                            break;
                        }
                    }
                    if (j) {
                        printf("?Error line number %d not found, cmd at %d, line %u", x, i, uLineCnt);
                        return 1;
                    }
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
                /* check address range */
                err = chkAddrRange(i, TRUE);
                if (err) return 2;
                if (ptrMode == 0 || ptrMode == 3) mem[y] = mem[x];
                else if (ptrMode == 1 || ptrMode == 4) {
                    pi = (mem + x);
                    g_iBuf = *pi;
                    pi = (mem + y);
                    *pi = g_iBuf;
                    pAddrInit();
                }
                else if (ptrMode == 2) {
                    pd = (mem + x);
                    g_dBuf = *pd;
                    pd = (mem + y);
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
            err = chkAddrRange(i, TRUE);
            if (err) return 2;
            else cmpProc(currCmd);
            break;
        case 'o':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            err = chkAddrRange(i, TRUE);
            if (err) return 2;
            else cmpProc(currCmd);
            break;
        case 'p':
            printf("%c", mem[addr]);
            break;
        case 'q':
            if (!isFileLoaded) {
                printf("?Error Quine requires loading file. cmd at %d", i);
                return 4;
            }
            else {
                while (!feof(pFile)) {
                    fgets(fBuf, BUF_SIZE, pFile);
                    printf(fBuf);
                }
                rewind(pFile);
                uLineCnt = 0;
            }
            break;
        case 'r':
            x = 0;
            y = 0;
            g_cmdCond = 0;
            isCondPresent = FALSE;
            break;
        case 's':
            if (!isFileLoaded) printf("?Error File not loaded");
            else return RUN_FILE;
            break;
        case 't':
            termination = TRUE;
            return 0;
            break;
        case 'u':
            if (!isFileLoaded) printf("?Error file not loaded");
            else {
                err = fclose(pFile);
                if (err) printf("?Error closing file");
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
            /* Check address range */
            err = chkAddrRange(i, FALSE);
            if (err) return 2;
            if (ptrMode == 2) getDblCond(i);
            writeDta();
            while (cmdBuf[i] < '0' || cmdBuf[i] > '9' || cmdBuf[i] == '-' || cmdBuf[i] == NULL) i++;
            break;
        case 'x':
            return -1;
        case 'y':
            if (!isDebugMode) {
                isDebugMode = TRUE;
                printf("Debug mode on");
            }
            else {
                isDebugMode = FALSE;
                printf("Debug mode off\n");
            }
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
            err = chkAddrRange(i, TRUE);
            if (err) return 2;
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
            err = chkAddrRange(i, TRUE);
            if (err) return 2;
            else {
                if (ptrMode == 0) {
                    pc = mem;
                    g_buf[0] = *(pc + x) % *(pc + y);
                }
                else if (ptrMode == 1) {
                    prepIntCalc(TRUE);
                    pi = g_buf + INT_SIZE;
                    g_iBuf = *pi % *(pi + 1);
                    pi = g_buf;
                    *pi = g_iBuf;
                    pAddrInit();
                }
                else if (ptrMode == 3) g_buf[0] = mem[x] % mem[y];
                else if (ptrMode == 4) {
                    prepIntCalc(TRUE);
                    pu = g_buf + INT_SIZE;
                    g_iBuf = *pu % *(pu + 1);
                    pu = g_buf;
                    *pu = g_iBuf;
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
                err = chkAddrRange(i, TRUE);
                if (err) return 2;
                err = cmpProc(currCmd);
                if (err) {
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
                err = chkAddrRange(i, TRUE);
                if (err) return 2;
                err = cmpProc(currCmd);
                if (err) {
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
            err = chkAddrRange(i, TRUE);
            if (err) return 2;
            else {
                if (ptrMode == 0) {
                    pc = g_buf;
                    *pc = (unsigned char)mem[x] * (unsigned char)mem[y];
                    pAddrInit();
                }
                else if (ptrMode == 1) {
                    prepIntCalc(TRUE);
                    pi = g_buf + INT_SIZE;
                    g_iBuf = *pi * *(pi + 1);
                    pi = g_buf;
                    *pi = g_iBuf;
                    pAddrInit();
                }
                else if (ptrMode == 2) {
                    prepDblCalc(TRUE);
                    pd = g_buf + DBL_SIZE;
                    g_dBuf = *pd * *(pd + 1);
                    pd = g_buf;
                    *pd = g_dBuf;
                    pAddrInit();
                }
                else if (ptrMode == 3) g_buf[0] = mem[x] * mem[y];
                else if (ptrMode == 4) {
                    prepIntCalc(TRUE);
                    pu = g_buf + INT_SIZE;
                    g_iBuf = *pu * *(pu + 1);
                    pu = g_buf;
                    *pu = g_iBuf;
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
            err = chkAddrRange(i, TRUE);
            if (err) return 2;
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            else {
                if (ptrMode == 0) {
                    pc = g_buf;
                    *pc = (unsigned char)mem[x] - (unsigned char)mem[y];
                    pAddrInit();
                }
                else if (ptrMode == 1) {
                    prepIntCalc(TRUE);
                    pi = g_buf + INT_SIZE;
                    g_iBuf = *pi - *(pi + 1);
                    pi = g_buf;
                    *pi = g_iBuf;
                    pAddrInit();
                }
                else if (ptrMode == 2) {
                    prepDblCalc(TRUE);
                    pd = g_buf + DBL_SIZE;
                    g_dBuf = *pd - *(pd + 1);
                    pd = g_buf;
                    *pd = g_dBuf;
                    pAddrInit();
                }
                else if (ptrMode == 3) g_buf[0] = mem[x] - mem[y];
                else if (ptrMode == 4) {
                    prepIntCalc(TRUE);
                    pu = g_buf + INT_SIZE;
                    g_iBuf = *pu - *(pu + 1);
                    pu = g_buf;
                    *pu = g_iBuf;
                    pAddrInit();
                }
            }
            break;
        case '_':
            err = chkAddrRange(i, FALSE);
            if (err) return 2;
            pd = (mem + addr);
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
            err = chkAddrRange(i, TRUE);
            if (err) return 2;
            iRet = condProc(i, 2);
            if (iRet != 2) {
                if (iRet == -1) return 1;
                condErr(2);
                return 1;
            }
            else {
                if (ptrMode == 0) {
                    pc = g_buf;
                    *pc = (unsigned char)mem[x] + (unsigned char)mem[y];
                    pAddrInit();
                }
                else if (ptrMode == 1) {
                    prepIntCalc(TRUE);
                    pi = g_buf + INT_SIZE;
                    g_iBuf = *pi + *(pi + 1);
                    pi = g_buf;
                    *pi = g_iBuf;
                    pAddrInit();
                }
                else if (ptrMode == 2) {
                    prepDblCalc(TRUE);
                    pd = g_buf + DBL_SIZE;
                    g_dBuf = *pd + *(pd + 1);
                    pd = g_buf;
                    *pd = g_dBuf;
                    pAddrInit();
                }
                else if (ptrMode == 3) g_buf[0] = mem[x] + mem[y];
                else if (ptrMode == 4) {
                    prepIntCalc(TRUE);
                    pu = g_buf + INT_SIZE;
                    g_iBuf = *pu + *(pu + 1);
                    pu = g_buf;
                    *pu = g_iBuf;
                    pAddrInit();
                }
            }
            break;
        case '=':
            err = chkAddrRange(i, FALSE);
            if (err) return 2;
            if (ptrMode == 0) {
                pc = mem;
                *(pc + addr) = (char)g_buf[0];
            }
            else if (ptrMode == 1 || ptrMode == 4) memCpy((mem + addr), INT_SIZE, g_buf);
            else if (ptrMode == 3) mem[addr] = g_buf[0];
            else if (ptrMode == 2) memCpy((mem + addr), DBL_SIZE, g_buf);
            break;
        case '\\':
            if (ptrMode == 3) printf("%u", mem[addr]);
            else {
                pAddrSet(USE_CURR_ADDR);
                if (ptrMode == 0) printf("%d", *pc);
                else if (ptrMode == 1) printf("%d", *pi);
                else if (ptrMode == 2) printf("%lld", roundftoi(pd));
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
            else {
                err = cmpProc(currCmd);
                if (err) {
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
            err = chkAddrRange(i, TRUE);
            if (err) return 2;
            else {
                if (ptrMode == 0) {
                    pAddrInit();
                    if (*(pc + x) > * (pc + y)) g_buf[0] = 1;
                    else if (*(pc + x) == *(pc + y)) g_buf[0] = 0;
                    else if (*(pc + x) < *(pc + y)) g_buf[0] = -1;
                }
                else if (ptrMode == 3) {
                    if (mem[x] > mem[y]) g_buf[0] = 1;
                    else if (mem[x] == mem[y]) g_buf[0] = 0;
                    else if (mem[x] < mem[y]) g_buf[0] = -1;
                }
                else if (ptrMode == 1) {
                    prepIntCalc(TRUE);
                    pi = g_buf;
                    if (*(pi + 1) > * (pi + 2)) g_buf[0] = 1;
                    else if (*(pi + 1) == *(pi + 2)) g_buf[0] = 0;
                    else if (*(pi + 1) < *(pi + 2)) g_buf[0] = -1;
                    pAddrInit();
                }
                else if (ptrMode == 4) {
                    prepIntCalc(TRUE);
                    pu = g_buf;
                    if (*(pu + 1) > * (pu + 2)) g_buf[0] = 1;
                    else if (*(pu + 1) == *(pu + 2)) g_buf[0] = 0;
                    else if (*(pu + 1) < *(pu + 2)) g_buf[0] = -1;
                    pAddrInit();
                }
                else if (ptrMode == 2) {
                    prepDblCalc(TRUE);
                    pd = g_buf;
                    if (*(pd + 1) > * (pd + 2)) g_buf[0] = 1;
                    else if (*(pd + 1) == *(pd + 2)) g_buf[0] = 0;
                    else if (*(pd + 1) < *(pd + 2)) g_buf[0] = -1;
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
        case ',':
            break;
        case '.':
            break;
        case '/':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                condErr(2);
                return 1;
            }
            err = chkAddrRange(i, TRUE);
            if (err) return 2;
            else {
                if (ptrMode == 0) {
                    pc = g_buf;
                    *pc = (unsigned char)mem[x] / (unsigned char)mem[y];
                    pAddrInit();
                }
                else if (ptrMode == 1) {
                    prepIntCalc(TRUE);
                    pi = g_buf + INT_SIZE;
                    g_iBuf = *pi / *(pi + 1);
                    pi = g_buf;
                    *pi = g_iBuf;
                    pAddrInit();
                }
                else if (ptrMode == 2) {
                    prepDblCalc(TRUE);
                    pd = g_buf + DBL_SIZE;
                    g_dBuf = *pd / *(pd + 1);
                    pd = g_buf;
                    *pd = g_dBuf;
                    pAddrInit();
                }
                else if (ptrMode == 3) {
                    g_buf[0] = mem[x] / mem[y];
                }
                else if (ptrMode == 4) {
                    prepIntCalc(TRUE);
                    pu = g_buf + INT_SIZE;
                    g_iBuf = *pu / *(pu + 1);
                    pu = g_buf;
                    *pu = g_iBuf;
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
                err = chkAddrRange(i, TRUE);
                if (err) return 2;
                err = cmpProc(currCmd);
                if (err) {
                    printf(" cmd at %d, line %u", i, uLineCnt);
                    return 1;
                }
            }
            break;
        case '[': /* REM: MUST CHECK COMPLICATED NESTED LOOP PRGS */
            bracketCnt = 0;
            if (mem[addr] == 0) {
                while (1) {
                    if (cmdBuf[i] == '[') bracketCnt++;
                    else if (cmdBuf[i] == ']' && bracketCnt) {
                        bracketCnt--;
                        if (!bracketCnt) break;
                    }
                    else if (cmdBuf[i] == ']' && !bracketCnt) break;
                    i++;
                }
            }
            break;
        case ']':
            bracketCnt = 0;
            if (mem[addr] != 0) {
                while (1) {
                    if (cmdBuf[i] == ']') bracketCnt--;
                    else if (cmdBuf[i] == '[' && bracketCnt) {
                        bracketCnt++;
                        if (!bracketCnt) break;
                    }
                    else if (cmdBuf[i] == '[' && !bracketCnt) break;
                    i--;
                }
            }
            break;
        case '(':
            err = chkAddrRange(i, TRUE);
            if (err) return 2;
            iRet = exeCond();
            if (iRet == 0) {
                while (1) {
                    if (cmdBuf[i] == '(') parenthesesCnt++;
                    else if (cmdBuf[i] == ')' && parenthesesCnt) parenthesesCnt--;
                    if (cmdBuf[i] == ')' && !parenthesesCnt) break;
                    i++;
                }
            }
            else if (iRet == 1) {
                break;
            }
            else if (iRet == -1) {
                printf(" cmd at %d, line %u", i, uLineCnt);
                return 1;
            }
            break;
        case ')':
            if (isDebugMode) printf("\nEnd of if statement at %d, line %u\n", i, uLineCnt);
            break;
        }
        if (isDebugMode) {
            printf("\nFile Loaded: %s, ptrMode: %d, ptr: %d, cmdPos: %d\n", isFileLoaded ? "TRUE" : "FALSE", ptrMode, addr, i);
            printf("Cond present: %s, x: %d, y: %d, g_cmdCond: %c, name buffer: %s\n", isCondPresent ? "TRUE" : "FALSE", x, y, g_cmdCond, nameBuf);
            printf("cmd: %c, dx: %.12g, byte: %#x\n", cmdBuf[i], dx, mem[addr]);
            printf("buf: 0x");
            int iTmp = 0;
            for (iTmp = 0; iTmp < 64; iTmp++) printf("%x", g_buf[iTmp]);
        }
    }
    return 0;
}

BOOL balanceChk() {
    int i = 0, parenthesesBalance = 0, bracketBalance = 0;

    while (cmdBuf[i] != NULL) {
        if (cmdBuf[i] == '(') parenthesesBalance++;
        else if (cmdBuf[i] == ')') parenthesesBalance--;
        else if (cmdBuf[i] == '[') bracketBalance++;
        else if (cmdBuf[i] == ']') bracketBalance--;
        i++;
    }
    if (parenthesesBalance == 0 && bracketBalance == 0) return FALSE;
    else return TRUE;
}

exeFile() {
    char rtnVal = 0;
    BOOL isUnbalanced = FALSE;
    uLineCnt = 1;
    isExec = TRUE;

    while (!feof(pFile)) {
        fgets(cmdBuf, BUF_SIZE, pFile);
        if (!feof(pFile) && cmdBuf[szLen(cmdBuf)] != NULL) cmdBuf[szLen(cmdBuf) - 1] = NULL;
        uLineCnt++;

        isUnbalanced = balanceChk();
        rtnVal = cmdProc();
        if (rtnVal) {
            if (rtnVal == 1) printf("\nCommand execution halted(Syntax Error)");
            else if (rtnVal == 2) printf("\nCommand execution halted(Memory/Value Range Error)");
            else if (rtnVal == 3) printf("\nCommand execution halted(Internal/External Error)");
            else if (rtnVal == 4) printf("\nCommand execution halted(Miscellaneous Error)");
            else if (rtnVal == -2) {
                printf("\nProgram handled error(%d)", x);
                initGlobalVar();
                uLineCnt = 0;
                rewind(pFile);
                isExec = FALSE;
                return -1;
            }
            uLineCnt = 0;
            rewind(pFile);
            isExec = FALSE;
            return 1;
        }
        if (isUnbalanced == TRUE) {
            printf("?Error parentheses and/or brackets are not balanced");
            uLineCnt = 0;
            isExec = FALSE;
            return 1;
        }
        if (termination) {
            termination = FALSE;
            rewind(pFile);
            uLineCnt = 0;
            isExec = FALSE;
            return 0;
        }
    }
    rewind(pFile);
    uLineCnt = 0;
    return 0;
}

main() {
    char rtnVal = 0;
    BOOL isUnbalanced = FALSE;
    clrscr();

    initGlobalVar();
    puts("Ascii BrainFuck Language Interpreter Prompt");
    printf("int size: %d Bytes, double size: %d Bytes\n", INT_SIZE, DBL_SIZE);
    printf("int range: %d to %d, double range: (+-) %.12g to %.12g\n", INT_MIN, INT_MAX, DBL_MIN, DBL_MAX);
    printf("Memory size: %d Bytes\nMaximum command length per line: %d Bytes with NULL\n", MEM_SIZE, BUF_SIZE);
    puts("***Type ? for commands***");

    while (1) {
        printf("\nREADY(%d,%04d)>> ", ptrMode, addr);
        fgets(cmdBuf, BUF_SIZE, stdin);
        if (szLen(cmdBuf) < BUF_SIZE) cmdBuf[szLen(cmdBuf) - 1] = NULL;
        isUnbalanced = balanceChk();
        if (isUnbalanced == TRUE) printf("?Error parentheses and/or brackets are not balanced");
        else {
            rtnVal = cmdProc();
            if (rtnVal == 1) printf("\nCommand execution halted(Syntax Error)");
            else if (rtnVal == 2) printf("\nCommand execution halted(Memory/Value Range Error)");
            else if (rtnVal == 3) printf("\nCommand execution halted(Internal/External Error)");
            else if (rtnVal == 4) printf("\nCommand execution halted(Miscellaneous Error)");
            else if (rtnVal == RUN_FILE) {
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
    puts("a: AND                                []: while(*ptr != 0) loop bracket");
    puts("b: Break                              `: name marker");
    puts("c: define Condition                   !: NEQ");
    puts("d: Decrement byte                     @: set float pointer");
    puts("e: EQU                                #: set int pointer");
    puts("f: Forward(set ptr to X)              $: set char pointer");
    puts("g: Getchar                            %: modulus");
    puts("h: Handle Error                       ^: bitwise XOR");
    puts("i: Increment byte                     &: bitwise AND");
    puts("j: Jump                               *: multiply");
    puts("k: Knockdown(initialize interpreter)  (): if parentheses");
    puts("l: Load file                          -: subtract");
    puts("m: Match(copy value at X to Y)        _: print as double");
    puts("n: NOT                                =: save product to ptr");
    puts("o: OR                                 +: add");
    puts("p: Print as char                      .: decimal, for double");
    puts("q: Quine                              ,: separator");
    puts("r: Remove condition(s)                <: decrement pointer");
    puts("s: Start file execution               >: increment pointer");
    puts("t: Terminate file execution           /: divide(*(p+X) / *(p+Y))");
    puts("u: Unload file                        ?: help(command list)");
    puts("v: jump to next line                  ;: REM(comment)");
    puts("w: Write X to byte                    :: compare");
    puts("x: close interpreter                  ': set signed type");
    puts("y: whY(toggle debugging mode)         \": set unsigned type");
    puts("z: Zero all bytes                     \\: print as int");
    puts("|: bitwise OR                         {: condition *(p+X) < *(p+Y)");
    puts("~: bitwise NOT                        }: condition *(p+X) > *(p+Y)");
    puts("For syntax & command usage, please refer to manual or Github page.");
    puts("Github page: https://github.com/DS1TPT/ABF_Lang");
    printf("Note: This program is licensed under Apache License Version 2.0");
}

void memFill(char* dst, char dta, int size) {
    char* ptr = dst;
    int i = 0;
    for (i = 0; i < size; i++) {
        *ptr = dta;
        ptr++;
    }
}

searchCh(char* sz, char* cp) {
    while (*sz != NULL) {
        if (*sz == *cp) return TRUE;
        sz++;
    }
    return FALSE;
}

unsigned szLen(char* sz) {
    unsigned i = 0;
    while (sz[i] != NULL) i++;
    return i;
}

void szCpy(char* dst, int size, char* src) {
    int i = 0;
    for (i = 0; i < size - 1; i++) *dst++ = *src++;
    *dst = NULL;
}

void memCpy(char* dst, int size, char* src) {
    int i = 0;
    for (i = 0; i < size; i++) *dst++ = *src++;
}

unsigned atou(char* sz) {
    unsigned i = 0, n = 0;
    while (*sz >= '0' && *sz <= '9') n = n * 10 + *sz++ - '0';
    return n;
}

long long roundftoi(double* pDb) {
    return ((*pDb > 0.0) ? (int)(*pDb + 0.5) : (int)(*pDb - 0.5));
}

void clrscr() {
    system("@cls||clear");
}