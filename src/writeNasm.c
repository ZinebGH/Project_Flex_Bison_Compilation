#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "writeNasm.h"


static void parsestatmList(Node* node, Hachage tabSymbole, char* func_name, FILE *nasmFile);

static int labelNumOr =0; 
static int labelNumAnd =0;
static int labelNumEqual=0;
static int labelNumDiff= 0;
static int labelNumWhile = 0;
static int labelNumIf = 0;
static int labelNumInf = 0;
static int labelNumInfOrEq = 0;
static int labelNumSup = 0;
static int labelNumSupOrEq = 0;
static int labelNumOpposite = 0;


static const char *StringFromRegister[] = {
	"rdi", "rsi", "rdx", "rcx", "r8", "r9"
};


// ecrire _start, et call main si besoin
static void writeStartFunc(FILE *nasmFile, Hachage tabSymbole){	
	fprintf(nasmFile, "_start:\n");
	Hachage tmp = tabSymbole->next;
	for(; tmp != NULL; tmp = tmp->next ){
		Elem *sousTable = tmp->element_tableau;
		if (sousTable->classe == Fonction && strcmp(sousTable->identifier, "main") ==0 ){
			fprintf(nasmFile, "\tcall main\n");
			break;
		}
	}
	fprintf(nasmFile, "\tmov rax, 60\n\tmov rdi, 0\n\tsyscall\n");
}

static void writeStartFile(Hachage tabSymbole, FILE* nasmFile){ // à ajouter le nom de fichier tpc au début de nasm
	//char formatInt[3] = "%";
	//strcat(formatInt, "d");
	char percent ='%';
	int resbSize =0;
	Elem* tmp = tabSymbole->element_tableau;
	while(tmp != NULL){
		if( tmp->next == NULL){
			resbSize= tmp->adresse;
		}
		tmp = tmp->next;
	}
	fprintf(nasmFile, "global _start\nextern print_registers\nextern printf\nextern scanf\n" );
	fprintf(nasmFile, "__static:\n\tresb %d\n", resbSize);
	fprintf(nasmFile, "section .data\n\t__format_int_print db \"%cd\", 10, 0\n\t__format_char_print db \"%cc\", 10, 0\n\t__format_int_reade db \"%cd\", 0\n\t__format_int_readc db \"%cc\", 0\n",percent, percent, percent, percent );	
	fprintf(nasmFile, "section .text\n");
	writeStartFunc(nasmFile, tabSymbole); 
}


static void subSpace(Hachage tabSymbole, char* funcName, FILE *nasmFile ){
	Hachage tmp = tabSymbole->next; // on commence par les fonctions et on saute les déclaration globales 

	for(; tmp != NULL; tmp = tmp->next){
		Elem *sousTable = tmp->element_tableau;
		if (sousTable->classe == Fonction && strcmp(sousTable->identifier, funcName) ==0 ){
			Elem *args = sousTable;
			int reg = 0;
			while(args->next!=NULL){
				if(args->adresse != 0 && args->classe == Argument){
					fprintf(nasmFile, "\tmov [rbp %d], %s\n",args->adresse, StringFromRegister[reg]);
					reg++;
				}
				args = args->next;
			}
			if(args->adresse != 0 && args->classe == Argument){
					fprintf(nasmFile, "\tmov [rbp %d], %s\n",args->adresse, StringFromRegister[reg]);		
			}
			fprintf(nasmFile, "\tsub rsp, %d\n",args->adresse + (-2 * args->adresse)); // flemme de faire valeur abs 
			return;
		}	
	}	
}
//avancer le sous tableau jusqu'a trouver l'argument (return 1) ou fin du tableau (return 0)
static int findArgInSubTable(Elem **soustable, char *ident){
	while( *soustable !=  NULL){
		if( strcmp((*soustable)->identifier, ident)==0){
			return 1;
		}
		*soustable = (*soustable)->next;
	}
	return 0;
}

//recherche de address de identifierdans table des symboles, et ecrit dans nasm, NB : beug potentiel : utilisation d'un fonction totalement vide 
static void writeIdent(Elem* soustable, FILE* nasmFile){
	fprintf(nasmFile, "\tmov rax, rbp\n\tadd rax, %d\n\tpush rax\n", soustable->adresse);
}

static void writePointer(Elem *sousTable, FILE* nasmFile){
	fprintf(nasmFile, "\tmov rax, rbp\n\tadd rax, %d\n\tpush qword [rax]\n", sousTable->adresse);
}


