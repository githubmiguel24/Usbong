#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"
#include <ctype.h>
#include <stdbool.h>
#include "wordhash.h"

// func prototypes
void lexer(FILE *file, FILE *symbolFileAppend);
Token digitChecker(FILE *file, int firstCh, int lineNumber);
Token identifierChecker(FILE *file, int firstCh, int lineNumber);
Token makeToken(TokenCategory cat, int tokenValue, const char *lexeme, int lineNumber);
void printToken(FILE *file, Token *t);
int checkExtension(const char *filename);
static const char *token_value_name(const Token *t);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("proper input: ./programName <filename.usb>", argv[0]);
        return EXIT_FAILURE;
    }
    initialize_table();
    const char *filename = argv[1];
    if (checkExtension(filename) == 0) {
        printf("Error: File must have .usb extension\n");
        return EXIT_FAILURE;
    }else {
        FILE *file = fopen(filename, "r");
        if (!file) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }else {
            printf("File opened successfully: %s\n", filename);
            fopen("Symbol Table.txt", "w");
            FILE *symbolFileAppend;
            symbolFileAppend = fopen("Symbol Table.txt", "a");
            fprintf(symbolFileAppend, "Lexeme           | Token Name\n");
            lexer(file, symbolFileAppend);
            fclose(file); 
            fclose(symbolFileAppend);
            return EXIT_SUCCESS;
        }
    }
}

//fn extension checker
int checkExtension(const char *filename) {
    const char *dot = strrchr(filename, '.');  //find last dot in filename
    if (!dot) return 0;                        // if no dot found = no extension
    if (strcmp(dot, ".usb") == 0) {  //if it matches .usb
        return 1;
    } else return 0;  //file is not .usb file
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

         case CAT_KEYWORD:
            switch (t->tokenValue) {
                case K_ANI: return "K_ANI";
                case K_TANIM: return "K_TANIM";
                case K_PARA: return "K_PARA";
                case K_HABANG: return "K_HABANG";
                case K_KUNG: return "K_KUNG";
                case K_KUNDI: return "K_KUNDI";
                case K_KUNDIMAN: return "K_KUNDIMAN";
                case K_GAWIN: return "K_GAWIN";
                case K_TIBAG: return "K_TIBAG";
                case K_TULOY: return "K_TULOY";
                case K_PANGKAT: return "K_PANGKAT";
                case K_STATIK: return "K_STATIK";
                case K_PRIBADO: return "K_PRIBADO";
                case K_PROTEKTADO: return "K_PROTEKTADO";
                case K_PUBLIKO: return "K_PUBLIKO";
                default: return "K_UNKNOWN";
            };

        case CAT_RESERVED:
            switch (t->tokenValue) {
                case R_TAMA: return "R_TAMA";
                case R_MALI: return "R_MALI";
                case R_UGAT: return "R_UGAT";
                case R_BALIK: return "R_BALIK";
                case R_BILANG: return "R_BILANG";
                case R_KWERDAS: return "R_KWERDAS";
                case R_TITIK: return "R_TITIK";
                case R_LUTANG: return "R_LUTANG";
                case R_BULYAN: return "R_BULYAN";
                case R_DOBLE: return "R_DOBLE";
                case R_WALA: return "R_WALA";
                default: return "R_UNKNOWN";
            };

        case CAT_NOISEWORD:
            switch (t->tokenValue) {
                case N_NG: return "N_NG";
                case N_AY: return "N_AY";
                case N_BUNGA: return "N_BUNGA";
                case N_WAKAS: return "N_WAKAS";
                case N_SA: return "N_SA";
                case N_ANG: return "N_ANG";
                case N_MULA: return "N_MULA";
                case N_ITAKDA: return "N_ITAKDA";
                default: return "N_UNKNOWN";
            };
         case CAT_COMMENT:
            switch (t->tokenValue) {
                case C_SINGLE_LINE: return "C_SINGLE_LINE";
                case C_MULTI_LINE: return "C_MULTI_LINE";
                default: return "C_UNKNOWN";
            }
        default: return "UNKNOWN_CATEGORY";
    }
}

