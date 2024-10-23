/*2.	Bank Employee: Bank employees manage customer accounts, including opening, modifying, or closing accounts, as well as processing loans.
o	Login System (one session per user)
o	Add New Customer
o	Modify Customer Details
o	Process Loan Applications
o	Approve/Reject Loans
o	View Assigned Loan Applications
View Customer Transactions( Passbook Kind 
o	Change Password
o	Logout
o	Exit

*/  
#ifndef EMPLOYEE_H
#define EMPLOYEE_H
#include "login.h"
#include <sys/file.h>
#include <stdlib.h>
// add new customer server side
void add_customer_to_file_server(int sock) {
    char name[50], address[100];
    int age,customer_id;
    float balance;
    char* status = "Pending";
    // Receive customer details from the client
    recv(sock, &customer_id, sizeof(customer_id), 0);
    recv(sock, name, sizeof(name), 0);
    recv(sock, &age, sizeof(age), 0);
    recv(sock, address, sizeof(address), 0);
    recv(sock, &balance, sizeof(balance), 0);

    // Open the customer details file for appending
    FILE *file = fopen("customerforactivation.txt", "a");
    if (file == NULL) {
        perror("Could not open customer activation file");
        return;
    }

    // Write customer details to the file
    fprintf(file, "%d %s %d %s %.2f %s\n", customer_id, name, age, address, balance,status);
    fclose(file);

    // Send confirmation back to the employee
    char *msg = "Customer added successfully!";
    send(sock, msg, strlen(msg), 0);
}
// add new customer client side 
void add_new_customer_client(int sock) {
    char name[50], address[100];
    int age,customer_id;
    float balance;

    printf("Enter Customer ID: ");
    scanf("%d", &customer_id);
    
    printf("Enter Customer Name: ");
    scanf("%s", name);

    printf("Enter Customer Age: ");
    scanf("%d", &age);

    printf("Enter Customer Address: ");
    scanf(" %[^\n]", address);  // Use this to read a string with spaces

    printf("Enter Initial Balance: ");
    scanf("%f", &balance);

    
    // Send customer details to server
    send(sock, &customer_id, sizeof(customer_id), 0);
    send(sock, name, sizeof(name), 0);
    send(sock, &age, sizeof(age), 0);
    send(sock, address, sizeof(address), 0);
    send(sock, &balance, sizeof(balance), 0);

    // Optionally receive a confirmation from the server
    char buffer[1024] = {0};
    recv(sock, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);
}
//modify customer details server side
void modify_customer_in_file_server(int sock) {
    int customer_id;
    char name[50], new_name[50];
    char address[100], new_address[100];
    int age, new_age;
    float balance;
    char line[256], buffer[1024] = {0};
    int found = 0;

    // Receive customer ID from the client
    recv(sock, &customer_id, sizeof(customer_id), 0);

    // Open the customer details file for reading and a temporary file for writing
    FILE *file = fopen("customerdetails.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    if (file == NULL || temp == NULL) {
        perror("Could not open customer details file");
        return;
    }

    // Receive new details from the client
    recv(sock, new_name, sizeof(new_name), 0);
    recv(sock, &new_age, sizeof(new_age), 0);
    recv(sock, new_address, sizeof(new_address), 0);

    while (fgets(line, sizeof(line), file) != NULL) {
        char current_name[50], current_address[100];
        int current_age,current_id;
        float current_balance;

        sscanf(line, "%d %s %d %s %f", &current_id, current_name, &current_age, current_address, &current_balance);

        // If this is the customer we want to modify
        if (current_id == customer_id) {
            // Modify only the provided fields
            if (strcmp(new_name, "") != 0) {
                strcpy(current_name, new_name);
            }
            if (new_age != -1) {
                current_age = new_age;
            }
            if (strcmp(new_address, "") != 0) {
                strcpy(current_address, new_address);
            }

            // Write modified customer details to the temporary file
            fprintf(temp, "%d %s %d %s %.2f\n", current_id, current_name, current_age, current_address, current_balance);
            found = 1; // Mark that we found and modified the customer
        } else {
            // Write unmodified customer details to the temporary file
            fprintf(temp, "%d %s %d %s %.2f\n", current_id, current_name, current_age, current_address, current_balance);
        }
    }

    fclose(file);
    fclose(temp);

    // Replace the old file with the new file
    remove("customerdetails.txt");
    rename("temp.txt", "customerdetails.txt");

    // Send confirmation back to the employee
    if (found) {
        strcpy(buffer, "Customer details updated successfully!");
    } else {
        strcpy(buffer, "Customer ID not found.");
    }
    send(sock, buffer, sizeof(buffer), 0);
}
//modify customer client side
void modify_customer_details_client(int sock) {
    int customer_id;
    char name[50], new_name[50];
    char address[100], new_address[100];
    int age, new_age;
    float balance, new_balance;

    printf("Enter Customer ID to modify: ");
    scanf("%d", &customer_id);

    // Send customer ID to the server
    send(sock, &customer_id, sizeof(customer_id), 0);

    // Get current details 

    printf("Enter new Name - press enter to skip: ");
    scanf(" %[^\n]", new_name); // Read new name

    printf("Enter new Age - enter -1 to skip: ");
    scanf("%d", &new_age); 

    printf("Enter new Address - press enter to skip: ");
    scanf(" %[^\n]", new_address); // Read new address

    // Send new details to server
    send(sock, new_name, sizeof(new_name), 0);
    send(sock, &new_age, sizeof(new_age), 0);
    send(sock, new_address, sizeof(new_address), 0);

    // Optionally receive a confirmation from the server
    char buffer[1024] = {0};
    recv(sock, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);
}

