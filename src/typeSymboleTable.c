#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typeSymboleTable.h"

static const char *StringFromKind[] = {
  "Program",
  "DeclStruct",
  "VarDeclList",
  "VarDec",
  "FuncDecList",
  "FuncDec",
  "ArgList",
  "Pointer",
  "ParamList",
  "Param",
  "Body",
  "StmtList",
  "Return",
  "Add",
  "Sub",
  "Mult",
  "Div",
  "Mod",
  "Assign",
  "Or",
  "And",
  "Equal",
  "Different",
  "Inf",
  "Sup",
  "InfOrEq",
  "SupOrEq",
  "IntLiteral",
  "CharLiteral",
  "Identifier",
  "Decl",
  "Int",
  "Char",
  "Void",
  "Adress",
  "Opposite",
  "ReadE",
  "ReadC",
  "Print",
  "While",
  "If",
  "Else",
  "Positif",
  "Negatif",
  "Error",
  "ValueOf"

  /* and all other node labels */
  /* The list must coincide with the enum in abstract-tree.h */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
};

static void addErreurToTable(TypeErreur *typeErrElem, Node* errInNode, Kind kind_dec, Kind kind_given, int args_Given, int args_expected){
	AnErreur *tmp_err = *typeErrElem;

	AnErreur* new_err = (AnErreur *)malloc(sizeof(AnErreur));
	if(new_err == NULL)
		return;
	if(errInNode != NULL)
		strcpy(new_err->variable,  errInNode->u.identifier);
	new_err->type_variable = kind_dec;
	new_err->type_assigned = kind_given;
	if(errInNode != NULL)
		new_err->line = errInNode->pos_line;
	new_err->number_Of_Args_Given =args_Given;
	new_err->number_Of_Args_expected = args_expected;
	new_err-> next = NULL;
	if(tmp_err == NULL ){
		*typeErrElem = new_err;
	}else{
		while(tmp_err->next!= NULL){
			tmp_err = tmp_err->next;
		}
		tmp_err->next = new_err;
	}
	return;
}

static Kind typeIdentInTable(Node *identifier, Hachage symboleTable, char* func_name, int *pointeur_left_value, int *pointeur_right_value){// parcours de table des symboles 
	
	Elem *tmp_global = NULL;
	for (Hachage tmp = symboleTable; tmp!= NULL; tmp = tmp->next){
		Elem* sousTable = tmp->element_tableau;

		if( sousTable != NULL && sousTable->classe == Global){
			for(Elem *elemSousList = sousTable; elemSousList!= NULL; elemSousList = elemSousList->next){
				if(strcmp(elemSousList->identifier,identifier->u.identifier) == 0)
					tmp_global = elemSousList;
			}
		}

		if(sousTable != NULL && sousTable->classe == Fonction && strcmp(sousTable->identifier, func_name) ==0){ // soit la sousList global, soit la sous list de fonction précise
			for(Elem *elemSousList = sousTable; elemSousList!= NULL; elemSousList = elemSousList->next){
				if(strcmp(elemSousList->identifier,identifier->u.identifier) == 0){
					return elemSousList->type;	
				}
			}
		}		
	}
	if(tmp_global != NULL)
		return tmp_global->type;
	return Error;
}

static int valeur_pointeur(Node *identifier_node, Hachage symboleTable, char* func_name){
	
	Node *identifier = identifier_node;
	Elem *tmp_global = NULL;
	
	if(identifier != NULL && identifier->kind == ValueOf){
		identifier = identifier->firstChild;
	}
	if(identifier == NULL || identifier->kind != Identifier){
		return 0;
	}

	for (Hachage tmp = symboleTable; tmp!= NULL; tmp = tmp->next){
		Elem* sousTable = tmp->element_tableau;

		if( sousTable != NULL && sousTable->classe == Global){ // soit la sousList global, soit la sous list de fonction précise
			for(Elem *elemSousList = sousTable; elemSousList!= NULL; elemSousList = elemSousList->next){
				if(strcmp(elemSousList->identifier,identifier->u.identifier) == 0){
					tmp_global = elemSousList;	
				}
			}
		}	

		if( sousTable != NULL && sousTable->classe == Fonction && strcmp(sousTable->identifier, func_name) ==0){ // soit la sousList global, soit la sous list de fonction précise
			for(Elem *elemSousList = sousTable; elemSousList!= NULL; elemSousList = elemSousList->next){
				if(strcmp(elemSousList->identifier,identifier->u.identifier) == 0){
					return elemSousList->pointeur;	
				}
			}
		}		
	}
	if(tmp_global != NULL)
		return tmp_global->pointeur;
	return 0;
}

