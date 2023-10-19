#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../common/types.h"

typedef enum {
  LiteralToken,
  IdentifierToken,

  LeftParenToken,
  RightParenToken,
  LeftBraceToken,
  RightBraceToken,
  LeftBracketToken,
  RightBracketToken,

  ArrowToken,
  PlusToken,

  SemicolonToken,

  // CommaToken,
  // DotToken,


  
  DefToken,

  EOFToken,


} TokenType;

typedef struct {
  TokenType type;
  const char *lexeme;
  size_t lex_length;
} Token;

typedef struct {
  size_t size;
  size_t count;
  Token **tokens;
} TokenList;

typedef struct {
  char *source;
  size_t pos;
  size_t source_length;
} Lexer;

Token *makeToken(TokenType type, const char *lexeme, const size_t lex_length) {
  Token *token = malloc(sizeof(Token));
  if (token == NULL) {
    printf("Error: Could not allocate memory for Token\n");
    return NULL;
  }
  token->type = type;
  token->lexeme = lexeme;
  token->lex_length = lex_length;
  return token;
}

bool match(Lexer *lexer, char *expected) {
  size_t expected_length = strlen(expected);
  if (lexer->pos + expected_length > lexer->source_length) return false;
  for (size_t i = 0; i < expected_length; i++) {
    if (lexer->source[lexer->pos + i] != expected[i]) return false;
  }
  lexer->pos += expected_length;
  return true;
}

bool identifierChar(char c) {
  return (c >= '!' && c <= '~') && c != '!' && c != '(' && c != ')' && c != '{' && c != '}' && c != '[' && c != ']' && c != ';';
  // I should also maybe add initialIdentifierChar so we can allow names like
  // my-func but not -my-func (?)
}

Token *getToken(Lexer *lexer) {
  char c = lexer->source[lexer->pos];
  while (c == ' ' || c == '\n' || c == '\t') {
    lexer->pos++;
    c = lexer->source[lexer->pos];
  }
  switch (c) {
    case '\0':
      return makeToken(EOFToken, NULL, 0);
    case '(':
      lexer->pos++;
      return makeToken(LeftParenToken, NULL, 0);
    case ')':
      lexer->pos++;
      return makeToken(RightParenToken, NULL, 0);
    case '{': 
      lexer->pos++;
      return makeToken(LeftBraceToken, NULL, 0);
    case '}':
      lexer->pos++;
      return makeToken(RightBraceToken, NULL, 0);
    case '[':
      lexer->pos++;
      return makeToken(LeftBracketToken, NULL, 0);
    case ']':
      lexer->pos++;
      return makeToken(RightBracketToken, NULL, 0);
    case '-':
      if (match(lexer, "->")) {
        return makeToken(ArrowToken, NULL, 0);
      }
      break;
    case '+':
      lexer->pos++;
      return makeToken(PlusToken, NULL, 0);
    case ';':
      lexer->pos++;
      return makeToken(SemicolonToken, NULL, 0);
    default:
      if (!identifierChar(c)) {
        printf("Error: Unknown character %c\n", c);
        return NULL;
      }
      size_t start = lexer->pos;
      while (identifierChar(c)) {
        lexer->pos++;
        c = lexer->source[lexer->pos];
      }
      size_t length = lexer->pos - start;
      char *lexeme = malloc(length + 1);
      if (lexeme == NULL) {
        printf("Error: Could not allocate memory for lexeme\n");
        return NULL;
      }
      strncpy(lexeme, lexer->source + start, length);
      lexeme[length] = '\0';
      return makeToken(IdentifierToken, lexeme, length);
  }
}

TokenList *makeTokenList(size_t s) {
  TokenList *tokenList = malloc(sizeof(TokenList));
  if (tokenList == NULL) {
    printf("Error: Could not allocate memory for TokenList\n");
    return NULL;
  }
  tokenList->size = s;
  tokenList->count = 0;
  tokenList->tokens = malloc(sizeof(Token *) * s);
  if (tokenList->tokens == NULL) {
    printf("Error: Could not allocate memory for TokenList->tokens\n");
    free(tokenList);
    return NULL;
  }
  return tokenList;
}

void addToken(TokenList *tokenList, Token *token) {
  if (tokenList->count == tokenList->size) {
    tokenList->size *= 2;
    tokenList->tokens = realloc(tokenList->tokens, sizeof(Token *) * tokenList->size);
    if (tokenList->tokens == NULL) {
      printf("Error: Could not reallocate memory for TokenList->tokens\n");
      free(tokenList);
      return;
    }
  }
  tokenList->tokens[tokenList->count] = token;
  tokenList->count++;
}

