#include "Parser.h"
#include <stdexcept>

// ================= 构造函数 =================
Parser::Parser(const std::vector<Token>& input) {
    tokens = input;
    pos = 0;
}

// ================= 工具函数 =================
const Token& Parser::cur() {
    if (pos >= (int)tokens.size())
        return tokens.back();
    return tokens[pos];
}

const Token& Parser::get() {
    if (pos >= (int)tokens.size())
        throw std::runtime_error("Unexpected EOF");
    return tokens[pos++];
}

bool Parser::match(TokenType t) {
    if (cur().type == t) {
        pos++;
        return true;
    }
    return false;
}

// ================= 错误处理 =================
void Parser::reportError(const std::string& msg) {
    errors.push_back({cur().line, msg});
}

void Parser::synchronize() {
    while (cur().type != TokenType::SEMICOLON &&
           cur().type != TokenType::KW_END &&
           cur().type != TokenType::PERIOD &&
           cur().type != TokenType::TK_EOF) {
        pos++;
    }
    if (cur().type == TokenType::SEMICOLON)
        pos++;
}

// ================= EXPECT =================
void Parser::expect(TokenType t) {
    if (cur().type == t) {
        pos++;
    } else {
        reportError("unexpected token");
        synchronize();
    }
}

// ================= PROGRAM =================
void Parser::parseProgram() {
    parseBlock();
    expect(TokenType::PERIOD);
}

// ================= BLOCK =================
void Parser::parseBlock() {

    if (cur().type == TokenType::KW_CONST)
        parseConstDecl();

    if (cur().type == TokenType::KW_VAR)
        parseVarDecl();

    while (cur().type == TokenType::KW_PROCEDURE)
        parseProcDecl();

    parseStatement();
}

// ================= CONST =================
void Parser::parseConstDecl() {
    expect(TokenType::KW_CONST);

    do {
        expect(TokenType::IDENTIFIER);
        expect(TokenType::EQ);
        expect(TokenType::NUMBER);
    } while (match(TokenType::COMMA));

    expect(TokenType::SEMICOLON);
}

// ================= VAR =================
void Parser::parseVarDecl() {
    expect(TokenType::KW_VAR);

    expect(TokenType::IDENTIFIER);

    while (match(TokenType::COMMA))
        expect(TokenType::IDENTIFIER);

    expect(TokenType::SEMICOLON);
}

// ================= PROC =================
void Parser::parseProcDecl() {
    expect(TokenType::KW_PROCEDURE);
    expect(TokenType::IDENTIFIER);
    expect(TokenType::SEMICOLON);

    parseBlock();

    expect(TokenType::SEMICOLON);
}

// ================= STATEMENT =================
void Parser::parseStatement() {

    if (!isStatementBegin(cur().type)) {
        reportError("invalid statement");
        synchronize();
        return;
    }

    if (cur().type == TokenType::KW_READ) {
        expect(TokenType::KW_READ);
        expect(TokenType::LPAREN);
        expect(TokenType::IDENTIFIER);
        expect(TokenType::RPAREN);
        return;
    }

    if (cur().type == TokenType::KW_WRITE) {
        expect(TokenType::KW_WRITE);
        expect(TokenType::LPAREN);
        parseExpression();
        expect(TokenType::RPAREN);
        return;
    }

    if (cur().type == TokenType::IDENTIFIER) {
        expect(TokenType::IDENTIFIER);
        expect(TokenType::ASSIGN);
        parseExpression();
        return;
    }

    if (cur().type == TokenType::KW_CALL) {
        expect(TokenType::KW_CALL);
        expect(TokenType::IDENTIFIER);
        return;
    }

    if (cur().type == TokenType::KW_BEGIN) {
        expect(TokenType::KW_BEGIN);

        parseStatementSequence();

        expect(TokenType::KW_END);
        return;
    }

    if (cur().type == TokenType::KW_IF) {
        expect(TokenType::KW_IF);
        parseCondition();
        expect(TokenType::KW_THEN);
        parseStatement();
        return;
    }

    if (cur().type == TokenType::KW_WHILE) {
        expect(TokenType::KW_WHILE);
        parseCondition();
        expect(TokenType::KW_DO);
        parseStatement();
        return;
    }
}

// ================= SEQUENCE =================
void Parser::parseStatementSequence() {
    parseStatement();

    while (match(TokenType::SEMICOLON)) {
        parseStatement();
    }
}

// ================= EXPRESSION =================
void Parser::parseExpression() {

    if (cur().type == TokenType::PLUS ||
        cur().type == TokenType::MINUS)
        get();

    parseTerm();

    while (cur().type == TokenType::PLUS ||
           cur().type == TokenType::MINUS) {
        get();
        parseTerm();
    }
}

// ================= TERM =================
void Parser::parseTerm() {

    parseFactor();

    while (cur().type == TokenType::MUL ||
           cur().type == TokenType::DIV) {
        get();
        parseFactor();
    }
}

// ================= FACTOR =================
void Parser::parseFactor() {

    if (cur().type == TokenType::IDENTIFIER) {
        get();
        return;
    }

    if (cur().type == TokenType::NUMBER) {
        get();
        return;
    }

    if (match(TokenType::LPAREN)) {
        parseExpression();
        expect(TokenType::RPAREN);
        return;
    }

    reportError("invalid factor");
    synchronize();
}

// ================= CONDITION =================
void Parser::parseCondition() {

    parseExpression();

    if (cur().type == TokenType::EQ ||
        cur().type == TokenType::NEQ ||
        cur().type == TokenType::LT ||
        cur().type == TokenType::LE ||
        cur().type == TokenType::GT ||
        cur().type == TokenType::GE) {

        get();
        parseExpression();
    } else {
        reportError("missing relational operator");
        synchronize();
    }
}

// ================= FIRST SET =================
bool Parser::isStatementBegin(TokenType t) {
    return t == TokenType::IDENTIFIER ||
           t == TokenType::KW_CALL ||
           t == TokenType::KW_BEGIN ||
           t == TokenType::KW_IF ||
           t == TokenType::KW_WHILE ||
           t == TokenType::KW_READ ||
           t == TokenType::KW_WRITE;
}

// ================= ERRORS =================
const std::vector<SyntaxError>& Parser::getErrors() const {
    return errors;
}