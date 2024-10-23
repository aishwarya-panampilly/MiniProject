#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "employee.h"
#include "admin.h"
#include "manager.h"
#include "customer.h"
#define PORT 8080

#define BUFFER_SIZE 1024
// Function to extract user ID and role from a buffer containing a message
void extract_user_info(const char *buffer, int *user_id, char *role) {
    const char *ptr = buffer;
    *user_id = -1; // Default value if user ID is not found
    role[0] = '\0'; // Default value if role is not found

    // Check for role keywords and user ID in the buffer
    if (strstr(buffer, "Admin") != NULL) {
        strcpy(role, "Admin");
    } else if (strstr(buffer, "Customer") != NULL) {
        strcpy(role, "Customer");
    } else if (strstr(buffer, "Employee") != NULL) {
        strcpy(role, "Employee");
    } else if (strstr(buffer, "Manager") != NULL) {
        strcpy(role, "Manager");
    }

    // Scan through the buffer to find an integer
    while (*ptr) {
        if (isdigit(*ptr)) {
            *user_id = atoi(ptr); // Convert the number found to an integer
            break; // Exit once we find the first number
        }
        ptr++;
    }
}


void login_client(int sock)
{
  char username[50];
    char password[50];
    char buffer[1024] = {0};
    printf("\n--- Login ---\n");
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);
    int user_id;
    char role[10];
    // Send login details to server
    send(sock, username, sizeof(username), 0);
    send(sock, password, sizeof(password), 0);

    // Receive welcome message from server
    recv(sock, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);
    
    // Extract user ID and role from the buffer
    extract_user_info(buffer, &user_id, role);
    
    if (user_id != -1) {
        printf("Extracted User ID: %d\n", user_id);
    } else {
        printf("User ID not found.\n");
    }

    if (strlen(role) > 0) {
        printf("Extracted Role: %s\n", role);
    } else {
        printf("Role not found.\n");
    }
    if (strcmp(role,"Admin") == 0){
      admin_menu(sock);}
    else if (strcmp(role,"Manager") == 0) {
      manager_menu(sock);
    }
    else if (strcmp(role,"Employee") == 0) {
      employee_menu(sock);
    }
    else if (strcmp(role,"Customer") == 0) {
      customer_menu(sock,user_id);
    }
    else
      printf ("Not good!");
    return ;
    
    
}
int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    // Define server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported\n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return -1;
    }

   /* // Get message from user
    printf("Enter message to send to server: ");
    fgets(buffer, BUFFER_SIZE, stdin);

    // Send message to server
    send(sock, buffer, strlen(buffer), 0);
    printf("Message sent to server: %s\n", buffer);

    // Read echoed message from server
    int valread = read(sock, buffer, BUFFER_SIZE);
    printf("Message from server: %s\n", buffer);
    */
    //making the homepage
    int ch;
    do {
      printf("\n------------BANK-------------\n");
      printf("1.Create new account\n");
      printf("2.Login\n");
      printf("3.Exit\n");
      printf("Enter your choice : ");
      scanf("%d",&ch);
      send(sock,&ch,sizeof(ch),0);
      switch(ch)
      {
          case 1: create_account_client(sock);
                  break;
          case 2: login_client(sock);
                  break;
          case 3: exit(0);
          default : printf("Incorrect option entered!\n");
      }
    } while (ch != 3);
    // Close socket
    close(sock);
    
    return 0;
}

