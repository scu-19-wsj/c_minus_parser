#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define MAXRESERVED 6

typedef enum
{
    /* book-keeping tokens */
    ENDFILE,
    ERROR,
    /* reserved words */
    IF,
    ELSE,
    INT,
    RETURN,
    VOID,
    WHILE,
    /* multicharacter tokens */
    ID,
    NUM,
    /* special symbols */
    ASSIGN,
    EQ,
    NEQ,
    LT,
    LE,
    GT,
    GE,
    PLUS,
    MINUS,
    TIMES,
    OVER,
    LPAREN,
    RPAREN,
    SEMI,
    COMMA,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE

} TokenType;

extern FILE *source;  /* source code text file */
extern FILE *listing; /* listing output text file */

extern int lineno; /* source line number for listing */

typedef enum
{
    StmtK,
    ExpK
} NodeKind;
typedef enum
{
    IfK,
    WhileK,  // While
    ReturnK, // Return
    AssignK, // Assign
    ParamsK,
    ParamK,
    FuncK,
    Var_DeclK,
    CompK, // compond_stmt -> { {var_declaration} (statement) }
} StmtKind;
typedef enum
{
    OpK, // Op
    ConstK,
    IdK,
    IntK,
    VoidK,
    Arry_ElemK,
    CallK,
    Arry_DeclK,
    ArgsK,
} ExpKind;

#define MAXCHILDREN 4
typedef struct treeNode
{
    struct treeNode *child[MAXCHILDREN];
    struct treeNode *sibling;
    int lineno;
    NodeKind nodekind;
    union
    {
        StmtKind stmt;
        ExpKind exp;
    } kind;
    union
    {
        TokenType op;
        int val;
        char *name;
    } attr;
} TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* EchoSource = TRUE causes the source program to
 * be echoed to the listing file with line numbers
 * during parsing
 */
extern int EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern int TraceParse;

#endif