#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <unordered_set>
#include "Token.h"

struct SyntaxError {
    int line;
    std::string message;
};

class Parser {
private:
    std::vector<Token> tokens;
    int pos;

    std::vector<SyntaxError> errors;
    std::unordered_set<std::string> procedures;

public:
    Parser(const std::vector<Token>& input);

    const Token& cur();
    const Token& get();
    bool match(TokenType t);
    void expect(TokenType t);

    void parseProgram();

    const std::vector<SyntaxError>& getErrors() const;

private:
    void parseBlock();
    void parseConstDecl();
    void parseVarDecl();
    void parseProcDecl();

    void parseStatement();
    void parseStatementSequence();

    void parseCondition();
    void parseExpression();
    void parseTerm();
    void parseFactor();

    bool isStatementBegin(TokenType t);

    void reportError(const std::string& msg);
    void reportErrorAt(int line, const std::string& msg);
    void synchronize();
};

#endif