// Print token as: lexeme | TOKEN_NAME
void printToken(FILE *file, Token *t) {
    const char *lex;
    lex = t->lexeme;
    const char *name = token_value_name(t);
    fprintf(file, "%s          | %s\n", lex, name);
}

//create a token
Token makeToken(TokenCategory cat, int tokenValue, const char *lexeme, int lineNumber) {
    Token t;
    t.category = cat;
    t.tokenValue = tokenValue;
    t.lexeme = malloc(strlen(lexeme) + 1);
    strcpy(t.lexeme, lexeme);
    t.lineNumber = lineNumber;
    return t;
}

// Lexer function that reads characters from the file and produces Token structs
void lexer (FILE *file, FILE *symbolFileAppend) {
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
            tok = makeToken(CAT_DELIMITER, D_SEMICOLON, ";", lineNumber);
            produced = true;
        } else if (c == '{') {
            tok = makeToken(CAT_DELIMITER, D_LBRACE, "{", lineNumber);
            produced = true;
        }else if (c == '}') {
            tok = makeToken(CAT_DELIMITER, D_RBRACE, "}", lineNumber);
            produced = true;
        } else if (c== '(') {
            tok = makeToken(CAT_DELIMITER, D_LPAREN, "(", lineNumber);
            produced = true;
        } else if (c== ')') {
            tok = makeToken(CAT_DELIMITER, D_RPAREN, ")", lineNumber);
            produced = true;
        } else if(c == '-'){
            tok = makeToken(CAT_OPERATOR, O_MINUS, "-", lineNumber);
            produced = true;
        }else if (c == '+'){
            tok = makeToken(CAT_OPERATOR, O_PLUS, "+", lineNumber);
            produced = true;
        } else if (c == '*'){
            tok = makeToken(CAT_OPERATOR, O_MULTIPLY, "*", lineNumber);
            produced = true;
        } else if (c == '/'){
            int next = fgetc(file);
            if(next == '/'){
                tok = makeToken(CAT_COMMENT, C_SINGLE_LINE, "//", lineNumber);
                produced = true;
            }else if(next == '*'){ 
                tok = makeToken(CAT_COMMENT, C_MULTI_LINE, "/*", lineNumber);
                produced = true;
            } else {
                ungetc(next, file);
                tok = makeToken(CAT_OPERATOR, O_DIVIDE, "/", lineNumber);
                produced = true;
            }
        }else if (c == '\"') {
            char kwerdas[256]; 
            int i = 0;
            int nextCh = fgetc(file);

            //check until another double quote or EOF
            while (nextCh != EOF && nextCh != '\"' && i < sizeof(kwerdas) - 1) {
                kwerdas[i++] = (char)nextCh;
                nextCh = fgetc(file);
            }

            kwerdas[i] = '\0'; // null-terminate

            if (nextCh != '\"') {
                // no more quote found, invalid token
                tok = makeToken(CAT_UNKNOWN, 0, kwerdas, lineNumber);
            } else {
                // found another quote, create KWERDAS literal token
                tok = makeToken(CAT_LITERAL, L_KWERDAS_LITERAL, kwerdas, lineNumber);
            }
            produced = true;
        } else if (c == '\"') {
            tok = makeToken(CAT_DELIMITER, D_QUOTE, "\"", lineNumber);
            produced = true;
        }else if (c == ','){
            tok = makeToken(CAT_DELIMITER, D_COMMA, ",", lineNumber);
            produced = true;
        } else if (c == '.'){
            tok = makeToken(CAT_DELIMITER, D_DOT, ".", lineNumber);
            produced = true;
        } else if (c == '^'){
            tok = makeToken(CAT_OPERATOR, O_POW, "^", lineNumber);
            produced = true;
        }else if (c == '%'){
            tok = makeToken(CAT_OPERATOR, O_MODULO, "%", lineNumber);
            produced = true;
        }else if (c =='['){
            tok = makeToken(CAT_DELIMITER, D_LBRACKET, "[", lineNumber);
            produced = true;
        }else if (c ==']'){
            tok = makeToken(CAT_DELIMITER, D_RBRACKET, "]", lineNumber);
            produced = true;  
        }else if (c == '&'){
            int next = fgetc(file);
            if(next == '&'){
                tok = makeToken(CAT_OPERATOR, O_AND, "&&", lineNumber);
                produced = true;
            }else { 
                ungetc(next, file); 
                char lex[2] = {c, '\0' };
                tok = makeToken(CAT_UNKNOWN, 0, lex, lineNumber);
                produced = true;
            }
        } else if (c == '|'){
            int next = fgetc(file);
            if(next == '|'){
                tok = makeToken(CAT_OPERATOR, O_OR, "||", lineNumber);
                produced = true;
            }else { 
                ungetc(next, file); 
                char lex[2] = { c, '\0' };
                tok = makeToken(CAT_UNKNOWN, 0, lex, lineNumber);
                produced = true;
            }
        }else if (c == '!'){
            tok = makeToken(CAT_OPERATOR, O_NOT, "!", lineNumber);
            produced = true;
        }else if (c == '>'){
            tok = makeToken(CAT_OPERATOR, O_GREATER, ">", lineNumber);
            produced = true;
        }else if (c == '<'){
            tok = makeToken(CAT_OPERATOR, O_LESS, "<", lineNumber);
            produced = true;
        } else if (c == '='){
             int next = fgetc(file);
            if(next == '='){
                tok = makeToken(CAT_OPERATOR, O_EQUAL, "==", lineNumber);
                produced = true;
            }else { 
                ungetc(next, file); 
                char lex[2] = {c, '\0' };
                tok = makeToken(CAT_OPERATOR, O_ASSIGN, "=", lineNumber);
                produced = true;
            }
        } else if (isdigit((unsigned char)c)){
                Token tnum = digitChecker(file, current, lineNumber);
                printToken(symbolFileAppend, &tnum);
                if (tnum.lexeme) free(tnum.lexeme);
                current = fgetc(file);
                continue;
        } else if(isalpha((unsigned char)c)) {
            Token tid = identifierChecker(file, current, lineNumber);
            printToken(symbolFileAppend, &tid);
            if (tid.lexeme) free(tid.lexeme);
            current = fgetc(file);
            continue;
        } else {
            char lex[2] = { c, '\0' };
            tok = makeToken(CAT_UNKNOWN, 0, lex, lineNumber);
            produced = true;
        }

        if (produced) {
            printToken(symbolFileAppend, &tok);
            
        }

        current = fgetc(file);
    }
}

Token digitChecker(FILE *file, int firstCh, int lineNumber) {
    int c = firstCh;
    char digit[300];
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
        return makeToken(CAT_UNKNOWN, 0, digit, lineNumber);
    }

    return makeToken(CAT_LITERAL, token.tokenValue, digit, lineNumber);
}

Token identifierChecker(FILE *file, int firstCh, int lineNumber) {
    char identifier[200];
    size_t x = 0;

    //inpt first char to array
    identifier[x++] = (char)firstCh;
    
    int c = fgetc(file);//get next char
    
    while (c != EOF && (isalnum((unsigned char)c) || c == '_') && x + 1 < sizeof(identifier)) {
        identifier[x++] = (char)c;
        c = fgetc(file);
    }

    identifier[x] = '\0';

    //last char is not part of identifier, so push back
    if (c != EOF) ungetc(c, file);

    HashEntry *entry = hashLookUp(identifier);

    if (entry) {
        return makeToken(entry->category, entry->tokenValue, identifier, lineNumber);
    }else {
        return makeToken(CAT_LITERAL, L_IDENTIFIER, identifier, lineNumber);
    }

}
