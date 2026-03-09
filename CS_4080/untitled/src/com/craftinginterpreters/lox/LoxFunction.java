package com.craftinginterpreters.lox;

import java.util.List;

class LoxFunction implements LoxCallable {

    final Stmt.Function declaration;
    final Environment closure;
    final boolean isInitializer;
    final LoxClass owner;

    LoxFunction(Stmt.Function declaration,
                Environment closure,
                boolean isInitializer,
                LoxClass owner) {

        this.declaration = declaration;
        this.closure = closure;
        this.isInitializer = isInitializer;
        this.owner = owner;
    }

    LoxFunction bind(LoxInstance instance) {

        Environment environment = new Environment(closure);
        environment.define("this", instance);

        return new LoxFunction(declaration, environment, isInitializer, owner);
    }

    @Override
    public int arity() {
        return declaration.params.size();
    }

    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {

        Environment environment = new Environment(closure);

        for (int i = 0; i < declaration.params.size(); i++) {
            environment.define(declaration.params.get(i).lexeme, arguments.get(i));
        }

        String previousMethod = interpreter.currentMethod;
        LoxClass previousClass = interpreter.currentClass;

        interpreter.currentMethod =
                (declaration.name == null) ? null : declaration.name.lexeme;
        interpreter.currentClass = owner;

        try {
            interpreter.executeBlock(declaration.body, environment);
        } catch (Return returnValue) {

            interpreter.currentMethod = previousMethod;
            interpreter.currentClass = previousClass;

            if (isInitializer) return closure.getAt(0, "this");
            return returnValue.value;
        }

        interpreter.currentMethod = previousMethod;
        interpreter.currentClass = previousClass;

        if (isInitializer) return closure.getAt(0, "this");
        return null;
    }

    public String toString() {
        if (declaration.name == null) return "<fn>";
        return "<fn " + declaration.name.lexeme + ">";
    }
}