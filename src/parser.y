%{
    #include <stdio.h>
    #include <string.h>

    #include <ctype.h>
    #include <string.h>
    #include <getopt.h>
    
    int yylex();

    int parse = 0;
    int result;
    void yyerror(char *s);
    
    extern int pos_line;
    extern int pos_char;
%}

%code requires {
    #include "writeNasm.h"
}

%union{
    char ident[100];
    char error[200];
    char type[5];
    char order[3];
    char equal[3];
    char sign;
    char character;
    int integer;
    Node *node;  
}



%{
    Node *abstractTree = NULL;
%}


%token STRUCT
%token <character>CHARACTER
%token <integer>NUM
%token <ident>IDENT
%token <type>TYPE
%token EQ
%token <order>ORDER
%token <sign>ADDSUB
%token DIVSTAR
%token OR
%token AND
%token READC
%token READE
%token PRINT
%token WHILE
%token IF
%nonassoc "then"
%nonassoc ELSE
%token RETURN
%token VOID


%type <node> Decls DeclStruct DeclVars Declarateurs DeclFoncts DeclFonct EnTeteFonct Parametres ListTypVar Corps SuiteInstr Instr LValue Exp TB FB M E T F Arguments ListExp
%type <node> Prog 

%%
Prog: Decls DeclFoncts  {               $$ = makeNode(Program); 
                                        addChild($$, $1); 
                                        Node *listFonc = makeNode(FuncDecList); addChild(listFonc, $2);
                                        addChild($$, listFonc); abstractTree = $$;}
    ; 

Decls: Decls TYPE Declarateurs ';'  {   Node *tmp;
                                         if(!strcmp($<type>2, "int")){tmp = makeNode(Int);};
                                         if(!strcmp($<type>2, "char")){tmp = makeNode(Char);};
                                         addChild($$, tmp);
                                         addChild(tmp, $3);
                                         $$ = $1; }
            |  Decls DeclStruct     {   addChild($$, $2); 
                                        $$ = $1; }   
            |                       {$$ = makeNode(VarDeclList);} ;

DeclStruct: STRUCT IDENT '{' DeclVars '}' ';' { $$ = makeNode(DeclStruct);
                                                Node *tmp_ident = makeNode(Identifier); 
                                                strcpy(tmp_ident->u.identifier,$2);
                                                addChild($$,tmp_ident);
                                                addSibling(tmp_ident, $4);
                                                }
            | STRUCT IDENT Declarateurs ';'  { $$ = makeNode(DeclStruct);
                                                Node *tmp_ident = makeNode(Identifier); 
                                                strcpy(tmp_ident->u.identifier,$2);
                                                addChild($$,tmp_ident);
                                                addSibling(tmp_ident, $3);
                                                }
            ;

DeclVars:
       DeclVars TYPE Declarateurs ';'   {

                                         Node *tmp;
                                         if(!strcmp($<type>2, "int")){tmp = makeNode(Int);};
                                         if(!strcmp($<type>2, "char")){tmp = makeNode(Char);};
                                         addChild($$, tmp);
                                         addChild(tmp, $3);
                                         $$ = $1;
                                        }
    |                                   {$$ = makeNode(VarDeclList);};

Declarateurs:
       Declarateurs ',' IDENT           {
                                         Node *tmp_node = makeNode(VarDec);
                                         Node *tmp_ident = makeNode(Identifier); 
                                         addSibling($$, tmp_node);
                                         addChild(tmp_node,tmp_ident);
                                         strcpy(tmp_ident->u.identifier,$3);
                                         }
    |  Declarateurs ',' '*' IDENT       {Node *tmp_node = makeNode(VarDec);
                                         Node *tmp = makeNode(Pointer);
                                         Node *tmp_ident = makeNode(Identifier); 
                                         addSibling(tmp, tmp_ident);
                                         addSibling($$, tmp_node);
                                         addChild(tmp_node,tmp);
                                         strcpy(tmp_ident->u.identifier,$4);                                         
                                        } 
    |  IDENT                            {$$ = makeNode(VarDec); Node *tmp = makeNode(Identifier); strcpy(tmp->u.identifier,$1); addChild($$,tmp);}
    |  '*' IDENT                        {$$ = makeNode(VarDec); Node *tmp_node = makeNode(Identifier); Node *tmp = makeNode(Pointer); strcpy(tmp_node->u.identifier,$2); addSibling(tmp, tmp_node); addChild($$,tmp);}
    ;

