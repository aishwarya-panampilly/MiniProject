#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

// Function to XOR encrypt/decrypt the password
/*
void xor_encrypt_decrypt(char* input, const char* key) {
    int key_len = strlen(key);
    int input_len = strlen(input);

    for (int i = 0; i < input_len; i++) {
        input[i] = input[i] ^ key[i % key_len];  // XOR each char with key
    }
}
*/
// Function to check login details
int check_login(const char* username, const char* encrypted_password) {
    //const char* key = "mysecretkey";  // Same key used for encryption

    // Open the file containing user details
    FILE *file = fopen("userdetails.txt", "r");
    if (!file) {
        perror("Could not open userdetails.txt");
        return 0;
    }

    char stored_username[50], stored_password[50];
    char decrypted_password[50];
    printf("%s\n",username);
    printf("%s\n",encrypted_password);
    
    // Read user details from the file line by line
    while (fscanf(file, "%s %s", stored_username, stored_password) != EOF) {
        printf("%s\n",stored_username);
        printf("%s\n",stored_password);
        if (strcmp(username, stored_username) == 0) {
            // Decrypt the stored password
            //strcpy(decrypted_password, stored_password);
           // xor_encrypt_decrypt(decrypted_password, key);

            // Compare the decrypted password with the received encrypted password
            if (strcmp(/*decrypted_password*/stored_password, encrypted_password) == 0) {
                fclose(file);
                return 1;  // Login successful
            }
        }
    }

    fclose(file);
    return 0;  // Login failed
}

// A list of currently logged-in users
char logged_in_users[100][50];  // Can store up to 100 logged-in users
int logged_in_count = 0;

// Function to check if a user is already logged in
int is_user_logged_in(const char* username) {
    for (int i = 0; i < logged_in_count; i++) {
        if (strcmp(logged_in_users[i], username) == 0) {
            return 1;  // User is already logged in
        }
    }
    return 0;  // User is not logged in
}

// Function to add a user to the logged-in list
void add_logged_in_user(const char* username) {
    strcpy(logged_in_users[logged_in_count], username);
    logged_in_count++;
}

// Function to remove a user from the logged-in list
void remove_logged_in_user(const char* username) {
    for (int i = 0; i < logged_in_count; i++) {
        if (strcmp(logged_in_users[i], username) == 0) {
            // Shift the remaining users down to fill the gap
            for (int j = i; j < logged_in_count - 1; j++) {
                strcpy(logged_in_users[j], logged_in_users[j + 1]);
            }
            logged_in_count--;
            break;
        }
    }
}

// Function to handle the client
void handle_client(int sock) {
    //char buffer[1024] = {0}; not working
    int username_len,password_len;
    char username[50], password[50];
    int choice;

    while (1) {
        // Receive the user's choice (login/logout/exit)
        recv(sock, &choice, sizeof(int), 0);

        if (choice == 1) {  // Login
            // Receive username and encrypted password from client
            recv(sock, &username_len, sizeof(int), 0);
            recv(sock, username, username_len, 0);
            recv(sock, &password_len, sizeof(int), 0);
            recv(sock, password, password_len, 0);

            // Check if the user is already logged in
            if (is_user_logged_in(username)) {
                char *already_logged_in = "You are already logged in!";
                send(sock, already_logged_in, strlen(already_logged_in), 0);
            } else {
                // Check login credentials
                if (check_login(username, password)) {
                    add_logged_in_user(username);  // Mark user as logged in
                    char *success = "Login successful!";
                    send(sock, success, strlen(success), 0);
                } else {
                    char *failure = "Login failed. Incorrect username or password.";
                    send(sock, failure, strlen(failure), 0);
                }
            }

        } else if (choice == 2) {  // Logout
            // Receive username for logout
            recv(sock, username, strlen(username)+1, 0);
            remove_logged_in_user(username);  // Remove user from logged-in list
            char *logout_msg = "You have been logged out!";
            send(sock, logout_msg, strlen(logout_msg), 0);

        } else if (choice == 3) {  // Exit
            printf("Client disconnected.\n");
            close(sock);
            break;
        }
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Socket creation
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Waiting for a connection...\n");
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Client connected!\n");
        handle_client(new_socket);  // Handle the client connection
    }
    return 0;
}

