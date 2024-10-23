#include <stdio.h>
#include "login.h"  // Include the login header

int main() {
    char username[100], password[100];
    char encrypted_password[100];  // Store the encrypted password
    char *login_result;

    // Prompt the user for username and password
    printf("Enter username: ");
    scanf("%s", username);

    printf("Enter password: ");
    scanf("%s", password);

    // Encrypt the password using Caesar cipher
    caesar_cipher_encrypt(password, encrypted_password);
    printf("%s\n",encrypted_password);
    // Check login credentials
    login_result = check_login(username, encrypted_password);
    printf("%s",login_result);
    // Output the result
    if (strcmp(login_result, "Invalid") == 0) {
        printf("Login failed: Invalid username or password.\n");
    } else {
        printf("Login successful: You are logged in as %s.\n", login_result);
    }

    return 0;
}