static void decideWhichPrint(Elem* soustable, FILE* nasmFile){
	if(soustable->type == Int){
		fprintf(nasmFile, "\tmov rdi, __format_int_print\n\tpop rsi\n\txor rax, rax\n\tcall printf\n" );
	}else{
		fprintf(nasmFile, "\tmov rdi, __format_char_print\n\tpop rsi\n\txor rax, rax\n\tcall printf\n" );
	}
}

//trouve le variable ident dans table de symbole et fait un traitement particulier dessus 
static void searchInTabSymbole(char *ident, Hachage tabSymbole,  char* func_name,  FILE *nasmFile , void (*pointeurFct)(Elem*, FILE*)){
	Hachage tmp = tabSymbole;
		for(; tmp != NULL; tmp = tmp->next){
			Elem *sousTable = tmp->element_tableau;
			if( sousTable!= NULL  && (sousTable->classe == Global || strcmp(sousTable->identifier,func_name) == 0 )) { 
				if(findArgInSubTable(&sousTable, ident)){
					(*pointeurFct)(sousTable, nasmFile);
					return;
				}
			}
		}
	return;
}


// choisir le format de print selon le type de premier valeur à afficher, 
static void writePrint(Node* node, Hachage tabSymbole, char* func_name,FILE *nasmFile){
	if(node == NULL){
		return;
	}
	switch (node->kind){
		case Identifier:
			searchInTabSymbole(node->u.identifier, tabSymbole, func_name, nasmFile, &decideWhichPrint);
			return;
		case IntLiteral:
			fprintf(nasmFile, "\tmov rdi, __format_int_print\n\tpop rsi\n\txor rax, rax\n\tcall printf\n" );
			return;
		case CharLiteral:
			fprintf(nasmFile, "\tmov rdi, __format_char_print\n\tpop rsi\n\txor rax, rax\n\tcall printf\n" );
			return;
		default:
			writePrint(node->firstChild, tabSymbole, func_name, nasmFile);
			break;
	}
}

static void writeFuncDeclaration(char *func_name, Hachage tabSymbole, FILE *nasmFile){
	Hachage tmp = tabSymbole;
	int reg ;

	for(; tmp != NULL; tmp = tmp->next){
		Elem *sousTable = tmp->element_tableau;
		if( sousTable!= NULL  && sousTable->classe == Fonction && strcmp(sousTable->identifier,func_name) == 0 ) { 
			reg =sousTable->nombreArgument-1;
			if(sousTable->nombreArgument > 6 ){
				reg = 5;
			}
			for (int i = reg; i >=0; i--){
				fprintf(nasmFile, "\tpop %s\n", StringFromRegister[i]);
			}
			fprintf(nasmFile, "\tcall %s\n",func_name);
			if( sousTable->type != Void){
				fprintf(nasmFile, "\tpush rax\n" );
			}
		}
	}
	






}

static void writeOr(FILE *nasmFile){
	fprintf(nasmFile, "\t;traitement Or\n");
	fprintf(nasmFile, "\tpop rax\n\tcmp rax , 0\n\tjne trueOr%d\n\tpop rcx\n\tcmp rcx, 0\n\tjne trueOr%d\n\t",labelNumOr, labelNumOr);
	fprintf(nasmFile, "push 0\n\tjmp restOr%d\n\ttrueOr%d:\n\tpush 1\n\trestOr%d:\n",labelNumOr,labelNumOr,labelNumOr);
	labelNumOr++;
}

static void writeAnd(FILE *nasmFile){

	fprintf(nasmFile, "\t;traitement And\n\tpop rax\n\tcmp rax ,0\n\tje falseAnd%d\npop rcx\n\tcmp rcx ,0\n\tje falseAnd%d\n", labelNumAnd, labelNumAnd);
	fprintf(nasmFile, "\tpush 1\n\tjmp restAnd%d\n\tfalseAnd%d:\n\tpush 0\n\trestAnd%d:\n", labelNumAnd,labelNumAnd,labelNumAnd);
	labelNumAnd++;
}

static void writeEqual(FILE* nasmFile){

	fprintf(nasmFile, "\t;traitement Equal\n\tpop rax\n\tpop rcx\n\tcmp rax, rcx\n\tje isEqual%d\n\tpush 0\n\tjmp restEqual%d\n",labelNumEqual,labelNumEqual);
	fprintf(nasmFile, "\tisEqual%d:\n\tpush 1\n\trestEqual%d:\n", labelNumEqual,labelNumEqual);
	labelNumEqual++;
}


