/* 
 The Rock-Paper-Scissors game.
*/

// In this program we will introduce loops

// START PROGRAM

// LOOPS:
// With arrays you can store an indefinte number of values and then iterate through arrays
// to process each one of those values.
// Loops are used to iterate through arrays

// For the RPS game we will use an array to store the values that can be called out.
// While three are commonly used, you can expand the game to use as many values as you want.
const call_options = ["Rock","Paper","Scissors"];

// There are many ways to loop or iterate through an array, we will show two.

// The "for" loop
// It has four parts: initial value, exit condition, step value, and the body.
// The initial value is the first number the loop uses.
// The exit condition	specifies that the loop should stop executing when the condition is false.
// Or, continue executing the loop while the condition is true.
// The step value is how much to increment (or a negative value for decrement) the value by.
// Based on this it should be clear that for loops can only use numerical values and conditions.
// The body can be a series of statements that are executed repeatedly (in a loop) until the loop exits.

// In all loops exit conditions are very important. 
// The combination of initial value and step must result in the exit condition becoming false at some point.
// If not, you will get something known as an "infinite loop" and the program will eventually fail.
// For example, initial value = 10, exit condition = value > 0, step = 1.
// This will start at 10 and count up by one, therefore always greater than 0 
// and the condition will never be false, thus the loop will never exit.

// This is an infinite loop. If you do try it, ctrl-c should stop the program.
// But be careful, you may end up needing to restart your computer.
/*
for(let i = 10; i > 0; i++){
	console.log("In an infinte loop");
}
*/

let index = 0; // Start the index at the first possible value

for(index; index < call_options.length; index++){ //initial value, exit condition, step.
	// body
	console.log(call_options[index]);
}

console.log("The for loop ran " + index + " times.");

// The while loop
// The while loop has two parts: exit condition and the body.
// The initial value should be set before the while loop starts.
// Since the while loop can have any condition, not just numerical,
// it can have any action in the body that will eventually reach the exit condition
// including a step value just like in a for loop.
// Like in any loop, the exit condition must become false at some point
// or you will end up with an infinite loop.

index = 0; // Reset the index to 0
while(index < call_options.length){ // exit condition.
	// body
	console.log(call_options[index]);
	// step or can be any action to reach the exit condition
	index++;
}

console.log("The while loop ran " + index + " times.");

// You can have a non-numerical condition in a while loop.
// For example, assume you want to only print the values in the array
// until a certain value is found.

let text = call_options[0]; // Initial condition, the first value in the array
index = 0; // Reset the index to 0
while(text !== "Paper"){ // exit condition
	text = call_options[index]; // Change the value to text to the current value in the array
	console.log(text); // Only "Rock" and "Paper" will be printed.
	index++;
}

console.log("The while loop ran " + index + " times.");

// CAUTION: If the array does not contain the value Paper then this will become an infinte loop
// To prevent that add another condition to exit when the length of the array has been reached.

index = 0; // reset index to 0
while(text !== "Not in array" && index !== call_options.length){ // exit conditions
	text = call_options[index]; // Change the value to text to the current value in the array
	console.log(text); // All values in the array will be printed.
	index++;
}

console.log("The while loop ran " + index + " times.");

// END PROGRAM
// To open the console type ctrl+` and go to the DEBUG CONSOLE tab

// Run the program by pressing the F5 key (could be Fn+F5 or just F5 depending on your keyboard configuration)
// Select the Node.js compiler if asked and see the display in the console.

