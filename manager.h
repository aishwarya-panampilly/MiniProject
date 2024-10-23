/*3.	Manager:
o	Login System (one session per user)
o	Activate/Deactivate Customer Accounts
o	Assign Loan Application Processes to Employees
o	Review Customer Feedback
o	Change Password
o	Logout
o	Exit
*/
//deactivate/activate customer accounts server side
#ifndef MANAGER_H
#define MANAGER_H
#include "login.h"
//manager activation function client side 
void handle_customer_approval_client(int sock) {
    int customer_id;
    char action[10];

    printf("Enter customer ID: ");
    scanf("%d", &customer_id);

    printf("Enter action (activate or deactivate): ");
    scanf("%s", action);

    // Send the customer ID and action to the server
    send(sock, &customer_id, sizeof(customer_id), 0);
    send(sock, action, sizeof(action), 0);

    // Receive the server's response
    char response[1024] = {0};
    read(sock, response, sizeof(response));
    printf("%s\n", response);
}

//manager activation function server side
void handle_customer_approval_server(int sock) {
    char buffer[1024], action[10];
    int customer_id,current_id;
    char name[100], address[100];
    int age;
    float balance;
    int found = 0;

    // Receive customer ID and action (activate or deactivate) from the client
    recv(sock, &customer_id, sizeof(customer_id), 0);
    recv(sock, action, sizeof(action), 0);

    // Open the customer details approval file
    FILE *approval_file = fopen("customerforactivation.txt", "r");
    if (approval_file == NULL) {
        perror("Could not open customerforactivation file");
        return;
    }

    // Open the customer details file and a temp file for modifications
    FILE *customer_file = fopen("customerdetails.txt", "a"); // For activation
    FILE *temp_file = fopen("temp_approval.txt", "w"); // For cleaning up approval file

    if (customer_file == NULL || temp_file == NULL) {
        perror("Could not open files for customer management");
        fclose(approval_file);
        return;
    }

    // Activate or Deactivate based on action
    if (strcmp(action, "activate") == 0) {

        while (fscanf(approval_file, "%d %s %d %s %f", &current_id, name, &age, address, &balance) != EOF) {
            if (current_id == customer_id) {
                found = 1;
                // Copy customer details to customerdetails.txt for activation
                fprintf(customer_file, "%d %s %d %s %.2f\n", customer_id, name, age, address, balance);
            } else {
                // Write unmodified lines to temp file
                fprintf(temp_file, "%d %s %d %s %.2f\n", current_id, name, age, address, balance);
            }
        }
         fflush(approval_file);
        fflush(temp_file);
        fclose(approval_file);
        fclose(temp_file);
        remove("customerforactivation.txt");
        rename("temp_approval.txt", "customerforactivation.txt");

        if (found) {
            send(sock, "Customer activated successfully.", strlen("Customer activated successfully."), 0);
        } else {
            send(sock, "Customer ID not found for activation.", strlen("Customer ID not found for activation."), 0);
        }

    } else if (strcmp(action, "deactivate") == 0) {
        FILE *temp_customer_file = fopen("temp_customer.txt", "w");
        FILE *customer_file_read = fopen("customerdetails.txt", "r");

        if (customer_file_read == NULL || temp_customer_file == NULL) {
            perror("Could not open files for deactivation");
            return;
        }

        while (fscanf(customer_file_read, "%d %s %d %s %f", &current_id, name, &age, address, &balance) != EOF) {
            if (current_id == customer_id) {
                found = 1;
                // Skip this line to remove the customer
                continue;
            } else {
                fprintf(temp_customer_file, "%d %s %d %s %.2f\n", current_id, name, age, address, balance);
            }
        }
         fflush(customer_file_read);
        fflush(temp_customer_file);
        fclose(customer_file_read);
        fclose(temp_customer_file);
        remove("customerdetails.txt");
        rename("temp_customer.txt", "customerdetails.txt");

        if (found) {
            send(sock, "Customer deactivated successfully.", strlen("Customer deactivated successfully."), 0);
        } else {
            send(sock, "Customer ID not found for deactivation.", strlen("Customer ID not found for deactivation."), 0);
        }
    } else {
        send(sock, "Invalid action.", strlen("Invalid action."), 0);
    }
}

