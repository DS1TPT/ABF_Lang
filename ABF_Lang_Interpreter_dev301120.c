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

/* DEV Version, 30-11-20. I still have long way to go */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h> /* atof, comment this if using Power C on c64 or c128 */
#define STDLIB_INCLUDED /* if you commented "#include <stdlib.h>", comment this also */
/* #define C64_128_POWERC_ENV */

#define FALSE 0
#define TRUE 1

/* SIZE AND VALUE LIMIT */
/* C64 part needs to be separated, but didn't erase them for later */
#ifdef C64_128_POWERC_ENV
#define CHAR_MIN 0
#define CHAR_MAX 255
#define INT_SIZE 2
#define DBL_SIZE 5
#define INT_MIN (-0x7fff - 1)
#define INT_MAX 0x7fff
#define UINT_MAX 0xffff
#define MEM_SIZE 2048
#define BUF_SIZE 256
#define NAME_SIZE 64
#define BACKSPACE 20
typedef char BOOL;
typedef char byte;
typedef unsigned uint;
#else /* assumes 32b or 64b systems */
#define INT_SIZE 4
#define DBL_SIZE 8
#define INT_MIN (-0x7fffffff - 1)
#define INT_MAX 0x7fffffff
#define CHAR_MIN -128
#define CHAR_MAX 127
#define UINT_MAX 0xffffffff
#define DBL_MIN 2.2250738585072014e-308
#define DBL_MAX 1.7976931348623158e+308
#define MEM_SIZE 4096
#define BUF_SIZE 1024
#define NAME_SIZE 512
#define BACKSPACE 8
typedef _Bool BOOL; /* C99 required */
typedef unsigned char byte;
typedef unsigned int uint;
#endif

/* BUFFERS */
char cmdBuf[BUF_SIZE], loopBuf[BUF_SIZE], nameBuf[NAME_SIZE];
char g_cmdCond = 0;
byte g_buf[64];
int g_iBuf = 0;
double g_dBuf = 0.0;

/* POINTERS & ARRAYS */
byte mem[MEM_SIZE];
char* pc = mem;
double* pd = mem;
int* pi = mem;
uint* pu = mem;
FILE* pFile = NULL;

/* STATUS MARKERS */
char condCmd = 0, ptrMode = 0; /* ptrMode 0: char, 1: int, 2: double, 3: uchar(byte), 4: uint */
BOOL isUnsigned = FALSE, isFileLoaded = FALSE, isCondPresent = FALSE, termination = FALSE, isDebugMode = FALSE;

/* MISCELLANEOUS */
int x = 0, y = 0;
double dx = 0.0;
int addr = 0;

void help(); /* too long to place over main so placed under the main fn */

void memFill(dst, dta, size)  /* fill mem with dta */
    char* dst;
    char dta;
    int size;
{
    char* ptr = dst;
    int i = 0;
    for (i = 0; i < size; i++) {
        *ptr = dta;
        ptr++;
    }
}

void initGlobalVar() {
    int err;
    memFill(mem, 0, MEM_SIZE);
    memFill(cmdBuf, 0, BUF_SIZE);
    memFill(loopBuf, 0, BUF_SIZE);
    memFill(nameBuf, 0, NAME_SIZE);
    memFill(g_buf, 0, 64);
    g_cmdCond = 0;
    g_iBuf = 0;
    g_dBuf = 0.0;
    x = 0, y = 0;
    dx = 0.0;
    addr = 0;
    condCmd = 0, ptrMode = 0;
    isUnsigned = FALSE, isCondPresent = FALSE, termination = FALSE, isDebugMode = FALSE;
    pc = mem;
    pi = mem;
    pd = mem;
    pu = mem;
}

searchCh(sz, cp) /* search if there's cp in sz */
    char* sz;
    char* cp;
{
    while (*sz != NULL) {
        if (*sz == *cp) return TRUE;
        sz++;
    }
    return FALSE;
}

szLen(sz)
    char* sz;
{
    unsigned int i = 0;
    while (sz[i] != NULL) i++;
    return i;
}