static void verifie_pointeur_op(Node *exp, Hachage symboleTable, char* func_name, TypeErreur *typeErr){
	int pointeur_arg;
	if(exp!= NULL && exp->kind == Pointer){
		pointeur_arg = valeur_pointeur(FIRSTCHILD(exp), symboleTable, func_name);
		if(pointeur_arg != 1)
			addErreurToTable(&(typeErr[PointerErr]),exp, exp->kind, exp->kind, 0,0 );
	}
	if(exp!= NULL && exp->kind == Adress){
		addErreurToTable(&(typeErr[PointerErr]),exp, exp->kind, exp->kind, 0,0 );
	}
}

Kind typeRetourOperation(Node *exp, Hachage symboleTable, TypeErreur* typeErr,char* func_name, int *pointeur_left_value, int *pointeur_right_value){
	Kind elem1, elem2;
	int right = *pointeur_right_value;
	int left = *pointeur_left_value;
	verifie_pointeur_op(FIRSTCHILD(exp), symboleTable, func_name, typeErr);
	verifie_pointeur_op(SECONDCHILD(exp), symboleTable, func_name, typeErr);
	elem1 = trouveTypeExp(FIRSTCHILD(exp), symboleTable, typeErr, func_name, &left, &right);
	elem2 = trouveTypeExp(SECONDCHILD(exp), symboleTable, typeErr, func_name, &left, &right);
	if( valeur_pointeur(FIRSTCHILD(exp), symboleTable, func_name) != valeur_pointeur(SECONDCHILD(exp), symboleTable, func_name))
		addErreurToTable(&(typeErr[PointerErr]),FIRSTCHILD(exp), FIRSTCHILD(exp)->kind, SECONDCHILD(exp)->kind, 0,0 );
		
	if(elem1 != Int || elem1 != Char)
		return elem1;
	else if(elem2 != Int || elem2 != Char)
		return elem2;
	return Int;
}

static int type_argument(Node *arg, Hachage symboleTable, char *func_name){

	if(arg->kind == Pointer)
		return 1 + valeur_pointeur(arg, symboleTable, func_name);
	if(arg->kind == Adress)
		return -1 + valeur_pointeur(arg, symboleTable, func_name);
	return valeur_pointeur(arg, symboleTable, func_name);
}

