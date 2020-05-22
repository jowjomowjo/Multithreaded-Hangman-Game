#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <random>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <vector>
#include <cctype>
#include <utility>
#include <algorithm>
#include <mutex>
#include <sstream>
#include <iomanip>
using namespace std;

const string FILENAME = "/home/fac/lillethd/cpsc3500/projects/p4/words.txt";
const int NUM_WORDS = 57127;
const int BUFFER_SIZE = 1024;
const int MAX_LEADERBOARD = 3; //we're only keeping track of first 3

pthread_mutex_t mutex_leaderboard = PTHREAD_MUTEX_INITIALIZER;

struct scores
{
    string name;
    float score;
};
scores leaderboard[MAX_LEADERBOARD];

//returns a random word from textfile
string getRandomWord()
{
    srand(time(NULL));
    ifstream file(FILENAME.c_str());
    string word;
    int index = rand() % NUM_WORDS;
    for(int i = 0; i < index; i++)
    {
        file >> word;
    }
    //file.close();
    return word;
}

bool compareScores(const scores &a, const scores &b)
{
    return a.score < b.score;
}

void* processClient(void* sockfd)
{
    long turns = 0;
    char buffer[BUFFER_SIZE] = {0};
    long socketfd = *((long*)sockfd);
    string guess;
    int correct_guesses = 0;

    //detach the calling thread
    pthread_detach(pthread_self());

    //choose a random word for client to guess
    string random_word = getRandomWord();

    //print the word to server screen
    cout << "Word: " + random_word << endl;

    //receive player's name from client
    //keeps looping if no name is received
    string name;
    char* token;
    do
	{
        //loop ensures everything is received
		while (!strchr(buffer, '|')) 
            recv(socketfd, buffer, BUFFER_SIZE, 0);
		token = strtok(buffer,"|");

        //if no name received, send "i" (invalid)
		if (!token)
		{
			memset(buffer, 0, sizeof(buffer));
			send(socketfd, "i", 1, 0);
		}
	} while(!token);
	send(socketfd, "v", 1, 0); //send "v" (valid) if name received 
	name = token;
  	cout << "Client: " << name << endl; 

    //clear buffer
    memset(buffer, 0, sizeof(buffer));

    //initialize everything on word_output as false
    int word_length = random_word.length();
    bool* word_output  = new bool[word_length];
    for(int i = 0; i < word_length; i++)
        word_output[i] = false;

    //the game starts here
    //keeps looping until the word has been guessed correctly
    string info;
    while(correct_guesses != word_length)
    {
        turns++;

        //send number of turns and word_output to the client
        info = to_string(turns) + "\n";
        for(int i = 0; i < word_length; i++)
        {
            if(word_output[i])
                info += random_word[i];
            else
                info += "-";
        }
        info += "|"; //add delimeter to mark end of turns and word_output
        while (send(socketfd, info.c_str(), info.length(), 0) != (unsigned int) info.length());

        //receive guess from client
        while (!strchr(buffer, '|')) 
            recv(socketfd, buffer, BUFFER_SIZE, 0);
        token = strtok(buffer, "|");
        if(!token)
            guess = "";
        else
            guess = token;

        /**
         * check if input is valid
         * i = incorrect
         * c = correct
         * x = invalid 
         * d = double entry (same character twice)
         **/
        string check = "i"; 
        if (guess.length() != 1) //guess should only be one character, if not, invalid
            check = "x";
        else
        {
            for(int i = 0; i < word_length; i++)
            {
                if(toupper(guess[0]) == random_word[i])
                {
                    if(word_output[i] == false)
                    {
                        correct_guesses++;
                        word_output[i] = true;
                        check = "c";
                    }
                    else
                        check = "d";
                }
            }
        }
        info = check + "\n";

        //check if guess was the final guess for the word
        //mark 0 or 1 to signal server if game has ended
        if(correct_guesses == word_length)
            info += "1\n"; 
        else
            info += "0\n"; 
        info += "|"; //add delimeter to mark end of info

        //send all the information to the client
        while (send(socketfd, info.c_str(), info.length(), 0) != (unsigned int) info.length());

        //wait for  signal from client for next turn
        recv(socketfd, buffer, 1, 0);

        //clear buffer
        memset(buffer, 0, sizeof(buffer));
    }

    //end / congrats message info (word & number of guesses)
	info = random_word + "\n" + to_string(turns) + "\n";

    //update leaderboard if player made top three
    float score = (float) turns / (float) word_length;

	pthread_mutex_lock(&mutex_leaderboard);

	//check if there's empty slots and fill that up first 
    bool updated = false;
	if (!updated)
    {
		for (int i = 0; i < MAX_LEADERBOARD; i++)
        {
			if (leaderboard[i].score == 0)
			{
				leaderboard[i].name = name;
				leaderboard[i].score = score;
				updated = true;
				break;
			}
        }
    }

	//if there's no more empty slots, compare the new score with the third place score
    //replace if the new score is smaller 
	if (!updated)
    {
		if (score < leaderboard[MAX_LEADERBOARD - 1].score)
		{
			leaderboard[MAX_LEADERBOARD - 1].name = name;
			leaderboard[MAX_LEADERBOARD - 1].score = score;
			updated = true;
		}
    }

	//if leaderboard is updated, sort 
	if (updated)
		sort(begin(leaderboard), end(leaderboard), compareScores);

	//leaderboard information
	for (int i = 0; i < MAX_LEADERBOARD; i++)
		if (leaderboard[i].score != 0)
		{
            //format the float to have 2 decimal places
			stringstream formatted_score;
			formatted_score << fixed << setprecision(2) << leaderboard[i].score;
			info += leaderboard[i].name + "\n" + formatted_score.str() + "\n";
		}

	pthread_mutex_unlock(&mutex_leaderboard);
	info += "|"; // add delimeter to mark end of info

    //send all the information to the client
	while (send(socketfd, info.c_str(), info.length(), 0) != (unsigned int) info.length());
	
	delete [] word_output;
	
	return (void*) turns;
}

//argv[1] = port number
int main(int argc, char* argv[])
{
    int server_fd;
    long new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int port_num = atoi(argv[1]);
    int addrlen = sizeof(address);
    vector<pthread_t> threadArr;
    
    //error checking for command line argument 
    if (argc != 2)
    {
        printf("\nNumber of arguments is incorrect\n");
        return -1;
    }

    if(port_num == 0) //atoi() returns 0 when fail
    {
        printf("\nPort number is invalid\n");
        return -1;
    }

    //create server socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        printf("\nCreate socket failed\n");
        return -1;
    }

    //attach socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        printf("\nSetsockopt failed\n");
        return -1;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port_num);

    //bind socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        printf("\nBind failed\n");
        return -1;
    }

    //listen to a maximum of 20 backlogged connections
    if (listen(server_fd, 20) < 0)
    {
        printf("\nListen failed\n");
        close(server_fd);
        return -1;
    }

    //initialize the leaderboard to all empty 
    for(int i = 0; i < MAX_LEADERBOARD; i++)
    {
        leaderboard[i].name = "";
        leaderboard[i].score = 0;
    }

    //accept connections and create new threads to process that client
    //the server will run forever until aborted or error creating thread
    int i = 0;
    while(true)
    {
        if((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
        {
            printf("\nAccept failed\n");
        }
        else
        {
            pthread_t new_thread;
            threadArr.push_back(new_thread);
            if(pthread_create(&threadArr[i], NULL, processClient, (void*)&new_socket) != 0)
            {
                printf("\nError creating thread\n");
                close(server_fd);
                return -1;
            }
        }
        i++;
    }
    
    return 0;
}
