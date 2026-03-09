package com.craftinginterpreters.lox;

import java.util.List;

abstract class Stmt {

    interface Visitor<R> {
        R visitBlockStmt(Block stmt);
        R visitExpressionStmt(Expression stmt);
        R visitPrintStmt(Print stmt);
        R visitVarStmt(Var stmt);
        R visitIfStmt(If stmt);
        R visitWhileStmt(While stmt);
        R visitBreakStmt(Break stmt);
        R visitFunctionStmt(Function stmt);
        R visitReturnStmt(Return stmt);
        R visitClassStmt(Class stmt);
        R visitSwitchStmt(Switch stmt);
    }

    static class Class extends Stmt {
        final Token name;
        final Expr.Variable superclass;
        final List<Expr.Variable> mixins;
        final List<Stmt.Function> methods;

        Class(Token name,
              Expr.Variable superclass,
              List<Expr.Variable> mixins,
              List<Stmt.Function> methods) {

            this.name = name;
            this.superclass = superclass;
            this.mixins = mixins;
            this.methods = methods;
        }

        <R> R accept(Visitor<R> visitor) {
            return visitor.visitClassStmt(this);
        }
    }

    static class Switch extends Stmt {
        final Expr expression;
        final List<Expr> cases;
        final List<List<Stmt>> bodies;
        final List<Stmt> defaultBranch;

        Switch(Expr expression,
               List<Expr> cases,
               List<List<Stmt>> bodies,
               List<Stmt> defaultBranch) {

            this.expression = expression;
            this.cases = cases;
            this.bodies = bodies;
            this.defaultBranch = defaultBranch;
        }

        @Override
        <R> R accept(Visitor<R> visitor) {
            return visitor.visitSwitchStmt(this);
        }
    }

    static class Block extends Stmt {
        final List<Stmt> statements;
        Block(List<Stmt> statements) { this.statements = statements; }
        <R> R accept(Visitor<R> visitor) { return visitor.visitBlockStmt(this); }
    }

    static class Expression extends Stmt {
        final Expr expression;
        Expression(Expr expression) { this.expression = expression; }
        <R> R accept(Visitor<R> visitor) { return visitor.visitExpressionStmt(this); }
    }

    static class Print extends Stmt {
        final Expr expression;
        Print(Expr expression) { this.expression = expression; }
        <R> R accept(Visitor<R> visitor) { return visitor.visitPrintStmt(this); }
    }

    static class Var extends Stmt {
        final Token name;
        final Expr initializer;
        Var(Token name, Expr initializer) {
            this.name = name;
            this.initializer = initializer;
        }
        <R> R accept(Visitor<R> visitor) { return visitor.visitVarStmt(this); }
    }

    static class If extends Stmt {
        final Expr condition;
        final Stmt thenBranch;
        final Stmt elseBranch;

        If(Expr condition, Stmt thenBranch, Stmt elseBranch) {
            this.condition = condition;
            this.thenBranch = thenBranch;
            this.elseBranch = elseBranch;
        }

        <R> R accept(Visitor<R> visitor) { return visitor.visitIfStmt(this); }
    }

    static class While extends Stmt {
        final Expr condition;
        final Stmt body;

        While(Expr condition, Stmt body) {
            this.condition = condition;
            this.body = body;
        }

        <R> R accept(Visitor<R> visitor) { return visitor.visitWhileStmt(this); }
    }

    static class Break extends Stmt {
        Break() {}
        <R> R accept(Visitor<R> visitor) { return visitor.visitBreakStmt(this); }
    }

    static class Function extends Stmt {
        final Token name;
        final List<Token> params;
        final List<Stmt> body;

        Function(Token name, List<Token> params, List<Stmt> body) {
            this.name = name;
            this.params = params;
            this.body = body;
        }

        <R> R accept(Visitor<R> visitor) {
            return visitor.visitFunctionStmt(this);
        }
    }

    static class Return extends Stmt {
        final Token keyword;
        final Expr value;

        Return(Token keyword, Expr value) {
            this.keyword = keyword;
            this.value = value;
        }

        <R> R accept(Visitor<R> visitor) {
            return visitor.visitReturnStmt(this);
        }
    }

    abstract <R> R accept(Visitor<R> visitor);
}