void freeTokenList(TokenList *tokenList) {
  for (size_t i = 0; i < tokenList->count; i++) {
    free(tokenList->tokens[i]); // might change to freeToken()
  }
  free(tokenList->tokens);
  free(tokenList);
}

char *readFile(const char *path) {
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    printf("Error: Could not open file %s\n", path);
    return NULL;
  }
  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *buffer = malloc(length + 1);
  if (buffer == NULL) {
    printf("Error: Could not allocate memory\n");
    return NULL;
  }
  size_t objectsRead = fread(buffer, 1, length, file);
  if (objectsRead != length) {
    printf("Error: Could not read file %s\n", path);
    free(buffer);
    return NULL;
  }
  fclose(file);
  buffer[length] = '\0';
  return buffer;
}

void printToken(Token *token) {
  switch (token->type) {
    // case LiteralToken:
    //   printf("LiteralToken: %.*s\n", (int)token->lex_length, token->lexeme);
    //   break;
    // case IdentifierToken:
    //   printf("IdentifierToken: %.*s\n", (int)token->lex_length, token->lexeme);
    //   break;
    // case LeftParenToken: 
    //   printf("LeftParenToken: %.*s\n", (int)token->lex_length, token->lexeme);
    //   break;
    case IdentifierToken:
      printf("IDENTIFIER %.*s\n", (int)token->lex_length, token->lexeme);
      break;
    case DefToken:
      printf("DEF\n");
      break;
    case LeftParenToken:
      printf("(\n");
      break;
    case RightParenToken:
      printf(")\n");
      break;
    case LeftBraceToken:
      printf("{\n");
      break;
    case RightBraceToken:
      printf("}\n");
      break;
    case LeftBracketToken:  
      printf("[\n");
      break;
    case RightBracketToken:
      printf("]\n");
      break;
    case ArrowToken:
      printf("->\n");
      break;
    case PlusToken:
      printf("+\n");
      break;
    case SemicolonToken:
      printf(";\n");
      break;
    case EOFToken:
      printf("EOF\n");
      break;
    default:
      printf("Error: Unknown token type\n");
      break;
  }
}

void printTokenList(TokenList *tokenList) {
  printf("size: %zu\n", tokenList->size);
  printf("count: %zu\n", tokenList->count);
  printf("tokens:\n");
  for (size_t i = 0; i < tokenList->count; i++) {
    Token *token = tokenList->tokens[i];
    printToken(token);
  }
}

int main(int argc, char **argv) {
  Lexer *lexer = malloc(sizeof(Lexer));
  lexer->source = readFile(argv[1]);
  lexer->pos = 0;
  lexer->source_length = strlen(lexer->source);

  if (lexer->source == NULL) return 1;

  TokenList *tl = makeTokenList(10);

  for (Token *token = getToken(lexer); token->type != EOFToken; token = getToken(lexer)) {
    addToken(tl, token);
  }

  // addToken(tl, makeToken(DefToken, NULL, 0));
  // addToken(tl, makeToken(IdentifierToken, "add", 3));
  // addToken(tl, makeToken(LeftParenToken, NULL, 0));
  // addToken(tl, makeToken(IdentifierToken, "ui64", 4));
  // addToken(tl, makeToken(IdentifierToken, "x", 1));
  // addToken(tl, makeToken(RightParenToken, NULL, 0));
  // addToken(tl, makeToken(ArrowToken, NULL, 0));
  // addToken(tl, makeToken(LeftParenToken, NULL, 0));
  // addToken(tl, makeToken(IdentifierToken, "ui64", 4));
  // addToken(tl, makeToken(IdentifierToken, "y", 1));
  // addToken(tl, makeToken(RightParenToken, NULL, 0));
  // addToken(tl, makeToken(ArrowToken, NULL, 0));
  // addToken(tl, makeToken(IdentifierToken, "ui64", 4));
  // addToken(tl, makeToken(LeftBraceToken, NULL, 0));
  // addToken(tl, makeToken(IdentifierToken, "x", 1));
  // addToken(tl, makeToken(PlusToken, NULL, 0));
  // addToken(tl, makeToken(IdentifierToken, "y", 1));
  // addToken(tl, makeToken(SemicolonToken, NULL, 0));
  // addToken(tl, makeToken(RightBraceToken, NULL, 0));
  // addToken(tl, makeToken(SemicolonToken, NULL, 0));
  // addToken(tl, makeToken(EOFToken, NULL, 0));



  printTokenList(tl);
  
  freeTokenList(tl);

  printf("source:\n%s\n", lexer->source);

  //free(lexer->source);
  free(lexer);

}