/* 
 The Rock-Paper-Scissors game.
*/

// We will write the logic for the RPS game using the constructs we have learnt so far.
// Part 1 (this program): We will start by generating random calls for the two players and displaying them on the console.
// Part 2 (next program):  We will add the logic to determine the winner of each round and add up the scores.

// START PROGRAM

// For the RPS game we will use an array to store the values that can be called out.
// While three are commonly used, you can expand the game to use as many values as you want.
const call_options = ["Rock","Paper","Scissors"];

// Let's add a few more variables
const player1_name = "Adam"; // Since we can only have two players, we do not need an array.
const player2_name = "Eve"; // And they will not change so can be "const".

let player1_score = 0; // These are variables with a number data type.
let player2_score = 0; // The values will keep changing so use "let" not "const".

const number_of_rounds = 10; // How many rounds to play?

// Define the random number generation program
function generateRandomNumber(min, max){ // min and max are the parameters
	// For now, don't worry about understanding this logic, but just know that the statement below
	// will give a number between the parameters min and max (which in our case will be 1 and 3)
	let random_number = Math.floor(Math.random() * (max - min + 1) ) + min;

	// The function will now return the random number between 1 and 3 wherever we need it in the program.
	return random_number;
}

// Now the actual game logic begins
// The objective is to generate a random call for each player using the random number generator function.
// Using the random number get the value of the call options from the array.
// The logic of the game is that Rock beats Scissors, Paper beats Rock and Scissors beat Paper.

// We have to run the rounds in a loop, let's use a for loop
let index = 1;
for(index; index <= number_of_rounds; index++){
	let player1_call_number = generateRandomNumber(1, 3);
	let player2_call_number = generateRandomNumber(1, 3);

	// Get the value from the array at the call index
	// Note that the random numbers are from 1 to 3 whereas our array indexes go from 0 to 2.
	// So subtract 1 to use as the index. Alternately, generate random numbers between 0 and 2.  

	let player1_call_value = call_options[player1_call_number - 1];
	let player2_call_value = call_options[player2_call_number - 1]; 

	console.log(player1_name + " called " + player1_call_value + ".");
	console.log(player2_name + " called " + player2_call_value + ".");
}

// END PROGRAM
// To open the console type ctrl+` and go to the DEBUG CONSOLE tab

// Run the program by pressing the F5 key (could be Fn+F5 or just F5 depending on your keyboard configuration)
// Select the Node.js compiler if asked and see the display in the console.