DeclFoncts:
       DeclFoncts DeclFonct             {$$ = $1; addSibling($1, $2);}
    |  DeclFonct                        {$$ = $1;}
    |  error                            {$$ = makeNode(Error); yyclearin;} 
    ;

DeclFonct:
       EnTeteFonct Corps                {$$ = makeNode(FuncDec); addChild($$, $1); addChild($$, $2);}
    ;
     
EnTeteFonct:
       TYPE IDENT '(' Parametres ')'    {if(!strcmp($<type>1, "int")){$$ = makeNode(Int);};
                                         if(!strcmp($<type>1, "char")){$$ = makeNode(Char);};
                                         Node *tmp = makeNode(Identifier); strcpy(tmp->u.identifier,$2); 
                                         addSibling($$, tmp);
                                         addSibling($$, $4);
                                        }
    |  TYPE '*' IDENT '(' Parametres ')'    {$$ = makeNode(Pointer);
                                             if(!strcmp($<type>1, "int")){addChild($$, makeNode(Int));};
                                             if(!strcmp($<type>1, "char")){addChild($$, makeNode(Char));};
                                             Node *tmp = makeNode(Identifier); strcpy(tmp->u.identifier,$3); 
                                             addSibling($$, tmp);
                                             addSibling($$, $5);}
    |  VOID IDENT '(' Parametres ')'    {$$ = makeNode(Void);
                                         Node *tmp = makeNode(Identifier); strcpy(tmp->u.identifier,$2); 
                                         addSibling($$, tmp);
                                         addSibling($$, $4);}
    |  error                            {$$ = makeNode(Error); yyclearin;}
    ;

Parametres:
       VOID                             {$$ = makeNode(ParamList);}               
    |  ListTypVar                       {$$ = makeNode(ParamList); addChild($$, $1); }
    ;

ListTypVar:
       ListTypVar ',' TYPE IDENT        {Node *res = makeNode(Param);
                                         Node *tmp_type = NULL;
                                         if(!strcmp($<type>3, "int")){tmp_type = makeNode(Int);};
                                         if(!strcmp($<type>3, "char")){tmp_type = makeNode(Char);};
                                         Node *tmp = makeNode(Identifier); strcpy(tmp->u.identifier,$4);
                                         addChild(tmp_type, tmp);
                                         addChild(res, tmp_type);
                                         addSibling($1, res);
                                         $$ = $1;}
    |  ListTypVar ',' TYPE '*' IDENT    {Node *res = makeNode(Param);
                                         Node *tmp_type = makeNode(Pointer);
                                         if(!strcmp($<type>3, "int")){addChild(tmp_type,makeNode(Int));};
                                         if(!strcmp($<type>3, "char")){addChild(tmp_type,makeNode(Char));};
                                         Node *tmp = makeNode(Identifier); strcpy(tmp->u.identifier,$5);
                                         addChild(tmp_type, tmp);
                                         addChild(res, tmp_type);
                                         addSibling($1, res);
                                         $$ = $1;}
    |  TYPE IDENT                       {$$ = makeNode(Param);
                                         Node *tmp_type = NULL;
                                         if(!strcmp($<type>1, "int")){tmp_type = makeNode(Int);};
                                         if(!strcmp($<type>1, "char")){tmp_type = makeNode(Char);};
                                         Node *tmp = makeNode(Identifier); strcpy(tmp->u.identifier,$2);
                                         addChild(tmp_type, tmp);
                                         addChild($$, tmp_type);
                                         }
    |  TYPE '*' IDENT                   {$$ = makeNode(Param);
                                         Node *tmp_type = makeNode(Pointer);
                                         if(!strcmp($<type>1, "int")){addChild(tmp_type,makeNode(Int));};
                                         if(!strcmp($<type>1, "char")){addChild(tmp_type,makeNode(Char));};
                                         Node *tmp = makeNode(Identifier); strcpy(tmp->u.identifier,$3);
                                         addChild(tmp_type, tmp);
                                         addChild($$, tmp_type);}
    |  error                            {$$ = makeNode(Error); yyclearin;}
    ;

