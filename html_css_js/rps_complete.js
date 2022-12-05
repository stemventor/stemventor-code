// We are now adding the RPS game logic
// We will create a second function playRound() for the game logic, and call the getUserCall function within that.
// Now on the HTML page button click we need to call the playRound function

// Variables defined outside all functions are called global variables
// Which means they can be used by any function.
// Variables within a function are local variables restricted for use by that function only.

const call_options = ["Rock","Paper","Scissors"];

const user_call = "";

let round_winner_call = "";
let user_score = 0;
let computer_score = 0;

let number_of_rounds = 0;
let no_score_count = 0;

function generateRandomNumber(min, max){
	let random_number = Math.floor(Math.random() * (max - min + 1) ) + min;
	return random_number;
}

function getUserCall() {
  // A function will often act upon values entered in the HTML form.
  // To get these values you have to use a library function
  // called getElementById which is called on the document.
  // This returns the element on which you can get its value attribute.

  // At the start of a function you often have to look ahead and reset or clear values that the last run of the function may have set.
  // Clear any previous calls and error messages
  document.getElementById('your-call').innerHTML = "";
  document.getElementById('computer-call').innerHTML = "";
  document.getElementById('round-winner-call').innerHTML = "";
  document.getElementById('error-message').innerHTML = "";
  
  const user_call = document.getElementById('user-call-input').value;
  
  // The console.log is a useful option to debug your code.
  // All browsers allow you to view the console by opening Developer Tools
  console.log(user_call);

  // We will add some checks to the input value to make sure it is one of the three options allowed. For this we can use some very useful JavaScript functions of which there are many.

  // Here we can test if the input value is one of the values in the call_options array using the includes function called on the array.
  // It returns true if the test value is contained in the array or false.
  if(call_options.includes(user_call)){
    // You can also set the value of an element by using the innerHTML property of the element.
    // Set the your-call element to display to whatever was input in the text box and retrieved above.
    document.getElementById('your-call').innerHTML = user_call;  
    // And return the value for use by the playRound() function
    return user_call;
  } else {
    // The entered value is not one of the valid values from the array
    // Display an error message on teh HTML page and return a null value
    document.getElementById('error-message').innerHTML = "Enter one of Rock, Paper, or Scissors!";

    return null;
  }
}

function playRound(){
  // First call the getUserCall function to get the user input
  const user_call = getUserCall();
  
  // Before starting any processing check if user_call is null (incorrect value entered) and if so exit the script, nothing to process further.
  if(user_call === null) exit;
  
  // Increment the number of rounds
  number_of_rounds++;
  

  let index = 1;
  
	let computer_call_number = generateRandomNumber(1, 3);
	let computer_call_value = call_options[computer_call_number - 1];

  // Update the HTML element to display the value
  document.getElementById('computer-call').innerHTML = computer_call_value;

  // Update the HTML element to display the number of rounds
  document.getElementById('rounds').innerHTML = number_of_rounds;

	if(user_call === "Rock" && computer_call_value === "Paper"){
		computer_score++;
    round_winner_call = "Computer wins!";
	}
	else if(user_call === "Rock" && computer_call_value === "Scissors"){
		user_score++;
    round_winner_call = "You win!";
	}
	else if(user_call === "Paper" && computer_call_value === "Rock"){
		user_score++;
    round_winner_call = "You win!";
	}
	else if(user_call === "Paper" && computer_call_value === "Scissors"){
		computer_score++;
    round_winner_call = "Computer wins!";
	}
	else if(user_call === "Scissors" && computer_call_value === "Rock"){
		computer_score++;
    round_winner_call = "Computer wins!";
	}
	else if(user_call === "Scissors" && computer_call_value === "Paper"){
		user_score++;
    round_winner_call = "You win!";
	}
	else{
		no_score_count++;
    round_winner_call = "Draw!";
	}

  // Update the HTML element to display the outcome for the round
  document.getElementById('round-winner-call').innerHTML = round_winner_call;

  // Update the HTML element to display the total user score
  document.getElementById('user-score').innerHTML = user_score;

  // Update the HTML element to display the total computer score
  document.getElementById('computer-score').innerHTML = computer_score;

  // Update the HTML element to display the number of draws
  document.getElementById('no-score-count').innerHTML = no_score_count;
  
}