#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

static TokenType token; /* holds current token */

/* function prototypes for recursive calls */
static TreeNode *program(void);
static TreeNode *declaration(void);
static void declaration_(TreeNode **, TreeNode *, TreeNode *);
static TreeNode *type_specifier(void);

static TreeNode *var_decl(void);
static void var_decl_(TreeNode **, TreeNode *, TreeNode *);

static TreeNode *param_list(void);
static TreeNode *param_list1(void);
static TreeNode *param_list2(void);
static TreeNode *param(void);

static TreeNode *stmt(void);
static TreeNode *exp_stmt(void);
static TreeNode *return_stmt();
static TreeNode *selection_stmt(void);
static TreeNode *iteration_stmt(void);
static TreeNode *compound_stmt(void);

static TreeNode *assign_stmt(void); // ?

static TreeNode *exp(void);
static TreeNode *simple_exp(TreeNode *);
static TreeNode *additive_exp(void);
static TreeNode *term(void);
static TreeNode *factor(void);
static void factor_(TreeNode **, TreeNode *);
static TreeNode *args(void);

static void syntaxError(char *message)
{
    fprintf(listing, "\n>>> ");
    fprintf(listing, "Syntax error at line %d: %s", lineno, message);
    // Error = TRUE;
}

static void match(TokenType expected)
{
    if (token == expected)
        token = getToken();
    else
    {
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        fprintf(listing, "      ");
    }
}

/* program ->  declaration  { declaration } */
TreeNode *program(void)
{
    TreeNode *t = declaration();
    TreeNode *p = t;
    while (token != ENDFILE)
    {
        TreeNode *q;
        q = declaration();
        if (q != NULL)
        {
            p->sibling = q;
            p = q;
        }
    }
    return t;
}

/*  type_specifier ->  int  |  void  */
TreeNode *type_specifier(void)
{
    TreeNode *t = NULL;
    switch (token)
    {
    case INT:
        t = newExpNode(IntK);
        match(INT);
        break;
    case VOID:
        t = newExpNode(VoidK);
        match(VOID);
        break;
    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }
    return t;
}

/* FuncK  or Var_DeclK */
/* declaration ->  type_specifier  ID  declaration’ */
TreeNode *declaration(void)
{
    TreeNode *t = NULL;
    TreeNode *tS = type_specifier();
    TreeNode *idNode = newExpNode(IdK);
    if (idNode != NULL && token == ID)
        idNode->attr.name = copyString(tokenString);
    match(ID);

    declaration_(&t, tS, idNode);

    return t;
}

/* declaration’ ->  ;  |  [ NUM ];  |  ( params )  compound_stmt  */
void declaration_(TreeNode **t, TreeNode *tyS, TreeNode *idNode)
{
    switch (token)
    {
    case SEMI:
        (*t) = newStmtNode(Var_DeclK);
        (*t)->child[0] = tyS;
        (*t)->child[1] = idNode;
        match(SEMI);
        break;
    case LBRACKET:
        (*t) = newStmtNode(Var_DeclK);
        (*t)->child[0] = tyS;

        match(LBRACKET);
        TreeNode *arrayDecl = newExpNode(Arry_DeclK);
        arrayDecl->child[0] = idNode;
        TreeNode *constNode = newExpNode(ConstK);
        if (constNode != NULL && token == NUM)
            constNode->attr.val = atoi(tokenString);
        arrayDecl->child[1] = constNode;
        (*t)->child[1] = arrayDecl;
        match(RBRACKET);

        match(SEMI);
        break;
    case LPAREN:
        (*t) = newStmtNode(FuncK);
        match(LPAREN);
        TreeNode *paramsNode = param_list();

        match(RPAREN);
        TreeNode *compNode = compound_stmt();
        if (t != NULL && compNode != NULL && paramsNode != NULL)
        {
            (*t)->child[0] = tyS;
            (*t)->child[1] = idNode;
            (*t)->child[2] = paramsNode;
            (*t)->child[3] = compNode;
        }
        break;

    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }
}

/* var_decl ->  type_specifier  ID  var_decl_
var_decl_ ->  ;  |  [ NUM ];  */

TreeNode *var_decl()
{
    TreeNode *t = NULL;
    TreeNode *tS = type_specifier();
    TreeNode *idNode = newExpNode(IdK);
    if (idNode != NULL && token == ID)
        idNode->attr.name = copyString(tokenString);
    match(ID);

    var_decl_(&t, tS, idNode);

    return t;
}

