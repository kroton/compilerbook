#include "9cc.h"

Vector* tokens;
int pos;

Token *new_token() {
  Token *token = malloc(sizeof(Token));
  vec_push(tokens, token);
  return token;
}

Token *get_token(int pos) {
  return tokens->data[pos];
}

void tokenize(char *p) {
  tokens = new_vector();

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    Token *token = new_token();

    if (strncmp(p, "==", 2) == 0) {
      token->ty = TK_EQ;
      token->input = p;
      p += 2;
      continue;
    }

    if (strncmp(p, "!=", 2) == 0) {
      token->ty = TK_NE;
      token->input = p;
      p += 2;
      continue;
    }

    if (strncmp(p, "<=", 2) == 0) {
      token->ty = TK_LE;
      token->input = p;
      p += 2;
      continue;
    }

    if (strncmp(p, ">=", 2) == 0) {
      token->ty = TK_GE;
      token->input = p;
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>') {
      token->ty = *p;
      token->input = p;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      token->ty = TK_NUM;
      token->input = p;
      token->val = strtol(p, &p, 10);
      continue;
    }

    error("トークナイズできません: %s", p);
    exit(1);
  }

  Token *token = new_token();
  token->ty = TK_EOF;
  token->input = p;
}

int consume(int ty) {
  if (get_token(pos)->ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *equality();
Node *relational();
Node *add();
Node *term();
Node *unary();
Node *mul();

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume(TK_EQ)) {
      node = new_node(TK_EQ, node, relational());
    } else if (consume(TK_NE)) {
      node = new_node(TK_NE, node, relational());
    } else {
      return node;
    }
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume('<')) {
      node = new_node('<', node, add());
    } else if (consume(TK_LE)) {
      node = new_node(TK_LE, node, add());
    } else if (consume('>')) {
      node = new_node('>', node, add());
    } else if (consume(TK_GE)) {
      node = new_node(TK_GE, node, add());
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume('+')) {
      node = new_node('+', node, mul());
    } else if (consume('-')) {
      node = new_node('-', node, mul());
    } else {
      return node;
    }
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume('*')) {
      node = new_node('*', node, unary());
    } else if (consume('/')) {
      node = new_node('/', node, unary());
    } else {
      return node;
    }
  }
}

Node *unary() {
  if (consume('+')) {
    return term();
  }
  if (consume('-')) {
    return new_node('-', new_node_num(0), term());
  }
  return term();
}

Node *term() {
  Token *token = get_token(pos);

  if (consume('(')) {
    Node *node = equality();
    if (!consume(')')) {
      error("開きカッコに対応する閉じカッコがありません: %s",
            token->input);
    }
    return node;
  }

  if (token->ty == TK_NUM) {
    ++pos;
    return new_node_num(token->val);
  }

  error("数値でも開きカッコでもないトークンです: %s", 
        token->input);
}