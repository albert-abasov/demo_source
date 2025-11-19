// ===== Imports =====
import { readFileSync } from "fs";
import helper from "./helper.js";

// ===== Simple Class =====
class Person {}

// ===== Class With Methods =====
class PersonWithMethods {
    constructor(name, age) {
        this.name = name;
        this.age = age;
    }

    greet() {
        console.log(`Hello, my name is ${this.name} and I'm ${this.age} years old.`);
    }
}

// ===== Function Declaration =====
function sum(a, b) {
    return a + b;
}

// ===== Arrow Functions =====
const multiply = (a, b) => a * b;

const applyDiscount = (price, discount) => {
    const reduced = price - discount;
    return reduced < 0 ? 0 : reduced;
};

const createProduct = (name, price) => ({
    id: crypto.randomUUID(),
    name,
    price
});

// ===== Exports Test =====
const PI = 3.14;

function area(r) {
    return PI * r * r;
}

const double = n => n * 2;

export { area, double, PI };

// ===== Inheritance =====
class Animal {
    speak() {}
}

class Dog extends Animal {
    speak() {
        console.log("Woof");
    }
}

// ===== Static + Private Fields / Methods =====
class Counter {
    #count = 0;

    static increment(counterInstance) {
        counterInstance.#count++;
    }

    #reset() {
        this.#count = 0;
    }
}
