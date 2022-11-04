/* 
 The Rock-Paper-Scissors game.
*/

// In this program we will introduce comparison and logical operators and conditional statements.

// START PROGRAM

// COMPARISON OPERATORS:
// These are used to compare two values (could be two variables or a variable and a value).
// The result of a comparison can be one of "true" or "false"
// These are known as Boolean values.

const call_options = ["Rock","Paper","Scissors"];

// "Equal To" Operator (===)
let result = call_options[0] === "Rock";
console.log(result); // Will print "true"

// "Not Equal To" Operator (===)
result = call_options[0] !== "Rock";
console.log(result); // Will print "false", since it is equal to "Rock"

result = call_options[0] !== "Paper";
console.log(result); // Will print "true", since it is not equal to "Paper"

// "Greater Than" operator (>)
let a = 1, b = 2; // Multiple variables can be delcared in the same let statement.
console.log(b > a); // Will print "true"
console.log(a > b); // Will print "false"

// "Less Than" operator (<)
console.log(a < b); // Will print "true"

// "Greater Than or Equal To" operator (>=)
console.log(1 >= a); // Will print "true"
console.log(b >= a); // Will print "true" 

// "Less Than or Equal To" operator (<=)
console.log(a <= 1); // Will print "true"
console.log(a <= b); // Will print "true" 
console.log(b <= a); // Will print "false"

// LOGICAL OPERATORS:
// These operators allow you to combine two or more conditional statements.
// There are two commonly used logical operators "AND" (&&) and "OR" (||) 
// As we know the result of a comparison can be one of "true" or "false".
// When multiple comparison statements are joined together with a logical operator
// the result is a Boolean value as per the following logic:
// With "AND" (&&) all conditions joined by the operator must be true for the overall result to be true.
// With "OR" (||) any one of the conditions joined by the operator must be true for the overall result to be true.

// AND
console.log(call_options[0] === "Rock" && call_options[1] === "Paper"); // Will print "true" since both are true
console.log(call_options[0] === "Rock" && call_options[1] === "Scissors"); // Will print "false" since one is not true

// OR
console.log(call_options[0] === "Rock" || call_options[1] === "Paper"); // Will print "true" since both are true
console.log(call_options[0] === "Rock" || call_options[1] === "Scissors"); // Will print "true" since at least one is true
console.log(call_options[0] === "Paper" || call_options[1] === "Scissors"); // Will print "false" since both are false

// CONDITIONAL STATEMENTS (if-else):
// These use conditional statements to decide what body of code should be executed.
// There are two parts: the "if" condition and body and the "else" body.
// If the condition provided to "if" is true that body is executed 
// or the "else" body is executed.

let random_call = "Rock";

if(random_call === call_options[0]){ // This condition is true
	console.log("Rock was called."); // so this body will be executed.
}else{
	console.log("Rock was not called"); // This will not be executed.
}

random_call = "Paper";

if(random_call === call_options[0]){ // This condition is false
	console.log("Rock was called."); // so this body will not be executed.
}else{
	console.log("Rock was not called"); // This will be excuted.
}

// END PROGRAM
// To open the console type ctrl+` and go to the DEBUG CONSOLE tab

// Run the program by pressing the F5 key (could be Fn+F5 or just F5 depending on your keyboard configuration)
// Select the Node.js compiler if asked and see the display in the console.

