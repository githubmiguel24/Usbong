//Contains main() function.
//Handles file input
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"
#include <ctype.h>
#include <stdbool.h>

// Prototypes
void lexer(FILE *file);
Token digitChecker(FILE *file, int firstCh, int lineNumber);
Token identifierChecker(FILE *file, int firstCh, int lineNumber);
Token make_token(TokenCategory cat, int tokenValue, const char *lexeme, int lineNumber);
void print_token(const Token *t);

int main() {

    //open file for source code reading
    FILE *file;
    file = fopen("sampleSourceCode.usb", "r");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    lexer(file);
    fclose(file); 
    return EXIT_SUCCESS;
}
//tokenValue
static const char *token_value_name(const Token *t) {
    if (!t) return "(null)";
    switch (t->category) {
        case CAT_DELIMITER:
            switch (t->tokenValue) {
                case D_LPAREN: return "D_LPAREN";
                case D_RPAREN: return "D_RPAREN";
                case D_LBRACE: return "D_LBRACE";
                case D_RBRACE: return "D_RBRACE";
                case D_LBRACKET: return "D_LBRACKET";
                case D_RBRACKET: return "D_RBRACKET";
                case D_COMMA: return "D_COMMA";
                case D_SEMICOLON: return "D_SEMICOLON";
                case D_COLON: return "D_COLON";
                case D_DOT: return "D_DOT";
                case D_QUOTE: return "D_QUOTE";
                case D_SQUOTE: return "D_SQUOTE";
                default: return "D_UNKNOWN";
            }

        case CAT_OPERATOR:
            switch (t->tokenValue) {
                case O_PLUS: return "O_PLUS";
                case O_MINUS: return "O_MINUS";
                case O_MULTIPLY: return "O_MULTIPLY";
                case O_DIVIDE: return "O_DIVIDE";
                case O_POW: return "O_POW";
                case O_MODULO: return "O_MODULO";
                case O_ASSIGN: return "O_ASSIGN";
                case O_EQUAL: return "O_EQUAL";
                case O_NOT_EQUAL: return "O_NOT_EQUAL";
                case O_LESS: return "O_LESS";
                case O_GREATER: return "O_GREATER";
                case O_LESS_EQ: return "O_LESS_EQ";
                case O_GREATER_EQ: return "O_GREATER_EQ";
                case O_AND: return "O_AND";
                case O_OR: return "O_OR";
                case O_NOT: return "O_NOT";
                default: return "O_UNKNOWN";
            }

        case CAT_LITERAL:
            switch (t->tokenValue) {
                case L_IDENTIFIER: return "L_IDENTIFIER";
                case L_BILANG_LITERAL: return "L_BILANG_LITERAL";
                case L_LUTANG_LITERAL: return "L_LUTANG_LITERAL";
                case L_KWERDAS_LITERAL: return "L_KWERDAS_LITERAL";
                case L_TITIK_LITERAL: return "L_TITIK_LITERAL";
                case L_BULYAN_LITERAL: return "L_BULYAN_LITERAL";
                default: return "L_UNKNOWN";
            }

        case CAT_KEYWORD: return "CAT_KEYWORD";
        case CAT_RESERVED: return "CAT_RESERVED";
        case CAT_NOISEWORD: return "CAT_NOISEWORD";
        case CAT_COMMENT: return "CAT_COMMENT";
        default: return "UNKNOWN_CATEGORY";
    }
}

// Print token as: lexeme | TOKEN_NAME
void print_token(const Token *t) {
    const char *lex = t->lexeme ? t->lexeme : "";
    const char *name = token_value_name(t);
    printf("%s | %s\n", lex, name);
}

//create a token
Token make_token(TokenCategory cat, int tokenValue, const char *lexeme, int lineNumber) {
    Token t;
    t.category = cat;
    t.tokenValue = tokenValue;
    if (lexeme) {
        t.lexeme = malloc(strlen(lexeme) + 1);
        if (t.lexeme) strcpy(t.lexeme, lexeme);
    } else {
        t.lexeme = NULL;
    }
    t.lineNumber = lineNumber;
    return t;
}

