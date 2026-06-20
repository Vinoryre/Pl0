#include "Lexer.h"
#include <fstream>
#include <iostream>
#include <cctype>
#include <iterator>
#include <stdexcept>
#include <unordered_map>

// ================= Constructor =================
Lexer::Lexer(const std::string& filename)
    : pos(0), line(1)
{
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        throw std::runtime_error("cannot open file: " + filename);
    }

    src.assign(std::istreambuf_iterator<char>(in),
               std::istreambuf_iterator<char>());

    counter.clear();
}

// ================= Public =================
std::vector<Token> Lexer::tokenize() {
    tokens.clear();

    while (true) {
        Token t = nextToken();
        tokens.push_back(t);
        counter[t.type]++;

        if (t.type == TokenType::TK_EOF) break;
    }

    return tokens;
}

// ================= 输出 =================
std::string Lexer::tokenToString(const Token& t) const {

    switch (t.type) {

        case TokenType::KW_CONST: return "<保留字,const>";
        case TokenType::KW_VAR: return "<保留字,var>";
        case TokenType::KW_PROCEDURE: return "<保留字,procedure>";
        case TokenType::KW_BEGIN: return "<保留字,begin>";
        case TokenType::KW_END: return "<保留字,end>";
        case TokenType::KW_IF: return "<保留字,if>";
        case TokenType::KW_THEN: return "<保留字,then>";
        case TokenType::KW_WHILE: return "<保留字,while>";
        case TokenType::KW_DO: return "<保留字,do>";
        case TokenType::KW_CALL: return "<保留字,call>";
        case TokenType::KW_READ: return "<保留字,read>";
        case TokenType::KW_WRITE: return "<保留字,write>";
        case TokenType::KW_ODD: return "<保留字,odd>";

        case TokenType::IDENTIFIER:
            return "<标识符," + t.lexeme + ">";

        case TokenType::NUMBER:
            return "<无符号整数," + t.lexeme + ">";

        case TokenType::PLUS: return "<运算符,+>";
        case TokenType::MINUS: return "<运算符,->";
        case TokenType::MUL: return "<运算符,*>";

        case TokenType::DIV: return "<运算符,/>";

        case TokenType::ASSIGN: return "<运算符,:=>";
        case TokenType::EQ: return "<运算符,=>";
        case TokenType::NEQ: return "<运算符,#>";
        case TokenType::LT: return "<运算符,<>";
        case TokenType::LE: return "<运算符,<=>";
        case TokenType::GT: return "<运算符,>>";
        case TokenType::GE: return "<运算符,>=>";

        case TokenType::SEMICOLON: return "<界符,;>";
        case TokenType::COMMA: return "<界符,,>";
        case TokenType::LPAREN: return "<界符,(>";
        case TokenType::RPAREN: return "<界符,)>";
        case TokenType::PERIOD: return "<界符,.>";

        // ================= 新增错误分类 =================
        case TokenType::LEX_ERROR_CHAR:
            return "<非法字符(串)," + t.lexeme + ">";

        case TokenType::LEX_ERROR_ID_LEN:
            return "<标识符长度超长," + t.lexeme + ">";

        case TokenType::LEX_ERROR_NUM_OVER:
            return "<无符号整数越界," + t.lexeme + ">";

        case TokenType::TK_EOF:
            return "<EOF>";

        default:
            return "<未知," + t.lexeme + ">";
    }
}

// ================= Character Ops =================
char Lexer::peek() {
    return pos >= src.size() ? '\0' : src[pos];
}

char Lexer::get() {
    if (pos >= src.size()) return '\0';
    char c = src[pos++];
    if (c == '\n') line++;
    return c;
}

char Lexer::peekNext() const {
    return (pos + 1 >= src.size()) ? '\0' : src[pos + 1];
}

// ================= Skip =================
void Lexer::skipWhitespace() {
    while (std::isspace((unsigned char)peek())) {
        get();
    }
}