void var_decl_(TreeNode **t, TreeNode *tyS, TreeNode *idNode)
{
    switch (token)
    {
    case SEMI:
    case COMMA:
    case RPAREN:
        (*t) = newStmtNode(Var_DeclK);
        (*t)->child[0] = tyS;
        (*t)->child[1] = idNode;
        match(token);
        break;
    case LBRACKET:
        (*t) = newStmtNode(Var_DeclK);
        (*t)->child[0] = tyS;

        match(LBRACKET);
        TreeNode *arrayDecl = newExpNode(Arry_DeclK);
        arrayDecl->child[0] = idNode;
        TreeNode *constNode = newExpNode(ConstK);
        if (constNode != NULL && token == NUM)
            constNode->attr.val = atoi(tokenString);
        arrayDecl->child[1] = constNode;
        (*t)->child[1] = arrayDecl;
        match(RBRACKET);

        match(SEMI);
        break;

    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }
}

/* param_list -> param_list1  |  param_list2 */
TreeNode *param_list()
{
    TreeNode *t = newStmtNode(ParamsK);
    switch (token)
    {
    case VOID:
        t->child[0] = param_list1();
        break;
    case INT:
        t->child[0] = param_list2();
        break;
    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }

    return t;
}

/* param_list1->  void  [ ID  [ [  ] ]  {  , param  } ] */
TreeNode *param_list1()
{
    TreeNode *t = newStmtNode(ParamK);
    TreeNode *voidNode = newExpNode(VoidK);
    if (t != NULL && voidNode != NULL)
        t->child[0] = voidNode;
    match(VOID);
    if (token == ID)
    {
        TreeNode *idNode = newExpNode(IdK);
        idNode->attr.name = copyString(tokenString);
        if (idNode != NULL && t != NULL)
            t->child[1] = idNode;
        match(ID);
        if (token == LBRACKET)
        {
            match(LBRACKET);
            TreeNode *empty = newExpNode(IdK);
            empty->attr.name = "";
            t->child[2] = empty;
            match(RBRACKET);
        }
        while (token == COMMA)
        {
            match(COMMA);
            TreeNode *p = param();
            t->sibling = p;
            t = p;
        }
    }
    return t;
}

/* param_list2->  int  ID  [ [  ] ]  {  , param  } */
TreeNode *param_list2()
{
    TreeNode *t = newStmtNode(ParamK);
    TreeNode *intNode = newExpNode(IntK);
    if (t != NULL && intNode != NULL)
        t->child[0] = intNode;
    match(INT);

    TreeNode *idNode = newExpNode(IdK);
    idNode->attr.name = copyString(tokenString);
    if (idNode != NULL && t != NULL)
        t->child[1] = idNode;
    match(ID);

    if (token == LBRACKET)
    {
        match(LBRACKET);
        TreeNode *empty = newExpNode(IdK);
        empty->attr.name = "";
        t->child[2] = empty;
        match(RBRACKET);
    }

    while (token == COMMA)
    {
        match(COMMA);
        TreeNode *p = param();
        t->sibling = p;
        t = p;
    }
    return t;
}

/* param ->  type_specifier ID  [ [  ] ] */
TreeNode *param()
{
    TreeNode *t = newStmtNode(ParamK);
    TreeNode *tyS = type_specifier();
    TreeNode *idNode = newExpNode(IdK);
    if (idNode != NULL && token == ID)
    {
        idNode->attr.name = copyString(tokenString);
        match(ID);
    }

    if (t != NULL && tyS != NULL && idNode != NULL)
    {
        t->child[0] = tyS;
        t->child[1] = idNode;
    }

    if (token == LBRACKET)
    {
        match(LBRACKET);
        TreeNode *empty = newExpNode(IdK);
        if (empty != NULL)
            empty->attr.name = "";
        match(RBRACKET);
        t->child[2] = empty;
    }
    return t;
}

/* statement -> expression_stmt  |  compound_stmt  |  selection_stmt  |
                   iteration_stmt  |  return_stmt */
TreeNode *stmt(void)
{
    TreeNode *t = NULL;
    switch (token)
    {
    case IF:
        t = selection_stmt();
        break;
    case WHILE:
        t = iteration_stmt();
        break;
    case RETURN:
        t = return_stmt();
        break;
    case ID:
    case LPAREN:
    case NUM:
        t = exp_stmt();
        break;

    case LBRACE:
        t = compound_stmt();
        break;
    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }
    return t;
}
/*
 * selection_stmt -> if ( expression ) statement [ else statement ]
 */
TreeNode *selection_stmt(void)
{
    TreeNode *t = newStmtNode(IfK);
    match(IF);
    match(LPAREN);
    if (t != NULL)
        t->child[0] = exp();
    match(RPAREN);
    if (t != NULL)
        t->child[1] = stmt();
    if (token == ELSE)
    {
        match(ELSE);
        if (t != NULL)
            t->child[2] = stmt();
    }
    return t;
}

