//Contains main() function.
//Handles file input and calls the lexer
#include <stdio.h>
#include "lexer.h"
#include "symbol_table.h"

int main(int argc, char* argv[]) {
    // Check for correct number of arguments
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source-file>" << std::endl;
        return 1;
    }

    const char* sourceFile = argv[1];

    // Open the source file
    std::ifstream file(sourceFile);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << sourceFile << std::endl;
        return 1;
    }

    // Create a lexer instance
    Lexer lexer(file);

    // Tokenize the input file
    Token token;
    do {
        token = lexer.getNextToken();
        std::cout << token.toString() << std::endl; // Print each token
    } while (token.type != TokenType::EOF_TOKEN);

    file.close();
    return 0;



}

