// What category this token belongs to
typedef enum {
    CAT_KEYWORD,
    CAT_RESERVED,
    CAT_NOISEWORD,
    CAT_OPERATOR,
    CAT_DELIMITER,
    CAT_LITERAL,
    CAT_COMMENT,
    CAT_UNKNOWN
} TokenCategory;

typedef enum {
    K_ANI,
    K_TANIM,
    K_PARA,
    K_HABANG,
    K_KUNG,
    K_KUNDI,
    K_KUNDIMAN,
    K_GAWIN,
    K_TIBAG,
    K_TULOY,
    K_PANGKAT,
    K_STATIK,
    K_PRIBADO,
    K_PROTEKTADO,
    K_PUBLIKO
} KeywordToken;

typedef enum {
    R_TAMA,
    R_MALI,
    R_UGAT,    // Main
    R_BALIK,   // Return
    R_BILANG,
    R_KWERDAS,
    R_TITIK,
    R_LUTANG,
    R_BULYAN,
    R_DOBLE,
    R_WALA
} ReservedToken;

typedef enum {
    N_NG,
    N_AY,
    N_BUNGA,
    N_WAKAS,
    N_SA,
    N_ANG,
    N_MULA,
    N_ITAKDA
} NoiseWordToken;

typedef enum {
    O_PLUS,        // +
    O_MINUS,       // -
    O_MULTIPLY,    // *
    O_DIVIDE,      // /
    O_POW,        // ^
    O_MODULO,      // %
    O_ASSIGN,      // =
    O_EQUAL,       // ==
    O_NOT_EQUAL,   // !=
    O_LESS,        // <
    O_GREATER,     // >
    O_LESS_EQ,     // <=
    O_GREATER_EQ,  // >=
    O_AND,         // &&
    O_OR,          // ||
    O_NOT          // !
} OperatorToken;

typedef enum {
    D_LPAREN,      // (
    D_RPAREN,      // )
    D_LBRACE,      // {
    D_RBRACE,      // }
    D_LBRACKET,    // [
    D_RBRACKET,    // ]
    D_COMMA,       // ,
    D_SEMICOLON,   // ;
    D_COLON,       // :
    D_DOT,         // .
    D_QUOTE,       // "
    D_SQUOTE       // '
} DelimiterToken;

typedef enum {
    L_IDENTIFIER,
    L_INT_LITERAL,
    L_FLOAT_LITERAL,
    L_STRING_LITERAL,
    L_CHAR_LITERAL,
    L_BOOL_LITERAL  // (Tama, Mali)
} LiteralToken;

typedef enum {
    C_SINGLE_LINE,  // //
    C_MULTI_LINE    // /* */
} CommentToken;

typedef struct {
    TokenCategory category;
    int tokenValue;          // Holds actual enum value from KeywordToken, OperatorToken, etc.
    char* lexeme;        // The actual string from the source code
    int lineNumber;    // Line number in source code
} Token;
