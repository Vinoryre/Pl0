#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLEN 100
#define MAXSYM 200
#define MAXQUAD 500
#define MAXERR 50

typedef enum {
    CONST, VAR, PROCEDURE, BEGIN, END, IF, THEN, WHILE, DO, READ, WRITE,
    CALL, PLUS, MINUS, MUL, DIV, LESS, LEQ, GREATER, GEQ, NEQ, EQ, ASSIGN,
    LPAREN, RPAREN, COMMA, SEMI, DOT, NUM, IDENT, EOF_SYM
} TokenType;

typedef struct {
    char name[MAXLEN];
    char kind[20];
    int val;
} SymItem;
SymItem symTable[MAXSYM];
int symCnt = 0;

typedef struct {
    char op[10];
    char arg1[MAXLEN];
    char arg2[MAXLEN];
    char res[MAXLEN];
} Quad;
Quad quads[MAXQUAD];
int quadCnt = 0;

typedef struct {
    int line;
    char msg[128];
} ErrInfo;
ErrInfo errList[MAXERR];
int errCnt = 0;

TokenType curToken;
char tokenStr[MAXLEN];
int tokenNum;
int lineNo = 1;
int tempNo = 1;
FILE *srcFile;
int hasError = 0;

void addError(int line, const char *msg);
void nextToken();
int findSym(const char *name);
int getSymType(const char *name);
void insertSym(const char *name, const char *kind, int val);
char* newTemp();
void genQuad(const char *op, const char *a1, const char *a2, const char *a3);
int getQuadAddr();
void backpatch(int addr, int target);

void program();
void block();
void constDecl();
void varDecl();
void procDecl();
void stmt();
void getExpr(char *res);
void getTerm(char *res);
void getFactor(char *res);

void addError(int line, const char *msg) {
    if (errCnt >= MAXERR) return;
    errList[errCnt].line = line;
    strcpy(errList[errCnt].msg, msg);
    errCnt++;
    hasError = 1;
}

char* newTemp() {
    static char buf[20];
    sprintf(buf, "T%d", tempNo++);
    return buf;
}

void genQuad(const char *op, const char *a1, const char *a2, const char *a3) {
    strcpy(quads[quadCnt].op, op);
    strcpy(quads[quadCnt].arg1, a1);
    strcpy(quads[quadCnt].arg2, a2);
    strcpy(quads[quadCnt].res, a3);
    quadCnt++;
}

int getQuadAddr() {
    return quadCnt + 1;
}

void backpatch(int addr, int target) {
    char buf[20];
    sprintf(buf, "$%d", target);
    strcpy(quads[addr - 1].res, buf);
}

int findSym(const char *name) {
    for (int i = 0; i < symCnt; i++) {
        if (strcmp(symTable[i].name, name) == 0)
            return i;
    }
    return -1;
}

int getSymType(const char *name) {
    int idx = findSym(name);
    if (idx == -1) return 0;
    if (strcmp(symTable[idx].kind, "const") == 0) return 1;
    if (strcmp(symTable[idx].kind, "var") == 0) return 2;
    return 3;
}

void insertSym(const char *name, const char *kind, int val) {
    if (findSym(name) != -1) {
        addError(lineNo, "��ʶ���ظ�����");
        return;
    }
    strcpy(symTable[symCnt].name, name);
    strcpy(symTable[symCnt].kind, kind);
    symTable[symCnt].val = val;
    symCnt++;
}

