#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define MAX_PASSWORD_LENGTH 50

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
    while ((c = getchar()) != '\n' && i < MAX_PASSWORD_LENGTH - 1) {
        if (c == 127 || c == 8) {  // Handle backspace
            if (i > 0) {
                i--;
                printf("\b \b");  // Move back, print space, move back again
            }
        } else {
            password[i++] = c;
            printf("*");
            fflush(stdout);
        }
    }
    password[i] = '\0';  // Null-terminate the password

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}

int main() {
    char password[MAX_PASSWORD_LENGTH];

    printf("Enter your password: ");
    password_hider(password);  // Call the password hider function
    printf("Your password is: %s\n", password);  // Display the password (for demonstration purposes)

    return 0;
}

