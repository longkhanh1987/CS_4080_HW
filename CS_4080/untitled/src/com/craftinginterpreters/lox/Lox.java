package com.craftinginterpreters.lox;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;

public class Lox {
    private static final Interpreter interpreter = new Interpreter();
    static boolean hadError = false;
    static boolean hadRuntimeError = false;

    public static void main(String[] args) throws IOException {
        if (args.length > 1) {
            System.out.println("Usage: jlox [script]");
            System.exit(64);
        } else if (args.length == 1) {
            runFile(args[0]);
        } else {
            runPrompt();
        }
    }

    private static void runFile(String path) throws IOException {
        byte[] bytes = Files.readAllBytes(Paths.get(path));
        run(new String(bytes, Charset.defaultCharset()));

        if (hadError) System.exit(65);
        if (hadRuntimeError) System.exit(70);
    }

    private static void runPrompt() throws IOException {
        InputStreamReader input = new InputStreamReader(System.in);
        BufferedReader reader = new BufferedReader(input);

        StringBuilder buffer = new StringBuilder();
        int openBraces = 0;

        for (;;) {
            System.out.print(openBraces > 0 ? "... " : "> ");
            String line = reader.readLine();
            if (line == null) break;

            buffer.append(line).append("\n");

            for (int i = 0; i < line.length(); i++) {
                char c = line.charAt(i);
                if (c == '{') openBraces++;
                else if (c == '}') openBraces--;
            }

            if (openBraces > 0) continue;

            runReplChunk(buffer.toString());

            buffer.setLength(0);
            hadError = false;
        }
    }

    // REPL accepts expressions (print result) and statements (execute)
    private static void runReplChunk(String source) {
        Scanner scanner = new Scanner(source);
        List<Token> tokens = scanner.scanTokens();

        // If first token starts a statement, don't try expression mode.
        if (!tokens.isEmpty()) {
            TokenType first = tokens.get(0).type;
            if (first == TokenType.VAR ||
                    first == TokenType.PRINT ||
                    first == TokenType.LEFT_BRACE ||
                    first == TokenType.IF ||
                    first == TokenType.WHILE ||
                    first == TokenType.FOR ||
                    first == TokenType.BREAK ||
                    first == TokenType.RETURN) {

                Parser stmtParser = new Parser(tokens);
                List<Stmt> statements = stmtParser.parse();
                if (hadError) return;
                interpreter.interpret(statements);
                return;
            }
        }

        // Try as expression first.
        Parser exprParser = new Parser(tokens);
        Expr expr = exprParser.parseExpression();
        if (!hadError && expr != null) {
            interpreter.interpretExpression(expr);
            return;
        }

        // Fall back to statements.
        hadError = false;
        Parser stmtParser = new Parser(tokens);
        List<Stmt> statements = stmtParser.parse();
        if (hadError) return;
        interpreter.interpret(statements);
    }

    private static void run(String source) {
        Scanner scanner = new Scanner(source);
        List<Token> tokens = scanner.scanTokens();

        Parser parser = new Parser(tokens);
        List<Stmt> statements = parser.parse();

        if (hadError) return;
        interpreter.interpret(statements);
    }

    // ---------- Error reporting ----------

    static void error(int line, String message) {
        report(line, "", message);
    }

    private static void report(int line, String where, String message) {
        System.err.println("[line " + line + "] Error" + where + ": " + message);
        hadError = true;
    }

    static void error(Token token, String message) {
        if (token.type == TokenType.EOF) {
            report(token.line, " at end", message);
        } else {
            report(token.line, " at '" + token.lexeme + "'", message);
        }
    }

    static void runtimeError(RuntimeError error) {
        System.err.println(error.getMessage() +
                "\n[line " + error.token.line + "]");
        hadRuntimeError = true;
    }
}