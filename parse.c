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

Token *get_current_token() {
  return get_token(pos);
}

void next_token() {
  ++pos;
}

int has_token() {
  return pos < tokens->len;
}

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') || 
         ('A' <= c && c <= 'Z') || 
         ('0' <= c && c <= '9') ||
         (c == '_');
}

char *dup_ident_name(char *p) {
  char *q = p;
  while (*q && is_alnum(*q)) ++q;
  return strndup(p, q - p);
}

void tokenize(char *p, int *idents) {
  tokens = new_vector();

  *idents = 0;
  Map *ident_offsets = new_map();
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    Token *token = new_token();

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      token->ty = TK_RETURN;
      token->input = p;
      p += 6;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      char *name = dup_ident_name(p);
      if (map_get(ident_offsets, name) == NULL) {
        int offset = ++(*idents);
        map_put(ident_offsets, name, (void *)offset);
      }
      token->ty = TK_IDENT;
      token->offset = (long) map_get(ident_offsets, name);
      token->input = p;
      p += strlen(name);
      continue;
    }

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

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == '=' || *p == ';') {
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
}

int consume(int ty) {
  if (get_current_token()->ty != ty)
    return 0;
  next_token();
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

Node *new_node_ident(int offset) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->offset = offset;
  return node;
}

Node *new_node_return(Node *lhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_RETURN;
  node->lhs = lhs;
  return node;
}

Node *stmt();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *term();
Node *unary();
Node *mul();

Vector* program() {
  Vector *code = new_vector();
  while (has_token()) {
    vec_push(code, stmt());
  }
  return code;
}

Node *stmt() {
  Node *node;

  if (consume(TK_RETURN)) {
    node = new_node_return(assign());
  } else {
    node = assign();
  }
  if (!consume(';')) {
    error("';'ではないトークンです: %s", get_current_token()->input);
  }
  return node;
}

Node *assign() {
  Node *node = equality();
  if (consume('=')) {
    node = new_node('=', node, assign());
  }
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume(TK_EQ)) {
      node = new_node(ND_EQ, node, relational());
    } else if (consume(TK_NE)) {
      node = new_node(ND_NE, node, relational());
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
      node = new_node(ND_LE, node, add());
    } else if (consume('>')) {
      node = new_node('>', node, add());
    } else if (consume(TK_GE)) {
      node = new_node(ND_GE, node, add());
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
  Token *token = get_current_token();

  if (consume('(')) {
    Node *node = assign();
    if (!consume(')')) {
      error("開きカッコに対応する閉じカッコがありません: %s",
            token->input);
    }
    return node;
  }

  if (token->ty == TK_NUM) {
    next_token();
    return new_node_num(token->val);
  }

  if (token->ty == TK_IDENT) {
    next_token();
    return new_node_ident(token->offset);
  }

  error("数値でも開きカッコでも変数でもないトークンです: %s", 
        token->input);
}