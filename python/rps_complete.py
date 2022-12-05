# The Rock-Paper-Scissors game.

# Note that Python uses indentation to identify blocks of code instead of {} that most other langauges use. Pay attention to the tabs and spaces you use.

# START PROGRAM

# import random a library used to generate random numbers
import random
 
# Python uses Lists instead of Arrays as in most languages but they are almost the same.
call_options = ["Rock","Paper","Scissors"]

# Initialize the variables you need in the program.
player_score = 0
computer_score = 0
number_of_rounds = 0
no_score_count = 0

# Display information to the user
print("Let's play Rock-Paper-Scissors")

# Get the user name as input from the user
player_name = input("Enter your name: ")
print("Hello " + player_name)

# Get the number of rounds to play as input from the user
# Since the input is a string data type you have to
# "cast" it to integer so you can perform arithmetic 
# calculations with the value.

# Validate input, keep asking until a valid positive
# number is entered.
while number_of_rounds <= 0:
  number_of_rounds = int(input("How many rounds should we play? "))  

def generateRandomNumber(min, max):
  return random.choice(call_options)

# Now the actual game logic begins

index = 1
for index in range(1, number_of_rounds+1):
  player_call = ""
  
  while player_call not in call_options:
    print("Enter Rock, Paper, or Scissors: ")
    player_call = input("Your call: ")
  else:
    computer_call = random.choice(call_options)
    print("Round ", index)
    print("--------")
    print(player_name + " called " + player_call + ".")
    print("Computer called " + computer_call + ".")
  
    if player_call == "Rock" and computer_call == "Paper":
      computer_score += 1  
      print("Computer won!")
    elif player_call == "Rock" and computer_call == "Scissors":
      player_score += 1
      print("Player won!")
    elif player_call == "Paper" and computer_call == "Rock":
      player_score += 1
      print("Player won!")
    elif player_call == "Paper" and computer_call == "Scissors":
      computer_score += 1
      print("Computer won!")
    elif player_call == "Scissors" and computer_call == "Rock":
      computer_score += 1
      print("Computer won!")
    elif player_call == "Scissors" and computer_call == "Paper":
      player_score += 1
      print("Player won!")
    else:
      no_score_count += 1
      print("Draw!")
    print("--------")
  
#Finally, print the scores and declare a winner.

print("-----------")
print("FINAL SCORE")
print("-----------")
print(player_name + " won: ", player_score)
print("Computer won: ", computer_score)
print("No Scores: ", no_score_count)

if player_score > computer_score:
  print(player_name + " wins!")
elif player_score < computer_score:
  print("Computer wins!")
else:
  print("It's a draw!")

# END PROGRAM