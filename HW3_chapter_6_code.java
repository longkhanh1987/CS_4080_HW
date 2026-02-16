package com.craftinginterpreters.lox;

import java.util.List;

import static com.craftinginterpreters.lox.TokenType.*;

class Parser {
  private static class ParseError extends RuntimeException {}

  private final List<Token> tokens;
  private int current = 0;

  Parser(List<Token> tokens) {
    this.tokens = tokens;
  }

  // Entry point.
  Expr parse() {
    try {
      return expression();
    } catch (ParseError error) {
      return null;
    }
  }

  // =========================
  // Grammar (with extensions)
  // =========================
  //
  // expression    → comma ;
  // comma         → conditional ( "," conditional )* ;
  // conditional   → equality ( "?" expression ":" conditional )? ;
  // equality      → comparison ( ( "!=" | "==" ) comparison )* ;
  // comparison    → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
  // term          → factor ( ( "-" | "+" ) factor )* ;
  // factor        → unary ( ( "/" | "*" ) unary )* ;
  // unary         → ( "!" | "-" ) unary | primary ;
  // primary       → NUMBER | STRING | "true" | "false" | "nil"
  //              | "(" expression ")" ;

  private Expr expression() {
    return comma();
  }

  // Challenge 1: comma operator (lowest precedence, left-associative)
  private Expr comma() {
    Expr expr = conditional();

    while (match(COMMA)) {
      Token comma = previous();
      Expr right = conditional();
      // Reuse Binary node (evaluate left, discard, return right in interpreter)
      expr = new Expr.Binary(expr, comma, right);
    }

    return expr;
  }

  // Challenge 2: ternary operator ?: (higher than comma, right-associative)
  private Expr conditional() {
    Expr expr = equality();

    if (match(QUESTION)) {
      // In C, the "then" side allows comma expressions.
      Expr thenBranch = expression();
      consume(COLON, "Expect ':' after then branch of conditional expression.");
      // Right-associative by recursing into conditional() on the else side.
      Expr elseBranch = conditional();
      expr = new Expr.Ternary(expr, thenBranch, elseBranch);
    }

    return expr;
  }

  private Expr equality() {
    Expr expr = comparison();

    while (match(BANG_EQUAL, EQUAL_EQUAL)) {
      Token operator = previous();
      Expr right = comparison();
      expr = new Expr.Binary(expr, operator, right);
    }

    return expr;
  }

  private Expr comparison() {
    Expr expr = term();

    while (match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
      Token operator = previous();
      Expr right = term();
      expr = new Expr.Binary(expr, operator, right);
    }

    return expr;
  }

  private Expr term() {
    Expr expr = factor();

    while (match(MINUS, PLUS)) {
      Token operator = previous();
      Expr right = factor();
      expr = new Expr.Binary(expr, operator, right);
    }

    return expr;
  }

  private Expr factor() {
    Expr expr = unary();

    while (match(SLASH, STAR)) {
      Token operator = previous();
      Expr right = unary();
      expr = new Expr.Binary(expr, operator, right);
    }

    return expr;
  }

  private Expr unary() {
    if (match(BANG, MINUS)) {
      Token operator = previous();
      Expr right = unary();
      return new Expr.Unary(operator, right);
    }

    return primary();
  }

  private Expr primary() {
    if (match(FALSE)) return new Expr.Literal(false);
    if (match(TRUE)) return new Expr.Literal(true);
    if (match(NIL)) return new Expr.Literal(null);

    if (match(NUMBER, STRING)) {
      return new Expr.Literal(previous().literal);
    }

    if (match(LEFT_PAREN)) {
      Expr expr = expression();
      consume(RIGHT_PAREN, "Expect ')' after expression.");
      return new Expr.Grouping(expr);
    }

    // Challenge 3: error productions for binary operator missing left operand.
    // Detect a binary operator at the *start* of an expression (where a primary is expected),
    // report the error, parse & discard a right operand at the appropriate precedence.
    if (match(PLUS, STAR, SLASH,
              EQUAL_EQUAL, BANG_EQUAL,
              GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {

      Token operator = previous();
      error(operator, "Missing left-hand operand.");

      // Parse and discard the RHS at the same precedence level this operator expects.
      switch (operator.type) {
        case STAR:
        case SLASH:
          unary();        // factor operands are unary
          break;

        case PLUS:
          factor();       // term operands are factor
          break;

        case GREATER:
        case GREATER_EQUAL:
        case LESS:
        case LESS_EQUAL:
          term();         // comparison operands are term
          break;

        case EQUAL_EQUAL:
        case BANG_EQUAL:
          comparison();   // equality operands are comparison
          break;

        default:
          // Shouldn't happen due to match() list above, but keep it safe.
          expression();
          break;
      }

      // Return something harmless so parsing can continue.
      return new Expr.Literal(null);
    }

    throw error(peek(), "Expect expression.");
  }

  // =========================
  // Helpers / error handling
  // =========================

  private boolean match(TokenType... types) {
    for (TokenType type : types) {
      if (check(type)) {
        advance();
        return true;
      }
    }
    return false;
  }

  private Token consume(TokenType type, String message) {
    if (check(type)) return advance();
    throw error(peek(), message);
  }

  private boolean check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
  }

  private Token advance() {
    if (!isAtEnd()) current++;
    return previous();
  }

  private boolean isAtEnd() {
    return peek().type == EOF;
  }

  private Token peek() {
    return tokens.get(current);
  }

  private Token previous() {
    return tokens.get(current - 1);
  }

  private ParseError error(Token token, String message) {
    Lox.error(token, message);
    return new ParseError();
  }

  // (Used later when you add statements.)
  private void synchronize() {
    advance();

    while (!isAtEnd()) {
      if (previous().type == SEMICOLON) return;

      switch (peek().type) {
        case CLASS:
        case FUN:
        case VAR:
        case FOR:
        case IF:
        case WHILE:
        case PRINT:
        case RETURN:
          return;
      }

      advance();
    }
  }
}
