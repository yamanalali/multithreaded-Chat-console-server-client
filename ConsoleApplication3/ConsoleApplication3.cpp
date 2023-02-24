#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <fstream>
#include <thread>
#include <regex>



using namespace std;
#define MAX_CLIENTS 10
#define DEFAULT_BUFLEN 4096
#pragma comment(lib, "ws2_32.lib") // Winsock library
#pragma warning(disable:4996)
SOCKET server_socket;
vector<string> history;

 

void RemoveWFL(std::string& line, const std::string& word)
{
	auto n = line.find(word);
	if (n != std::string::npos)
	{
		line.erase(n, word.length());
	}
}
 
void treeList(int clSok) {
	std::ifstream input("TcpGroup.txt");
	for (std::string line; std::getline(input, line);)
	{
		cout << line << endl;
		send(clSok, line.c_str(), line.size() + 1, 0);
	}
}

void WriteLog(string line) {
	// get the current date and time
	time_t now = time(0);
	char* date_time = ctime(&now);

	ofstream foutput;
	ifstream finput;
	finput.open("Logs.txt");
	foutput.open("Logs.txt", ios::app);
	if (finput.is_open())
		foutput << line << " ---- " << date_time;
	finput.close();
	foutput.close();
}

bool compute_parity_bit(const std::string& message) {
	int num_ones = 0;
	for (auto c : message) {
		if (c == '1') {
			num_ones++;
		}
	}
	return num_ones % 2 == 1;
}


