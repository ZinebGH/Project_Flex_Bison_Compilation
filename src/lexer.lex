%{
    #include "parser.h"
    int pos_line = 1;
    int pos_char = 0;
%}

%option noinput
%option nounput
%x comment

%%
struct						    {pos_char += yyleng; return STRUCT;}
^.*[\n<<EOF>>]  				{strcpy(yylval.error, yytext); REJECT;}
void 							{pos_char += yyleng; return VOID;}
int|char 						{pos_char += yyleng; strcpy(yylval.type, yytext); return TYPE;}
print 							{pos_char += yyleng; return PRINT;}
\|\|							{pos_char += yyleng; return OR;}
&& 								{pos_char += yyleng; return AND;}
[=!]= 							{pos_char += yyleng; strcpy(yylval.equal, yytext); return EQ;}
if 								{pos_char += yyleng; return IF;}
else 							{pos_char += yyleng; return ELSE;}
return 							{pos_char += yyleng; return RETURN;}
while 							{pos_char += yyleng; return WHILE;}
readc 							{pos_char += yyleng; return READC;}
reade 							{pos_char += yyleng; return READE;}
[\<\>]=? 						{pos_char += yyleng; sscanf(yytext, "%s", yylval.order); return ORDER;}
\+|\- 							{pos_char += yyleng; yylval.sign = yytext[0]; return ADDSUB;}
[a-zA-Z][a-zA-Z0-9_]*			{pos_char += yyleng; strcpy(yylval.ident, yytext); return IDENT;}
'\\?.'							{pos_char += yyleng; yylval.character = yytext[1]; return CHARACTER;}
[0-9]+							{pos_char += yyleng; yylval.integer = atoi(yytext); return NUM; }
[/][*]							{BEGIN comment; pos_char += 1;}
<comment>[*][/] 				{BEGIN INITIAL; pos_char += 1;}
<comment>.						{pos_char += yyleng;}
<comment>\n 					{pos_line += 1; pos_char = 1;}
\/\/.*\n 						{pos_line += 1; pos_char = 1;}
\t 								{pos_char += 3;} 	
" "								{pos_char += 1;}
[\=\+\-*\/%!&,;\(\)\{\}\[\]]	{pos_char += 1; return yytext[0];}
\n 								{pos_line += 1; pos_char =1;}
.								{return 0;}
%%

