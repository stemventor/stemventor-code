/* 
 The programs from this point on will be written with a task in mind.
 We will simulate the Rock-Paper-Scissors (RPS) game where the computer
 generates random values for two players, plays a defined number of times
 and adds up the scores.
 Ideally it should be user against the computer for which you need to prorgam
 a way to take in user input. We will do that in a later program.
 The program will build up in steps, adding a new programming construct
 at each step till we have the final program. 
*/

// In this program we will introduce arrays

// START PROGRAM

// ARRAYS:
// Arrays are an extension to variables which allow you to store a
// collection of similar values. There are two useful things you can achieve
// with arrays that you cannot with simple variables.
// One, you can store an indefinte number of values and then iterate through arrays
// to process each one of those values.
// Two, you can reference any of the values in the array by its position in the 
// array, also known as the index.

// Declare and initialize an array that contains the days of the week
// This is as an example and not required for the RPS game

const days_of_the_week = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday","Sunday"];

// You can get the name of the day of the week based on its sequence in the week
// using an array index

const first_day = days_of_the_week[0]; // Note that array indexes start from 0 not 1
console.log(first_day); // Will print Monday

console.log(days_of_the_week[1]); // Will print Tuesday

const last_day = days_of_the_week[6];
console.log(last_day); // Will print Sunday

// You have to be careful with array indexes. Using an index that is greater
// than the number of elements will cause what is known as a "run time error".
// Which means your program will compile fine but could fail when it is running.

// You assume 7 days of the week, but since indexes start from 0, the last index will be 6
// This will compile ok but return an undefined value when you run the program.

console.log(days_of_the_week[7]);

// To avoid this most languages come with a function to get the length of the array
// since it is can change dynamically.
// Then when you want to get the last element you use the length value to determine the index
// which will be one less than the length (indexes start from 0, right?)

const length_of_array = days_of_the_week.length;
console.log("Length of the array is " + length_of_array);
console.log(days_of_the_week[length_of_array - 1]);

// For the RPS game we will use an array to store the values that can be called out.
// While three are commonly used, you can expand the game to use as many values as you want.
const call_options = ["Rock","Paper","Scissors"];

// END PROGRAM
// To open the console type ctrl+` and go to the DEBUG CONSOLE tab

// Run the program by pressing the F5 key (could be Fn+F5 or just F5 depending on your keyboard configuration)
// Select the Node.js compiler if asked and see the display in the console.

