# Sockets and Signals- Programming assignment

Modify the program described in Assignment 3 in such a way that the two programs can be run on different machines. The program receiver.c will now be the client program that will interact with the user. Call it client.c. The processor.c will be the backend program – call it server.c. The client and server programs have the same functionality as described in Assignment 3 except that they should use sockets as a communication mechanism that will allow the client and the server to be run on different machines. 

When the user presses Ctrl-C on the server.c terminal, the server program should display a summary, which should show the number of lines received so far and the total digit count across all input seen by the server.

Please submit a zip file labeled as Assignment4_Yourname.zip. It should include a PDF file with explanation of what your code does (this is optional if your code is self explanatory with comments) and two C source files – client.c and server.c.

