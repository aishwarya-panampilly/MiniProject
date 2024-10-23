/*1.	Customer: Customers log in using their credentials and have access to banking operations such as viewing account balances, depositing or withdrawing money, transferring funds, and applying for loans. The system ensures proper locking mechanisms to avoid race conditions when performing concurrent operations.
o	Login System (one session per user)
o	View Account Balance
o	Deposit Money
o	Withdraw Money
o	Transfer Funds
o	Apply for a Loan
o	Change Password
o	Adding Feedback
o	View Transaction History
o	Logout
o	Exit
*/
// now its using customer id.. you have to change it after logging in with username and password.. they will have an associated id.. only that will be used by the user
//account balance server side
#ifndef CUSTOMER_H
#define CUSTOMER_H
#include <sys/file.h>
#include <time.h>
#include <stdlib.h>
#include "login.h"
//view transaction history 
// first log in the transactions - server side
void log_transaction_server(int customer_id, const char* transaction_type, float amount, float new_balance) {
    char filename[100], timestamp[50];
    FILE *file;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    // Create filename based on customer ID
    snprintf(filename, sizeof(filename), "transactionhistory_%d.txt", customer_id);

    // Open file in append mode
    file = fopen(filename, "a");
    if (file == NULL) {
        perror("Could not open transaction history file");
        return;
    }

    // Get current timestamp
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

    // Log transaction in format: Date, Transaction Type, Amount, New Balance
    fprintf(file, "Date: %s\nType: %s\nAmount: %.2f\nBalance: %.2f\n\n", 
            timestamp, transaction_type, amount, new_balance);

    fclose(file);
}
// Function to send transaction history to the client
void send_transaction_history_server(int client_socket,int customer_id) {
    FILE *file;
    char line[1024];
    char filename[50];
    // Generate filename for the specific customer
    snprintf(filename, sizeof(filename), "transactionhistory_%d.txt", customer_id);
    
    file = fopen(filename, "r");
    if (file == NULL) {
        // If file is not found, send an error message to client
        char *error_msg = "Error: Transaction history not found for this customer.\n";
        send(client_socket, error_msg, strlen(error_msg), 0);
        return;
    }

    // Sending the transaction history line by line
    while (fgets(line, sizeof(line), file) != NULL) {
        send(client_socket, line, strlen(line), 0);
    }

    fclose(file);
    char *eot_msg = "END_OF_HISTORY";
    send(client_socket, eot_msg, strlen(eot_msg), 0);
}
//send transaction history client
void send_transaction_history_client(int sock, int customer_id)
{
  int bytes_received;
  char buffer[1024] = {0};
  char *end_signal = "END_OF_HISTORY";
    int end_signal_len = strlen(end_signal);
  
    send(sock,&customer_id,sizeof(customer_id),0);
  // Keep receiving the response from the server until it finishes
    while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';  // Null-terminate the received data
        if (strstr(buffer, end_signal) != NULL) {
            // Print everything before "END_OF_HISTORY"
            buffer[strstr(buffer, end_signal) - buffer] = '\0';
            printf("%s", buffer);
            break;
          }
        printf("%s", buffer);  
    }

    if (bytes_received == 0) {
        printf("\nTransaction history received successfully.\n");
    } else if (bytes_received < 0) {
        perror("Error receiving transaction history");
    }
   printf("\nReturning to menu...\n");
}
  
