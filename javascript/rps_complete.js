/* 
 The Rock-Paper-Scissors game.
*/

// START PROGRAM

const call_options = ["Rock","Paper","Scissors"];

const player1_name = "Adam";
const player2_name = "Eve";

let player1_score = 0;
let player2_score = 0;

const number_of_rounds = 10;
let no_score_count = 0;

function generateRandomNumber(min, max){
	let random_number = Math.floor(Math.random() * (max - min + 1) ) + min;
	return random_number;
}

// Now the actual game logic begins

let index = 1;
for(index; index <= number_of_rounds; index++){
	let player1_call_number = generateRandomNumber(1, 3);
	let player2_call_number = generateRandomNumber(1, 3);

	let player1_call_value = call_options[player1_call_number - 1];
	let player2_call_value = call_options[player2_call_number - 1]; 

	console.log("Round " + index);
	console.log("--------");
	console.log(player1_name + " called " + player1_call_value + ".");
	console.log(player2_name + " called " + player2_call_value + ".");

	if(player1_call_value === "Rock" && player2_call_value === "Paper"){
		player2_score++;
    console.log(player2_name + " won!");
	}
	else if(player1_call_value === "Rock" && player2_call_value === "Scissors"){
		player1_score++;
    console.log(player1_name + " won!");
	}
	else if(player1_call_value === "Paper" && player2_call_value === "Rock"){
		player1_score++;
    console.log(player1_name + " won!");
	}
	else if(player1_call_value === "Paper" && player2_call_value === "Scissors"){
		player2_score++;
    console.log(player2_name + " won!");
	}
	else if(player1_call_value === "Scissors" && player2_call_value === "Rock"){
		player2_score++;
    console.log(player2_name + " won!");
	}
	else if(player1_call_value === "Scissors" && player2_call_value === "Paper"){
		player1_score++;
    console.log(player1_name + " won!");
	}
	else{
		no_score_count++;
    console.log("Draw");
	}
  console.log("--------");
}

// Finally, print the scores and declare a winner.

console.log("-----------");
console.log("FINAL SCORE");
console.log("-----------");
console.log("Adam won: " + player1_score);
console.log("Eve won: " + player2_score);
console.log("No Scores: " + no_score_count);

if(player1_score > player2_score){
	console.log("Adam wins!");
}
else if(player1_score < player2_score){
	console.log("Eve wins!");
} else{
	console.log("It's a draw!");
}

// END PROGRAM
// To open the console type ctrl+` and go to the DEBUG CONSOLE tab

// Run the program by pressing the F5 key (could be Fn+F5 or just F5 depending on your keyboard configuration)
// Select the Node.js compiler if asked and see the display in the console.