void szCpy(dst, size, src) /* copy src's dta to dst, writes null */
    char* dst;
    int size;
    char* src;
{
    int i = 0;

    for (i = 0; i < size - 1; i++) *dst++ = *src++;
    *dst = NULL;
}

memCpy(dst, size, src) /* copy src's dta to dst, does not write null */
    char* dst;
    int size;
    char* src;
{
    int i = 0;

    for (i = 0; i < size; i++) *dst++ = *src++;
}

nameProc(cmdPos) /* get name and store it to nameBuf */
    int cmdPos;
{
    char* cp = cmdBuf + cmdPos;
    char* strBuf[BUF_SIZE];
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

char* findNextCond(pSz) /* find next condition using ','. returns pos of ',' if it's present */
    char* pSz;
{
    while (*pSz != NULL) {
        if (*pSz >= '0' && *pSz <= '9') pSz++;
        else if (*pSz == ',') return ++pSz; /* found separator */
        else {
            if (*pSz == '.') printf("?Detected Decimal('.') in integer conditions.");
            return pSz; /* returns pSz if *pSz is not between '0' to '9' and neq to ',' */
        }
    }
    return pSz;
}

void getDblCond(cmdPos) /* set double type condition, returns FALSE(err) or TRUE(norm)*/
    int cmdPos;
{
    char* cp = cmdBuf + cmdPos + 1;
    char strBuf[64];
    memFill(strBuf, 0, 64);
    int i = 0;
    
    szCpy(strBuf, 64, cp);
    dx = atof(strBuf);
    printf("\n%.12g\n", dx);
    memCpy(g_buf, DBL_SIZE, &dx);
    /* FOR DEBUGGING */
    for (i = 0; i < 8; i++) {
        printf("%x ", g_buf[i]);
    }
}

condProc(cmdPos, condNum) /* set int type conditions, returns condition count */
    int cmdPos;
    int condNum;
{
    char* cp = cmdBuf + cmdPos + 1;
    char* strBuf[34];
    memFill(strBuf, 0, 34);
    int condCnt = 0, i = 0;
    BOOL isCmpCmd = FALSE;
    
    szCpy(strBuf, 34, cp);
    for (i = 0; i < condNum; i++) {
        printf("DEBUG: %s\n", cp);
        if (cp == NULL) 
            return i;
        else if (i == 0) {
            x = atoi(cp);
            condCnt++;
        }
        else if (i == 1) {
            y = atoi(cp);
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
        if (i == 0 || i == 1) cp = findNextCond(cp);
    }
    return condCnt;
}

void condErr(condCnt) /* prints condition count error */
    int condCnt;
{ 
    printf("?Error condition count is not %d", condCnt); 
}

void prepIntCalc(isY) /* store int value to g_buf for calculation */
    BOOL isY;
{
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

void prepDblCalc(isY) /* store double value to g_buf for calculation */
    BOOL isY;
{
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

void pAddrSet() { /* set pointers except for char types to mem + addr */
    if (ptrMode == 1) pi = (mem + addr);
    else if (ptrMode == 2) pd = (mem + addr);
    else if (ptrMode == 4) pu = (mem + addr);
}

void pAddrInit() { /* reset pointers except for char types to mem */
    if (ptrMode == 1) pi = mem;
    else if (ptrMode == 2) pd = mem;
    else if (ptrMode == 4) pu = mem;
}

void writeDta() { /* write data to mem */
    if (ptrMode == 0 || ptrMode == 3) {
        mem[addr] = x;
        return;
    }
    else if (ptrMode == 1) {
        pi = g_buf;
        *pi = x;
        memCpy((mem + addr), INT_SIZE, g_buf);
        if (isDebugMode) {
            pAddrSet();
            printf("%d\n", *pi);
        }
    }
    else if (ptrMode == 2) {
        memCpy((mem + addr), DBL_SIZE, g_buf);
        if (isDebugMode) {
            pAddrSet();
            printf("%.12g\n", *pd);
        }
    }
    else if (ptrMode == 4) {
        pu = g_buf;
        *pu = x;
        memCpy((mem + addr), INT_SIZE, g_buf);
        if (isDebugMode) {
            pAddrSet();
            printf("%u\n", *pu);
        }
    }
    pAddrInit();
}

cmpProc(cmd) /* process compare commands except for ':' */
    char cmd;
{
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
    else if (ptrMode == 1) {
        isSnglCond = searchCh("n~", cmd);
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
        isIllegal = searchCh("|~^&", cmd);
        if (isIllegal == TRUE) {
            printf("?Error bitwise operation on floating-point(%c).", cmd);
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
            g_iBuf = !*(pi + 1);
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

cmdProc() { /* Interpret commands */
    BOOL err = FALSE;
    int bracketCtr = 0, condCnt = 0, iRet = 0, i = 0, j = 0;
    char currCmd;

    for (i = 0; cmdBuf[i] != NULL; i++) {
        currCmd = cmdBuf[i];
        switch (currCmd) {
        case 'a':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                condErr(2);
                return 1;
            }
            else cmpProc(currCmd);
            break;
        case 'b':
            while (cmdBuf[i] != ']') i++;
            break;
        case 'c':
            iRet = condProc(i, 3);
            if (iRet != 3) {
                condErr(3);
                isCondPresent = FALSE;
                g_cmdCond = NULL;
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
            if (mem[addr] > -128 || isUnsigned == FALSE) ++mem[addr];
            else if (mem[addr] > 0 || isUnsigned == TRUE) ++mem[addr];
            else printf("?Error lower limit of byte value. cmd at %d", i);
            break;
        case 'e':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                condErr(2);
                return 1;
            }
            else cmpProc(currCmd);
            break;
        case 'f':
            iRet = condProc(i, 1);
            if (iRet != 1) {
                condErr(1);
                return 1;
            }
            else {
                addr = x;
                x = 0;
            }
            break;
        case 'g':
            mem[addr] = getchar();

            break;
        case 'h':
            if (isFileLoaded == FALSE) {
                printf("?This command can be used ONLY in file(s). cmd at %d", i);
                return 1;
            }
            else {
                iRet = condProc(i, 1);
                if (iRet != 1) {
                    condErr(1);
                    return 1;
                }
                else {
                    g_iBuf = x;
                    return -2;
                }
            }
            break;
        case 'i':
            if (mem[addr] < 127 || isUnsigned == FALSE) ++mem[addr];
            else if (mem[addr] < 255 || isUnsigned == TRUE) ++mem[addr];
            else printf("?Error upper limit of byte value, cmd at %d", i);
            break;
        case 'j':
            if (isFileLoaded == FALSE) {
                printf("?This command can be used ONLY in file(s). cmd at %d", i);
                return 1;
            }
            else {
                /* rewind file and find corresponding number */
            }
            break;
        case 'k':
            initGlobalVar();
            break;
        case 'l':
            iRet = nameProc(i);
            if (iRet == 0) {
                printf("?Name marker error, cmd at %d", i);
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
            memFill(nameBuf, 0, NAME_SIZE);
            break;
        case 'm':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                condErr(2);
                return 1;
            }
            else {
                /* implement copy */
            }
            break;
        case 'n':
            iRet = condProc(i, 1);
            if (iRet != 1) {
                condErr(1);
                return 1;
            }
            else cmpProc(currCmd);
            break;
        case 'o':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                condErr(2);
                return 1;
            }
            else cmpProc(currCmd);
            break;
        case 'p':
            printf("%c", mem[addr]);
            break;
        case 'q':
            if (isFileLoaded == FALSE) {
                printf("?Error Quine requires loading file. cmd at %d", i);
                return 1;
            }
            else {
                /* implement quine */
            }
            break;
        case 'r':
            x = 0;
            y = 0;
            g_cmdCond = 0;
            isCondPresent = FALSE;
            break;
        case 's':
            if (isFileLoaded == FALSE) printf("?Error File not loaded");
            else {
                /* implement file execution */
            }
            break;
        case 't':
            termination = TRUE;
            break;
        case 'u':
            if (isFileLoaded == FALSE) printf("?Error file not loaded");
            else {
                err = fclose(pFile);
                if (err) printf("?Error closing file");
                else {
                    pFile = NULL;
                    isFileLoaded = FALSE;
                }
            }
            break;
        case 'v':
            return 0;
        case 'w':
            if (ptrMode != 2) {
                condCnt = condProc(i, 1);
                if (condCnt != 1) {
                    condErr(1);
                    return 1;
                }
                if ((x > CHAR_MAX || x < CHAR_MIN) && ptrMode == 0) {
                    printf("?Error value X is out of range. cmd at %d", i);
                    return 2;
                }
                else if ((x > UCHAR_MAX || x < 0) && ptrMode == 3) {
                    printf("?Error value X is out of range. cmd at %d", i);
                    return 2;
                }
                /* Does not check value range for int, uint, and double */
                writeDta();
            }
            else {
                getDblCond(i);
                writeDta();
            }
            while (cmdBuf[i] < '0' || cmdBuf[i] > '9' || cmdBuf[i] == '-') i++;
            break;
        case 'x':
            return -1;
        case 'y':
            if (isDebugMode == FALSE) {
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
                condErr(2);
                return 1;
            }
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
            iRet = condProc(i, 2);
            if (iRet != 2) {
                condErr(2);
                return 1;
            }
            else cmpProc(currCmd);
            break;
        case '^':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                condErr(2);
                return 1;
            }
            else {
                err = cmpProc(currCmd);
                if (err) {
                    printf(" cmd at %d", i);
                }
            }
            break;
        case '&':
            iRet = condProc(i, 2);
            if (iRet != 2) {
                condErr(2);
                return 1;
            }
            else {
                err = cmpProc(currCmd);
                if (err) {
                    printf(" cmd at %d", i);
                }
            }
            break;
        case '*':
            /* implement multiplication */
            break;
        case '-':
            condCnt = condProc(i, 2);
            if (condCnt != 2) {
                condErr(2);
                return 1;
            }
            if (x > (MEM_SIZE - 1) || x < 0 || y >(MEM_SIZE - 1) || y < 0) {
                printf("?Error address is out of range. cmd at %d", i);
                return 2;
            }
            /* check condition and implement calculation part */
            break;
        case '_':
            pAddrSet();
            printf("%.12g", *pd);
            pAddrInit();
            break;
        case '+':
            /* implement addition */
            break;
        case '=':
            if (ptrMode == 0 || ptrMode == 3) mem[addr] = g_buf[0];
            else if (ptrMode == 1 || ptrMode == 4) memCpy((mem + addr), INT_SIZE, g_buf);
            else if (ptrMode == 2) memCpy((mem + addr), DBL_SIZE, g_buf);
            break;
        case '\\':
            if (ptrMode == 0) printf("%d", mem[addr]);
            else if (ptrMode == 3) printf("%u", mem[addr]);
            else {
                pAddrSet();
                if (ptrMode == 1) printf("%d", *pi);
                else if (ptrMode == 2) printf("%d", *pd);
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
                    printf(" cmd at %d", i);
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
            else cmpProc(currCmd); /* erase cmpProc and implement ':' calculation */
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
                printf("?Error address lower limit. cmd at %d", i);
                return 2;
            }
            break;
        case '>':
            if (addr < 4096) ++addr;
            else {
                printf("?Error address upper limit. cmd at %d", i);
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
            else cmpProc(currCmd); /* erase cmdProc and implement division */
            break;
        case '?':
            help();
            break;
        case '`':
            printf("?Error name marker without command");
            return 1;
        case '~':
            iRet = condProc(i, 1);
            if (iRet != 1) {
                condErr(1);
                return 1;
            }
            else {
                err = cmpProc(currCmd);
                if (err) {
                    printf(" cmd at %d", i);
                }
            }
            break;
        case '[': /* REM: loop implementation is NOT finished! */
            if (mem[addr] == 0) while (cmdBuf[i] != ']') i++;
            break;
        case ']':
            if (mem[addr] != 0) while (cmdBuf[i] != '[') i--;
            break;
        case '(':
            /* implement if. if condition is false then goto corresponding ')' */
            break;
        case ')':

            break;
        }
        if (isDebugMode) {
            printf("\nFile Loaded: %s, ptrMode: %d, ptr: %d, cmdPos: %d\n", isFileLoaded ? "TRUE" : "FALSE", ptrMode, addr, i);
            printf("Cond present: %s, x: %d, y: %d, g_cmdCond: %c, name buffer: %s\n", isCondPresent ? "TRUE" : "FALSE", x, y, g_cmdCond, nameBuf);
            printf("cmd: %c, bracketCtr: %d, dx: %.12g, buf: %s, byte: %#x", cmdBuf[i], bracketCtr, dx, g_buf, mem[addr]);
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

main() {
    char rtnVal = 0;
    BOOL isUnbalanced = FALSE;

    initGlobalVar();
    puts("Alphabetical BrainFuck Language Interpreter Prompt");
    puts("***Type ? for help***");

    while (1) {
        printf("\nREADY>> ");
        fgets(cmdBuf, BUF_SIZE, stdin);
        cmdBuf[szLen(cmdBuf) - 1] = NULL; /* write NULL */
        szCpy(loopBuf, BUF_SIZE, cmdBuf);
        isUnbalanced = balanceChk();
        if (isUnbalanced == TRUE) printf("?Error parentheses and/or brackets are not balanced");
        else {
            rtnVal = cmdProc();
            if (rtnVal == 1) printf("\nCommand execution halted(Syntax Error)");
            else if (rtnVal == 2) printf("\nCommand execution halted(Memory/Value Range Error)");
            else if (rtnVal == 3) printf("\nCommand execution halted(Internal/External Error)");
            else if (rtnVal == 4) printf("\nCommand execution halted(Miscellaneous Error)");
            else if (rtnVal == 10) {
                rtnVal = fileExe();
                if (rtnVal == -2) {
                    printf("\nProgram handled error(%d)", g_iBuf);
                    initGlobalVar();
                }
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
    puts("The Alphabetical BrainFuck commands");
    puts("a: AND                                []: while(*ptr != 0) loop bracket");
    puts("b: Break                              `: name marker");
    puts("c: Condition define                   !: NEQ");
    puts("d: Decrement byte                     @: toggle 64b float pointer");
    puts("e: EQU                                #: toggle 32b int pointer");
    puts("f: Forward(set ptr to X)              $: toggle 8b char pointer");
    puts("g: Getchar                            %: remainder of division");
    puts("h: Handle Error                       ^: bitwise XOR");
    puts("i: Increment byte                     &: bitwise AND");
    puts("j: Jump                               *: multiply");
    puts("k: knockdown(initialize interpreter)  (): if parentheses");
    puts("l: Load file                          -: subtract");
    puts("m: Match(copy value at X to Y)        _: print as float(64b)");
    puts("n: NOT                                =: save product to ptr");
    puts("o: OR                                 +: add");
    puts("p: Print as char                      .: decimal(point), for float");
    puts("q: Quine                              ,: separator");
    puts("r: Remove condition                   <: decrement pointer");
    puts("s: Start file execution               >: increment pointer");
    puts("t: Terminate file execution           /: divide(X / Y)");
    puts("u: Unload file                        ?: help(command list)");
    puts("v: jump to next line                  ;: REM");
    puts("w: Write X to byte                    :: compare X and Y");
    puts("x: close interpreter                  ': toggle signed type");
    puts("y: WHY(toggle debugging mode)         \": toggle unsigned type");
    puts("z: Zero all bytes                     \\: print as int");
    puts("|: bitwise OR                         {: condition X < Y");
    puts("~: bitwise NOT                        }: condition X > Y");
    puts("For syntax & command usage, please refer to manual or Github page.");
    puts("Github page: https://github.com/DS1TPT/ABF_Lang");
    printf("Note: This program is licensed under Apache License Version 2.0");
    /*
    commodore 64 commands
    '_' == (up arrow)
    '{' == (shift + '+')
    '}' == (shift + '-')
    '\' == (GBP sign)
    '`' == (left arrow)
    */
}