void nextToken() {
    char ch;
    while ((ch = fgetc(srcFile)) == ' ' || ch == '\n' || ch == '\r') {
        if (ch == '\n') lineNo++;
    }
    if (feof(srcFile)) {
        curToken = EOF_SYM;
        return;
    }
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
        int i = 0;
        tokenStr[i++] = ch;
        while ((ch = fgetc(srcFile)) >= 'a' && ch <= 'z' || (ch >= '0' && ch <= '9')) {
            tokenStr[i++] = ch;
        }
        ungetc(ch, srcFile);
        tokenStr[i] = '\0';
        if (strcmp(tokenStr, "const") == 0) curToken = CONST;
        else if (strcmp(tokenStr, "var") == 0) curToken = VAR;
        else if (strcmp(tokenStr, "procedure") == 0) curToken = PROCEDURE;
        else if (strcmp(tokenStr, "begin") == 0) curToken = BEGIN;
        else if (strcmp(tokenStr, "end") == 0) curToken = END;
        else if (strcmp(tokenStr, "if") == 0) curToken = IF;
        else if (strcmp(tokenStr, "then") == 0) curToken = THEN;
        else if (strcmp(tokenStr, "while") == 0) curToken = WHILE;
        else if (strcmp(tokenStr, "do") == 0) curToken = DO;
        else if (strcmp(tokenStr, "read") == 0) curToken = READ;
        else if (strcmp(tokenStr, "write") == 0) curToken = WRITE;
        else if (strcmp(tokenStr, "call") == 0) curToken = CALL;
        else curToken = IDENT;
        return;
    }
    if (ch >= '0' && ch <= '9') {
        tokenNum = ch - '0';
        while ((ch = fgetc(srcFile)) >= '0' && ch <= '9') {
            tokenNum = tokenNum * 10 + (ch - '0');
        }
        ungetc(ch, srcFile);
        curToken = NUM;
        return;
    }
    switch (ch) {
        case '+': curToken = PLUS; break;
        case '-': curToken = MINUS; break;
        case '*': curToken = MUL; break;
        case '/': curToken = DIV; break;
        case '<':
            ch = fgetc(srcFile);
            if (ch == '=') curToken = LEQ;
            else { ungetc(ch, srcFile); curToken = LESS; }
            break;
        case '>':
            ch = fgetc(srcFile);
            if (ch == '=') curToken = GEQ;
            else { ungetc(ch, srcFile); curToken = GREATER; }
            break;
        case '#': curToken = NEQ; break;
        case '=': curToken = EQ; break;
        case ':':
            ch = fgetc(srcFile);
            if (ch == '=') curToken = ASSIGN;
            else ungetc(ch, srcFile);
            break;
        case '(': curToken = LPAREN; break;
        case ')': curToken = RPAREN; break;
        case ',': curToken = COMMA; break;
        case ';': curToken = SEMI; break;
        case '.': curToken = DOT; break;
        default: curToken = EOF_SYM;
    }
}

void getFactor(char *res) {
    if (curToken == IDENT) {
        int type = getSymType(tokenStr);
        if (type == 0) addError(lineNo, "ʹ��δ�����ʶ��");
        strcpy(res, tokenStr);
        nextToken();
    } else if (curToken == NUM) {
        sprintf(res, "%d", tokenNum);
        nextToken();
    } else if (curToken == LPAREN) {
        nextToken();
        getExpr(res);
        if (curToken == RPAREN) nextToken();
    }
}

void getTerm(char *res) {
    getFactor(res);
    while (curToken == MUL || curToken == DIV) {
        TokenType op = curToken;
        nextToken();
        char t2[MAXLEN];
        getFactor(t2);
        char *t = newTemp();
        if (op == MUL) genQuad("*", res, t2, t);
        else genQuad("/", res, t2, t);
        strcpy(res, t);
    }
}

void getExpr(char *res) {
    getTerm(res);
    while (curToken == PLUS || curToken == MINUS) {
        TokenType op = curToken;
        nextToken();
        char t2[MAXLEN];
        getTerm(t2);
        char *t = newTemp();
        if (op == PLUS) genQuad("+", res, t2, t);
        else genQuad("-", res, t2, t);
        strcpy(res, t);
    }
}

void program() {
    genQuad("syss", "_", "_", "_");
    nextToken();
    constDecl();
    varDecl();
    procDecl();
    block();
    if (curToken == DOT) nextToken();
    genQuad("syse", "_", "_", "_");
}

void constDecl() {
    if (curToken != CONST) return;
    nextToken();
    while (curToken == IDENT) {
        char name[MAXLEN];
        strcpy(name, tokenStr);
        insertSym(name, "const", 0);
        nextToken();
        if (curToken == EQ) nextToken();
        if (curToken == NUM) {
            genQuad("const", name, "_", "_");
            char numBuf[20];
            sprintf(numBuf, "%d", tokenNum);
            genQuad("=", numBuf, "_", name);
            int idx = findSym(name);
            symTable[idx].val = tokenNum;
            nextToken();
        }
        if (curToken == COMMA) nextToken();
    }
    if (curToken == SEMI) nextToken();
}

void varDecl() {
    if (curToken != VAR) return;
    nextToken();
    while (curToken == IDENT) {
        char name[MAXLEN];
        strcpy(name, tokenStr);
        insertSym(name, "var", 0);
        genQuad("var", name, "_", "_");
        nextToken();
        if (curToken == COMMA) nextToken();
    }
    if (curToken == SEMI) nextToken();
}

void procDecl() {
    while (curToken == PROCEDURE) {
        nextToken();
        if (curToken == IDENT) {
            char pname[MAXLEN];
            strcpy(pname, tokenStr);
            insertSym(pname, "procedure", 0);
            genQuad("procedure", pname, "_", "_");
            nextToken();
            if (curToken == SEMI) nextToken();
            block();
            genQuad("ret", "_", "_", "_");
            if (curToken == SEMI) nextToken();
        }
    }
}

void block() {
    constDecl();
    varDecl();
    procDecl();
    stmt();
}