static void writeDiff(FILE* nasmFile){

	fprintf(nasmFile, "\t;traitement Diffrent\n\tpop rax\n\tpop rcx\n\tcmp rax, rcx\n\tje isNotDiff%d\n\tpush 1\n\tjmp restDiff%d\n",labelNumDiff,labelNumDiff);
	fprintf(nasmFile, "\tisNotDiff%d:\n\tpush 0\n\trestDiff%d:\n", labelNumDiff,labelNumDiff);
	labelNumDiff++;
}

static void writeInf(FILE* nasmFile){

	fprintf(nasmFile, "\t;traitement Inf\n\tpop rax\n\tpop rcx\n\tcmp rax, rcx\n\tjg isNotInf%d\n\tpush 0\n\tjmp restInf%d\n",labelNumInf,labelNumInf);
	fprintf(nasmFile, "\tisNotInf%d:\n\tpush 1\n\trestInf%d:\n", labelNumInf,labelNumInf);
	labelNumInf++;
}

static void writeSup(FILE* nasmFile){

	fprintf(nasmFile, "\t;traitement Inf\n\tpop rax\n\tpop rcx\n\tcmp rcx, rax\n\tjg isNotInf%d\n\tpush 0\n\tjmp restInf%d\n",labelNumSup,labelNumSup);
	fprintf(nasmFile, "\tisNotInf%d:\n\tpush 1\n\trestInf%d:\n", labelNumSup,labelNumSup);
	labelNumSup++;
}

static void writeInfOrEq(FILE* nasmFile){

	fprintf(nasmFile, "\t;traitement InfOrEq\n\tpop rax\n\tpop rcx\n\tcmp rcx, rax\n\tjng isNotInfOrEq%d\n\tpush 0\n\tjmp restInfOrEq%d\n",labelNumInfOrEq,labelNumInfOrEq);
	fprintf(nasmFile, "\tisNotInfOrEq%d:\n\tpush 1\n\trestInfOrEq%d:\n", labelNumInfOrEq,labelNumInfOrEq);
	labelNumInfOrEq++;
}

static void writeSupOrEq(FILE* nasmFile){

	fprintf(nasmFile, "\t;traitement SupOrEq\n\tpop rax\n\tpop rcx\n\tcmp rax, rcx\n\tjng isNotSupOrEq%d\n\tpush 0\n\tjmp restSupOrEq%d\n",labelNumSupOrEq,labelNumSupOrEq);
	fprintf(nasmFile, "\tisNotSupOrEq%d:\n\tpush 1\n\trestSupOrEq%d:\n", labelNumSupOrEq,labelNumSupOrEq);
	labelNumSupOrEq++;
}

static void writeOpposite(FILE* nasmFile){

	fprintf(nasmFile, "\t;traitement Negation booleenne\n\tpop rax\n\tcmp rax, 1\n\tje opposite%d\n\tpush 1\n\tjmp restOpposite%d\n",labelNumOpposite,labelNumOpposite);
	fprintf(nasmFile, "\topposite%d:\n\tpush 0\n\trestOpposite%d:\n", labelNumOpposite,labelNumOpposite);
	labelNumOpposite++;
}