// Lexer function that reads characters from the file and produces Token structs
void lexer (FILE *file) {
   int current  = fgetc(file);
   int lineNumber = 1;
    while (current != EOF)
    {
        char c = (char) current;
        Token tok;
        bool produced = false;

        if (c == '\n') {
            lineNumber++;
        }
       
        if (isspace((unsigned char)c)) {
            //ignore
        } else if(c == ';') {
            tok = make_token(CAT_DELIMITER, D_SEMICOLON, ";", lineNumber);
            produced = true;
        } else if (c == '{') {
            tok = make_token(CAT_DELIMITER, D_LBRACE, "{", lineNumber);
            produced = true;
        }else if (c == '}') {
            tok = make_token(CAT_DELIMITER, D_RBRACE, "}", lineNumber);
            produced = true;
        } else if (c== '(') {
            tok = make_token(CAT_DELIMITER, D_LPAREN, "(", lineNumber);
            produced = true;
        } else if (c== ')') {
            tok = make_token(CAT_DELIMITER, D_RPAREN, ")", lineNumber);
            produced = true;
        } else if(c == '-'){
            tok = make_token(CAT_OPERATOR, O_MINUS, "-", lineNumber);
            produced = true;
        }else if (c == '+'){
            tok = make_token(CAT_OPERATOR, O_PLUS, "+", lineNumber);
            produced = true;
        } else if (c == '*'){
            tok = make_token(CAT_OPERATOR, O_MULTIPLY, "*", lineNumber);
            produced = true;
        } else if (c == '/'){
            int next = fgetc(file);
            if(next == '/'){
                tok = make_token(CAT_COMMENT, C_SINGLE_LINE, "//", lineNumber);
                produced = true;
            }else if(next == '*'){ 
                tok = make_token(CAT_COMMENT, C_MULTI_LINE, "/*", lineNumber);
                produced = true;
            } else {
                ungetc(next, file);
                tok = make_token(CAT_OPERATOR, O_DIVIDE, "/", lineNumber);
                produced = true;
            }
        }else if (c == '\'') {
            char nextCh = (char)fgetc(file);
            if (isalpha((unsigned char)nextCh)) {
                // Handle character literal ('a', 'b', etc.)
                char lex[2] = {nextCh, '\0'};
                tok = make_token(CAT_LITERAL, L_TITIK_LITERAL, lex, lineNumber);
                
                // Consume closing quote
                int closeQuote = fgetc(file);
                if (closeQuote != '\'') {
                    ungetc(closeQuote, file);  // Push back if not a closing quote
                    tok = make_token(CAT_UNKNOWN, 0, lex, lineNumber);
                }
                produced = true;
            } else {
        // Not a valid character literal, treat as delimiter
                ungetc(nextCh, file);
                tok = make_token(CAT_DELIMITER, D_SQUOTE, "\'", lineNumber);
                produced = true;
            }
        } else if (c == '\"') {
        tok = make_token(CAT_DELIMITER, D_QUOTE, "\"", lineNumber);
        produced = true;
        }else if (c == ','){
            tok = make_token(CAT_DELIMITER, D_COMMA, ",", lineNumber);
            produced = true;
        } else if (c == '.'){
            tok = make_token(CAT_DELIMITER, D_DOT, ".", lineNumber);
            produced = true;
        } else if (c == '^'){
            tok = make_token(CAT_OPERATOR, O_POW, "^", lineNumber);
            produced = true;
        }else if (c == '%'){
            tok = make_token(CAT_OPERATOR, O_MODULO, "%", lineNumber);
            produced = true;
        }else if (c =='['){
            tok = make_token(CAT_DELIMITER, D_LBRACKET, "[", lineNumber);
            produced = true;
        }else if (c ==']'){
            tok = make_token(CAT_DELIMITER, D_RBRACKET, "]", lineNumber);
            produced = true;  
        }else if (c == '&'){
            int next = fgetc(file);
            if(next == '&'){
                tok = make_token(CAT_OPERATOR, O_AND, "&&", lineNumber);
                produced = true;
            }else { 
                ungetc(next, file); 
                char lex[2] = {c, '\0' };
                tok = make_token(CAT_UNKNOWN, 0, lex, lineNumber);
                produced = true;
            }
        } else if (c == '|'){
            int next = fgetc(file);
            if(next == '|'){
                tok = make_token(CAT_OPERATOR, O_OR, "||", lineNumber);
                produced = true;
            }else { 
                ungetc(next, file); 
                char lex[2] = { c, '\0' };
                tok = make_token(CAT_UNKNOWN, 0, lex, lineNumber);
                produced = true;
            }
        }else if (c == '!'){
            tok = make_token(CAT_OPERATOR, O_NOT, "!", lineNumber);
            produced = true;
        }else if (c == '>'){
            tok = make_token(CAT_OPERATOR, O_GREATER, ">", lineNumber);
            produced = true;
        }else if (c == '<'){
            tok = make_token(CAT_OPERATOR, O_LESS, "<", lineNumber);
            produced = true;
        } else if (c == '='){
             int next = fgetc(file);
            if(next == '='){
                tok = make_token(CAT_OPERATOR, O_EQUAL, "==", lineNumber);
                produced = true;
            }else { 
                ungetc(next, file); 
                char lex[2] = {c, '\0' };
                tok = make_token(CAT_OPERATOR, O_ASSIGN, "=", lineNumber);
                produced = true;
            }
        } else if (isdigit((unsigned char)c)){
                Token tnum = digitChecker(file, current, lineNumber);
                print_token(&tnum);
                if (tnum.lexeme) free(tnum.lexeme);
                current = fgetc(file);
                continue;
        } else if(isalpha((unsigned char)c)) {
            Token tid = identifierChecker(file, current, lineNumber);
            print_token(&tid);
            if (tid.lexeme) free(tid.lexeme);
            current = fgetc(file);
            continue;
        } else {
            char lex[2] = { c, '\0' };
            tok = make_token(CAT_UNKNOWN, 0, lex, lineNumber);
            produced = true;
        }

        if (produced) {
            print_token(&tok);
            if (tok.lexeme) free(tok.lexeme);
        }

        current = fgetc(file);
    }
}

