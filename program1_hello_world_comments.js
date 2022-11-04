/* 
 This is a very simple program that does nothing but print
 Hello World to the console. It has been a traditional approach for
 many years to start learning a new language with a Hello World program.
 
 This will get you started on understanding and using the IDE
 and making sure your development environment is working fine.
  
 First thing to learn, these lines of text starting with 
 a forward slash (/) and an asterisk (*) 
 and ending with an asterisk (*) and a forward slash (/)
 are comments. These are used to explain the lines of code and
 are ignored by the compiler and computer when executing the code.
 You can type anything in comments, except the comment start 
 and end markers.
*/

// Starting a line with two forward slashes is another way to write a comment
// This is better for single line comments, while the previous approach 
// is better for multi-line comments.

// The actual program starts now:
// START PROGRAM

// VARIABLES:
// Variables store values that are required for a program either as starting values
// or calculated values. Variables are usually descriptive names that help the programmer
// understand what the value is for.
// Variable names cannot start with a number, some special characters are not allowed,
// and they can't be some reserved words that the programming language uses.
// For example you cannot name a variable as let or const.

let hello_world = "Hello World!"; // Define a variable and assign it a text value
// let ewirjdn = "Hello World!"; // Is also ok but the variable name is very hard to remember 

console.log(hello_world); // Print the text to the console

// A variable can also be declared as constant (although that sounds a bit confusing)
// What it means is that once it is assigned a value at the time of declaration it cannot
// be changed by the program.

const greeting = "Hello ";
let person_name = "";

person_name = "You.";
console.log(greeting + person_name); // You can concatenate two value with a + sign

person_name = "Me."
console.log(greeting + person_name);

// A change to the value of greeting won't be allowed
// Uncomment and see the error
// greeting = "Hi ";

// Note the semicolon (;) at the end of each stetement. It is mandatory in some languages, 
// optional in JavaScript but a good practice to help you understand the end of a statement. 

// END PROGRAM
// To open the console type ctrl+` and go to the DEBUG CONSOLE tab

// Run the program by pressing the F5 key (could be Fn+F5 or just F5 depending on your keyboard configuration)
// Select the Node.js compiler if asked and see the display in the console.