static void switchstatmList(Node* exp, Hachage tabSymbole, char* func_name, FILE *nasmFile){

		switch (exp->kind){
			case Assign : // int, char, pointer, identifier
				fprintf(nasmFile, "\t;Assigne\n" );
				fprintf(nasmFile, "\tpop rbx\n\tpop rax\n\tmov qword [rax], rbx\n"); // write assigne
				break;
			case Print:
				fprintf(nasmFile, "\t;Print\n");
				writePrint(exp,  tabSymbole,func_name, nasmFile);
				break;
			case Add :
				fprintf(nasmFile, "\t;Add\n");
				fprintf(nasmFile, "\tpop rbx\n\tpop rax\n\tadd rax, rbx\n\tpush rax\n");
				break;
			case Sub:
				fprintf(nasmFile, "\t;Sub\n");
				fprintf(nasmFile,"\tpop rbx\n\tpop rax\n\tsub rax, rbx\n\tpush rax\n" );
				break;
			case Mult:
				fprintf(nasmFile, "\t;Mult\n");
				fprintf(nasmFile,"\tpop rbx\n\tpop rax\n\timul rax, rbx\n\tpush rax\n" );
				break;
			case Div:
				fprintf(nasmFile, "\t;Div\n");
				fprintf(nasmFile, "\tpop rcx\n\tpop rax\n\tmov rdx , 0\n\tidiv rcx\n\tpush rax\n");
				break;
			case Mod:
				fprintf(nasmFile, "\t;Mod\n");
				fprintf(nasmFile, "\tpop rcx\n\tpop rax\n\tmov rdx , 0\n\tidiv rcx\n\tpush rdx\n");
				break;
			case ValueOf: 
				fprintf(nasmFile, "\t;ValueOf\n");
				fprintf(nasmFile, "\tpop rax\n\tpush qword [rax]\n");
				break;
			case ReadE: //beug quand on fait 2 reade, probléme avec alignement de pile
				fprintf(nasmFile, "\t;ReadE\n" );
				fprintf(nasmFile, "\tmov rdi, __format_int_reade\n\tpop rsi\n\txor rax, rax\n\tcall scanf\n");
				break;
			case ReadC:
				fprintf(nasmFile, "\t;ReadC\n" );
				fprintf(nasmFile, "\tmov rdi, __format_int_readc\n\tpop rsi\n\txor rax, rax\n\tcall scanf\n");
				break;
			case Identifier:		
				//writeIdent(exp->u.identifier, tabSymbole, func_name, nasmFile);
				if(exp->firstChild != NULL && (FIRSTCHILD(exp)->kind == ArgList || FIRSTCHILD(exp)->kind == Void)){
					fprintf(nasmFile, "\t;function %s\n", exp->u.identifier);
					writeFuncDeclaration(exp->u.identifier, tabSymbole, nasmFile );
				}else{
					fprintf(nasmFile, "\t;Identifier\n");
					searchInTabSymbole(exp->u.identifier, tabSymbole, func_name, nasmFile, &writeIdent);
				}
				break;
			case IntLiteral:
				fprintf(nasmFile, "\t;IntLiteral\n");			
				fprintf(nasmFile,"\tpush %d\n", exp->u.integer);
				break;
			case CharLiteral:
				fprintf(nasmFile, "\t;CharLiteral\n");
				fprintf(nasmFile, "\tpush \'%c\'\n",exp->u.character );
				break;
			case Or:
				writeOr(nasmFile);
				break;
			case And:
				writeAnd(nasmFile);
				break;
			case Equal:
				writeEqual(nasmFile);
				break;
			case Different:
				writeDiff(nasmFile);
				break;
			case Inf :
				writeInf(nasmFile);
				break;
			case Sup:
				writeSup(nasmFile);
				break;
			case InfOrEq:
				writeInfOrEq(nasmFile);
				break;
			case SupOrEq:
				writeSupOrEq(nasmFile);
				break;
			case Opposite:
				writeOpposite(nasmFile);
				break;
			case Return:
				fprintf(nasmFile, "\t;return\n\tpop rax\n");
			default :
				break;
		}
}

static int writeIf(Node* node, Hachage tabSymbole, char* func_name, FILE *nasmFile){
	int cpt = labelNumIf;
	labelNumIf += 1;
	fprintf(nasmFile, "\t;If\n");
	Node *tmp = FIRSTCHILD(node)->nextSibling;
	FIRSTCHILD(node)->nextSibling = NULL;
	parsestatmList(FIRSTCHILD(node), tabSymbole, func_name, nasmFile);
	FIRSTCHILD(node)->nextSibling = tmp;
	fprintf(nasmFile, "\tpop rax\n\tcmp rax, 1\n");
	if(node->nextSibling == NULL || node->nextSibling->kind != Else)
		fprintf(nasmFile, "\tjne not_if%d\n",cpt);
	else
		fprintf(nasmFile, "\tjne else%d\n", cpt);

	parsestatmList(SECONDCHILD(node),tabSymbole,func_name,nasmFile);

	if(node->nextSibling == NULL || node->nextSibling->kind != Else)
		fprintf(nasmFile, "\tnot_if%d:\n",cpt);
	else
		fprintf(nasmFile, "\tjmp finElse%d\n",cpt);		

	return cpt;
}

static void writeElse(Node* node, Hachage tabSymbole, char* func_name, int cpt, FILE *nasmFile){

	fprintf(nasmFile, "\telse%d:\n",cpt);
	parsestatmList(FIRSTCHILD(node),tabSymbole,func_name,nasmFile);	
	fprintf(nasmFile, "\tfinElse%d:\n", cpt);
}

