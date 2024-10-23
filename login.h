// put login details of all members admin,manager,employee, customer in this 
//adding guards
#ifndef LOGIN_H
#define LOGIN_H
#include <termios.h>

void change_password_server(int sock) {
    char old_password[50], new_password[50];
    char line[256], file_password[50], file_role[10] = {0},file_username[50];
    int found = 0,file_user_id;
    int id;

    // Receive user ID from client
    recv(sock, &id, sizeof(id), 0);

    // Receive old password from client
    recv(sock, old_password, sizeof(old_password), 0);

    // Receive new password from client
    recv(sock, new_password, sizeof(new_password), 0);

    // Open userdetails file for reading and updating
    FILE *file = fopen("userdetails.txt", "r+");
    if (file == NULL) {
        perror("Could not open userdetails.txt");
        return;
    }

    // Temporary file for writing updated data
    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        perror("Could not open temp file");
        fclose(file);
        return;
    }

    // Go through userdetails.txt and find the matching user ID and password
    while (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "%s %s %s %d", file_role,file_username, file_password,&file_user_id);

        // If this is the user we're updating, check the password
        if ((id == file_user_id) && strcmp(old_password, file_password) == 0) {
            // Update the password in the temp file
            fprintf(temp, "%s %s %s %d\n", file_role,file_username, new_password, file_user_id);
            found = 1;
        } else {
            // Copy the unchanged line to the temp file
            fprintf(temp, "%s", line);
        }
    }

    fclose(file);
    fclose(temp);

    if (found) {
        // Replace old userdetails.txt with updated file
        remove("userdetails.txt");
        rename("temp.txt", "userdetails.txt");

        // Send success message to client
        char *success_msg = "Password changed successfully!";
        send(sock, success_msg, strlen(success_msg), 0);
    } else {
        // Remove temp file
        remove("temp.txt");

        // Send failure message to client
        char *failure_msg = "Password change failed! User ID or old password incorrect.";
        send(sock, failure_msg, strlen(failure_msg), 0);
    }
}
//client code 
void change_password_client(int sock) {
    char old_password[50], new_password[50];
    char buffer[1024] = {0};
    int id;
    
    printf("Enter ID for confirmation: ");
    scanf("%d",&id);
    // Input old password
    printf("Enter old password: ");
    scanf("%s", old_password);

    // Input new password
    printf("Enter new password: ");
    scanf("%s", new_password);

    // Send user ID to the server
    send(sock, &id, sizeof(id), 0);

    // Send old password to the server
    send(sock, old_password, strlen(old_password), 0);

    // Send new password to the server
    send(sock, new_password, strlen(new_password), 0);

    // Receive response from the server
    read(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);  // Display the server's response
}

void create_account_server(int sock) {
    int id;
    char role[20], username[50], password[50];
    char line[256], buffer[1024];
    int found = 0;

    // Receive user/employee/manager ID from the client
    recv(sock, &id, sizeof(id), 0);

    // Check customerdetails.txt for the ID
    FILE *file = fopen("customerdetails.txt", "r");
    if (file != NULL) {
        while (fgets(line, sizeof(line), file) != NULL) {
            int customer_id;
            sscanf(line, "%d", &customer_id);
            if (customer_id == id) {
                strcpy(role, "ct");
                found = 1;
                break;
            }
        }
        fclose(file);
    }

    // If not found, check employeedetails.txt
    if (!found) {
        file = fopen("employeedetails.txt", "r");
        if (file != NULL) {
            while (fgets(line, sizeof(line), file) != NULL) {
                int employee_id;
                sscanf(line, "%d", &employee_id);
                if (employee_id == id) {
                    strcpy(role, "emp");
                    found = 1;
                    break;
                }
            }
            fclose(file);
        }
    }

    // If not found, check managerdetails.txt
    if (!found) {
        file = fopen("managerdetails.txt", "r");
        if (file != NULL) {
            while (fgets(line, sizeof(line), file) != NULL) {
                int manager_id;
                sscanf(line, "%d", &manager_id);
                if (manager_id == id) {
                    strcpy(role, "man");
                    found = 1;
                    break;
                }
            }
            fclose(file);
        }
    }

    // If user ID is found, display the welcome message
    if (found) {
        snprintf(buffer, sizeof(buffer), "Welcome! Please create your username and password.\n");
        send(sock, buffer, strlen(buffer), 0);

        // Receive the username and password from the client
        recv(sock, username, sizeof(username), 0);
        recv(sock, password, sizeof(password), 0);

        // Save the user details to userdetails.txt (append mode)
        FILE *userfile = fopen("userdetails.txt", "a");
        if (userfile != NULL) {
            fprintf(userfile, "%s %s %s %d\n", role, username, password, id);
            fclose(userfile);

            // Inform the client that the account creation is successful
            strcpy(buffer, "Account created successfully! You can now log in.\n");
            send(sock, buffer, strlen(buffer), 0);
        } else {
            strcpy(buffer, "Error creating account. Please try again.\n");
            send(sock, buffer, strlen(buffer), 0);
        }
    } else {
        // If user ID is not found, inform the client
        strcpy(buffer, "Invalid ID. No such user/employee/manager found.\n");
        send(sock, buffer, strlen(buffer), 0);
    }
}
void create_account_client(int sock) {
    int id;
    char username[50], password[50];
    char buffer[1024];

    // Ask the user for their ID
    printf("Enter your user/employee/manager ID: ");
    scanf("%d", &id);

    // Send the ID to the server
    send(sock, &id, sizeof(id), 0);

    // Receive and display the server's welcome message or error message
    int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    // If the user is valid, ask for username and password
    if (strstr(buffer, "Welcome") != NULL) {
        printf("Enter your desired username: ");
        scanf("%s", username);

        printf("Enter your desired password: ");
        scanf("%s", password);

        // Send the username and password to the server
        send(sock, username, sizeof(username), 0);
        send(sock, password, sizeof(password), 0);

        // Receive confirmation from the server
        bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("%s", buffer);
        }
    }
}
#endif
