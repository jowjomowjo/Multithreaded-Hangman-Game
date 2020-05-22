# Client-Server-Application
 A client and server program to play Hangman. The one key difference here is that the player never loses.
 They just keep making guesses until they get it right, and the program will keep track of how many guesses they took to guess the word. 
 Their score for the game will be the number of guesses they took divided by the number of letters in the word. 
 Client programs can connect to the server to play the game. Clients send their guess to the server. 
 The server responds by sending the current game status back to the client.
 The server will choose a new word at random for each client that connects, so different players get different words and the same player 
 will also get a different word if they connect to play another time.

The process of the game goes as follows:
Here is the process to play the game: 
 
1. The player (client) is prompted to enter their name and the name is sent to the server. 
2. A word is chosen at random for the player to guess. The number of letters in the word is sent to the client. 
3. The player may guess a letter. 
4. The client sends the guess to the server. 
5. The server checks if the word contains the guessed letter, and if so, at which positions in the word the letter is found.
   (We want to find all matches, not just the first match.) 
6. The server responds to the client with the result of the guess, which should include:  
    o whether the guess was correct or not 
    o how many guesses have been made so far (including this one - so the server will return 1 after the first guess) 
    o A correct guess should also include all the index positions of where in the word that letter was found.
      (Be sure to include all matches, so there may be more than one index value to return...) 
7. The server should also check if the game is won after each guess. The player wins when they have guessed all the letters in the word. 
   If the game is won, continue to step 8; otherwise go back to step 3 and repeat. 
8. When the game is won, the server will respond with a victory message that indicates the number of turns it took to guess the word. 
   (Don't forget to include the final guess as a turn!) 
9. In addition, the server will send a leader board indicating the name of the top three players along with their scores 
   (lower scores are better). 
 
