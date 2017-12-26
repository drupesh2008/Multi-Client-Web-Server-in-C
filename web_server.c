#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>
#include<time.h> //To get Date/Time of Request/Response


#define CONNMAX 1000
#define BYTES 1024

char *ROOT,*reqline[3];
int listenfd, clients[CONNMAX];
int content_length = 0;
void error(char *);
void startServer(char *);
void respond(int);

void getDate(int n);
void getContentLength(int n);
void getContentType(int n);
void lastModifiedDate(int n);
void getServerInfo(int n);

int main(int argc, char* argv[])
{
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char c;    
	
	//Default Values PATH = ~/ and PORT=10000
	char PORT[6];
	ROOT = getenv("PWD");
	strcpy(PORT,"10000");

	int slot=0;

	//Parsing the command line arguments
	while ((c = getopt (argc, argv, "p:r:")) != -1)
		switch (c)
		{
			case 'r':
				ROOT = malloc(strlen(optarg));
				strcpy(ROOT,optarg);
				break;
			case 'p':
				strcpy(PORT,optarg);
				break;
			case '?':
				fprintf(stderr,"Wrong arguments given!!!\n");
				exit(1);
			default:
				exit(1);
		}
	
	printf("Server started at port no. %s%s%s with root directory as %s%s%s\n","\033[92m",PORT,"\033[0m","\033[92m",ROOT,"\033[0m");
	// Setting all elements to -1: signifies there is no client connected
	int i;
	for (i=0; i<CONNMAX; i++)
		clients[i]=-1;
	startServer(PORT);

	// ACCEPT connections
	while (1)
	{
		addrlen = sizeof(clientaddr);
		clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

		if (clients[slot]<0)
			error ("accept() error");
		else
		{
			if ( fork()==0 )
			{
				respond(slot);
				exit(0);
			}
		}

		while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;
	}

	return 0;
}

//start server
void startServer(char *port)
{
	struct addrinfo hints, *res, *p;

	// getaddrinfo for host
	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo( NULL, port, &hints, &res) != 0)
	{
		perror ("getaddrinfo() error");
		exit(1);
	}
	// socket and bind
	for (p = res; p!=NULL; p=p->ai_next)
	{
		listenfd = socket (p->ai_family, p->ai_socktype, 0);
		if (listenfd == -1) continue;
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
	}
	if (p==NULL)
	{
		perror ("socket() or bind()");
		exit(1);
	}

	freeaddrinfo(res);

	// listen for incoming connections
	if ( listen (listenfd, 1000000) != 0 )
	{
		perror("listen() error");
		exit(1);
	}
}



//client connection
void respond(int n)
{
	char mesg[99999], data_to_send[BYTES], path[99999];
	int rcvd, fd, bytes_read;

	memset( (void*)mesg, (int)'\0', 99999 );

	rcvd=recv(clients[n], mesg, 99999, 0);

	if (rcvd<0)    // receive error
		fprintf(stderr,("recv() error\n"));
	else if (rcvd==0)    // receive socket closed
		fprintf(stderr,"\nClient disconnected upexpectedly.\n");
	else    // message received
	{
		printf("%s", mesg);
		reqline[0] = strtok (mesg, " \t\n");

		//Check if the method is GET or HEAD and return the responses
		if ( strncmp(reqline[0], "GET\0", 4)==0 || strncmp(reqline[0], "HEAD\0", 4)==0 ) 
		{
			reqline[1] = strtok (NULL, " \t");
			reqline[2] = strtok (NULL, " \t\n");
			if ( (strncmp( reqline[2], "HTTP/1.0", 8)!=0 ) && (strncmp( reqline[2], "HTTP/1.1",8)!=0) )
			{    
				
                  write(clients[n], "\nHTTP/1.0 400 Bad Request\n\n", 30); //The request cannot be fulfilled due to bad syntax.
			}
			else
			{
				if ( strncmp(reqline[1], "/\0", 2)==0 )
					reqline[1] = "/index.html";        //Because if no file is specified, index.html will be opened by default (like it happens in APACHE...

				strcpy(path, ROOT);
				strcpy(&path[strlen(ROOT)], reqline[1]);
				printf("file: %s\n", path);

				if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
				{
					//GET Header Field
					printf("File Found\n");
	
                    
                    //Response Fields
					write(clients[n], "\nHTTP/1.0 200 OK\n",17); //Response msg
					getDate(n);  //Date Field
					getServerInfo(n);
					getContentLength(n);
					getContentType(n);
                    lastModifiedDate(n);
					//ADD SERVER information

   
					//Body of the file only to print for GET METHOD
					if(strncmp(reqline[0], "GET\0", 4)==0)
					{
					    while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
						    write (clients[n], data_to_send, bytes_read);
					}	
				}
				else    write(clients[n], "\nHTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
			}
		}
		else
		{
			 char error_msg[50];
			 strcpy(error_msg,reqline[3]);
			 strcat(error_msg," \n405 Method not allowed.");
			 write(clients[n], error_msg, strlen(error_msg));
		}
	}

	//Closing SOCKET
	shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
	close(clients[n]);
	clients[n]=-1;
}
 
//Functions for the Responses

//To get Date/time
void getDate(int n)
 {
  char buf[1000],a[1000];
  int len;
  time_t now = time(0);
  struct tm time_now = *gmtime(&now);
  strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &time_now);
  strcpy(a,"Date: ");
  strcat(a,buf);
  strcat(a,"\n");
  len = strlen(a);
  write(clients[n],a,len);
  
}

//To get Server information
void getServerInfo(int n) 
{
	char command[50];
	strcpy(command,"apache2 -v");
					
    FILE *ls = popen(command, "r");
    char buf[256];
   
    fgets(buf, 40, ls);
           //write(clients[n],"\n",3);
           write(clients[n],buf,strlen(buf));
        
    pclose(ls);
}

//To get Content-Length
void getContentLength(int n) 
{
	char command[50],filename[30];
	strcpy(command,"wc -c < ");
	strcpy(filename,reqline[1]);
	//Removing the backslash from reqline[1]
		memmove(filename, filename+1, strlen(filename));

	strcat(command,filename);
					
    FILE *ls = popen(command, "r");
    char buf[256];
    write(clients[n],"Accept-Ranges: Bytes",20);
    write(clients[n],"\nContent-Length: ",17);
    while (fgets(buf, sizeof(buf), ls) != 0) {
           write(clients[n],buf,strlen(buf));
        }
    pclose(ls);
}


//To get Content-Type
void getContentType(int n) //Content Type :
{
	char command[50],filename[30];
	strcpy(command,"file --mime-type -b ");
	strcpy(filename,reqline[1]);
	//Removing the backslash from reqline[1]
		memmove(filename, filename+1, strlen(filename));

	strcat(command,filename);
					
    FILE *ls = popen(command, "r");
    char buf[256];
    write(clients[n],"Content-Type: ",17);
    while (fgets(buf, sizeof(buf), ls) != 0) {
           write(clients[n],buf,strlen(buf));
        }
    pclose(ls);
}	

//To get Last Modified Date
void lastModifiedDate(int n)
{   
    char filename[500],buf[500],a[500];

	 strcpy(filename,reqline[1]);
	//Removing the backslash from reqline[1]
		memmove(filename, filename+1, strlen(filename));

	struct stat attrib;
    stat(filename, &attrib);
    char date[10];
    //strftime(date, 20, "%d-%m-%y", localtime(&(attrib.st_ctime)));
    strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&(attrib.st_ctime)));
    strcpy(a,"Last-Modified: ");
    strcat(a,buf);
    write(clients[n],a,strlen(a));
}
