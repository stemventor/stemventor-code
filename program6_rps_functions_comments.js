/* 
 The Rock-Paper-Scissors game.
*/

// In this program we will introduce functions and libraries.

// START PROGRAM

// FUNCTIONS
// Most programs have blocks of code with some calculations or logic that are
// required to be used multiple times in the program.
// Instead of writing the same logic everywhere it is required, you can use
// what are known as "functions".
// Functions make take zero, one or more "parameters" as inputs and ususally "return" a value.
// Wherever required in the program the function is "called".

// Functions can be user-defined, for whatever logic the program may need.
// There are also a pre-defined set of functions made available with programming languages
// that implement very commonly used logic that many programs are likely to need.
// A collection of such functions that ship with a langauge are known as "libraries".

// A function has a name just like a variable, but is defined with the keyword "function".

// A random number generating function
function generateRandomNumber(min, max){ // min and max are the parameters

	// There is a library of functions called the Math library which has a random number generator function.
	let fractional_random_number = Math.random(); // Call the function, no parameters required.
	console.log(fractional_random_number);

	// This however generates fractional numbers between 0 and 1
	// For our RPS game we want a random number between 1 and 3, for a random call option
	// For that we ad some additional logic around the Math.random() function

	// For now, don't worry about understanding this logic, but just know that the statement below
	// will give a number between the parameters min and max (which in our case will be 1 and 3)
	let random_number = Math.floor(Math.random() * (max - min + 1) ) + min;

	// The function will now return the random number between 1 and 3 wherever we need it in the program.
	return random_number;
}

// Now in the main program call the function as many times as required, with different parameters if required.

let random_call_option = generateRandomNumber(1, 3)
console.log(random_call_option);

random_call_option = generateRandomNumber(100, 1000)
console.log(random_call_option);

random_call_option = generateRandomNumber(0, 99)
console.log(random_call_option);

// END PROGRAM
// To open the console type ctrl+` and go to the DEBUG CONSOLE tab

// Run the program by pressing the F5 key (could be Fn+F5 or just F5 depending on your keyboard configuration)
// Select the Node.js compiler if asked and see the display in the console.

