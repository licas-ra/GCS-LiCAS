/*
 * Ground Control Station Program for AlexAndros Robot Arms
 *
 * Author: Alejandro Suarez Fernandez-Miranda
 * Date: 27 August 2020
 **/


// Standard library
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>


// Structure declaration
typedef struct
{
	int * socketGCS2Odroid;
	struct sockaddr_in * addrOdroid;
	bool * endFlag;
} THREAD_ARGS;


typedef struct
{
	char header[3];		// "GCS" character sequence
	int code;			// 0: keep alive message
} __attribute__((packed)) GCS_PACKET;


// Function declaration
static void * keepAliveThreadFunction(void * args);
int sendControlmessage(int code, int socketGCS2Odroid, struct sockaddr_in * addrOdroid);


// Namespaces
using namespace std;


int main(int argc, char ** argv)
{
	THREAD_ARGS threadArgs;
	struct sockaddr_in addrOdroid;
    struct hostent * host;
	pthread_t keepAliveThread;
	int socketGCS2Odroid = -1;
	int error = 0;
	int code = 0;
	bool endFlag = false;
	
	
	cout << endl;
	cout << "----------------------------------------------------" << endl;
	cout << "Ground Control Station (GCS) Program for LiCAS" << endl << endl;
	cout << "Author: Alejandro Suarez Fernandez-Miranda" << endl;
	cout << "Date: Date: 27 August 2020" << endl;
	cout << "----------------------------------------------------" << endl << endl;
	
	
	// Check if the numer of arguments is correct
	if(argc != 3)
	{
		cout << "ERROR: invalid number of arguments. Specify host IP address and UDP port." << endl;
		error = 1;
	}
	else
	{
		// Open the socket in datagram mode
		socketGCS2Odroid = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    	if(socketGCS2Odroid < 0)
    	{
    		cout << endl << "ERROR: [in main] could not open socket." << endl;
    		error = 1;
   		}

		// Solve tha name of the remote Odroid
    	host = gethostbyname(argv[1]);
    	if(host == NULL)
	    {
	        cout << "ERROR: [in main] could not get host by name." << endl;
	        error = 1;
	    }

		// Set the address of the host
		if(error == 0)
		{
	    	bzero((char*)&addrOdroid, sizeof(struct sockaddr_in));
	    	addrOdroid.sin_family = AF_INET;
	    	bcopy((char*)host->h_addr, (char*)&addrOdroid.sin_addr.s_addr, host->h_length);
	    	addrOdroid.sin_port = htons(atoi(argv[2]));
	    }
	    else
	    	close(socketGCS2Odroid);
	    
	    
	    if(error == 0)
	    {
	    	// Create thread for sending a periodic keep alive message
	    	threadArgs.socketGCS2Odroid = &socketGCS2Odroid;
	    	threadArgs.addrOdroid = &addrOdroid;
	    	threadArgs.endFlag = &endFlag;
			if(pthread_create(&keepAliveThread, NULL, &keepAliveThreadFunction, (void*)&threadArgs))
			{
				cout << "ERROR: [in main] could not create keep alive thread." << endl;
				error = 1;
			}
			else
			{
				// Wait 100 ms
				usleep(100000);
				
				// Command read loop
				do
				{
					cout << "Code: ";
					scanf("%d", &code);
					if(code < 0)
					{
						endFlag = true;
						cout << "Sending termination code..." << endl;
						sendControlmessage((int)-1, socketGCS2Odroid, &addrOdroid);
						usleep(100000);
						sendControlmessage((int)-1, socketGCS2Odroid, &addrOdroid);
						usleep(100000);
						sendControlmessage((int)-1, socketGCS2Odroid, &addrOdroid);
						usleep(100000);
					}
					else
					{
						sendControlmessage(code, socketGCS2Odroid, &addrOdroid);
					}
					
					fflush(stdin);
				} while (endFlag == false);
				
			}
	    }
	    
	    // Close socket
	    close(socketGCS2Odroid);
	    socketGCS2Odroid = -1;
	}
	


	return 0;
}



/*
 * Thread for sensing periodic keep alive messages
 **/
static void * keepAliveThreadFunction(void * args)
{
	THREAD_ARGS * threadArgs = (THREAD_ARGS*)args;
	GCS_PACKET controlPacket;
	struct timeval t0;
	struct timeval t1;
	double t = 0;
	const double keepAlivePeriod = 0.5;
	int error = 0;
	
	
	cout << "Keep Alive thread function started" << endl;
	
	// Init timer
	gettimeofday(&t0, NULL);
	
	// Keep Alive message loop
	while(*(threadArgs->endFlag) == false && error == 0)
	{
		// Compute the elapsed time since last keep alive message
		gettimeofday(&t1, NULL);
		t = (t1.tv_sec - t0.tv_sec) + 1e-6*(t1.tv_usec - t0.tv_usec);
		
		// Send the data packet to the corresponding IP address
		if(t >= keepAlivePeriod)
		{
			// Build the packet
			strncpy(controlPacket.header, "GCS", 3);
			controlPacket.code = 0;
			
			// Send the packet
			if(sendto(*(threadArgs->socketGCS2Odroid), (char*)&controlPacket, sizeof(GCS_PACKET), 0, (struct sockaddr*)(threadArgs->addrOdroid), sizeof(struct sockaddr)) < 0)
			{
				error = 1;
				cout << "ERROR: [in keepAliveThreadFunction] could not send data." << endl;
			}
			
			// Reset timer
			gettimeofday(&t0, NULL);
		}
		else
		{
			// Wait 10 ms
			usleep(10000);
		}
	}
	cout << "Keep Alive thread function terminated" << endl;
	
	
	return 0;
}


int sendControlmessage(int code, int socketGCS2Odroid, struct sockaddr_in * addrOdroid)
{
	GCS_PACKET controlPacket;
	int error = 0;
	
	
	// Build the packet
	strncpy(controlPacket.header, "GCS", 3);
	controlPacket.code = code;
			
	// Send the packet
	if(sendto(socketGCS2Odroid, (char*)&controlPacket, sizeof(GCS_PACKET), 0, (struct sockaddr*)addrOdroid, sizeof(struct sockaddr)) < 0)
	{
		cout << "ERROR: [in sendControlmessage] could not send data" << endl;
		error = 1;
	}
	
	return error;
}


