const handlers = [
    () => console.log("A"),
    (x) => x * 2,
];

function getHandler() {
    return () => console.log("Hi");
}

(() => console.log("hello"))();

button.addEventListener("click", () => console.log("clicked"));

const doubled = numbers.map(n => n * 2);
