/*
FILE NAME: FILESERVER.C
CREATED BY:163054001 AMIT PATHANIA
This files creates a central server. File takes one commandline input arguments ie own listning port.
Clients will use Server IP and port to connect this server for publish and search files.
The code has following main operations:
JOIN : Whenever new client want to join network, server will be connected
PUBLISH : Clients can publish file to server for sharing with filename,filepath and 
their port no.Details of the file will be stored in publish list file. (filelist.txt)
SEARCH : Clients can search file using keyword . It uses bash script (searchscript.sh) for searching file in publish list.
once file search is complete ,the client is disconnected to allow other peers to connect and reduce load on server.
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include<netdb.h>
#include<errno.h>
#include<arpa/inet.h>
#include <signal.h>
#include <time.h>

#define ERROR     -1  // defines error message
#define MAX_CLIENTS    4 //defines maximum number of clients that can connect simultaneously
#define MAX_BUFFER    512 //used to set max size of buffer for send recieve data 

int add_IP(char*); // function to add IP to the list
int update_IPlist(char *);//function to update IP_list once client disconnected
time_t current_time; // variable to store current time


main(int argc, char **argv)  
{
	int sock; // sock is socket desriptor for server 
	int new; // socket descriptor for new client
	struct sockaddr_in server; //server structure 
	struct sockaddr_in client; //structure for server to bind to particular machine
	int sockaddr_len=sizeof (struct sockaddr_in);	//stores length of socket address
	


	//variables for publish operation
	char buffer[MAX_BUFFER]; // Receiver buffer; 
	char file_name[MAX_BUFFER];//Buffer to store filename,path and port recieved from peer
	char *peer_ip;//variable to store IP address of peer

	//varriable for search operation
	char user_key[MAX_BUFFER];//file keyword for search by user
	int len;// variable to measure MAX_BUFFER of incoming stream from user

	int pid;// to manage child process

	if (argc < 2)    // check whether port number provided or not
	{ 
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}


	/*get socket descriptor */
	if ((sock= socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
	{ 
		perror("server socket error: ");  // error checking the socket
		exit(-1);  
	} 
	 
	/*server structure */ 
	server.sin_family = AF_INET; // protocol family
	server.sin_port =htons(atoi(argv[1])); // Port No and htons to convert from host to network byte order. atoi to convert asci to integer
	server.sin_addr.s_addr = INADDR_ANY;//INADDR_ANY means server will bind to all netwrok interfaces on machine for given port no
	bzero(&server.sin_zero, 8); //padding zeros
	
	/*binding the socket */
	if((bind(sock, (struct sockaddr *)&server, sockaddr_len)) == ERROR) //pointer casted to sockaddr*
	{
		perror("bind");
		exit(-1);
	}

	/*listen the incoming connections */
	if((listen(sock, MAX_CLIENTS)) == ERROR) // listen for max connections
	{
		perror("listen");
		exit(-1);
	}

	while(1)
	{
		if ((new = accept(sock, (struct sockaddr *)&client, &sockaddr_len)) == ERROR) // accept takes pointer to variable containing len of struct
		{
			perror("ACCEPT.Error accepting new connection");
			exit(-1);
		}

		pid=fork(); //creates separate process for each client at server

		if (!pid) // For multiple connections.this is the child process
		{ 
            	close(sock); // close the socket for other connections
            		
		printf("New client connected from port no %d and IP %s\n", ntohs(client.sin_port), 
inet_ntoa(client.sin_addr));
	 	peer_ip = inet_ntoa(client.sin_addr);
	 	add_IP(peer_ip); //wadding Client IP into IP List
		
		while(1)
		{
		len=recv(new, buffer , MAX_BUFFER, 0);
		buffer[len] = '\0';
		printf("%s\n",buffer);

		//conenctionerror checking
			if(len<=0)//connection closed by client or error
				{
				if(len==0)//connection closed
				{
					printf("Peer %s hung up\n",inet_ntoa(client.sin_addr));
					update_IPlist(peer_ip);
				}
				else //error
				{
					perror("ERROR IN RECIEVE");
				}
			close(new);//closing this connection
			exit (0);
			}//clos if loop

			//PUBLISH OPERATION
		if(buffer[0]=='p' && buffer[1]=='u' && buffer[2]=='b') // check if user wants to publish a file
		{
	
		/*Recieve publish files from peer */
		//adding publised file details to publish list
		char* fileinfo = "filelist.txt";
		FILE *filedet = fopen(fileinfo, "a+");

		if(filedet==NULL) // if unable to open file
        {
            printf("Unable to open File.Error");
            return 1;  
        }   
        
        else
		{
		fwrite("\n", sizeof(char), 1, filedet);			
			
		len=recv(new, file_name, MAX_BUFFER, 0); //recv file details from client, data is pointer where data recd will be stored
		fwrite(&file_name, len,1, filedet);
		char Report[] = "File published"; 
		send(new,Report,sizeof(Report),0);		

		fwrite("\t",sizeof(char),1, filedet);
		
		peer_ip = inet_ntoa(client.sin_addr); // return the IP
		
		//printf("%s\n", peer_ip);
		fwrite(peer_ip,1, strlen(peer_ip), filedet);		//adding peer IP address to given file	
		fclose(filedet);
		printf("%s\n","FILE PUBLISHED");

		}


		}// close if pub check loop

		//SEARCH OPERATION
		else if(buffer[0]=='s' && buffer[1]=='e' && buffer[2]=='a') //check keyword for search sent by client
		{
		
		bzero(buffer,MAX_BUFFER); // clearing the buffer by padding

		len=recv(new, user_key, MAX_BUFFER, 0); //recv keyword from client to search
		char Report3[] = "Keyword recieved"; 
		send(new,Report3,sizeof(Report3),0);
		user_key[len] = '\0';
		printf("%s\n",user_key);

		char command[50];// variable to store systm command

		/* Call the Script */
		system("chmod 755 ./searchscript.sh"); //command executed to give execute permissions to script
		strcpy(command, "./searchscript.sh ");
		strcat(command,user_key );          // appends user key as argument to bash command
   		system(command);

   		//system("cd ; chmod +x script.sh ; ./searchscript.sh");

   		 char* search_result = "searchresult.txt";
		 char buffer[MAX_BUFFER]; 
		 
		 FILE *file_search = fopen(search_result, "r"); // open and read file conataining result of searchscript.sh
		 if(file_search == NULL)
		    {
		        fprintf(stderr, "ERROR while opening file on server.");
				exit(1);
		    }

		    bzero(buffer, MAX_BUFFER); 
		    int file_search_send; 
		    while((file_search_send = fread(buffer, sizeof(char), MAX_BUFFER, file_search))>0) //send search result to peer
		    {
		        len=send(new, buffer, file_search_send, 0);

		        if(len < 0)
		        {
		            fprintf(stderr, "ERROR: File not found");
		            exit(1);
		        }
		        bzero(buffer, MAX_BUFFER);
		    }
		    fclose(file_search);
		    char Reportsearch[] = "Search complete.You are disconnected from server now.Connect again for further actions"; 
        	send(new,Reportsearch,sizeof(Reportsearch),0);
        	
		    printf("Search complete!!!!\n");
		    printf("Client disconnected from port no %d and IP %s\n", ntohs(client.sin_port), 
inet_ntoa(client.sin_addr));
		    peer_ip = inet_ntoa(client.sin_addr); // return the IP
			update_IPlist(peer_ip);
		    //kill(pid,SIGKILL);
			close(new); // disconnect this client so that other users can connect server
			exit(0);
		}// close search condition

		//TERMINATE OPERATION:when user want to disconnect from server
		else if(buffer[0]=='t' && buffer[1]=='e' && buffer[2]=='r')
		{
		printf("Client disconnected from port no %d and IP %s\n", ntohs(client.sin_port), 
inet_ntoa(client.sin_addr));
		peer_ip = inet_ntoa(client.sin_addr); // return the IP
		update_IPlist(peer_ip);
		//kill(pid,SIGKILL);
		close(new);// close the connection
		exit(0);
		} //close terminate loop
	
	}// close while loop inside fork.server will keep listening client till disconnected
	} // close if loop for fork function
	close(new);
	}// close main while loop
	printf("Server shutting down \n");
	close(sock);
}

int update_IPlist(char *peer_ip)
{
//updating Client IP in IP List
		char* peerinfo = "peerIPlist.txt";
		FILE *peerdet = fopen(peerinfo, "a+");

		if(peerdet==NULL) // if unable to open file
        {
            printf("Unable to open IPList File.Error");
            return -1;  
        }   
        
        else
		{
		fwrite("\n", sizeof(char), 1, peerdet);			
		// write details of IP of client
		fwrite(peer_ip,1, strlen(peer_ip), peerdet);		//adding peer IP address to given file	
		char update[] = "  disconnected from server at ";
		fwrite(update,1, strlen(update), peerdet);
		current_time = time(NULL);
		fwrite(ctime(&current_time),1, strlen(ctime(&current_time)), peerdet);
		fclose(peerdet);
	}
}

int add_IP(char *peer_ip)
{
//adding Client IP in IP List
		char* peerinfo = "peerIPlist.txt";
		FILE *peerdet = fopen(peerinfo, "a+");

		if(peerdet==NULL) // if unable to open file
        {
            printf("Unable to open IPList File.Error");
            return -1;  
        }   
        
        else
		{
		fwrite("\n", sizeof(char), 1, peerdet);			
		// write details of IP of client
		fwrite(peer_ip,1, strlen(peer_ip), peerdet);		//adding peer IP address to given file	
		char update[] = "  connected to server at ";
		fwrite(update,1, strlen(update), peerdet);
		current_time = time(NULL);
		fwrite(ctime(&current_time),1, strlen(ctime(&current_time)), peerdet);
		fclose(peerdet);
	}
}
