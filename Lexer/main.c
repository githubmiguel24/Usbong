//Contains main() function.
//Handles file input
#include <stdio.h>
#include <stdlib.h>
#include "tokens.h"
#include <ctype.h>

void lexer(FILE *file); 
void digitChecker(FILE *file, int firstCh);
void identifierChecker();
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

// Simple lexer function that reads characters from the file and prints them
void lexer (FILE *file) {
   int current  = fgetc(file);
    while (current != EOF)
    {
        char c = (char) current;
        if(c == ';') {
            printf("SEMICOLON TOKEN\n");
        } else if (c == '{') {
            printf("LBRACE TOKEN\n");
        } else if (c == '}') {
            printf("RBRACE TOKEN\n");
        } else if (c== '(') {
            printf("OPEN PAREN TOKEN\n");
        } else if (c== ')') {
            printf("CLOSE PAREN TOKEN\n");
        } else if (isspace(c)) {
            // Ignore whitespace
        } else if (isdigit(c)){
            digitChecker(file, current);
        } else if(isalpha(c)) {
            identifierChecker(file, current);
        } else {
            printf("UNKNOWN TOKEN: %c\n", c);
        }
        current = fgetc(file);
    }
       
}

void digitChecker(FILE *file, int firstCh) {
    char c = firstCh;
    char digit[100];
    int x = 0;
    int currentChar;

    // Make sure first character is digit
    if (!isdigit(c)) return;

    // Keep reading digits
    while (isdigit(c)) {
        digit[x++] = c;
        currentChar = fgetc(file);
        c = (char) currentChar;
    }

    // Null-terminate the string
    digit[x] = '\0';

    // Push back the non-digit character so it can be processed by the next logic
    ungetc(c, file);

    printf("DIGIT TOKEN: %s\n", digit);
}   

void identifierChecker(FILE *file, int firstCh) {
    int c = firstCh;                     // use int for fgetc/EOF
    char identifier[100];
    int x = 0;

    if (c == EOF) return;
    if (!isalpha((unsigned char)c)) return;

    // Read and collect identifier chars, starting with firstCh
    while (c != EOF && (isalnum((unsigned char)c) || c == '_') && x < (int)sizeof(identifier) - 1) {
        identifier[x++] = (char)c;
        c = fgetc(file);     // get next char (may be EOF or a non-identifier)
    }

    identifier[x] = '\0';

    // push back the non-identifier char so lexer can handle it
    if (c != EOF) ungetc(c, file);

    printf("IDENTIFIER TOKEN: %s\n", identifier);
}