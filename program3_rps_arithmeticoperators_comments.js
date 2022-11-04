/* 
 The Rock-Paper-Scissors game.
*/

// In this program we will introduce arithmetic operators.

// START PROGRAM

// Operators act on one or more variables to perform some action.
// There are multiple types of operators, we will look at two commonly used types:
// Arithmetic and Comparison
 
// ARITHMETIC operators:
// These are quite straightforward and perform an arithmetic calculation.
// Note: When assigning a value to a variable you use the '=' operator 
// which is known as the assignment operator.

let a = 20;
let b = 10;
let c = 9;

let result = a + b;
console.log(result); // Addition, will print 30

// Or perform calculations directly in the log statement
console.log(a - b); // Subtraction, will print 10
console.log(a * b + c); // Multiplication and Addition, prioritised as BODMAS, will print 209
console.log(a * (b + c)); // Brackets to override BODMAS, will print 380
console.log(a / b); // Division, will print 2
console.log(a % b); // Modulus or Remainder, will print 0
console.log(a % c); // Modulus or Remainder, will print 2

// In the above examples there were two "operands" the operators worked with (a, b, and c).
// You can also have operators work on a single operand to change a variable value.

console.log(a); // Will print 20, the initial value, we have not changed it so far.

// Now start changing the value of a
a = a + 1;
console.log(a); // Will print 21

// There is an alternative way to do this
a += 1; // This is the same as a = a + 1
console.log(a); // Will print 22

a *= 2; // This is the same as a = a * 2
console.log(a); // Will print 44

// Only for increment and decrement there is a third way
a++;
console.log(a); // Will print 45

a--;
console.log(a); // Will print 44

// END PROGRAM
// To open the console type ctrl+` and go to the DEBUG CONSOLE tab

// Run the program by pressing the F5 key (could be Fn+F5 or just F5 depending on your keyboard configuration)
// Select the Node.js compiler if asked and see the display in the console.