Corps: '{' DeclVars SuiteInstr '}'      {$$ = makeNode(Body); addChild($$, $2); addChild($$, $3);}
    ;

SuiteInstr:
       SuiteInstr Instr                 {if($1->kind == StmtList){
                                            addChild($$, $2);}
                                        }
    |                                   {$$ = makeNode(StmtList);}
    ;

Instr:
       LValue '=' Exp ';'               {$$= makeNode(Assign); addChild($$, $1); addChild($$, $3);}
    |  READE '(' IDENT ')' ';'          {$$ = makeNode(ReadE); Node *tmp = makeNode(Identifier); strcpy(tmp->u.identifier,$3); addChild($$, tmp);}
    |  READC '(' IDENT ')' ';'          {$$ = makeNode(ReadC); Node *tmp = makeNode(Identifier); strcpy(tmp->u.identifier,$3); addChild($$, tmp);}
    |  PRINT '(' Exp ')' ';'            {$$ = makeNode(Print); addChild($$, $3);}
    |  IF '(' Exp ')' Instr             %prec "then" {$$ = makeNode(If); addChild($$, $3); addChild($$, $5);} 
    |  IF '(' Exp ')' Instr ELSE Instr  {Node *tmp_if = makeNode(If); Node *tmp_else = makeNode(Else); addChild(tmp_if, $3); addChild(tmp_if, $5); addChild(tmp_else, $7); 
                                         addSibling(tmp_if, tmp_else); $$ = tmp_if; }
    |  WHILE '(' Exp ')' Instr          {$$ = makeNode(While); addChild($$, $3); addSibling($3, $5);}
    |  IDENT '(' Arguments  ')' ';'     {$$ = makeNode(Identifier); strcpy($$->u.identifier,$1); addChild($$, $3);}
    |  RETURN Exp ';'                   {$$ =makeNode(Return); addChild($$, $2);}
    |  RETURN ';'                       {$$ =makeNode(Return);}
    |  '{' SuiteInstr '}'               {$$ = $2;}
    |  ';'                              {$$ = $$;}
    |  error                            {$$ = makeNode(Error); yyclearin;} 
    ;

Exp :  Exp OR TB                        {$$ = makeNode(Or); addChild($$, $1); addChild($$, $3);}
    |  TB                               {$$ = $1;}
    |  error                            {$$ = makeNode(Error); yyclearin;} 
    ;

TB  :  TB AND FB                        {$$ = makeNode(And); addChild($$, $1); addChild($$, $3);}
    |  FB                               {$$ = $1;}
    ;
FB  :  FB EQ M                          {if(!strcmp($<equal>2, "==")){$$ = makeNode(Equal);};
                                         if(!strcmp($<equal>2, "!=")){$$ = makeNode(Different);}; 
                                         addChild($$, $1); addChild($$, $3);}
    |  M                                {$$ = $1;}
    ;
M   :  M ORDER E                        {if(!strcmp($<order>2, "<=")){$$ = makeNode(InfOrEq);};
                                         if(!strcmp($<order>2, ">=")){$$ = makeNode(SupOrEq);};
                                         if(!strcmp($<order>2, "<")){$$ = makeNode(Inf);};
                                         if(!strcmp($<order>2, ">")){$$ = makeNode(Sup);};
                                         addChild($$, $1); addChild($$, $3);}
    |  E                                {$$ = $1;}
    ;
E   :  E ADDSUB T                       {if($2 == '+'){$$ = makeNode(Add);}; if($2 == '-'){$$ = makeNode(Sub);}; addChild($$, $1); addChild($$, $3);}
    |  T                                {$$ = $1;}
    ;    
T   :  T '*' F                          {$$ = makeNode(Mult); addChild($$, $1); addChild($$, $3);}
    |  T '/' F                          {$$ = makeNode(Div); addChild($$, $1); addChild($$, $3);}
    |  T '%' F                          {$$ = makeNode(Mod); addChild($$, $1); addChild($$, $3);}
    |  F                                {$$ = $1;}
    ;
