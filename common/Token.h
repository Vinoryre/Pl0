#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {

    // ================= 关键字 =================
    KW_CONST,
    KW_VAR,
    KW_PROCEDURE,

    KW_BEGIN,
    KW_END,

    KW_IF,
    KW_THEN,

    KW_WHILE,
    KW_DO,

    KW_CALL,

    KW_READ,
    KW_WRITE,

    KW_ODD,      // 新增


    // ================= 标识符与常数 =================
    IDENTIFIER,
    NUMBER,


    // ================= 算术运算符 =================
    PLUS,
    MINUS,
    MUL,
    DIV,


    // ================= 赋值与关系运算符 =================
    ASSIGN,      // :=

    EQ,          // =
    NEQ,         // #

    LT,          // <
    LE,          // <=

    GT,          // >
    GE,          // >=


    // ================= 界符 =================
    SEMICOLON,   // ;
    COMMA,       // ,

    LPAREN,      // (
    RPAREN,      // )

    PERIOD,      // .


    // ================= 词法错误 =================
    LEX_ERROR_CHAR,      // 非法字符
    LEX_ERROR_ID_LEN,    // 标识符超长
    LEX_ERROR_NUM_OVER,  // 数字越界


    // ================= 文件结束 =================
    TK_EOF
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
};

#endif