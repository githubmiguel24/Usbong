#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"
#include <ctype.h>
#include <stdbool.h>
#include "wordhash.h"
//States
typedef enum {
    S_START,   //Start state

    //Words without quotes(" ")
    S_IDENTIFIER,       
    S_KEYWORD,  // Note: These are final states, decided *after* S_IDENTIFIER
    S_RESERVE,  // Note: These are final states, decided *after* S_IDENTIFIER
    S_NOISE,    // Note: These are final states, decided *after* S_IDENTIFIER

    //Numbers
    S_NUMBER_BILANG,
    S_NUMBER_LUTANG,

    //Strings & characters
    S_KWERDAS_HEAD,   //for double quote start
    S_KWERDAS_BODY,    //main string
    S_KWERDAS_TAIL,    //last double quote

    //Strings & characters
    S_TITIK_HEAD,   //for single quote start
    S_TITIK_BODY,    //main charatcer 
    S_TITIK_TAIL,    //last single quote

    // Operators
    S_OP_PLUS,         // + (Final State)
    S_OP_MINUS,        // - (Final State)
    S_OP_MULTIPLY,     // * (Final State)
    S_OP_DIVIDE_HEAD,      // /  (may lead to comments)
    S_OP_DIVIDE_TAIL,  // /? → // or /* or / (This state is unused, logic is in S_OP_DIVIDE_HEAD)
    S_OP_ASSIGN_HEAD,      // =
    S_OP_ASSIGN_TAIL,  // == (Final State)
    S_OP_NOT_HEAD,         // !
    S_OP_NOT_TAIL,     // != or ! (Final State)
    S_OP_LESS_HEAD,        // <
    S_OP_LESS_TAIL,    // <= or < (Final State)
    S_OP_GREATER_HEAD,     // >
    S_OP_GREATER_TAIL, // >= or > (Final State)
    S_OP_AND_HEAD,     //&
    S_OP_AND_TAIL,     // && (Final State)
    S_OP_OR_HEAD,      // |  
    S_OP_OR_TAIL,      // || (Final State)

    //Comments
    S_COMMENT_SINGLE,  // //
    S_COMMENT_MULTI_HEAD,   // /*
    S_COMMENT_MULTI_TAIL, // checking for */

    // Delimiters
    S_DELIMITER,       // ( ) { } [ ] , . ; etc. (Final State)

    // End / Unknown
    S_UNKNOWN,
    S_DONE // (Unused in this implementation)
} LexerState;


// func prototypes
void lexer(FILE *file, FILE *symbolFileAppend);
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
            fopen("Symbol Table.txt", "w"); // Clear the file
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

