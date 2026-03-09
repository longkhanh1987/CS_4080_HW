package com.craftinginterpreters.lox;

import java.util.List;
import java.util.Map;

class LoxClass implements LoxCallable {

    final String name;
    final LoxClass superclass;
    private final Map<String, LoxFunction> methods;

    LoxClass(String name, LoxClass superclass,
             Map<String, LoxFunction> methods) {
        this.name = name;
        this.superclass = superclass;
        this.methods = methods;
    }

    LoxFunction findMethod(String name) {

        LoxClass root = this;

        while (root.superclass != null) {
            root = root.superclass;
        }

        return root.findMethodDown(name);
    }

    LoxFunction findMethodDown(String name) {

        if (methods.containsKey(name)) {
            return methods.get(name);
        }

        if (superclass != null) {
            return superclass.findMethodDown(name);
        }

        return null;
    }

    LoxFunction findMethodBelow(String name) {

        if (superclass == null) return null;

        if (superclass.methods.containsKey(name)) {
            return superclass.methods.get(name);
        }

        return superclass.findMethodBelow(name);
    }

    @Override
    public int arity() {
        LoxFunction initializer = findMethod("init");
        if (initializer == null) return 0;
        return initializer.arity();
    }

    @Override
    public Object call(Interpreter interpreter,
                       List<Object> arguments) {

        LoxInstance instance = new LoxInstance(this);

        LoxFunction initializer = findMethod("init");

        if (initializer != null) {
            initializer.bind(instance).call(interpreter, arguments);
        }

        return instance;
    }

    public String toString() {
        return name;
    }
}