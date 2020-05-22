#include <iostream>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
using namespace std;

const int BUFFER_SIZE = 1024;

//argv[1] = IP address, argv[2] = port number
int main(int argc, const char* argv[])
{
    string name;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    int port_num = atoi(argv[2]);

    //error checking for command line arguments
    if(argc != 3)
    {
        printf("\nNumber of arguments is incorrect\n");
        return -1;
    }

    if(port_num == 0) //atoi() returns 0 when fail
    {
        printf("\nPort number is invalid\n");
        return -1;
    }

    //create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nSocket creation failed\n");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_num);

    //convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported\n");
        return -1;
    }

    //connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection failed\n");
        return -1;
    }

    //ask user for their name and send name to the server
    //if no name is given, loop again
    do
    {
        cout << "Welcome to Hangman!" << endl;
        cout << "Enter your name: ";
        getline(cin, name);
        name += "|"; //add a delimeter to mark end of name

        //loop ensures everything is sent
        while(send(sock, name.c_str(), name.length(), 0) != (unsigned int) name.length());
        
        //make sure a name is entered
        recv(sock, buffer, 1, 0); 
        if(buffer[0] == 'i')
            printf("Invalid name. Re-enter.\n\n");
    } while(buffer[0] == 'i');

    //the game starts here
    //keep looping until the word have been guessed correctly 
    char* token;
    do
    {        
        //clear buffer
        memset(buffer, 0, sizeof(buffer));

        cout << endl;

        //receive number of turns and word_output
        while (!strchr(buffer, '|')) 
            recv(sock, buffer, BUFFER_SIZE, 0);
        strtok(buffer, "|");
        token = strtok(buffer, "\n");
        cout << "Turn " << token << endl;
        token = strtok(NULL, "\n");
        cout << "Word: " << token << endl;

        //clear buffer
        memset(buffer, 0, sizeof(buffer));

        //ask user for a guess and send guess to server
        string guess;
        cout << "Enter your guess: ";
        getline(cin, guess);
        guess += "|"; //add delimeter to mark end of guess
        while (send(sock, guess.c_str(), guess.length(), 0) != (unsigned int) guess.length());

        //receive guess response from the server
        while(!strchr(buffer, '|'))
            recv(sock, buffer, BUFFER_SIZE, 0);
        strtok(buffer, "|");
        token = strtok(buffer, "\n");
        if(*token == 'c')
            cout << "Correct!" << endl;
        else if(*token == 'x')
            cout << "Guess should be one character." << endl;
        else if(*token == 'd')
            cout << "You have entered that guess." << endl;
        else
            cout << "Incorrect!" << endl;

        //check if game has ended and send signal to start next turn
        token = strtok(NULL, "\n");
        send(sock, "|", 1, 0);
    } while(*token == '0'); //the game ends here when token == '1'

    //print the complete word and number of guesses taken to the screen
    while (!strchr(buffer, '|')) 
        recv(sock, buffer, BUFFER_SIZE, 0);
    strtok(buffer, "|");
    token = strtok(buffer, "\n");
    cout << endl << "Congratulations! You guessed the word " << token << "!!" << endl;
    token = strtok(NULL, "\n");
    cout << "It took " << token << " turns to guess the word correctly." << endl;

    //print leaderboard information 
    cout << endl << "Leader board: " << endl;
    token = strtok(NULL, "\n");
    int i = 1;
    while(token)
    {
        cout << "   " << i << "." << token << " ";
        token = strtok(NULL, "\n");
        cout << token;
        token = strtok(NULL, "\n");
        cout << endl;
        i++;
    }

    //close connection with server 
    close(sock);

    return 0;
}