// Lexer function that reads characters from the file and produces Token structs
void lexer (FILE *file, FILE *symbolFileAppend) {
   
    LexerState currentState = S_START;
    char lexemeBuffer[1024]; // Buffer for building the current lexeme
    int lexemeIndex = 0;
    int lineNumber = 1;
    int tokenStartLine = 1; // Line number where the current token started
    
    int c; // Current character

    //FA LOOP START --> reads/chcks 1 character per iteration.
    while (true) { // Loop until EOF is explicitly handled
        
        c = fgetc(file); // Get next char
        Token tok;
        switch (currentState) {
            
            // --- START STATE ---
            // This is the main router. It decides which state to go to
            // based on the first character of a new token.
            case S_START:
                lexemeIndex = 0; // Reset buffer
                memset(lexemeBuffer, 0, sizeof(lexemeBuffer));
                tokenStartLine = lineNumber;

                if (c == EOF) {
                    return; // get out of loop if 
                }

                if (isspace(c)) { //ignore white spaces
                    if (c == '\n') {
                        lineNumber++;
                    }
                    //Stay in S_START
                    continue; 
                }

                //if not space: input current char
                lexemeBuffer[lexemeIndex++] = (char)c;

                if (isalpha(c)) {
                    currentState = S_IDENTIFIER;
                } else if(c == '_'){
                    currentState = S_UNKNOWN;
                } else if (isdigit(c)) {
                    currentState = S_NUMBER_BILANG;
                } else if (c == '"') {
                    currentState = S_KWERDAS_HEAD;
                } else if (c == '\'') {
                    currentState = S_TITIK_HEAD;
                } else if (c == '/') {
                    currentState = S_OP_DIVIDE_HEAD;
                } else if (c == '&') {
                    currentState = S_OP_AND_HEAD;
                } else if (c == '|') {
                    currentState = S_OP_OR_HEAD;
                } else if (c == '=') {
                    currentState = S_OP_ASSIGN_HEAD;
                } else if (c == '!') {
                    currentState = S_OP_NOT_HEAD;
                } else if (c == '<') {
                    currentState = S_OP_LESS_HEAD;
                } else if (c == '>') {
                    currentState = S_OP_GREATER_HEAD;
                } else {
                    //Single character lexemes are auto final state
                    lexemeBuffer[lexemeIndex] = '\0';
                    switch (c) {
                        case '+': tok = makeToken(CAT_OPERATOR, O_PLUS, lexemeBuffer, tokenStartLine); break;
                        case '-': tok = makeToken(CAT_OPERATOR, O_MINUS, lexemeBuffer, tokenStartLine); break;
                        case '*': tok = makeToken(CAT_OPERATOR, O_MULTIPLY, lexemeBuffer, tokenStartLine); break;
                        case '^': tok = makeToken(CAT_OPERATOR, O_POW, lexemeBuffer, tokenStartLine); break;
                        case '%': tok = makeToken(CAT_OPERATOR, O_MODULO, lexemeBuffer, tokenStartLine); break;
                        
                        case ';': tok = makeToken(CAT_DELIMITER, D_SEMICOLON, lexemeBuffer, tokenStartLine); break;
                        case '{': tok = makeToken(CAT_DELIMITER, D_LBRACE, lexemeBuffer, tokenStartLine); break;
                        case '}': tok = makeToken(CAT_DELIMITER, D_RBRACE, lexemeBuffer, tokenStartLine); break;
                        case '(': tok = makeToken(CAT_DELIMITER, D_LPAREN, lexemeBuffer, tokenStartLine); break;
                        case ')': tok = makeToken(CAT_DELIMITER, D_RPAREN, lexemeBuffer, tokenStartLine); break;
                        case '[': tok = makeToken(CAT_DELIMITER, D_LBRACKET, lexemeBuffer, tokenStartLine); break;
                        case ']': tok = makeToken(CAT_DELIMITER, D_RBRACKET, lexemeBuffer, tokenStartLine); break;
                        case ',': tok = makeToken(CAT_DELIMITER, D_COMMA, lexemeBuffer, tokenStartLine); break;
                        case '.': tok = makeToken(CAT_DELIMITER, D_DOT, lexemeBuffer, tokenStartLine); break;
                        
                        default:
                            tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine);
                            break;
                    }
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; // Reset for next token
                }
                break;

    
            case S_IDENTIFIER:
                if (isalnum(c) || c == '_') {
                    // Keep consuming characters and stay in this state
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_IDENTIFIER;
                } else {
                    if (c != EOF) 
                        ungetc(c, file); // Put it back
                        lexemeBuffer[lexemeIndex] = '\0'; // Finalize
                        HashEntry *entry = hashLookUp(lexemeBuffer);
                    if (entry) {
                        tok = makeToken(entry->category, entry->tokenValue, lexemeBuffer, tokenStartLine);
                    } else {
                        tok = makeToken(CAT_LITERAL, L_IDENTIFIER, lexemeBuffer, tokenStartLine);
                    }
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; // Reset
                }
                break;

            //Numbers (BILANG & LUTANG) States
            case S_NUMBER_BILANG:
                if (isdigit(c)) {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    // Stay in S_NUMBER_BILANG
                } else if (c == '.') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_NUMBER_LUTANG; // Transition
                } else if(isalpha(c)){ //unexpected char 
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_UNKNOWN;
                }else {
                    if (c != EOF){
                        ungetc(c, file);
                    }
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_LITERAL, L_BILANG_LITERAL, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; // Reset
                }
                break; 

            case S_NUMBER_LUTANG:
                if (isdigit(c)) {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                } else {
                    if (c != EOF)
                        ungetc(c, file);
                    lexemeBuffer[lexemeIndex] = '\0';
                    //check if . is last number (error checking)
                    if (lexemeBuffer[lexemeIndex - 1] == '.') { // e.g., "123."
                        tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine);
                    } else {
                        tok = makeToken(CAT_LITERAL, L_LUTANG_LITERAL, lexemeBuffer, tokenStartLine);
                    }
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; // Reset
                }
            break; 

            //KWERDAS STATES
            case S_KWERDAS_HEAD: //previous input is double quotes
                lexemeIndex = 0; // Reset buffer to *not* include the quotes
                if (c == '"') {//means end of string
                    currentState = S_KWERDAS_TAIL; // Go to TAIL state
                    continue; 
                }
                if (c == EOF || c == '\n') {
                    if (c != EOF) 
                        ungetc(c, file);//error checking
                        lexemeBuffer[0] = '"'; // Show the unterminated quote
                        lexemeBuffer[1] = '\0';
                        tok = makeToken(CAT_DELIMITER, D_QUOTE, lexemeBuffer, tokenStartLine);
                        printToken(symbolFileAppend, &tok);
                        //current state is final state therefore go to start state
                        currentState = S_START;
                    if(c == '\n') 
                        lineNumber++;
                } else {
                    //not eof or next line therefore part of the kwerdas
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_KWERDAS_BODY;
                }
            break;

            case S_KWERDAS_BODY:
                if (c == '"') {
                    currentState = S_KWERDAS_TAIL; //second quote --> end of string
                } else if (c == EOF || c == '\n') {
                    //error check
                    if (c != EOF) 
                        ungetc(c, file);
                        lexemeBuffer[lexemeIndex] = '\0';
                        tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine); // Unterminated string
                        printToken(symbolFileAppend, &tok);
                        currentState = S_START; //go to next lexeme
                    if(c == '\n') 
                        lineNumber++;
                } else {
                    // Stay in body, consume char
                    lexemeBuffer[lexemeIndex++] = (char)c;
                }
            break;

            case S_KWERDAS_TAIL: //input: second " (final state)
                if (c != EOF) 
                    ungetc(c, file); //rewind to prev char
                    lexemeBuffer[lexemeIndex] = '\0'; // Finalize the *body*
                    tok = makeToken(CAT_LITERAL, L_KWERDAS_LITERAL, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; //move on to next lexeme
            break;

            // for potential chars
            case S_TITIK_HEAD: // Saw '
                lexemeIndex = 0; // Reset buffer to *not* include the quotes
                if (c == '\'' || c == EOF || c == '\n') {
                    //error or final state
                    if (c != EOF)
                        ungetc(c, file);
                        lexemeBuffer[0] = '\'';
                        lexemeBuffer[1] = '\0';
                        Token tok = makeToken(CAT_DELIMITER, D_SQUOTE, lexemeBuffer, tokenStartLine); 
                        printToken(symbolFileAppend, &tok);
                        currentState = S_START;
                    if(c == '\n') 
                        lineNumber++;
                } else {
                    // this mean character is the next input
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_TITIK_BODY;
                }
                break;
            
            case S_TITIK_BODY: // Saw char 
                if (c == '\'') {
                    currentState = S_TITIK_TAIL; //send to final state
                } else {
                    //error check
                    if (c != EOF) 
                        ungetc(c, file); 
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    //go to next lexeme
                    currentState = S_START;
                }
                break;

            case S_TITIK_TAIL: //previous input: /
                if (c != EOF) 
                    ungetc(c, file);
                lexemeBuffer[lexemeIndex] = '\0';
                tok = makeToken(CAT_LITERAL, L_TITIK_LITERAL, lexemeBuffer, tokenStartLine);
                printToken(symbolFileAppend, &tok);
                currentState = S_START;
                break;
            
            case S_OP_DIVIDE_HEAD: //prev input: /
                if (c == '/') {
                    //comment 
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_COMMENT_SINGLE;
                } else if (c == '*') {
                    // commment 
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_COMMENT_MULTI_HEAD;
                } else {
                    //divide operator
                    if (c != EOF) ungetc(c, file); 
                    lexemeBuffer[lexemeIndex] = '\0'; // Lexeme is just "/"
                    tok = makeToken(CAT_OPERATOR, O_DIVIDE, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; 
                }
                break; 

            case S_COMMENT_SINGLE:
                if (c == '\n' || c == EOF) {
                    //single line
                    if (c != EOF) ungetc(c, file); 
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_COMMENT, C_SINGLE_LINE, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; 
                } else {
                    lexemeBuffer[lexemeIndex++] = (char)c; // Consume
                }
                break;

            case S_COMMENT_MULTI_HEAD:
                if (c == '\n') 
                lineNumber++;
            
                if (c == '*') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_COMMENT_MULTI_TAIL;
                } else if (c == EOF) {
                    // --- FINAL STATE (Error) ---
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine); // Unterminated comment
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; // Will be caught by EOF check
                } else {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    // Stay in S_COMMENT_MULTI_HEAD
                }
                break; 

            case S_COMMENT_MULTI_TAIL: // Saw /*...*
                 if (c == '\n') 
                 lineNumber++;
                 
                if (c == '/') {
                    // --- FINAL STATE (Multi-Line Comment) ---
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_COMMENT, C_MULTI_LINE, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; 
                } else if (c == '*') {
                    lexemeBuffer[lexemeIndex++] = (char)c; // Saw another *, e.g. "/***"
                    // Stay in S_COMMENT_MULTI_TAIL
                } else if (c == EOF) {
                     // --- FINAL STATE (Error) ---
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine); // Unterminated comment
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START;
                } else {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_COMMENT_MULTI_HEAD; // Not a /, go back
                }
                break; 

            case S_OP_AND_HEAD: // Saw &
                if (c == '&') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    //final state
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_OPERATOR, O_AND, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; 
                } else {
                    // --- FINAL STATE (UNKNOWN) ---
                    if (c != EOF) ungetc(c, file);
                    lexemeBuffer[lexemeIndex] = '\0'; // Lexeme is just "&"
                    tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START;
                }
                break;
            
            case S_OP_OR_HEAD: // Saw |
                 if (c == '|') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    // --- FINAL STATE (OR Operator) ---
                    // This is S_OP_OR_TAIL
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_OPERATOR, O_OR, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; 
                } else {
                    // --- FINAL STATE (UNKNOWN) ---
                    if (c != EOF) ungetc(c, file);
                    lexemeBuffer[lexemeIndex] = '\0'; // Lexeme is just "|"
                    tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START;
                }
                break; 

            case S_OP_ASSIGN_HEAD: // Saw =
                if (c == '=') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    // --- FINAL STATE (EQUAL Operator) ---
                    // This is S_OP_ASSIGN_TAIL
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_OPERATOR, O_EQUAL, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; 
                } else {
                    // --- FINAL STATE (ASSIGN Operator) ---
                    if (c != EOF) ungetc(c, file);
                    lexemeBuffer[lexemeIndex] = '\0'; // Lexeme is just "="
                    tok = makeToken(CAT_OPERATOR, O_ASSIGN, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START;
                }
                break; 
            
            case S_OP_NOT_HEAD: // Saw !
                if (c == '=') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    // --- FINAL STATE (NOT_EQUAL Operator) ---
                    // This is S_OP_NOT_TAIL
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_OPERATOR, O_NOT_EQUAL, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; 
                } else {
                    // --- FINAL STATE (NOT Operator) ---
                    if (c != EOF) ungetc(c, file);
                    lexemeBuffer[lexemeIndex] = '\0'; // Lexeme is just "!"
                    tok = makeToken(CAT_OPERATOR, O_NOT, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START;
                }
                break;

            case S_OP_LESS_HEAD: // has <
                if (c == '=') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    // --- FINAL STATE (LESS_EQ Operator) ---
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_OPERATOR, O_LESS_EQ, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; 
                } else {
                    // --- FINAL STATE (LESS Operator) ---
                    if (c != EOF) ungetc(c, file);
                    lexemeBuffer[lexemeIndex] = '\0'; // Lexeme is just "<"
                    tok = makeToken(CAT_OPERATOR, O_LESS, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START;
                }
                break;

            case S_OP_GREATER_HEAD: // Saw >
                if (c == '=') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    // --- FINAL STATE (GREATER_EQ Operator) ---
                    // This is S_OP_GREATER_TAIL
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_OPERATOR, O_GREATER_EQ, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; 
                } else {
                    // --- FINAL STATE (GREATER Operator) ---
                    if (c != EOF) ungetc(c, file);
                    lexemeBuffer[lexemeIndex] = '\0'; // Lexeme is just ">"
                    tok = makeToken(CAT_OPERATOR, O_GREATER, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START;
                }
                break;
            
            case S_UNKNOWN: // Saw '_' or other invalid char
                if (isspace(c) || c == EOF || strchr("+-*/^%&|=!<>(){}[],.;", c)) {
                    // --- FINAL STATE (UNKNOWN) ---
                    if (c != EOF) ungetc(c, file);
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START;
                    if (c == '\n') lineNumber++; // Don't miss this
                } else {
                    // Keep consuming invalid chars
                    lexemeBuffer[lexemeIndex++] = (char)c;
                }
                break;
            
            case S_KEYWORD:
            case S_RESERVE:
            case S_NOISE:
            case S_OP_PLUS:
            case S_OP_MINUS:
            case S_OP_MULTIPLY:
            case S_OP_DIVIDE_TAIL:
            case S_OP_ASSIGN_TAIL:
            case S_OP_NOT_TAIL:
            case S_OP_LESS_TAIL:
            case S_OP_GREATER_TAIL:
            case S_OP_AND_TAIL:
            case S_OP_OR_TAIL:
            case S_DELIMITER:
            case S_DONE:
                fprintf(stderr, "Lexer Error: Entered unreachable state %d on line %d.\n", currentState, lineNumber);
                currentState = S_START;
                break;

        } // end switch(currentState)
    } // end while
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

//Print token as: lexeme | TOKEN_NAME
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
