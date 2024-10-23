#include <stdio.h>
#include <string.h>

void xor_encrypt_decrypt(char* input, const char* key) {
    int key_len = strlen(key);
    int input_len = strlen(input);

    for (int i = 0; i < input_len; i++) {
        input[i] = input[i] ^ key[i % key_len];  // XOR each char with key
    }
}

void print_encrypted_password(char* password, const char* key) {
    // Store original password to print it after encryption
    char original_password[50];
    strcpy(original_password, password);

    // Encrypt the password
    xor_encrypt_decrypt(password, key);

    // Print original and encrypted passwords
    printf("Original Password: %s\n", original_password);
    
    // Print encrypted password as hex values for clarity
    printf("Encrypted Password: ");
    for (int i = 0; i < strlen(original_password); i++) {
        printf("%02X", (unsigned char)password[i]);
    }
    printf("\n");
}

int main() {
    char password[50]; // Buffer to hold the password
    const char* key = "mysecretkey"; // The key used for XOR encryption

    // Input password from user
    printf("Enter password to encrypt: ");
    scanf("%s", password); // Simple input; consider using secure input for production

    print_encrypted_password(password, key);

    return 0;
}