static void checkTypeArgs(Node* ArgList, char *func_name, char* func_Called, int fun_lineno, TypeErreur* typeErr, Elem* sousTable, Hachage symboleTable, int *pointeur_left_value, int *pointeur_right_value){
	
	Elem *param = sousTable;
	Node *errNode = (Node *)malloc(sizeof(Node));
	if(errNode == NULL)
		return;
	errNode->pos_line = fun_lineno;
	strcpy(errNode->u.identifier,"");
	int args_given = 0, args_expected = 0;
	Kind type_arg, type_param;
	for (Node* tmp = ArgList->firstChild ;tmp != NULL; tmp = tmp->nextSibling ){
		type_arg = trouveTypeExp(tmp, symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
		if(param == NULL || param->classe != Argument){
			errNode->pos_line = tmp->pos_line;
			while(tmp != NULL){
				tmp = tmp->nextSibling;
				args_expected += 1;
			}
			strcpy(errNode->u.identifier,func_Called);
			addErreurToTable(&(typeErr[NumberOfArgs]),errNode, type_param, type_arg, args_given, args_expected);
			return;
		}
		type_param = param->type;
		errNode->pos_line = tmp->pos_line;
		strcpy(errNode->u.identifier,param->identifier);

		if( /*valeur_pointeur(tmp, symboleTable, func_name)*/type_argument(tmp, symboleTable, func_name)  + param->pointeur != 0){
			addErreurToTable(&(typeErr[PointerErr]),errNode, type_param, type_arg, 0,0 );
		}

		if(type_param != type_arg){
			addErreurToTable(&(typeErr[ParamErr]),errNode, type_param, type_arg, 0,0 );
		}
		args_given += 1; 
		args_expected += 1;
		param = param->next;
	}
	if(param != NULL && param->classe == Argument){
		while(param != NULL && param->classe == Argument){
			param = param->next;
			args_given += 1;
		}
		strcpy(errNode->u.identifier,func_Called);
		addErreurToTable(&(typeErr[NumberOfArgs]),errNode, type_param, type_arg, args_given, args_expected);
	}
}

static Kind traiteFonction(Node *exp,Hachage symboleTable, TypeErreur* typeErr, char* func_name, int *pointeur_left_value, int *pointeur_right_value){

	for (Hachage tmp = symboleTable; tmp!= NULL; tmp = tmp->next){
		Elem* sousTable = tmp->element_tableau;
		if( sousTable != NULL && sousTable->classe == Fonction && strcmp(exp->u.identifier, sousTable->identifier) ==0){ // soit la sousList global, soit la sous list de fonction précise
			checkTypeArgs(FIRSTCHILD(exp), func_name, exp->u.identifier, exp->pos_line, typeErr, sousTable->next, symboleTable, pointeur_left_value, pointeur_right_value);
			return sousTable->type;
		}		
	}
	return Void;
}

Kind verifie_eq(Node *exp, Hachage symboleTable, TypeErreur* typeErr,char* func_name, int *pointeur_left_value, int *pointeur_right_value){
	Kind elem1, elem2;
	verifie_pointeur_op(FIRSTCHILD(exp), symboleTable, func_name, typeErr);
	verifie_pointeur_op(SECONDCHILD(exp), symboleTable, func_name, typeErr);
	elem1 = trouveTypeExp(FIRSTCHILD(exp), symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
	elem2 = trouveTypeExp(SECONDCHILD(exp), symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
	if( valeur_pointeur(FIRSTCHILD(exp), symboleTable, func_name) != valeur_pointeur(SECONDCHILD(exp), symboleTable, func_name))
		addErreurToTable(&(typeErr[PointerErr]),FIRSTCHILD(exp), FIRSTCHILD(exp)->kind, FIRSTCHILD(exp)->kind, 0,0 );
		
	if(elem1 != Int && elem1 != Char)
		addErreurToTable(&(typeErr[TypeBooleenErr]),FIRSTCHILD(exp), FIRSTCHILD(exp)->kind, SECONDCHILD(exp)->kind, 0,0 );
	else if(elem2 != Int && elem2 != Char)
		addErreurToTable(&(typeErr[TypeBooleenErr]),SECONDCHILD(exp), SECONDCHILD(exp)->kind, FIRSTCHILD(exp)->kind, 0,0 );
	return Int;
}

Kind verifie_order(Node *exp, Hachage symboleTable, TypeErreur* typeErr,char* func_name, int *pointeur_left_value, int *pointeur_right_value){
	Kind elem1, elem2;
	verifie_pointeur_op(FIRSTCHILD(exp), symboleTable, func_name, typeErr);
	verifie_pointeur_op(SECONDCHILD(exp), symboleTable, func_name, typeErr);
	elem1 = trouveTypeExp(FIRSTCHILD(exp), symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
	elem2 = trouveTypeExp(SECONDCHILD(exp), symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
	if( valeur_pointeur(FIRSTCHILD(exp), symboleTable, func_name) == 1 || valeur_pointeur(SECONDCHILD(exp), symboleTable, func_name) == 1)
		addErreurToTable(&(typeErr[PointerErr]),FIRSTCHILD(exp), FIRSTCHILD(exp)->kind, FIRSTCHILD(exp)->kind, 0,0 );
		
	if(elem1 != Int && elem1 != Char)
		addErreurToTable(&(typeErr[TypeBooleenErr]),FIRSTCHILD(exp), FIRSTCHILD(exp)->kind, SECONDCHILD(exp)->kind, 0,0 );
	else if(elem2 != Int && elem2 != Char)
		addErreurToTable(&(typeErr[TypeBooleenErr]),SECONDCHILD(exp), SECONDCHILD(exp)->kind, FIRSTCHILD(exp)->kind, 0,0 );
	return Int;
}

static Kind verifie_op_unaire_int(Node *exp, Hachage symboleTable, TypeErreur* typeErr,char* func_name, int *pointeur_left_value, int *pointeur_right_value){
	Kind elem1;
	verifie_pointeur_op(FIRSTCHILD(exp), symboleTable, func_name, typeErr);
	elem1 = trouveTypeExp(FIRSTCHILD(exp), symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
	if( valeur_pointeur(FIRSTCHILD(exp), symboleTable, func_name) == 1)
		addErreurToTable(&(typeErr[PointerErr]),FIRSTCHILD(exp), FIRSTCHILD(exp)->kind, FIRSTCHILD(exp)->kind, 0,0 );
		
	if(elem1 != Int)
		addErreurToTable(&(typeErr[AssigneErr]),FIRSTCHILD(exp), FIRSTCHILD(exp)->kind, Int, 0,0 );
	return Int;
}

static Kind verifie_op_unaire_print(Node *exp, Hachage symboleTable, TypeErreur* typeErr,char* func_name, int *pointeur_left_value, int *pointeur_right_value){
	Kind elem1;
	verifie_pointeur_op(FIRSTCHILD(exp), symboleTable, func_name, typeErr);
	elem1 = trouveTypeExp(FIRSTCHILD(exp), symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
	if( valeur_pointeur(FIRSTCHILD(exp), symboleTable, func_name) == 1)
		addErreurToTable(&(typeErr[PointerErr]),FIRSTCHILD(exp), FIRSTCHILD(exp)->kind, FIRSTCHILD(exp)->kind, 0,0 );
		
	if(elem1 != Int && elem1 != Char)
		addErreurToTable(&(typeErr[AssigneErr]),FIRSTCHILD(exp), FIRSTCHILD(exp)->kind, Void, 0,0 );
	return elem1;
}

static Kind verifie_op_unaire_char(Node *exp, Hachage symboleTable, TypeErreur* typeErr,char* func_name, int *pointeur_left_value, int *pointeur_right_value){
	Kind elem1;
	verifie_pointeur_op(FIRSTCHILD(exp), symboleTable, func_name, typeErr);
	elem1 = trouveTypeExp(FIRSTCHILD(exp), symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
	if( valeur_pointeur(FIRSTCHILD(exp), symboleTable, func_name) == 1)
		addErreurToTable(&(typeErr[PointerErr]),FIRSTCHILD(exp), FIRSTCHILD(exp)->kind, FIRSTCHILD(exp)->kind, 0,0 );
		
	if(elem1 != Int)
		addErreurToTable(&(typeErr[AssigneErr]),FIRSTCHILD(exp), FIRSTCHILD(exp)->kind, Void, 0,0 );
	return elem1;
}

Kind trouveTypeExp(Node *exp, Hachage symboleTable, TypeErreur* typeErr,char* func_name, int *pointeur_left_value, int *pointeur_right_value){

	switch(exp->kind){
		case IntLiteral:
			return Int;
		case CharLiteral:
			return Char;
		case Identifier:
			if(FIRSTCHILD(exp)!=NULL)
				return traiteFonction(exp, symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
			else
				return typeIdentInTable(exp, symboleTable, func_name, pointeur_left_value, pointeur_right_value);
		case ValueOf:
			return trouveTypeExp(FIRSTCHILD(exp), symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
		case Add:
			return typeRetourOperation(exp, symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
		case Sub:
			return typeRetourOperation(exp, symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
		case Mult:
			return typeRetourOperation(exp, symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
		case Div:
			return typeRetourOperation(exp, symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
		case Mod:
			return typeRetourOperation(exp, symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
		case Adress:
			(*pointeur_right_value) -= 1;
			return trouveTypeExp(FIRSTCHILD(exp), symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
		case Pointer:
			(*pointeur_right_value) += 1;
			return trouveTypeExp(FIRSTCHILD(exp), symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
		case Inf:
			return verifie_order(exp,symboleTable, typeErr, func_name,pointeur_left_value, pointeur_right_value);
		case Sup:
			return verifie_order(exp,symboleTable, typeErr, func_name,pointeur_left_value, pointeur_right_value);
		case InfOrEq:
			return verifie_order(exp,symboleTable, typeErr, func_name,pointeur_left_value, pointeur_right_value);
		case SupOrEq:
			return verifie_order(exp,symboleTable, typeErr, func_name,pointeur_left_value, pointeur_right_value);
		case Equal:
			return verifie_eq(exp,symboleTable, typeErr, func_name,pointeur_left_value, pointeur_right_value);
		case Different:
			return verifie_eq(exp,symboleTable, typeErr, func_name,pointeur_left_value, pointeur_right_value);
		case Or:
			return verifie_order(exp,symboleTable, typeErr, func_name,pointeur_left_value, pointeur_right_value);
		case And:
			return verifie_order(exp,symboleTable, typeErr, func_name,pointeur_left_value, pointeur_right_value);
		case Opposite:
			return verifie_op_unaire_int(exp,symboleTable, typeErr, func_name,pointeur_left_value, pointeur_right_value);
		case Positif:
			return verifie_op_unaire_int(exp,symboleTable, typeErr, func_name,pointeur_left_value, pointeur_right_value);
		case Negatif:
			return verifie_op_unaire_int(exp,symboleTable, typeErr, func_name,pointeur_left_value, pointeur_right_value);
		case Print:
			return verifie_op_unaire_print(exp,symboleTable, typeErr, func_name,pointeur_left_value,pointeur_right_value);
		case ReadC:
			return verifie_op_unaire_char(exp,symboleTable, typeErr, func_name,pointeur_left_value,pointeur_right_value);
		case ReadE:
			return verifie_op_unaire_int(exp,symboleTable, typeErr, func_name,pointeur_left_value,pointeur_right_value);
		default:
			return exp->kind;
	}
}

static void verification_type_pointeur(Node *firstChild, Kind kind_tmp, Kind kind_id, int pointeur_left_value, int pointeur_right_value, Hachage symboleTable, char* func_name,  TypeErreur* typeErr){
	if( pointeur_left_value + valeur_pointeur(firstChild, symboleTable, func_name) != pointeur_right_value + valeur_pointeur(firstChild->nextSibling, symboleTable, func_name))
		addErreurToTable(&(typeErr[PointerErr]),firstChild, firstChild->kind, firstChild->kind, 0,0 );

	if(kind_tmp != kind_id)
		addErreurToTable(&(typeErr[AssigneErr]),firstChild, kind_tmp, kind_id , 0,0 );	
}

static void verification_type_pointeur_assign(Node *firstChild, Kind kind_tmp, Kind kind_id, int pointeur_left_value, int pointeur_right_value, Hachage symboleTable, char* func_name,  TypeErreur* typeErr){
	printf("passe");
	printf(">>> %d\n", valeur_pointeur(firstChild, symboleTable, func_name));
	printf(">>> %d\n", valeur_pointeur(firstChild->nextSibling, symboleTable, func_name));
	if( pointeur_left_value - valeur_pointeur(firstChild, symboleTable, func_name) != pointeur_right_value - valeur_pointeur(firstChild->nextSibling, symboleTable, func_name))
		addErreurToTable(&(typeErr[PointerErr]),firstChild, firstChild->kind, firstChild->kind, 0,0 );

	if(kind_tmp != kind_id)
		addErreurToTable(&(typeErr[AssigneErr]),firstChild, kind_tmp, kind_id , 0,0 );	
}

static void typeAssigne(Node *exp, Hachage symboleTable, TypeErreur* typeErr,char* func_name ){
	Kind kind_op, kind_id;
	Node *firstChild =FIRSTCHILD(exp);
	int pointeur_left_value = 0, pointeur_right_value = 0; // faire un test les deux 0, ou les deux 1

	Kind kind_tmp = typeIdentInTable(firstChild, symboleTable, func_name, &pointeur_left_value, &pointeur_right_value);
	switch(SECONDCHILD(exp)->kind ){
		case IntLiteral:
			if(kind_tmp != Int)
				addErreurToTable(&(typeErr[AssigneErr]),firstChild, kind_tmp, Int , 0,0 );
			break ;
		case CharLiteral:
			if(kind_tmp != Char)
				addErreurToTable(&(typeErr[AssigneErr]),firstChild, kind_tmp, Char , 0,0 );
			break;
		case Identifier:
			if(FIRSTCHILD(exp)!=NULL)
				kind_id = traiteFonction(SECONDCHILD(exp), symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			else
				kind_id = typeIdentInTable(SECONDCHILD(exp), symboleTable, func_name, &pointeur_left_value, &pointeur_right_value);
			verification_type_pointeur(firstChild, kind_tmp, kind_id, pointeur_left_value, pointeur_right_value, symboleTable, func_name, typeErr);
			break;
		case Add:
			kind_op = trouveTypeExp(SECONDCHILD(exp), symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			verification_type_pointeur(firstChild, kind_tmp, kind_op, pointeur_left_value, pointeur_right_value, symboleTable, func_name, typeErr);
			break;
		case Sub:
			kind_op = trouveTypeExp(SECONDCHILD(exp), symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			verification_type_pointeur(firstChild, kind_tmp, kind_op, pointeur_left_value, pointeur_right_value, symboleTable, func_name, typeErr);
			break;
		case Mult:
			kind_op = trouveTypeExp(SECONDCHILD(exp), symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			verification_type_pointeur(firstChild, kind_tmp, kind_op, pointeur_left_value, pointeur_right_value, symboleTable, func_name, typeErr);
			break;
		case Div:
			kind_op = trouveTypeExp(SECONDCHILD(exp), symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			verification_type_pointeur(firstChild, kind_tmp, kind_op, pointeur_left_value, pointeur_right_value, symboleTable, func_name, typeErr);
			break;
		case Mod:
			kind_op = trouveTypeExp(SECONDCHILD(exp), symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			verification_type_pointeur(firstChild, kind_tmp, kind_op, pointeur_left_value, pointeur_right_value, symboleTable, func_name, typeErr);
			break;
		case ValueOf:
			kind_id = trouveTypeExp(SECONDCHILD(exp), symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			verification_type_pointeur_assign(firstChild, kind_tmp, kind_id, pointeur_left_value, pointeur_right_value, symboleTable, func_name, typeErr);
			break;
		case Adress:
			pointeur_right_value = -1;
			kind_id = trouveTypeExp(SECONDCHILD(exp), symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			verification_type_pointeur_assign(firstChild, kind_tmp, kind_id, pointeur_left_value, pointeur_right_value, symboleTable, func_name, typeErr);
			break;
		case Pointer:
			pointeur_right_value = 1;
			kind_id = trouveTypeExp(SECONDCHILD(exp), symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			verification_type_pointeur(firstChild, kind_tmp, kind_id, pointeur_left_value, pointeur_right_value, symboleTable, func_name, typeErr);
			break;
		case Inf:
			verifie_order(exp,symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			break;
		case Sup:
			verifie_order(exp,symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			break;
		case InfOrEq:
			verifie_order(exp,symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			break;
		case SupOrEq:
			verifie_order(exp,symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			break;
		case Equal:
			verifie_eq(exp,symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			break;
		case Different:
			verifie_eq(exp,symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			break;
		case Or:
			verifie_order(exp,symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			break;
		case And:
			verifie_order(exp,symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			break;
		case Opposite:
			verifie_op_unaire_int(exp,symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			break;
		case Positif:
			verifie_op_unaire_int(exp,symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			break;
		case Negatif:
			verifie_op_unaire_int(exp,symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
			break;
		case Print:
			verifie_op_unaire_print(exp,symboleTable, typeErr, func_name,&pointeur_left_value,&pointeur_right_value);
			break;
		case ReadC:
			verifie_op_unaire_char(exp,symboleTable, typeErr, func_name,&pointeur_left_value,&pointeur_right_value);
			break;
		case ReadE:
			verifie_op_unaire_int(exp,symboleTable, typeErr, func_name,&pointeur_left_value,&pointeur_right_value);
			break;
		default:
			break;

	}
}

static void verifie_valeur_retour(Node *exp, Hachage symboleTable, TypeErreur *typeErr, char* func_name, Node *fonction){
	int pointeur_left_value = valeur_pointeur(fonction, symboleTable, func_name);
	int pointeur_right_value = 0;
	Kind kind, kind_fonc;
	Node* tmp = FIRSTCHILD(exp);

	if(FIRSTCHILD(exp)->kind == Pointer){
		pointeur_right_value += 1;
		kind = trouveTypeExp(FIRSTCHILD(exp)->firstChild, symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
		tmp = FIRSTCHILD(exp)->firstChild;
	}
	else if(FIRSTCHILD(exp)->kind == Adress){
		pointeur_right_value -= 1;
		kind = trouveTypeExp(FIRSTCHILD(exp)->firstChild, symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
		tmp = FIRSTCHILD(exp)->firstChild;
	}
	else
		kind = trouveTypeExp(FIRSTCHILD(exp), symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);

	if(FIRSTCHILD(fonction)->kind == Pointer){
		pointeur_right_value += 1;
		kind_fonc = trouveTypeExp(FIRSTCHILD(fonction)->firstChild, symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
	}
	else if(FIRSTCHILD(fonction)->kind == Adress){
		pointeur_right_value -= 1;
		kind_fonc = trouveTypeExp(FIRSTCHILD(fonction)->firstChild, symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
	}
	else
		kind_fonc = trouveTypeExp(FIRSTCHILD(fonction), symboleTable, typeErr, func_name, &pointeur_left_value, &pointeur_right_value);
	
	if(kind == Error)
		return;

	if( pointeur_left_value + valeur_pointeur(tmp, symboleTable, func_name) != pointeur_right_value + valeur_pointeur(tmp->nextSibling, symboleTable, func_name))
		addErreurToTable(&(typeErr[ReturnErr]),tmp, kind, kind_fonc, 0,0 );

	if(kind != kind_fonc)
		addErreurToTable(&(typeErr[ReturnErr]),tmp, kind, kind_fonc , 0,0 );	
}

static void verifier_booleen(Node *exp, Hachage symboleTable, TypeErreur *typeErr, char* func_name, int *pointeur_left_value, int *pointeur_right_value){

	Kind elem1;
	verifie_pointeur_op(FIRSTCHILD(exp), symboleTable, func_name, typeErr);
	elem1 = trouveTypeExp(FIRSTCHILD(exp), symboleTable, typeErr, func_name, pointeur_left_value, pointeur_right_value);
	if( valeur_pointeur(FIRSTCHILD(exp), symboleTable, func_name) == 1)
		addErreurToTable(&(typeErr[PointerErr]),FIRSTCHILD(exp), FIRSTCHILD(exp)->kind, FIRSTCHILD(exp)->kind, 0,0 );
		
	if(elem1 != Int)
		addErreurToTable(&(typeErr[AssigneErr]),FIRSTCHILD(exp), FIRSTCHILD(exp)->kind, FIRSTCHILD(exp)->kind, 0,0 );
}

static void parsestatmList(Node *node, Hachage symboleTable, TypeErreur* typeErr, char* func_name, Node* fonction){

	int left = 0, right = 0;
	if(node == NULL){
		return;
	}
	for(Node *exp = node->firstChild; exp != NULL; exp = exp->nextSibling){
		switch (exp->kind){
			case Inf:
				verifie_order(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case Sup:
				verifie_order(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case InfOrEq:
				verifie_order(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case SupOrEq:
				verifie_order(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case Equal:
				verifie_eq(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case Different:
				verifie_eq(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case Or:
				verifie_order(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case And:
				verifie_order(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case Opposite:
				verifie_op_unaire_int(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case Positif:
				verifie_op_unaire_int(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case Negatif:
				verifie_op_unaire_int(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case Assign :
				typeAssigne(exp, symboleTable, typeErr, func_name);
				break;
			case If : 
				verifier_booleen(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case Else: 
				break;
			case While:
				verifier_booleen(exp,symboleTable, typeErr, func_name,&left,&right); 
				break;
			case Return:
				verifie_valeur_retour(exp, symboleTable, typeErr, func_name, fonction);
				break;
			case Print:
				verifie_op_unaire_print(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case ReadC:
				verifie_op_unaire_char(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case ReadE:
				verifie_op_unaire_int(exp,symboleTable, typeErr, func_name,&left,&right);
				break;
			case Identifier:

				if(FIRSTCHILD(exp)!=NULL)
					traiteFonction(exp, symboleTable, typeErr, func_name,&left,&right);
				else
					typeIdentInTable(exp, symboleTable, func_name, &left, &right);
				break;
			default :
				break;
		}
		parsestatmList(exp, symboleTable,typeErr, func_name, fonction);
	}
}

static void verification_fonction_main(Hachage symboleTable, TypeErreur* typeErr){
	for (Hachage tmp = symboleTable; tmp!= NULL; tmp = tmp->next){
		Elem* sousTable = tmp->element_tableau;
		if( sousTable != NULL && (sousTable->classe == Fonction && strcmp(sousTable->identifier, "main") ==0))
		return;		
	}
	addErreurToTable(&(typeErr[Main]), NULL, Void, Void, 0, 0);
}

static void verifie_return(Node *statmList, Node *fonc, Hachage symboleTable, TypeErreur* typeErr){

	Node *tmp_fct = (Node *)malloc(sizeof(Node));
	if(tmp_fct == NULL)
		return;
	if(FIRSTCHILD(fonc)->kind == Void)
		return;

	for(tmp_fct = statmList->firstChild; tmp_fct != NULL; tmp_fct= tmp_fct->nextSibling){
		if(tmp_fct->kind == Return)
			return;
	}
	addErreurToTable(&(typeErr[ReturnErr]),fonc, Void, fonc->kind , 0,0 );	
}

static void parcoursFuncDecList(Node *tmp_child, Node *tree,  Hachage symboleTable, TypeErreur* typeErr){
	Node *tmp_fct = (Node *)malloc(sizeof(Node));
	if(tmp_fct == NULL)
		return ;
	
	verification_fonction_main(symboleTable, typeErr);
	for(tmp_fct = tmp_child->firstChild; tmp_fct != NULL; tmp_fct= tmp_fct->nextSibling){ // liste des FuncDec
		Node *statmList = SECONDCHILD(FOURTHCHILD(tmp_fct)); // statement list cf l'arbre 
		verifie_return(statmList, tmp_fct, symboleTable, typeErr);
		parsestatmList(statmList, symboleTable, typeErr, SECONDCHILD(tmp_fct)->u.identifier, tmp_fct);
	}
}

void elementsType(Node *tree,  Hachage symboleTable, TypeErreur* typeErr){
	if( tree == NULL){
		return;
	}

	/*// idée de séparer le tableau de symbole en sous liste global, et sous liste de fonction actuelle 
	if( symboleTable->element_tableau != NULL && symboleTable->element_tableau->classe == Global){ 
		
	}*/
	//printSymboleTable(symboleTable);
	Node *tmp_child = (Node *)malloc(sizeof(Node));
	if(tmp_child == NULL)
		return;
	for(tmp_child = tree->firstChild; tmp_child != NULL; tmp_child = tmp_child->nextSibling){
		if(tmp_child->kind == FuncDecList){
			parcoursFuncDecList(tmp_child, tree, symboleTable, typeErr);
		}
	}
}

void printTypeErrTable(TypeErreur *typeErrElem){

	for(int err_typ = 0; err_typ< 7; err_typ++){
		TypeErreur elemTab = typeErrElem[err_typ];
		for (AnErreur* err = elemTab; err!= NULL ; err = err->next){
			switch (err_typ) {
				case ParamErr:
					printf("\033[0;31mErreur sémantique: \033[0mLe parametre %s est de type %s et non de type %s à la ligne %d\n", err->variable, StringFromKind[err->type_variable], StringFromKind[err->type_assigned], err->line);
					break;	
				case AssigneErr:
					if(err->type_variable != Error)
						printf("\033[0;35mWarning: \033[0m%s  est un %s et non un %s à la ligne %d\n", err->variable, StringFromKind[err->type_variable], StringFromKind[err->type_assigned], err->line);
					break;

				case PointerErr:
					printf("\033[0;31mErreur sémantique: \033[0mProbleme de pointeur à la ligne %d\n", err->line);
					break;

				case NumberOfArgs:
					printf("\033[0;31mErreur sémantique: \033[0mLa fonction %s comporte %d paramètres et vous l'appelez avec %d arguments à la ligne %d\n", err->variable, err->number_Of_Args_Given, err->number_Of_Args_expected, err->line);
					break;
				case Main:
					printf("\033[0;31mErreur sémantique: \033[0mVous n'avez pas défini la fonction main\n");
					break;
				case ReturnErr:
					printf("\033[0;35mWarning: \033[0mLa valeur de retour de la fonction est un %s et non un %s à la ligne %d\n", StringFromKind[err->type_variable], StringFromKind[err->type_assigned], err->line);
					break;
				case TypeBooleenErr:
					printf("\033[0;31mErreur sémantique: \033[0mVous comparez un %s et un %s à la ligne %d\n", StringFromKind[err->type_variable], StringFromKind[err->type_assigned], err->line);
				default: 
					break;
			}
		}
	}
}