/* abstract-tree.h */


typedef enum {
  Program,
  DeclStruct,
  VarDeclList,
  VarDec,
  FuncDecList,
  FuncDec,
  ArgList,
  Pointer,
  ParamList,
  Param,
  Body,
  StmtList,
  Return,
  Add,
  Sub,
  Mult,
  Div,
  Mod,
  Assign,
  Or,
  And,
  Equal,
  Different,
  Inf,
  Sup,
  InfOrEq,
  SupOrEq,
  IntLiteral,
  CharLiteral,
  Identifier,
  Decl,
  Int,
  Char,
  Void,
  Adress,
  Opposite,
  ReadE,
  ReadC,
  Print,
  While,
  If,
  Else,
  Positif,
  Negatif,
  Error,
  ValueOf
  /* and allother node labels */
  /* The list must coincide with the strings in abstract-tree.c */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
} Kind;




typedef struct Node {
  Kind kind;
  union {
    int integer;
    char character;
    char identifier[64];
  } u;
  struct Node *firstChild, *nextSibling;
  int pos_line;
} Node;

  

Node *makeNode(Kind kind);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node*node);
void printTree(Node *node);

#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling
#define FOURTHCHILD(node) node->firstChild->nextSibling->nextSibling->nextSibling