//assign loan applications processes to employees server side
void assign_loans_server(int sock) {
    char line[256];
    char loan_type[50];
    int customer_id;
    float loan_amount;
    int assigned_employee;

    // Open loan applications file for reading
    FILE *file = fopen("loan_applications.txt", "r");
    if (file == NULL) {
        perror("Could not open loan application file");
        return;
    }

    // Open assignment file to save manager's assignments
    FILE *assignments = fopen("manager_assignments.txt", "a");
    if (assignments == NULL) {
        perror("Could not open manager assignments file");
        fclose(file);
        return;
    }

    // Read loan applications and assign employees based on user input
    while (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "%d %s %f", &customer_id, loan_type, &loan_amount);
        
        // Send loan application details
        char msg[256];
        snprintf(msg, sizeof(msg), "Customer ID: %d, Loan Type: %s, Amount: %.2f\n", customer_id, loan_type, loan_amount);
        send(sock, msg, strlen(msg), 0); 

        // Wait for the assigned employee ID from the client
        recv(sock, &assigned_employee, sizeof(assigned_employee), 0);
        
        // Save assignment (customer_id, assigned_employee)
        fprintf(assignments, "%d %d\n", customer_id, assigned_employee);
    }

    // Signal the end of processing
    char *msg_confirmation = "END_OF_LOANS";
    send(sock, msg_confirmation, strlen(msg_confirmation), 0);

    fclose(file);
    fclose(assignments);
}
//client side
void manager_assign_loans_client(int sock) {
    char buffer[1024] = {0};
    char loan_info[256];

    while (1) {
        int bytes_received = recv(sock, loan_info, sizeof(loan_info), 0);
        if (bytes_received <= 0) {
            break; // Break if there is an error or connection is closed
        }

        // Null-terminate the received data
        loan_info[bytes_received] = '\0';
        
         // Check for end signal
        if (strcmp(loan_info, "END_OF_LOANS") == 0) {
            printf("All loans have been assigned.\n");
            break; // Exit the loop if this message is received
        }
        printf("Loan Application: %s", loan_info); 

        int assigned_employee;
        printf("Enter Employee ID to assign this loan: ");
        scanf("%d", &assigned_employee); // Get employee ID

        // Send the employee ID to the server
        send(sock, &assigned_employee, sizeof(assigned_employee), 0);
    }

    // Receive confirmation from server
    read(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);  // Display the server's confirmation message
}

//read feedback server side
void read_customer_feedback_server(int sock) {
    FILE *file = fopen("feedback.txt", "r");
    if (file == NULL) {
        perror("Could not open feedback file");
        return;
    }

    char line[256];
    char feedback_buffer[1024] = "Customer Feedback:\n";

    // Read each line of feedback and append it to the buffer
    while (fgets(line, sizeof(line), file) != NULL) {
        strcat(feedback_buffer, line);  // Append feedback line to buffer
    }

    fclose(file);

    // Send feedback to the manager
    send(sock, feedback_buffer, strlen(feedback_buffer), 0);
}
void read_customer_feedback_client(int sock) {
    char buffer[1024] = {0};

    // Request feedback from the server
    printf("Fetching customer feedback...\n");

    // Receive the feedback from the server
    int bytes_received = read(sock, buffer, sizeof(buffer) - 1);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';  
        printf("%s\n", buffer);        
    } else {
        printf("No feedback received or an error occurred.\n");
    }
}

//employee options displayed client side
void manager_menu(int sock) {
    int choice;
    do {
        printf("--------MANAGER MENU-----------\n");
        printf("1. Activate/Deactivate Customer accounts\n");
        printf("2. Assign loan applications to employees\n");
        printf("3. Review Customer feedback\n");
        printf("4. Change password\n");
        printf("5. Logout\n");
        printf("6. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);
        send(sock, &choice, sizeof(choice), 0);

        switch (choice) {
            case 1:
                handle_customer_approval_client(sock);
                break;
            case 2:
                manager_assign_loans_client(sock);
                break;
            case 3:
                read_customer_feedback_client(sock);
                break;
            case 4: 
                change_password_client(sock);  // Option to change password
                break;
            case 5:
                printf("Logging out...\n");
                return;
            case 6:
                exit(0);
                break;
            default:
                printf("Invalid choice, please try again.\n");
        }
    } while (choice != 6);
}
// customer menu option for the server
void handle_manager_request(int sock) {
    int choice;
    while (recv(sock, &choice, sizeof(choice), 0) > 0) {
        switch (choice) {
            case 1:
                handle_customer_approval_server(sock);
                break;
            case 2:
                assign_loans_server(sock);  
                break;
            case 3:
                read_customer_feedback_server(sock);
                break;
            case 4: 
                change_password_server(sock);  // Handle change password request
                break;
            case 5:
                printf("Manager logged out.\n");
                return;  // Logout, exit the loop
            case 6:
                printf("Exiting\n");
                break;
            default:
                printf("Invalid choice received from client.\n");
        }
    }
}
#endif
