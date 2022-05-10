#ifndef __typeSymoboleTable__
#define __typeSymoboleTable__ 

#include "symboleTable.h"



typedef enum {
	PointerErr, NumberOfArgs, ParamErr, Main, TypeBooleenErr, AssigneErr, ReturnErr
}DiffrentErrTypes;


typedef struct typeErr{
	char variable[100]; // variable avec l'erreur 
	Kind type_variable; // type de variable au déclaration
	Kind type_assigned; // le type donnée au variable != type_variable (d'ou l'erreur)
	int line;			// ligne dans le fichier de l'erreur 
	int number_Of_Args_Given; // nombre des arguments passé
	int number_Of_Args_expected; //nombre des arguments attendu
  struct typeErr *next;
} AnErreur , *TypeErreur;





void elementsType(Node *tree,  Hachage symboleTable, TypeErreur* typeErr);
Kind typeRetourOperation(Node *exp, Hachage symboleTable, TypeErreur* typeErr,char* func_name, int *pointeur_left_value, int *pointeur_right_value);
Kind trouveTypeExp(Node *exp, Hachage symboleTable, TypeErreur* typeErr,char* func_name, int *pointeur_left_value, int *pointeur_right_value);
Kind verifie_order(Node *exp, Hachage symboleTable, TypeErreur* typeErr,char* func_name, int *pointeur_left_value, int *pointeur_right_value);

void printTypeErrTable(TypeErreur *typeErrElem);


#endif