/* iteration_stmt -> while ( expression ) statement  */
TreeNode *iteration_stmt(void)
{
    TreeNode *t = newStmtNode(WhileK);
    match(WHILE);
    match(LPAREN);
    if (t != NULL)
        t->child[0] = exp();
    match(RPAREN);
    TreeNode *stmtNode = NULL;
    if (t != NULL)
    {
        stmtNode = stmt();
    }
    if (t != NULL && stmtNode != NULL)
        t->child[1] = stmtNode;

    return t;
}

/* return_stmt ->  return  [  expression  ]; */
TreeNode *return_stmt(void)
{
    TreeNode *t = newStmtNode(ReturnK);
    match(RETURN);
    if (token == ID || token == LPAREN || token == NUM)
        t->child[0] = exp();

    match(SEMI);
    return t;
}

/* expression_stmt -> expression ;  |  ; */
TreeNode *exp_stmt(void)
{
    TreeNode *t = NULL;
    switch (token)
    {
    case ID:
    case LPAREN:
    case NUM:
        t = exp();
        match(SEMI);
        break;
    case SEMI:
        match(SEMI);
        break;
    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }

    return t;
}

/* compound_stmt -> {  { var_declaration }  { statement }  } */
TreeNode *compound_stmt()
{
    TreeNode *t = newStmtNode(CompK);
    match(LBRACE);
    TreeNode *p = t->child[0], *q = NULL;
    while (token != RBRACE)
    {
        switch (token)
        {
        case VOID:
        case INT:
            q = var_decl();
            if (q != NULL)
            {
                if (t->child[0] == NULL)
                    t->child[0] = p = q;
                else
                {
                    p->sibling = q;
                    p = q;
                }
            }
            break;
        case SEMI:
        case ID:
        case LPAREN:
        case NUM:
        case LBRACE:
        case IF:
        case WHILE:
        case RETURN:
            q = stmt();
            if (q != NULL)
            {
                if (t->child[0] == NULL)
                    t->child[0] = p = q;
                else
                {
                    p->sibling = q;
                    p = q;
                }
            }
            break;
        default:
            syntaxError("unexpected token -> ");
            printToken(token, tokenString);
            token = getToken();
            break;
        }
    }

    match(RBRACE);
    return t;
}

/* factor ->  ( expression )  |  ID  factor’  |  NUM */
/* factor’ ->  [expression]  |  ( args )  |  ε */
TreeNode *factor(void)
{
    TreeNode *t = NULL;
    TreeNode *idNode = NULL;
    switch (token)
    {
    case LPAREN:
        match(LPAREN);
        t = exp();
        match(RPAREN);
        break;
    case ID: /* Array_ElemK || CallK */
        idNode = newExpNode(IdK);
        if (idNode != NULL)
        {
            idNode->attr.name = copyString(tokenString);
            match(ID);
        }
        switch (token)
        {
        case TIMES:
        case OVER:
        case MINUS:
        case PLUS:
        case LE:
        case LT:
        case GT:
        case GE:
        case EQ:
        case NEQ:
        case SEMI:
        case RPAREN:
        case COMMA:
            t = idNode;
            break;
        case LPAREN:
        case LBRACKET:
            factor_(&t, idNode);
            break;
        default:
            syntaxError("unexpected token -> ");
            printToken(token, tokenString);
            token = getToken();
            break;
        }
        break;

    case NUM:
        t = newExpNode(ConstK);
        if (t != NULL && token == NUM)
            t->attr.val = atoi(tokenString);
        match(NUM);
        break;

    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }
    return t;
}

/* factor’ ->  [expression]  |  ( args )  |  ε */
void factor_(TreeNode **t, TreeNode *idNode)
{
    switch (token)
    {
    case LBRACKET:
        match(LBRACKET);
        (*t) = newExpNode(Arry_ElemK);
        (*t)->child[0] = idNode;
        (*t)->child[1] = exp();
        match(RBRACKET);
        break;
    case LPAREN:
        match(LPAREN);
        (*t) = newExpNode(CallK);
        (*t)->child[0] = idNode;
        TreeNode *temp = args();
        if (temp != NULL)
            (*t)->child[1] = temp;
        match(RPAREN);
        break;
    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }
}

