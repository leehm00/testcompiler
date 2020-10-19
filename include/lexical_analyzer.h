#ifndef _LEXICAL_ANALYZER_H_
#define _LEXICAL_ANALYZER_H_

#include <stdio.h>

extern int fileno (FILE *__stream) __THROW __wur;

#ifndef YYTOKENTYPE
#define YYTOKENTYPE
typedef enum cminus_token_type {
<<<<<<< HEAD
	//运算
	ADD = 259,
	SUB = 260,
	MUL = 261,
	DIV = 262,
	LT = 263,
	LTE = 264,
	GT = 265,
	GTE = 266,
	EQ = 267,
	NEQ = 268,
	ASSIN = 269,
	//符号
	SEMICOLON = 270,//分号
	COMMA = 271,//逗号
	LPARENTHESE = 272,//左小括号
	RPARENTHESE = 273,//右小括号
	LBRACKET = 274,
	RBRACKET = 275,//中括号
	LBRACE = 276,
	RBRACE = 277,//大括号
	//关键字
	ELSE = 278,
	IF = 279,
	INT = 280,
	FLOAT = 281,
	RETURN = 282,
	VOID = 283,
	WHILE = 284,
	//ID和NUM
	IDENTIFIER = 285,
	INTEGER = 286,
	FLOATPOINT = 287,
	ARRAY = 288,
	LETTER = 289,
	//others
	EOL = 290,
	COMMENT = 291,
	BLANK = 292,
	ERROR = 258
=======
    //运算
    ADD = 259,
    SUB = 260,
    MUL = 261,
    DIV = 262,
    LT = 263,
    LTE = 264,
    GT = 265,
    GTE = 266,
    EQ = 267,
    NEQ = 268,
    ASSIN = 269,
    //符号
    SEMICOLON = 270,
    COMMA = 271,
    LPARENTHESE = 272,
    RPARENTHESE = 273,
    LBRACKET = 274,
    RBRACKET = 275,
    LBRACE = 276,
    RBRACE = 277,
    //关键字
    ELSE = 278,
    IF = 279,
    INT = 280,
    FLOAT = 281,
    RETURN = 282,
    VOID = 283,
    WHILE = 284,
    //ID和NUM
    IDENTIFIER = 285,
    INTEGER = 286,
    FLOATPOINT = 287,
    ARRAY = 288,
    LETTER = 289,
    //others
    EOL = 290,
    COMMENT = 291,
    BLANK = 292,
    ERROR = 258
>>>>>>> upstream/master

} Token;


typedef struct{
    char text[256];
    int token;
    int lines;
<<<<<<< HEAD
	int pos_start;
	int pos_end;
=======
    int pos_start;
    int pos_end;
>>>>>>> upstream/master
} Token_Node;

#define MAX_NUM_TOKEN_NODE 1024

void analyzer(char* input_file, Token_Node* token_stream);

#endif /* YYTOKENTYPE */
#endif /* lexical_analyzer.h */
