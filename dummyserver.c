#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "employee.h"
#include "admin.h"
#include "manager.h"
#include "customer.h"
#define PORT 8080
#define MAX_CLIENTS 10

#define PORT 8080
#define BUFFER_SIZE 1024
int login_server(int new_socket, int* user_id)
{
  char new_username[50],stored_username[50];
    char new_password[50],stored_password[50];
    char role[10];
    int found = 0;
    char buffer[1024] = {0};
    int opt;
    // Receive username and password from client
    recv(new_socket,new_username, sizeof(new_username), 0);
    recv(new_socket, new_password, sizeof(new_password), 0);

    // Check the credentials from the userdetails.txt file
    FILE *file = fopen("userdetails.txt", "r");
    if (file == NULL) {
        perror("Could not open userdetails file");
        return 0;
    }

    while (fscanf(file, "%s %s %s %d", role, stored_username, stored_password,user_id) != EOF) {
        if (strcmp(new_username,stored_username) == 0 && strcmp(new_password,stored_password) == 0) {
            found = 1;
            break;
        }
    }
    fclose(file);
    if (found) {
        // Create welcome message based on the role
        if (strcmp(role, "ad") == 0) {
            snprintf(buffer,sizeof(buffer), "Welcome Admin! Your User ID is %d.\n", *user_id);
            opt = 1;
        } else if (strcmp(role, "man") == 0) {
            snprintf(buffer, sizeof(buffer),"Welcome Manager! Your User ID is %d.\n", *user_id);
            opt = 2;
        } else if (strcmp(role, "emp") == 0) {
            snprintf(buffer, sizeof(buffer),"Welcome Employee! Your User ID is %d.\n", *user_id);
            opt = 3;
        } else if (strcmp(role, "ct") == 0) {
            snprintf(buffer, sizeof(buffer),"Welcome Customer! Your User ID is %d.\n", *user_id);
            opt = 4;
        } else {
            snprintf(buffer, sizeof(buffer),"Welcome User! Your User ID is %d.\n", *user_id);
            opt = 0;
        }
    } else {
        snprintf(buffer, sizeof(buffer),"Invalid username or password. Please try again.\n");
        opt = 0;
    }
    send(new_socket, buffer, strlen(buffer), 0);
    return opt;
}
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    int opt; // cause client-server not working
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    // Accept an incoming connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

   /* // Read data from the client
    int valread = read(new_socket, buffer, BUFFER_SIZE);
    printf("Message from client: %s\n", buffer);
    
    // Echo the message back to the client
    send(new_socket, buffer, valread, 0);
    printf("Echoed message back to client.\n");*/
    //login receipts
    int id = 0;
    int ch;
    while (recv(new_socket, &ch, sizeof(ch), 0) > 0) {
      switch(ch) {
        case 1 : create_account_server(new_socket);
                 break;
        case 2 : {
                  opt = login_server(new_socket,&id);
                  if (opt == 1) {
                    printf("Admin Server\n");
                    handle_admin_request(new_socket);
                    }
                  else if (opt == 2) {
                    printf("Manager Request\n");
                    handle_manager_request(new_socket);
                    }
                  else if (opt == 3) {
                    printf("Employee Request\n");
                    handle_employee_request(new_socket);
                    }
                    else if (opt == 4) {
                      printf("Customer Request\n");
                      handle_customer_request(new_socket,id);
                    }
                    else
                      printf("error\n!");
                    }
                 break;
        case 3 : printf("Exiting\n");
                 break;
        default : printf("Incorrect option entered");
        }
      }
    // Close sockets
    close(new_socket);
    close(server_fd);
    
    return 0;
}