//account balance server side
void view_account_balance_server(int sock, int customer_id) {
    char line[256], file_name[50], file_address[100];
    int file_age, file_customer_id;
    float file_balance;
    int found = 0;

    // Receive customer ID from client
   // recv(sock, &customer_id, sizeof(customer_id), 0);

    // Open customerdetails file
    FILE *file = fopen("customerdetails.txt", "r");
    if (file == NULL) {
        perror("Could not open customerdetails.txt");
        return;
    }

    // Search for the customer ID in the file
    while (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "%d %s %d %s %f", &file_customer_id, file_name, &file_age, file_address, &file_balance);

        // If the customer ID matches, send the account balance to the client
        if (customer_id == file_customer_id) {
            found = 1;
            char buffer[1024];
            sprintf(buffer, "Account Balance for %d: Rs.%.2f", file_customer_id, file_balance);
            send(sock, buffer, strlen(buffer), 0);
            break;
        }
    }

    fclose(file);

    if (!found) {
        // If customer not found, send an error message
        char *error_msg = "Customer not found!";
        send(sock, error_msg, strlen(error_msg), 0);
    }
}
//view account balance client side
void view_account_balance_client(int sock,int customer_id) {
    char buffer[1024] = {0};

    // Send customer ID to the server
  //  send(sock, &customer_id, sizeof(customer_id), 0);

    // Receive and print the account balance from the server
    read(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}

//deposit money server side
void deposit_money_server(int sock,int customer_id) {
    char name[50], address[100], line[256];
    int age, found = 0, file_customer_id;
    float balance, deposit_amount,new_balance;
    
    // Receive customer ID and deposit amount from client
  //  recv(sock, &customer_id, sizeof(customer_id), 0);
    recv(sock, &deposit_amount, sizeof(deposit_amount), 0);
  
    // Open customer details file
    FILE *file = fopen("customerdetails.txt", "r+");
    if (file == NULL) {
        perror("Could not open customer details file");
        return;
    }

    // File descriptor for file locking
    int fd = fileno(file);

    // Apply exclusive lock on the file for writing
    if (flock(fd, LOCK_EX) == -1) {
        perror("Could not lock file");
        fclose(file);
        return;
    }

    // Open a temporary file to store modified details
    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        perror("Could not open temp file");
        fclose(file);
        return;
    }

    // Search for the customer ID and update balance
    while (fgets(line, sizeof(line), file) != NULL) {
        //printf("running through\n");
        sscanf(line, "%d %s %d %s %f", &file_customer_id, name, &age, address, &balance);

        if (customer_id == file_customer_id) {
            balance += deposit_amount;// Update balance
            new_balance = balance;
             printf("New Balance for Customer ID %d: %.2f\n", customer_id,new_balance);
            found = 1;
        }
        // Write the details to the temp file
        fprintf(temp, "%d %s %d %s %.2f\n", file_customer_id, name, age, address, balance);
       
  }

    fclose(file);
    fclose(temp);

    if (found) {
        // Replace the original file with the updated one
        remove("customerdetails.txt");
        rename("temp.txt", "customerdetails.txt");
        // Log the transaction
        log_transaction_server(customer_id, "Deposit", deposit_amount, new_balance);
        // Send success message to client
        char success_msg[256];
        sprintf(success_msg, "Deposit successful! New balance: $%.2f", new_balance);
        send(sock, success_msg, strlen(success_msg), 0);
    } else {
        // Customer ID not found, send error message to client
        char *error_msg = "Customer not found!";
        send(sock, error_msg, strlen(error_msg), 0);
    }

    // Release the file lock
    flock(fd, LOCK_UN);
}
//deposit money client side 
void deposit_money_client(int sock,int customer_id) {
    float deposit_amount;
    char buffer[1024] = {0};

    printf("Enter the deposit amount: ");
    scanf("%f", &deposit_amount);

    // Send customer ID and deposit amount to the server
   // send(sock, &customer_id, sizeof(customer_id), 0);
    send(sock, &deposit_amount, sizeof(deposit_amount), 0);

    // Receive and print the server response
    read(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}
//withdraw money server side
void withdraw_money_server(int sock,int customer_id) {
    char name[50], address[100], line[256];
    int age, found = 0, file_customer_id;
    float balance, withdraw_amount,new_balance;
    
    // Receive customer ID and withdrawal amount from client
    //recv(sock, &customer_id, sizeof(customer_id), 0);
    recv(sock, &withdraw_amount, sizeof(withdraw_amount), 0);

    // Open customer details file
    FILE *file = fopen("customerdetails.txt", "r+");
    if (file == NULL) {
        perror("Could not open customer details file");
        return;
    }

    // File descriptor for file locking
    int fd = fileno(file);

    // Apply exclusive lock on the file for writing
    if (flock(fd, LOCK_EX) == -1) {
        perror("Could not lock file");
        fclose(file);
        return;
    }

    // Open a temporary file to store modified details
    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        perror("Could not open temp file");
        fclose(file);
        return;
    }

    // Search for the customer ID and update balance
    while (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "%d %s %d %s %f", &file_customer_id, name, &age, address, &balance);

        if (customer_id == file_customer_id) {
            if (balance >= withdraw_amount) {
                balance -= withdraw_amount; // Update balance after withdrawal
                new_balance = balance;
                found = 1;
            } else {
                // Not enough balance
                char *error_msg = "Insufficient balance!";
                send(sock, error_msg, strlen(error_msg), 0);
                fclose(file);
                fclose(temp);
                remove("temp.txt");
                flock(fd, LOCK_UN);  // Unlock the file
                return;
            }
        }
        // Write the details to the temp file
        fprintf(temp, "%d %s %d %s %.2f\n", file_customer_id, name, age, address, balance);
    }

    fclose(file);
    fclose(temp);

    if (found) {
        // Replace the original file with the updated one
        remove("customerdetails.txt");
        rename("temp.txt", "customerdetails.txt");
        // Log the transaction
        log_transaction_server(customer_id, "Withdrawal", withdraw_amount, new_balance);
        // Send success message to client
        char success_msg[256];
        sprintf(success_msg, "Withdrawal successful! New balance: $%.2f", new_balance);
        send(sock, success_msg, strlen(success_msg), 0);
    } else {
        // Customer ID not found, send error message to client
        char *error_msg = "Customer not found!";
        send(sock, error_msg, strlen(error_msg), 0);
    }

    // Release the file lock
    flock(fd, LOCK_UN);
}
//withdraw money client side
void withdraw_money_client(int sock,int customer_id) {
    float withdraw_amount;
    char buffer[1024] = {0};

    // Input customer ID and withdrawal amount
   
    printf("Enter the withdrawal amount: ");
    scanf("%f", &withdraw_amount);

    // Send customer ID and withdrawal amount to the server
   // send(sock, &customer_id, sizeof(customer_id), 0);
    send(sock, &withdraw_amount, sizeof(withdraw_amount), 0);

    // Receive and print the server response
    read(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}
//server side transfer funds
void transfer_funds_server(int sock,int from_customer_id) {
    int to_customer_id,from_file_customer_id, to_file_customer_id;
    char name[50], address[100], line[256];
    int age, found_from = 0, found_to = 0;
    float balance, transfer_amount, from_balance, to_balance,from_new_balance,to_new_balance;
    
    // Receive customer IDs and transfer amount from client
   // recv(sock, &from_customer_id, sizeof(from_customer_id), 0);
    recv(sock, &to_customer_id, sizeof(to_customer_id), 0);
    recv(sock, &transfer_amount, sizeof(transfer_amount), 0);

    // Open customer details file
    FILE *file = fopen("customerdetails.txt", "r+");
    if (file == NULL) {
        perror("Could not open customer details file");
        return;
    }

    // File descriptor for file locking
    int fd = fileno(file);

    // Apply exclusive lock on the file for writing
    if (flock(fd, LOCK_EX) == -1) {
        perror("Could not lock file");
        fclose(file);
        return;
    }

    // Open a temporary file to store modified details
    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        perror("Could not open temp file");
        fclose(file);
        return;
    }

    // Search for both customer IDs and update balances
    while (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "%d %s %d %s %f", &from_file_customer_id, name, &age, address, &balance);

        if (from_customer_id == from_file_customer_id) {
            if (balance >= transfer_amount) {
                from_balance = balance - transfer_amount;// Update balance after transfer
                from_new_balance = from_balance;
                found_from = 1;
            } else {
                // Not enough balance to transfer
                char *error_msg = "Insufficient balance to transfer!";
                send(sock, error_msg, strlen(error_msg), 0);
                fclose(file);
                fclose(temp);
                remove("temp.txt");
                flock(fd, LOCK_UN);  // Unlock the file
                return;
            }
        } else if (to_customer_id == from_file_customer_id) {
            to_balance = balance + transfer_amount;// Update balance after receiving transfer
            to_new_balance = to_balance;
            found_to = 1;
        }

        // Write the details to the temp file
        if (found_from && (from_customer_id == from_file_customer_id) ){
            fprintf(temp, "%d %s %d %s %.2f\n", from_file_customer_id, name, age, address, from_balance);
        } else if (found_to && (to_customer_id == from_file_customer_id)) {
            fprintf(temp, "%d %s %d %s %.2f\n", from_file_customer_id, name, age, address, to_balance);
        } else {
            fprintf(temp, "%d %s %d %s %.2f\n", from_file_customer_id, name, age, address, balance);
        }
    }

    fclose(file);
    fclose(temp);

    if (found_from && found_to) {
        // Replace the original file with the updated one
        remove("customerdetails.txt");
        rename("temp.txt", "customerdetails.txt");
        
        // Log transactions for both sender and receiver
        log_transaction_server(from_customer_id, "Transfer Out", transfer_amount, from_new_balance);
        log_transaction_server(to_customer_id, "Transfer In", transfer_amount, to_new_balance);
        
        // Send success message to client
        char success_msg[256];
        sprintf(success_msg, "Transfer successful! Your new balance: $%.2f", from_new_balance);
        send(sock, success_msg, strlen(success_msg), 0);
    } else if (!found_from) {
        // Sender customer ID not found
        char *error_msg = "Sender customer not found!";
        send(sock, error_msg, strlen(error_msg), 0);
    } else if (!found_to) {
        // Receiver customer ID not found
        char *error_msg = "Receiver customer not found!";
        send(sock, error_msg, strlen(error_msg), 0);
    }

    // Release the file lock
    flock(fd, LOCK_UN);
}
//make a function so that the receipient can see their transferred funds
//client side transfer funds
void transfer_funds_client(int sock,int from_customer_id) {
    int  to_customer_id;
    float transfer_amount;
    char buffer[1024] = {0};

    printf("Enter the recipient's Customer ID: ");
    scanf("%d", &to_customer_id);
    printf("Enter the transfer amount: ");
    scanf("%f", &transfer_amount);

    // Send from customer ID, to customer ID, and transfer amount to the server
   // send(sock, &from_customer_id, sizeof(from_customer_id), 0);
    send(sock, &to_customer_id, sizeof(to_customer_id), 0);
    send(sock, &transfer_amount, sizeof(transfer_amount), 0);

    // Receive and print the server response
    read(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}
//server side loan application 
void apply_loan_server(int sock,int customer_id) {
    char loan_type[50];
    float loan_amount;
    char buffer[1024] = {0};

    // Receive loan details from client (Customer)
   // recv(sock, &customer_id, sizeof(customer_id), 0);
    recv(sock, loan_type, sizeof(loan_type), 0);
    recv(sock, &loan_amount, sizeof(loan_amount), 0);
    
    // Write loan request to the loan application file
    FILE *file = fopen("loan_applications.txt", "a");
    if (file == NULL) {
        perror("Could not open loan application file");
        return;
    }

    int fd = fileno(file);
    if (flock(fd, LOCK_EX) == -1) {
        perror("Could not lock file");
        fclose(file);
        return;
    }

    // Save the loan request (customer_id, loan_type, loan_amount)
    fprintf(file, "%d %s %.2f\n", customer_id, loan_type, loan_amount);
    fclose(file);

    // Unlock the file
    flock(fd, LOCK_UN);

    // Confirm loan application submission to customer
    char *msg = "Loan application submitted successfully!";
    send(sock, msg, strlen(msg), 0);
}
//client side application for loan
void customer_apply_for_loan_client(int sock,int customer_id) {
    char loan_type[50];
    float loan_amount;
    char buffer[1024] = {0};

    // Input loan details
   
    printf("Enter Loan Type: ");
    scanf("%s", loan_type);
    printf("Enter Loan Amount: ");
    scanf("%f", &loan_amount);

    // Send loan details to the server
   // send(sock, &customer_id, sizeof(customer_id), 0);
    send(sock, loan_type, sizeof(loan_type), 0);
    send(sock, &loan_amount, sizeof(loan_amount), 0);

    // Receive confirmation from server
    read(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}

// Function to handle feedback from customer server side
void receive_feedback_server(int sock,int customer_id) {
    char feedback[1024] = {0};
    int fd;
    
    // Receive customer ID
    //recv(sock, &customer_id, sizeof(customer_id), 0);

    // Receive feedback
    recv(sock, feedback, sizeof(feedback), 0);

    // Open feedback file for appending
    FILE *file = fopen("feedback.txt", "a");
    if (file == NULL) {
        perror("Could not open feedback file");
        return;
    }

    // Get file descriptor for locking
    fd = fileno(file);
    
    // Lock the file to prevent concurrent writes
    if (flock(fd, LOCK_EX) == -1) {
        perror("Could not lock feedback file");
        fclose(file);
        return;
    }

    // Write the feedback to the file
    fprintf(file, "Customer ID: %d\nFeedback: %s\n\n", customer_id, feedback);

    // Unlock the file and close
    flock(fd, LOCK_UN);
    fclose(file);

    // Send confirmation back to client
    char *msg = "Thank you for your feedback!";
    send(sock, msg, strlen(msg), 0);
}

// Function for customer to send feedback - client side
void send_feedback_client(int sock,int customer_id) {
    char feedback[1024];
    char buffer[1024] = {0};

    // Input feedback
    printf("Enter your feedback: ");
    getchar(); // clear the buffer from previous input
    fgets(feedback, sizeof(feedback), stdin);
    feedback[strcspn(feedback, "\n")] = 0;  // Remove trailing newline

    // Send customer ID and feedback to server
    //send(sock, &customer_id, sizeof(customer_id), 0);
    send(sock, feedback, sizeof(feedback), 0);

    // Receive confirmation from server
    read(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}
//server side transaction h

//custmer options displayed client side
void customer_menu(int sock, int id) {
    int choice;
    do {
        printf("---------CUSTOMER MENU-----------\n");
        printf("1. View Account Balance\n");
        printf("2. Deposit Money\n");
        printf("3. Withdraw Money\n");
        printf("4. Transfer Funds\n");
        printf("5. Apply for Loan\n");
        printf("6. Add Feedback\n");
        printf("7. View Transaction History\n");
        printf("8. Change Password\n");
        printf("9. Logout\n");
        printf("10. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        // Send choice to server
        send(sock, &choice, sizeof(choice), 0);

        switch (choice) {
            case 1:
                view_account_balance_client(sock,id);  // Option to view balance
                break;
            case 2:
                deposit_money_client(sock,id);  // Option to deposit money
                break;
            case 3:
                withdraw_money_client(sock,id);  // Option to withdraw money
                break;
            case 4:
                transfer_funds_client(sock,id);  // Option to transfer funds
                break;
            case 5:
                customer_apply_for_loan_client(sock,id);  // Option to apply for loan
                break;
            case 6:
                send_feedback_client(sock,id);  // Option to add feedback
                break;
            case 7:
                send_transaction_history_client(sock,id);  // Option to view transaction history
                break;
            case 8: 
                change_password_client(sock);  // Option to change password
                break;
            case 9:
                printf("Logging out...\n");
                return;
            case 10:
                exit(0);
            default:
                printf("Invalid choice, please try again.\n");
        }
    } while (choice != 10);
}
// customer menu option for the server
void handle_customer_request(int sock,int id) {
    int choice;
    while (recv(sock, &choice, sizeof(choice), 0) > 0) {
        switch (choice) {
            case 1:
                view_account_balance_server(sock,id);  // Handle view balance request
                break;
            case 2:
                deposit_money_server(sock,id);  // Handle deposit request
                break;
            case 3:
                withdraw_money_server(sock,id);  // Handle withdraw request
                break;
            case 4:
                transfer_funds_server(sock,id);  // Handle transfer funds request
                break;
            case 5:
                apply_loan_server(sock,id);  // Handle loan application request
                break;
            case 6:
                receive_feedback_server(sock,id);  // Handle feedback submission
                break;
            case 7:
                send_transaction_history_server(sock,id);  // Handle transaction history request
                break;
            case 8: 
                change_password_server(sock);  // Handle change password request
                break;
            case 9:
                printf("Customer logged out.\n");
                return;  // Logout, exit the loop
            case 10:
                printf("Exiting\n");
                break;
            default:
                printf("Invalid choice received from client.\n");
        }
    }
}
#endif
