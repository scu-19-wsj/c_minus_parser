#include "globals.h"
#include "scan.h"
#include "parse.h"
#include "util.h"

/* allocate global variables */
int lineno = 0;
FILE *source;
FILE *listing;

/* allocate and set tracing flags */
int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = TRUE;

int main(int argc, char *argv[])
{

    char pgm[120]; /* source code file name */
    char out[120]; /* output file name */
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        exit(1);
    }
    strcpy(pgm, argv[1]);
    if (strchr(pgm, '.') == NULL)
        strcat(pgm, ".c-");
    source = fopen(pgm, "r");
    if (source == NULL)
    {
        fprintf(stderr, "File %s not found\n", pgm);
        exit(1);
    }

    /* send listing to screen */
    // listing = stdout;
    /* send listing to file */
    listing = fopen(strcat(strtok(argv[1], "."), ".txt"), "w");

    // Scan
    /* fprintf(listing, "CMINUS COMPILATION:\n");

    while (getToken() != ENDFILE)
        ; */

    // Parse
    fprintf(listing, "CMINUS PARSING:\n");
    TreeNode *syntaxTree = parse();
    if (TraceParse)
    {
        fprintf(listing, "\nSyntax tree:\n");
        printTree(syntaxTree);
    }

    fclose(source);
    fclose(listing);
    return 0;
}