//approve/reject loans server side
//remove employee id from header??
void review_loans_server(int sock, int employee_id) {
    char line[256];
    int approval_status,customer_id,assigned_employee;
    char buffer[1024] = {0};

    // Open manager's assignment file
    FILE *assignments = fopen("manager_assignments.txt", "r");
    if (assignments == NULL) {
        perror("Could not open manager assignments file");
        return;
    }

    // Open approval file to save employee's decisions
    FILE *approvals = fopen("employee_approvals.txt", "a");
    if (approvals == NULL) {
        perror("Could not open employee approvals file");
        return;
    }

    int fd_approvals = fileno(approvals);
    if (flock(fd_approvals, LOCK_EX) == -1) {
        perror("Could not lock approvals file");
        fclose(approvals);
        return;
    }

    // Send loan applications assigned to this employee for review
    while (fgets(line, sizeof(line), assignments) != NULL) {
        sscanf(line, "%d %d", &customer_id, &assigned_employee);

        // Check if this loan is assigned to the current employee
        if (assigned_employee == employee_id) {
            // Send loan details to employee for review
            sprintf(buffer, "Customer ID: %d\nLoan Application: %d\n", customer_id, assigned_employee);
            send(sock, buffer, strlen(buffer), 0);

            // Employee makes approval decision (1 for approve, 0 for reject)
            recv(sock, &approval_status, sizeof(approval_status), 0);

            // Save the decision in the approvals file
            fprintf(approvals, "%d %d\n", customer_id, approval_status);
        }
    }

    // Unlock file and close files
    flock(fd_approvals, LOCK_UN);
    fclose(assignments);
    fclose(approvals);

    // Confirm to employee
    char *msg = "Loan application review completed!";
    send(sock, msg, strlen(msg), 0);
}
//approve loans client side 
void employee_review_loans_client(int sock, int employee_id) {
    int customer_id;
    int approval_status;
    char buffer[1024] = {0};
    
    // Request to review loans assigned to this employee
    send(sock, &employee_id,sizeof(employee_id), 0);

    // Receive loan details
    read(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);

    // For each loan, approve or reject
    printf("Enter 1 to approve, 0 to reject: ");
    scanf("%d", &approval_status);

    // Send decision to server
    send(sock, &approval_status, sizeof(approval_status), 0);

    // Receive confirmation from server
    read(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}
//view loan applications client side
void view_assigned_loan_applications_client(int sock) {
    // Send request to the server to get assigned loan applications
    char request[] = "GET_LOAN_APPLICATIONS"; // Define a request string
    send(sock, request, sizeof(request), 0);

    // Buffer to receive loan applications from server
    char buffer[1024] = {0};
    
    // Receive loan applications from the server
    recv(sock, buffer, sizeof(buffer), 0);
    
    printf("Assigned Loan Applications:\n%s\n", buffer);
}
//view loan applications server side
void view_assigned_loan_applications_server(int sock) {
    char buffer[1024] = {0};
    FILE *file = fopen("loan_applications.txt", "r"); // Assuming loan applications are stored here
    if (file == NULL) {
        perror("Could not open loan applications file");
        return;
    }

    // Prepare to send the applications
    char line[256];
    strcpy(buffer, ""); // Clear the buffer

    // Read loan applications from the file and format for sending
    while (fgets(line, sizeof(line), file) != NULL) {
        strcat(buffer, line); // Concatenate each line to buffer
    }

    fclose(file);

    // Send the applications back to the employee
    send(sock, buffer, sizeof(buffer), 0);
}
//view customer transactions, client side
void view_customer_transactions_client(int sock) {
    int customer_id;
    
    // Prompt the employee for the customer ID
    printf("Enter Customer ID to view transactions: ");
    scanf("%d", &customer_id);

    // Send the customer ID to the server
    send(sock, &customer_id, sizeof(customer_id), 0);

    // Buffer to receive transaction history from the server
    char buffer[1024] = {0};

    // Receive transaction history from the server
    recv(sock, buffer, sizeof(buffer), 0);

    printf("Transaction History for Customer ID %d:\n%s\n", customer_id, buffer);
}
//view customer transactions server side
void view_customer_transactions_server(int sock) {
    int customer_id;
    char line[1024];
    // Receive the customer ID from the client
    recv(sock, &customer_id, sizeof(customer_id), 0);
    
    FILE *file;
    char filename[50];

    snprintf(filename, sizeof(filename), "transactionhistory_%d.txt", customer_id);
    // Buffer to store transaction history
    file = fopen(filename, "r");
    if (file == NULL) {
        // If file is not found, send an error message to client
        char *error_msg = "Error: Transaction history not found for this customer.\n";
        send(sock, error_msg, strlen(error_msg), 0);
        return;
    }

     while (fgets(line, sizeof(line), file) != NULL) {
        send(sock, line, strlen(line), 0);
    }

    fclose(file);
    
}
//employee options displayed client side
void employee_menu(int sock) {
    int choice;
    do {
        printf("--------EMPLOYEE MENU-----------\n");
        printf("1. Add new customer\n");
        printf("2. Modify customer details\n");
        printf("3. Approve/Reject Loans\n");
        printf("4. View Assigned loan applications\n");
        printf("5. View customer transactions\n");
        printf("6. Change Password\n");
        printf("7. Logout\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        // Send choice to server
        send(sock, &choice, sizeof(choice), 0);

        switch (choice) {
            case 1:
                add_new_customer_client(sock); 
                break;
            case 2:
                modify_customer_details_client(sock);  
                break;
            case 3: {
                printf("Enter your ID for confirmation: ");
                int id;
                scanf("%d",&id);
                send(sock,&id,sizeof(id),0);
                employee_review_loans_client(sock,id);
                }
                break;
            case 4:
                view_assigned_loan_applications_client(sock);
                break;
            case 5:
                view_customer_transactions_client(sock);
                break;
            case 6: 
                change_password_client(sock);  // Option to change password
                break;
            case 7:
                printf("Logging out...\n");
                return;
            case 8 :
                 exit(0);
            default:
                printf("Invalid choice, please try again.\n");
        }
    } while (choice != 8);
}
// customer menu option for the server
void handle_employee_request(int sock) {
    int choice;
    while (recv(sock, &choice, sizeof(choice), 0) > 0) {
        switch (choice) {
            case 1:
                add_customer_to_file_server(sock);  
                break;
            case 2:
                modify_customer_in_file_server(sock);  
                break;
            case 3: {
                int id;
                recv(sock,&id,sizeof(id),0);
                review_loans_server(sock,id); 
                }
                break;
            case 4:
                view_assigned_loan_applications_server(sock);  
                break;
            case 5:
                view_customer_transactions_server(sock);  
                break;
            case 6: 
                change_password_server(sock);  // Handle change password request
                break;
            case 7:
                printf("Customer logged out.\n");
                return;  // Logout, exit the loop
            case 8 :
                printf("Exiting..\n");
                break;
            default:
                printf("Invalid choice received from client.\n");
        }
    }
}
#endif