void Lexer::skipComment() {

    if (peek() != '/') return;

    // //
    if (peekNext() == '/') {
        get(); get();
        while (peek() != '\0' && peek() != '\n') get();
        return;
    }

    // /* */
    if (peekNext() == '*') {
        get(); get();
        while (peek() != '\0') {
            if (peek() == '*' && peekNext() == '/') {
                get(); get();
                return;
            }
            get();
        }
    }
}

// ================= Main Lexer =================
Token Lexer::nextToken() {

    while (true) {
        skipWhitespace();

        if (peek() == '/' && (peekNext() == '/' || peekNext() == '*')) {
            skipComment();
            continue;
        }
        break;
    }

    int l = line;
    char c = peek();

    if (c == '\0') {
        return {TokenType::TK_EOF, "", l};
    }

    // ================= Identifier =================
    if (std::isalpha((unsigned char)c)) {
        std::string s;

        while (std::isalnum((unsigned char)peek())) {
            s.push_back(get());
        }

        static std::unordered_map<std::string, TokenType> kw = {
            {"const", TokenType::KW_CONST},
            {"var", TokenType::KW_VAR},
            {"procedure", TokenType::KW_PROCEDURE},
            {"begin", TokenType::KW_BEGIN},
            {"end", TokenType::KW_END},
            {"if", TokenType::KW_IF},
            {"then", TokenType::KW_THEN},
            {"while", TokenType::KW_WHILE},
            {"do", TokenType::KW_DO},
            {"call", TokenType::KW_CALL},
            {"read", TokenType::KW_READ},
            {"write", TokenType::KW_WRITE},
            {"odd", TokenType::KW_ODD}
        };

        auto it = kw.find(s);
        if (it != kw.end()) {
            return {it->second, s, l};
        }

        // 标识符过长
        if (s.size() > 8) {
            return {TokenType::LEX_ERROR_ID_LEN, s, l};
        }

        return {TokenType::IDENTIFIER, s, l};
    }

    // ================= Number =================
    if (std::isdigit((unsigned char)c)) {
        std::string num;

        while (std::isdigit((unsigned char)peek())) {
            num.push_back(get());
        }

        // 非法：2a
        if (std::isalpha((unsigned char)peek())) {
            std::string bad = num;
            while (std::isalnum((unsigned char)peek())) {
                bad.push_back(get());
            }
            return {TokenType::LEX_ERROR_CHAR, bad, l};
        }

        // 越界
        if (num.size() > 8) {
            return {TokenType::LEX_ERROR_NUM_OVER, num, l};
        }

        return {TokenType::NUMBER, num, l};
    }

    // ================= Operators =================
    switch (c) {

        case '+': get(); return {TokenType::PLUS, "+", l};
        case '-': get(); return {TokenType::MINUS, "-", l};
        case '*': get(); return {TokenType::MUL, "*", l};
        case '/': get(); return {TokenType::DIV, "/", l};

        case ':':
            get();
            if (peek() == '=') {
                get();
                return {TokenType::ASSIGN, ":=", l};
            }
            return {TokenType::LEX_ERROR_CHAR, std::string(1, ':'), l};

        case '=': get(); return {TokenType::EQ, "=", l};
        case '#': get(); return {TokenType::NEQ, "#", l};

        case '<':
            get();
            if (peek() == '=') {
                get();
                return {TokenType::LE, "<=", l};
            }
            return {TokenType::LT, "<", l};

        case '>':
            get();
            if (peek() == '=') {
                get();
                return {TokenType::GE, ">=", l};
            }
            return {TokenType::GT, ">", l};

        case ';': get(); return {TokenType::SEMICOLON, ";", l};
        case ',': get(); return {TokenType::COMMA, ",", l};
        case '(': get(); return {TokenType::LPAREN, "(", l};
        case ')': get(); return {TokenType::RPAREN, ")", l};
        case '.': get(); return {TokenType::PERIOD, ".", l};

        default:
            return {TokenType::LEX_ERROR_CHAR, std::string(1, get()), l};
    }
}

// ================= Report =================
void Lexer::printReport() const {
    std::cout << "\n===== Lexer Report =====\n";
    for (auto& p : counter) {
        std::cout << (int)p.first << " : " << p.second << "\n";
    }
}