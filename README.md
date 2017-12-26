# Multi-Client-Web-Server-in-C
A simple web server with the implementation of GET and HEAD Requests.

/*  INSTRUCTIONS

**To start the server type following in terminal :-**

1. gcc web_server
2. ./a.out
           

Default it will open at port no. 10000
   
   :-> In case that port number is already in use change the port no by using -p as shown below :-

                    :./a.out -p 5000(or any port number >1024 & <65k)

**To open the client and connect** :-

                    1. telnet localhost port_number(the one where you started the server at) 
                    2. GET file_path HTTP/1.0

       --> File must be present in the root directory where the server started.
       --> You can check your root directory in the terminal where the server started.
       
       For example if there is a file named "hello.html" in the directory where the server started 
                    : GET /hello.html HTTP/1.0

        --> Similarly for HEAD method use HEAD instead of GET                                            
*/
