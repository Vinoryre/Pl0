#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer {
public:
    explicit Lexer(const std::string& filename);

    // 词法分析入口
    std::vector<Token> tokenize();

    // Token转字符串（输出用）
    std::string tokenToString(const Token& t) const;

    // 输出统计信息（加分项）
    void printReport() const;

private:
    // 源程序
    std::string src;

    // 当前扫描位置
    size_t pos;

    // 当前行号
    int line;

    // Token序列
    std::vector<Token> tokens;

    // Token统计
    std::unordered_map<TokenType, int> counter;

    // 关键字表
    static const std::unordered_map<std::string, TokenType> keywords;

private:
    // 查看当前字符
    char peek();

    // 读取当前字符
    char get();

    // 查看下一个字符
    char peekNext() const;

    // 跳过空白符
    void skipWhitespace();

    // 跳过注释
    void skipComment();

    // 获取下一个Token
    Token nextToken();

    // 读取标识符/关键字
    Token readIdentifier();

    // 读取无符号整数
    Token readNumber();
};

#endif