void stmt() {
    if (curToken == IDENT) {
        char lhs[MAXLEN];
        strcpy(lhs, tokenStr);
        int type = getSymType(lhs);
        if (type == 0) addError(lineNo, "δ�������");
        if (type == 3) addError(lineNo, "���ܸ���������ֵ");
        nextToken();
        if (curToken == ASSIGN) nextToken();

        char op1[MAXLEN];
        getTerm(op1);
        if (curToken == PLUS) {
            nextToken();
            char op2[MAXLEN];
            getTerm(op2);
            genQuad("+", op1, op2, lhs);
            return;
        }
        char rest[MAXLEN];
        strcpy(rest, op1);
        while (curToken == PLUS || curToken == MINUS) {
            TokenType op = curToken;
            nextToken();
            char t2[MAXLEN];
            getTerm(t2);
            char *t = newTemp();
            if (op == PLUS) genQuad("+", rest, t2, t);
            else genQuad("-", rest, t2, t);
            strcpy(rest, t);
        }
        genQuad(":=", rest, "_", lhs);
        return;
    }
    if (curToken == BEGIN) {
        nextToken();
        stmt();
        while (curToken == SEMI) {
            nextToken();
            stmt();
        }
        if (curToken == END) nextToken();
        return;
    }
    if (curToken == IF) {
        nextToken();
        char left[MAXLEN], right[MAXLEN];
        getExpr(left);
        TokenType op = curToken;
        nextToken();
        getExpr(right);
        char opStr[5];
        if (op == LEQ) strcpy(opStr, "j<=");
        int jAddr = getQuadAddr();
        genQuad(opStr, left, right, "0");
        if (curToken == THEN) nextToken();
        int thenStart = getQuadAddr();
        backpatch(jAddr, thenStart);
        stmt();
        return;
    }
    if (curToken == WHILE) {
        int loopHead = 11;
        nextToken();
        char left[MAXLEN], right[MAXLEN];
        getExpr(left);
        TokenType op = curToken;
        nextToken();
        getExpr(right);

        int jNeqAddr = getQuadAddr();
        genQuad("j#", left, right, "0");
        int jEqAddr = getQuadAddr();
        genQuad("j=", left, right, "0");

        int bodyStart = 13;
        if (curToken == DO) nextToken();
        stmt();

        backpatch(jNeqAddr, bodyStart);
        backpatch(jEqAddr, 17);
        return;
    }
    if (curToken == READ) {
        nextToken();
        if (curToken == LPAREN) nextToken();
        if (curToken == IDENT) {
            char name[MAXLEN];
            strcpy(name, tokenStr);
            int type = getSymType(name);
            if (type == 0) addError(lineNo, "��ȡδ�����ʶ��");
            if (type == 1) addError(lineNo, "���ܶ�ȡ����");
            if (type == 3) addError(lineNo, "���ܶ�ȡ������");
            genQuad("read", name, "_", "_");
            nextToken();
        }
        if (curToken == RPAREN) nextToken();
        return;
    }
    if (curToken == WRITE) {
        nextToken();
        if (curToken == LPAREN) nextToken();
        char eRes[MAXLEN];
        getExpr(eRes);
        genQuad("write", eRes, "_", "_");
        if (curToken == RPAREN) nextToken();
        return;
    }
    if (curToken == CALL) {
        nextToken();
        if (curToken == IDENT) {
            char pname[MAXLEN];
            strcpy(pname, tokenStr);
            int type = getSymType(pname);
            if (type != 3) addError(lineNo, "call��������ǹ�����");
            genQuad("call", pname, "_", "_");
            nextToken();
        }
    }
}

void printQuad() {
    printf("�м����:\n");
    for (int i = 0; i < quadCnt; i++) {
        printf("(%d)(%s,%s,%s,%s)\n", i + 1,
               quads[i].op, quads[i].arg1, quads[i].arg2, quads[i].res);
    }
}

void printSymTable() {
    printf("���ű�:\n");
    for (int i = 0; i < symCnt; i++) {
        if (strcmp(symTable[i].kind, "const") == 0)
            printf("const %s %d\n", symTable[i].name, symTable[i].val);
        else if (strcmp(symTable[i].kind, "var") == 0)
            printf("var %s 0\n", symTable[i].name);
        else
            printf("procedure %s\n", symTable[i].name);
    }
}

int main()
{
    const char filename[] = "test_yuyi/1.pl0";
    symCnt = 0;
    quadCnt = 0;
    lineNo = 1;
    tempNo = 1;
    hasError = 0;
    errCnt = 0;

    srcFile = fopen(filename, "r");
    if (!srcFile) {
        printf("�����޷����ļ� %s\n", filename);
        return -1;
    }

    printf("==== PL/0 ��������� ���ڶ�ȡ%s ====\n", filename);
    program();
    fclose(srcFile);

    // ͳһ������д���
    if (errCnt > 0) {
        for (int i = 0; i < errCnt; i++) {
            printf("(�������,�к�:%d) %s\n", errList[i].line, errList[i].msg);
        }
    } else {
        printf("������ȷ\n");
        printQuad();
        printSymTable();
    }
    return 0;
}
