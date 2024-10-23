#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <termios.h>

#define PORT 8080

// Function to hide password input and show asterisks immediately
void password_hider(char* password) {
    struct termios oldt, newt;
    int i = 0;
    char c;

    // Turn off terminal echo and canonical mode (to read one character at a time)
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);  // Disable echo and canonical mode
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Read password character by character
    while ((c = getchar()) != '\n' && i < 49) {
        if (c == 127 || c == 8) {  // Handle backspace
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else {
            password[i++] = c;
            printf("*");
            fflush(stdout);
        }
    }
    password[i] = '\0';

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}

// Function to XOR encrypt/decrypt the password
/*void xor_encrypt_decrypt(char* input, const char* key) {
    int key_len = strlen(key);
    int input_len = strlen(input);

    for (int i = 0; i < input_len; i++) {
        input[i] = input[i] ^ key[i % key_len];  // XOR each char with key
    }
}*/

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char username[50], password[50], buffer[1024] = {0};
    const char* key = "mysecretkey";  // Key for XOR encryption
    int choice;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/Address not supported\n");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed\n");
        return -1;
    }

    while (1) {
        printf("Choose an option:\n1. Login\n2. Logout\n3. Exit\n");
        scanf("%d", &choice);
        send(sock, &choice, sizeof(int), 0);

        if (choice == 1) {  // Login
            printf("Enter username: ");
            scanf("%s", username);-%
            getchar();
            printf("Enter password: ");
            password_hider(password);  // Hide password input
            printf("password: %s\n",password);
           // xor_encrypt_decrypt(password, key);  // Encrypt password
            int username_len = strlen(username)+1;
            int password_len = strlen(password)+1;
            // Send username and encrypted password to server
            send(sock, &username_len,sizeof(int),0);
            send(sock, username, username_len, 0);
            send(sock, &password_len, sizeof(int),0);
            send(sock, password, password_len, 0);
            recv(sock, buffer, sizeof(buffer), 0);  // Receive response
            printf("%s\n", buffer);

        } else if (choice == 2) {  // Logout
            printf("Enter username to logout: ");
            scanf("%s", username);
            send(sock, username, strlen(username) + 1, 0);
            recv(sock, buffer, sizeof(buffer), 0);  // Receive response
            printf("%s\n", buffer);

        } else if (choice == 3) {  // Exit
            close(sock);
            break;
        }
    }

    return 0;
}

