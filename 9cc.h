#ifndef __9CC_H
#define __9CC_H

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char *fmt, ...);

// container
typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

Vector *new_vector();
void vec_push(Vector *, void *);
void runtest();

// parse
enum {
  TK_NUM = 256, // 整数トークン
  TK_IDENT,     // 識別子
  TK_RETURN,    // return
  TK_EQ,        // ==
  TK_NE,        // !=
  TK_LE,        // <=
  TK_GE,        // >=
};

typedef struct {
  int ty;      // トークンの型
  int val;     // tyがTK_NUMの場合、その数値
  char *input; // トークン文字列(エラーメッセージ用)
} Token;

enum {
  ND_NUM = 256, // 整数のノードの型
  ND_IDENT,     // 識別子のノードの型
  ND_RETURN,    // return
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LE,        // <=
  ND_GE,        // >=
};

typedef struct Node {
  int ty;           // 演算子かND_NUM
  struct Node* lhs; // 左辺
  struct Node* rhs; // 右辺
  int val;          // tyがND_NUMの場合のみ使う
  char name;        // tyがND_IDENTの場合のみ使う
} Node;

void tokenize();
Vector* program();

// codegen
void gen(Node *node);

#endif