Token digitChecker(FILE *file, int firstCh, int lineNumber) {
    int c = firstCh;
    char digit[200];
    int x = 0;
    Token token;
    token.category = CAT_LITERAL;
    token.tokenValue = L_BILANG_LITERAL;
    bool hasDecimalPoint = false;
    // Read digits and dots (mark invalid if multiple dots)
    while (c != EOF && x + 1 < sizeof(digit) && (isdigit((unsigned char)c) || c == '.')) {
        digit[x++] = (char)c;
        if (c == '.') {
            if (hasDecimalPoint) {
                token.category = CAT_UNKNOWN; // mark invalid
            } else {
                hasDecimalPoint = true;
                token.tokenValue = L_LUTANG_LITERAL;
            }
        }
        c = fgetc(file);
    }

    // Null-terminate
    digit[x] = '\0';

    // push back last read non-number char
    if (c != EOF) ungetc(c, file);

    if (token.category == CAT_UNKNOWN || (hasDecimalPoint && digit[x-1] == '.')) {
        return make_token(CAT_UNKNOWN, 0, digit, lineNumber);
    }

    return make_token(CAT_LITERAL, token.tokenValue, digit, lineNumber);
}

Token identifierChecker(FILE *file, int firstCh, int lineNumber) {
    char identifier[200];
    size_t x = 0;

    // Add the first character
    identifier[x++] = (char)firstCh;
    
    int c = fgetc(file);
    // Read and collect identifier chars
    while (c != EOF && (isalnum((unsigned char)c) || c == '_') && x + 1 < sizeof(identifier)) {
        identifier[x++] = (char)c;
        c = fgetc(file);
    }

    identifier[x] = '\0';

    // push back the last non-identifier char
    if (c != EOF) ungetc(c, file);

    return make_token(CAT_LITERAL, L_IDENTIFIER, identifier, lineNumber);
}