F   :  ADDSUB F                         {if($<sign>1 == '+'){$$ = makeNode(Positif);};
                                         if($<sign>1 == '-'){$$ = makeNode(Negatif);};
                                         addChild($$,$2);}
    |  '!' F                            {$$ = makeNode(Opposite); addChild($$, $2);}
    |  '&' IDENT                        {$$ = makeNode(Adress); Node *tmp = makeNode(Identifier); strcpy(tmp->u.identifier,$2); addChild($$, tmp);}
    |  '(' Exp ')'                      {$$ = $2;}
    |  NUM                              {$$ = makeNode(IntLiteral); $$->u.integer = $1;}
    |  CHARACTER                        {$$ = makeNode(CharLiteral); $$->u.character = $1;}
    |  LValue                           {$$ = $1;}
    |  IDENT '(' Arguments  ')'         {$$ = makeNode(Identifier); strcpy($$->u.identifier,$1); addChild($$, $3);}
    |  '*' IDENT '(' Arguments  ')'     {$$ = makeNode(Pointer); Node *tmp = makeNode(Identifier); strcpy($$->u.identifier,$2); addChild($$, tmp); addChild($$, $4);}
    ;
LValue:
       IDENT                            {$$ = makeNode(Identifier); strcpy($$->u.identifier,$1);}
    |  '*' IDENT                        {$$ = makeNode(Pointer); Node *tmp = makeNode(Identifier); strcpy(tmp->u.identifier,$2); addChild($$, tmp);}
    ;
Arguments:
       ListExp                          {$$ = makeNode(ArgList); addChild($$, $1);}
    |                                   {$$ = makeNode(Void);};
ListExp:
       ListExp ',' Exp                  {$$ = $1; addSibling($1, $3);}
    |  Exp                              {$$ = $1;}
    ;
%%


int main(int argc, char** argv) {
    
    result = yyparse();
    int c = 0;
    char *f = "";
    if(parse  > 0){
        printf("\033[0;31mIl y a %d erreur dans votre code\033[0m\n", parse);
    }
    else{        
        char *file_name = (char *)malloc(100 * sizeof(char));

        Element redefError = NULL;
        Element declError = NULL;
        TypeErreur *typeError = (TypeErreur*) malloc(sizeof(TypeErreur) *7) ;
        for(int i= 0;i<7;i++){
            typeError[i] = NULL;
        }

       
        Hachage tab = symbolTable(abstractTree, &redefError, &declError);
        elementsType(abstractTree, tab, typeError );
        while((c = getopt (argc, argv, "[tsh]:")) != -1)
            switch (c){
            case 't':
               
                printTree(abstractTree);
                break;
            case 's':
                
                printSymboleTable(tab);
                break;
            case 'h': 
                printf("./tpcc [OPTIONS] FILE.tpc \n [OPTIONS] : \n -t,--tree affiche l’arbre abstrait sur la sortie standard \n -s,--symtabs affiche toutes les tables des symboles sur la sortie standard \n -h,--help affiche une description de l’interface utilisateur et termine l’exécution \n ");
                return 1;
            default:
                break;
            }
        
        
        

        nasmFile(file_name, f);

        printRedefErrTable(redefError);
        printfDeclErrTable(declError);
        printTypeErrTable(typeError);

        if(redefError == NULL && declError == NULL ){
            
            writeNasmFile(tab, abstractTree, file_name);
        }

        else if(redefError != NULL || declError != NULL){
            return 2;
        }
          
        for(int i= 0;i<5;i++){
            if(typeError[i] != NULL)
                return 2;
        }
        

    }
    return result || parse != 0;
}

void yyerror(char *s){
    fprintf(stderr, "%s", yylval.error);

    for(int i = 0; i< (pos_char - 1); i++)
        fprintf(stderr, " ");
    
    fprintf(stderr, "\n^\n");
    fprintf(stderr, "\033[1;31m%s \033[m:  near line \033[1;31m%d\033[m in caracter \033[1;31m%d \033[m\n\n", s , pos_line, pos_char);
    parse += 1;
}