// ------------------------------------------------Start Main -------------------------------------------------------------
int main() {
	
	string clientData[MAX_CLIENTS][3];

	system("title Server");
	puts("Start server...");
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code: %d", WSAGetLastError());
		return 1;
	}

	// create a socket
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket: %d", WSAGetLastError());
		return 2;
	}

	puts("socket Created... |");

	// prepare the sockaddr_in structure
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	// bind socket
	if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code: %d", WSAGetLastError());
		return 3;
	}

	 
	// listen to incoming connections
	listen(server_socket, MAX_CLIENTS);

	// accept and incoming connection
	puts("Server is waiting for incoming connections...\nPlease, start a client app.");

	// size of our receive buffer, this is string length
	// set of socket descriptors
	// fd means "file descriptors"
	fd_set readfds; //
	SOCKET client_socket[MAX_CLIENTS] = {};

	while (true) {
		FD_ZERO(&readfds);
		FD_SET(server_socket, &readfds);
		for (int i = 0; i < MAX_CLIENTS; i++) {
			SOCKET s = client_socket[i];
			if (s > 0) {
				FD_SET(s, &readfds);
			}
		}

		// wait for an activity on any of the sockets, timeout is NULL, so wait indefinitely
		if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
			printf("select function call failed with error code : %d", WSAGetLastError());
			return 4;
		}

		// if something happened on the master socket, then its an incoming connection new client socket
		SOCKET new_socket;
		sockaddr_in address;
		int addrlen = sizeof(sockaddr_in);
		if (FD_ISSET(server_socket, &readfds)) {
			if ((new_socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0) {
				perror("accept function error");
				return 5;
			}

			// inform server side of socket number - used in send and recv commands
			printf("New connection, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
			cerr << "Wating User info ....." << endl;
		 
			// add new socket to array of sockets
			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					std::cout << "Adding to list of sockets at index :  "<< i << " socket number " << new_socket << std::endl;
					break;
				}
			}

		
		}		      
		 
		for (int i = 0; i < MAX_CLIENTS; i++){
			SOCKET s = client_socket[i];
			// if client presend in read sockets
			if (FD_ISSET(s, &readfds))
			{
				// get details of the client
				getpeername(s, (sockaddr*)&address, (int*)&addrlen);
				char client_message[DEFAULT_BUFLEN];
				int client_message_length = recv(s, client_message, DEFAULT_BUFLEN, 0);
				client_message[client_message_length] = '\0';
				clientData[i][0] = client_socket[i];
				clientData[i][2] = "OFFline";
				string input = client_message;
				if (input =="GROUP") {
					string welcMsg = "................. Welcome to the chat server ................. \r\nPlease Enter ( CON- ) User name & Last name : ";
					send(client_socket[i], welcMsg.c_str(), welcMsg.size() + 1, 0);
				}



				// ------------------------------------------------ CONNECT USER  -------------------------------------------------------------
				string ch = "CON-";
				if (input.find(ch) != string ::npos)  {
					int OnlineCNum = 0;
					// get user name and last name & save it in to 2D Array 
					std::string line(input);
					RemoveWFL(line, "CON-");
					clientData[i][1] = line.c_str();  
					for (int z = 0; z < MAX_CLIENTS; z++) {
							if (clientData[z][0] != "\0") {
								string line2;
								string searchString = clientData[z][1];
								int deleteNum=0;
								int testnum= -1;
								// Open the file
								ifstream file("TcpGroup.txt");
								vector<string> lines;
								if (file.is_open()) {
									while (getline(file, line2)) {
										lines.push_back(line2);
										if (line2.find(searchString) != string::npos) {
											 testnum = deleteNum;
									}
										deleteNum++;
									}
									file.close();
								}

								ofstream write_file;
								write_file.open("TcpGroup.txt");
								for (int e = 0; e < lines.size(); e++) {
									if (e != testnum) {
										write_file << lines[e] << endl;
									}
									else {
										clientData[z][2] = "Online";
										write_file  << "User Name :" << clientData[z][1] << " | Statuse : " << clientData[z][2]<<"\n";
										 string HMSG = "---------------------- Welcome In TCPG group ---------------------- ";
										 /*send(client_socket[i], HMSG.c_str(), HMSG.size() + 1, 0);*/
									}
								}

								if (testnum != -1 && OnlineCNum == 0) {
									OnlineCNum++;
									treeList(client_socket[i]);
								}
							 
								write_file.close();

								if (testnum == -1 && OnlineCNum == 0){
									OnlineCNum++;
									// insert new user to the group 
									ofstream foutput;
									ifstream finput;
									finput.open("TcpGroup.txt");
									foutput.open("TcpGroup.txt", ios::app);
									clientData[z][2] = "Online";
									if (finput.is_open())
										foutput << "User Name : " << clientData[z][1] << " | Status : " << clientData[z][2];
									finput.close();
									foutput.close();
									string JMSG = "---------------------- You Joined the TCPG group ---------------------- ";
								    send(client_socket[i], JMSG.c_str(), JMSG.size() + 1, 0);
									// send list to user 
									treeList(client_socket[i]);
								 
								}
							 
							}
						cout << endl;
					}
					 
				}

				string msg = "LIST";
				if (input.find(msg) != string::npos) {
					treeList(client_socket[i]);
				}

                // ------------------------------------------------SEND MESSAGE -------------------------------------------------------------
				string sClient;
				char cha = '>';
				std::size_t pos = input.find(cha);
				string messageCon;
				if (pos != std::string::npos) {
					// get user name    
					messageCon = input.substr(pos + 1);
					sClient = input.substr(0, pos - 1);

					cout << "Form : " << clientData[i][1] << " To : " << sClient  << " -> " <<messageCon << endl;
					string msg = "Form : " + clientData[i][1] + " To : " + sClient + " -> " + messageCon;
					WriteLog(msg);
					// the simple parity check algorthem

					bool parity_bit = compute_parity_bit(messageCon);
					std::cout << "Parity bit: " << parity_bit << std::endl;


					// check user is online 
					int checkNum = 0;
					for (int w = 0; w < MAX_CLIENTS; w++) {
						if (client_socket[w] != 0) {
							if (clientData[i][1] == sClient) {
								checkNum++;
								string msg = "You can't send to yourself";
								send(client_socket[w], msg.c_str(), msg.size() + 1, 0);
							}
							else if (clientData[w][1] == sClient) {
								checkNum++;
								// statr send message for the sClient 
								string newMess = "From " + clientData[i][1] + "  : " + messageCon;
								send(client_socket[w], newMess.c_str(), newMess.size() + 1, 0);
							}
						}
					}
					if (checkNum == 0) {
						string MSG3 = "You can not send a message for this user :( Because he is offline ";
						send(client_socket[i], MSG3.c_str(), MSG3.size() + 1, 0);
					}

				}
				
				// ------------------------------------------------LEAVE THE GRUP -------------------------------------------------------------


				string check_exit = client_message;
				if (check_exit == "off")
				{
					cout << "Client #" << client_socket[i] << " is off\n";
					client_socket[i] = 0;
					if (client_socket[i] == 0) {
						string line2;
						string searchString = clientData[i][1];
						int deleteNum = 0;
						int testnum = -1;
						// Open the file
						ifstream file("TcpGroup.txt");
						vector<string> lines;
						if (file.is_open()) {
							while (getline(file, line2)) {
								lines.push_back(line2);
								if (line2.find(searchString) != string::npos) {
									testnum = deleteNum;
								}
								deleteNum++;
							}
							file.close();
						}

						ofstream write_file;
						write_file.open("TcpGroup.txt");
						for (int e = 0; e < lines.size(); e++) {
							if (e != testnum) {
								write_file << lines[e] << endl;
							}
							else {
								clientData[i][2] = "Offline";
								write_file << "User Name :" << clientData[i][1] << " | Statuse : " << clientData[i][2] << "\n";
							}
						}



						write_file.close();
					}
				}

				

				string temp = client_message;
				

			}
			}
		 
	}

	WSACleanup();
}