static void writeWhile(Node* node, Hachage tabSymbole, char* func_name, FILE *nasmFile){

	int cpt = labelNumWhile;
	labelNumWhile ++;
	fprintf(nasmFile, "\t;While\n");
	fprintf(nasmFile, "\twhile%d:\n", cpt);	
	Node *tmp = FIRSTCHILD(node)->nextSibling;
	FIRSTCHILD(node)->nextSibling = NULL;
	parsestatmList(FIRSTCHILD(node), tabSymbole, func_name, nasmFile);
	FIRSTCHILD(node)->nextSibling = tmp;
	fprintf(nasmFile, "\tpop rax\n\tcmp rax, 1\n");
	fprintf(nasmFile, "\tjne finWhile%d\n", cpt);
	parsestatmList(SECONDCHILD(node), tabSymbole, func_name, nasmFile);
	fprintf(nasmFile, "\tjmp while%d\n", cpt);
	fprintf(nasmFile, "\tfinWhile%d:\n", cpt);

}

/*
	NB : beug potentiel pour print, appel récursif avant push rdi/rsi par rapport les address de variable a printer  
*/


static void parsestatmList(Node* node, Hachage tabSymbole, char* func_name, FILE *nasmFile){
	if(node == NULL){
		return;
	}
	int valeur_else;
	for(Node *exp = node; exp != NULL; exp = exp->nextSibling){
		switch (exp->kind){
			case If : 
				valeur_else = writeIf(exp, tabSymbole, func_name, nasmFile);
				break;
			case Else : 
				writeElse(exp, tabSymbole, func_name, valeur_else, nasmFile);
				break;
			case While :
				writeWhile(exp, tabSymbole, func_name, nasmFile);
				break;
			case Pointer:
				fprintf(nasmFile, "\t;Pointers %s\n", exp->u.identifier);
				searchInTabSymbole(FIRSTCHILD(exp)->u.identifier, tabSymbole, func_name, nasmFile, &writePointer);
				break;
			default :
				parsestatmList(exp->firstChild, tabSymbole, func_name, nasmFile); // parcours les sous elements
		}
		switchstatmList(exp, tabSymbole, func_name, nasmFile);		
	}
}


static void parseFuncDecList(Hachage tabSymbole, Node* funcList, FILE *nasmFile){
	for(Node *func = funcList->firstChild; func != NULL; func = func->nextSibling){
		fprintf(nasmFile, "%s:\n",  SECONDCHILD(func)->u.identifier);
		if (strcmp(SECONDCHILD(func)->u.identifier, "main")!=0){
			fprintf(nasmFile, "\tpush rbp\n" );
		}
		fprintf(nasmFile, "\tmov rbp, rsp\n"); // début de fonction
		
		
		subSpace(tabSymbole, SECONDCHILD(func)->u.identifier, nasmFile); // on réserve de la mémoire dans la pile, en fonction de nombre des variables
		Node *statmList = SECONDCHILD(FOURTHCHILD(func)); // statement list, cf l'arbre 
		parsestatmList(statmList, tabSymbole, SECONDCHILD(func)->u.identifier, nasmFile );
		fprintf(nasmFile, "\t;fin\n\tmov rsp, rbp\n");
		if (strcmp(SECONDCHILD(func)->u.identifier, "main")!=0){
			fprintf(nasmFile, "\tpop rbp\n" );
		}
		fprintf(nasmFile, "\tret\n");
	}
}


static void parse(Hachage tabSymbole, Node* tree, FILE *nasmFile){

	for(Node *tmp_child = tree->firstChild; tmp_child != NULL; tmp_child = tmp_child->nextSibling){
		if(tmp_child->kind == VarDeclList){
			// traitement var dec list
		}else{
			parseFuncDecList(tabSymbole, tmp_child, nasmFile);
		}
	}
}



int writeNasmFile(Hachage tabSymbole, Node* tree, char *file_name){
	FILE *nasmFile; 
	if((nasmFile = fopen(file_name, "w+")) == NULL){
		printf("Error while opening nasm file\n");
		return 0;
	}

	writeStartFile(tabSymbole, nasmFile);
	
	parse(tabSymbole, tree, nasmFile);

	
	
	fclose(nasmFile);
	return 1;
}










char* reverseName(char* file_name){
    char* new = (char *)malloc(100 * sizeof(char));
    for(int i = strlen(file_name); i > 0; i--) {
        strncat(new, &(file_name[i]), 1);
    }
    return new;
}


char* nasmFile(char* file_name, char* f){
	//RECUPERER LE NOM DU FICHIER DANS LA COMMANDE DU BASH 
    // for(int i = sizeof(f) - 1; f[i] != '/'; i--){

    //     strncat(file_name, &(f[i]), 1);
    // }
    // file_name = reverseName(file_name);
    // strcat(file_name, ".asm");

    if(strcmp(file_name, "") == 0){
        strcpy(file_name, "_anonymous.asm");
    }
    return file_name;
}