/* expression -> var = expression  |  simple_expression */
TreeNode *exp(void)
{
    TreeNode *t = NULL;
    TreeNode *idNode = NULL;
    TreeNode *expNode = NULL;
    TreeNode *arrayElemNode = NULL;
    TreeNode *numNode = NULL;
    TreeNode *argsNode = NULL;
    TreeNode *callNode = NULL;
    switch (token)
    {
    case ID:
        idNode = newExpNode(IdK);
        idNode->attr.name = copyString(tokenString);
        match(ID);

        if (token == LBRACKET)
        {
            arrayElemNode = newExpNode(Arry_ElemK);
            match(LBRACKET);
            arrayElemNode->child[0] = idNode;
            expNode = exp();
            arrayElemNode->child[1] = expNode;
            match(RBRACKET);
            if (token == ASSIGN)
            {
                t = newStmtNode(AssignK);
                match(ASSIGN);
                t->child[0] = arrayElemNode;
                t->child[1] = exp();
            }
            else if (token == OVER || token == TIMES || token == MINUS || token == PLUS || relop(token) || token == SEMI || token == RPAREN || token == COMMA)
            {
                t = simple_exp(arrayElemNode);
            }
        }
        else if (token == LPAREN)
        {
            match(LPAREN);
            argsNode = args();
            match(RPAREN);
            callNode = newExpNode(CallK);
            callNode->child[0] = idNode;
            if (argsNode != NULL)
                callNode->child[1] = argsNode;
            t = simple_exp(callNode);
        }
        else
        {
            if (token == ASSIGN)
            {
                t = newStmtNode(AssignK);
                match(ASSIGN);
                t->child[0] = idNode;
                t->child[1] = exp();
            }
            else if (token == OVER || token == TIMES || token == MINUS || token == PLUS || relop(token) || token == SEMI || token == RPAREN || token == COMMA)
            {
                t = simple_exp(idNode);
            }
        }
        break;
    case LPAREN:
        match(LPAREN);
        expNode = exp();
        match(RPAREN);
        t = simple_exp(expNode);
        break;
    case NUM:
        numNode = newExpNode(ConstK);
        if (numNode != NULL && token == NUM)
            numNode->attr.val = atoi(tokenString);
        t = simple_exp(numNode);
        match(NUM);
        break;
    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }
    return t;
}

TreeNode *simple_exp(TreeNode *addNode)
{
    TreeNode *t = NULL;
    TreeNode *addExpNode = NULL;
    TreeNode *termNode = NULL;
    if (addNode != NULL)
    {
        while (token == OVER || token == TIMES)
        {
            termNode = newExpNode(OpK);
            termNode->attr.op = token;
            match(token);
            termNode->child[0] = addNode;
            termNode->child[1] = term();
        }
        if (termNode == NULL)
            termNode = addNode;
        while (token == MINUS || token == PLUS)
        {
            addExpNode = newExpNode(OpK);
            addExpNode->attr.op = token;
            match(token);
            addExpNode->child[0] = termNode;
            addExpNode->child[1] = additive_exp();
        }
        if (addExpNode == NULL)
            addExpNode = addNode;
        if (relop(token))
        {
            t = newExpNode(OpK);
            t->attr.op = token;
            match(token);
            t->child[0] = addExpNode;
            t->child[1] = additive_exp();
        }

        if (t == NULL)
            t = addNode;
    }
    return t;
}

/* additive_expression ->  term  { addop term } */
TreeNode *additive_exp()
{
    TreeNode *t = NULL;
    TreeNode *termNode = term();
    if (token == PLUS || token == MINUS)
    {
        t = newExpNode(OpK);
        t->attr.op = token;
        match(token);
        t->child[0] = termNode;
        t->child[1] = additive_exp();
    }
    else if (relop(token) || token == RPAREN || token == SEMI)
    {
        t = termNode;
    }
    return t;
}

TreeNode *term(void)
{
    TreeNode *t = NULL;
    TreeNode *factorNOde = factor();
    if (token == OVER || token == TIMES)
    {
        t = newExpNode(OpK);
        t->attr.op = token;
        match(token);
        t->child[0] = factorNOde;
        t->child[1] = term();
    }
    else if (token == MINUS || token == PLUS || token == RPAREN || token == SEMI)
    {
        t = factorNOde;
    }
    return t;
}

/* args -> expression   {  , expression }   |  empty */
TreeNode *args(void)
{
    TreeNode *t = NULL;
    switch (token)
    {
    case RPAREN:
        return NULL;
        break;
    case ID:
    case LPAREN:
    case NUM:
        t = newExpNode(ArgsK);
        t->child[0] = exp();
        TreeNode *p = t->child[0];
        while (token == COMMA)
        {
            match(COMMA);
            p->sibling = exp();
            p = p->sibling;
        }
        break;
    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }

    return t;
}

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly
 * constructed syntax tree
 */
TreeNode *parse(void)
{
    TreeNode *t;
    token = getToken();
    t = program();
    if (token != ENDFILE)
        syntaxError("Code ends before file\n");
    return t;
}