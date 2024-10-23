/*4.	Administrator: The Administrator has the highest level of access and manages both customer and employee accounts.
o	Login System (one session per user)
o	Add New Bank Employee
o	Modify Customer/Employee Details
o	Manage User Roles
o	Change Password
o	Logout
o	Exit
*/
//adding guards
#ifndef ADMIN_H
#define ADMIN_H
//adding new employee 
#define MAX 1024
#include <stdlib.h>
#include "login.h" //need this as well

// this function invoked by client to get employee details
void add_new_employee_client(int sock)
{
  //stuff entered in the client side must be sent to the server side
  char ename[100];
  int eage,empid; 
  float esal;
  printf ("--------------NEW EMPLOYEE DETAILS----------------\n");
  printf("Enter employee id : ");
  scanf("%d",&empid);
  printf ("Enter employee name : ");
  scanf("%s",ename);
  printf("Enter employee age : ");
  scanf("%d",&eage);
  printf("Enter employee salary : ");
  scanf("%f",&esal);
  
  //send details to server
  send(sock, &empid, sizeof(empid),0);
  send(sock, ename, sizeof(ename), 0);
  send(sock, &eage, sizeof(eage),0);
  send(sock, &esal, sizeof(esal),0);
  
  //receive acknowledgement from server 
  char buffer[MAX] = {0};
  read (sock,buffer,sizeof(buffer));
  printf("%s",buffer); //response that details have been entered into file 
}

//server function that enters details in the employee file
void handle_new_employee_server(int sock)
{
  char name[100];
  int age,id;
  float sal;
  
  //receive details from the client
  recv(sock, &id,sizeof(id),0);
  recv(sock, name,sizeof(name),0);
  recv(sock, &age,sizeof(age),0);
  recv(sock, &sal,sizeof(sal),0);
  
  //append the employee details to employeedetails.txt
  FILE *file = fopen("employeedetails.txt","a");
  if (file == NULL) {
        perror("Could not open employee details file");
        char *error_msg = "Failed to add new employee!";
        send(sock, error_msg, strlen(error_msg), 0);
        return;
    }
    fprintf(file, "%d %s %d %.2f\n",id, name, age, sal);
    fclose(file);

    // Send confirmation back to the client
    char *success_msg = "New employee added successfully!";
    send(sock, success_msg, strlen(success_msg), 0);
}
//adding new manager
void add_new_manager_client(int sock)
{
  //stuff entered in the client side must be sent to the server side
  char ename[100];
  int eage,empid; 
  float esal;
  printf ("--------------NEW MANAGER DETAILS----------------\n");
  printf("Enter manager id : ");
  scanf("%d",&empid);
  printf ("Enter manager name : ");
  scanf("%s",ename);
  printf("Enter manager age : ");
  scanf("%d",&eage);
  printf("Enter manager salary : ");
  scanf("%f",&esal);
  
  //send details to server
  send(sock, &empid, sizeof(empid),0);
  send(sock, ename, sizeof(ename), 0);
  send(sock, &eage, sizeof(eage),0);
  send(sock, &esal, sizeof(esal),0);
  
  //receive acknowledgement from server 
  char buffer[MAX] = {0};
  read (sock,buffer,sizeof(buffer));
  printf("%s",buffer); //response that details have been entered into file 
}
//manager server handling
void handle_new_manager_server(int sock)
{
  char name[100];
  int age,id;
  float sal;
  
  //receive details from the client
  recv(sock, &id,sizeof(id),0);
  recv(sock, name,sizeof(name),0);
  recv(sock, &age,sizeof(age),0);
  recv(sock, &sal,sizeof(sal),0);
  
  FILE *file = fopen("managerdetails.txt","a");
  if (file == NULL) {
        perror("Could not open manager details file");
        char *error_msg = "Failed to add new manager!";
        send(sock, error_msg, strlen(error_msg), 0);
        return;
    }
    fprintf(file, "%d %s %d %.2f\n",id, name, age, sal);
    fclose(file);

    // Send confirmation back to the client
    char *success_msg = "New manager added successfully!";
    send(sock, success_msg, strlen(success_msg), 0);
}
//modifying customer details
void modify_customer_details_client_admin(int sock) {
    char new_name[50], new_address[100];
    int customer_id,new_age;
    float new_balance;
    char buffer[1024] = {0};

    // Request customer ID from admin
    printf("Enter the customer ID you want to modify: ");
    scanf("%d", &customer_id);

    // Send customer ID to server
    send(sock, &customer_id, sizeof(customer_id), 0);

    // Receive and display the current customer details from the server
    read(sock, buffer, sizeof(buffer));
    
    // Check if the customer ID was found
    if (strstr(buffer, "Customer ID not found!") != NULL) {
        // If the customer ID was not found, return from the function
        return;
    }
    
    printf("Current details of the customer:\n%s\n", buffer);
    

    // Now enter the new details
    printf("Enter new name: ");
    scanf("%s", new_name);
    printf("Enter new age: ");
    scanf("%d", &new_age);
    printf("Enter new address: ");
    scanf("%s", new_address);
    printf("Enter new balance: ");
    scanf("%f", &new_balance);

    // Send the updated details to the server
    send(sock, new_name, sizeof(new_name), 0);
    send(sock, &new_age, sizeof(new_age), 0);
    send(sock, new_address, sizeof(new_address), 0);
    send(sock, &new_balance, sizeof(new_balance), 0);

    // Receive confirmation from the server
    read(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}
//server code to modify customer file
void modify_customer_in_file_server_admin(int sock) {
    char name[50], address[100], new_name[50], new_address[100];
    int customer_id,current_id,age, new_age;
    float balance, new_balance;
    char line[256], buffer[1024] = {0};
    int found = 0;

    // Receive the customer ID from the client
    recv(sock, &customer_id, sizeof(customer_id), 0);

    // Open the customer details file
    FILE *file = fopen("customerdetails.txt", "r");
    if (file == NULL) {
        perror("Could not open customer details file");
        return;
    }

    // Open a temporary file for writing the modified data
    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        perror("Could not open temp file");
        fclose(file);  // Close the original file before returning
        return;
    }

    // Search for the customer by ID
    while (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "%d %s %d %s %f", &current_id, name, &age, address, &balance);

        // If this is the customer to be modified
        if (current_id == customer_id) {
            found = 1;

            // Send current customer details to the client
            sprintf(buffer, "Customer ID: %d\nName: %s\nAge: %d\nAddress: %s\nBalance: %.2f\n", current_id, name, age, address, balance);
            send(sock, buffer, strlen(buffer), 0);

            // Receive the new details (or unchanged if admin doesn't modify them)
            recv(sock, new_name, sizeof(new_name), 0);
            recv(sock, &new_age, sizeof(new_age), 0);
            recv(sock, new_address, sizeof(new_address), 0);
            recv(sock, &new_balance, sizeof(new_balance), 0);

            // Update only if a new value was provided (otherwise keep the old one)
            if (strlen(new_name) > 0) strcpy(name, new_name);
            if (new_age > 0) age = new_age;
            if (strlen(new_address) > 0) strcpy(address, new_address);
            if (new_balance > 0) balance = new_balance;

            // Write the modified customer details to the temp file
            fprintf(temp, "%d %s %d %s %.2f\n", current_id, name, age, address, balance);
        } else {
            // Write unmodified customer details to the temp file
            fprintf(temp, "%s", line);
        }
    }

    fclose(file);
    fclose(temp);

    // Replace the old file with the new one
    if (found) {
        remove("customerdetails.txt");
        rename("temp.txt", "customerdetails.txt");

        char success_msg[] = "Customer details updated successfully!";
        send(sock, success_msg, strlen(success_msg), 0);
    } else {
        // Remove temp file if no customer was found
        remove("temp.txt");
        char error_msg[] = "Customer ID not found!";
        send(sock, error_msg, strlen(error_msg), 0);
    }
}
//client modification of employee details
void modify_employee_details_client(int sock) {
    char ename[50];
    int eage,empid;
    float sal;
    char buffer[MAX] = {0};

    // Request employee ID from admin which has to be modified
    printf("Enter employee id to be modified: ");
    scanf("%d",&empid);

    // Send employee ID to server
    send(sock, &empid, sizeof(empid), 0);

    // Receive and display the current employee details from the server
    read(sock, buffer, sizeof(buffer));
    printf("Current details of the employee :\n%s\n", buffer);

    // Now enter the new details ( modifying all details here)
    printf("Enter new name: ");
    scanf("%s", ename);
    printf("Enter new age: ");
    scanf("%d", &eage);
    printf("Enter new salary: ");
    scanf("%f", &sal);

    // Send the updated details to the server
    send(sock, &empid, sizeof(empid), 0);
    send(sock, ename, strlen(ename), 0);
    send(sock, &eage, sizeof(eage), 0);
    send(sock, &sal, sizeof(sal),0 );

    // Receive confirmation from the server
    read(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}
//server modification of employee details
void modify_employee_in_file_server(int sock) {
    char ename[50], new_ename[50];
    int eage, new_eage,id, new_id;
    float sal, new_sal;
    char line[256], buffer[1024] = {0};
    int found = 0;

    // Receive the employee ID from the client
    recv(sock, &new_id, sizeof(new_id), 0);

    // Open the customer details file
    FILE *file = fopen("employeedetails.txt", "r");
    if (file == NULL) {
        perror("Could not open employee details file");
        return;
    }

    // Open a temporary file for writing the modified data
    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        perror("Could not open temp file");
        fclose(file);  // Close the original file before returning
        return;
    }
    // Search for the employee by ID
    while (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "%d %s %d %f", &id, ename, &eage, &sal);

        // If this is the customer to be modified
        if (id == new_id) {
            found = 1;

            // Send current customer details to the client
            sprintf(buffer, "Employee ID: %d\nName: %s\nAge: %d\nSalary: %.2f\n", id, ename, eage,sal);
            send(sock, buffer, strlen(buffer), 0);

            recv(sock, new_ename, strlen(new_ename), 0);
            recv(sock, &new_eage, sizeof(new_eage), 0);
            recv(sock, &new_sal, sizeof(new_sal), 0);

            if (strlen(new_ename) > 0) strcpy(ename, new_ename);
            if (new_eage > 0) eage = new_eage;
            if (new_sal > 0) sal = new_sal;

            fprintf(temp,"%d %s %d %.2f\n", id, ename, eage, sal);
        } else {
            // Write unmodified customer details to the temp file
            fprintf(temp, "%s", line);
        }
    }

    fclose(file);
    fclose(temp);

    // Replace the old file with the new one
    if (found) {
        remove("employeedetails.txt");
        rename("temp.txt", "employeedetails.txt");

        char success_msg[] = "Employee details updated successfully!";
        send(sock, success_msg, strlen(success_msg), 0);
    } else {
        // Remove temp file if no customer was found
        remove("temp.txt");
        char error_msg[] = "Employee ID not found!";
        send(sock, error_msg, strlen(error_msg), 0);
    }
}
//modify manager details client
void modify_manager_details_client(int sock) {
    char ename[50];
    int eage,empid;
    float sal;
    char buffer[MAX] = {0};

    printf("Enter manager id to be modified: ");
    scanf("%d",&empid);

    // Send employee ID to server
    send(sock, &empid, sizeof(empid), 0);

    // Receive and display the current employee details from the server
    read(sock, buffer, sizeof(buffer));
    printf("Current details of the manager :\n%s\n", buffer);

    // Now enter the new details ( modifying all details here)
    printf("Enter new name: ");
    scanf("%s", ename);
    printf("Enter new age: ");
    scanf("%d", &eage);
    printf("Enter new salary: ");
    scanf("%f", &sal);

    // Send the updated details to the server
    send(sock, &empid, sizeof(empid), 0);
    send(sock, ename, strlen(ename), 0);
    send(sock, &eage, sizeof(eage), 0);
    send(sock, &sal, sizeof(sal),0 );

    // Receive confirmation from the server
    read(sock, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}
//modify manageer server
void modify_manager_in_file_server(int sock) {
    char ename[50], new_ename[50];
    int eage, new_eage,id, new_id;
    float sal, new_sal;
    char line[256], buffer[1024] = {0};
    int found = 0;

    // Receive the employee ID from the client
    recv(sock, &new_id, sizeof(new_id), 0);

    // Open the customer details file
    FILE *file = fopen("managerdetails.txt", "r");
    if (file == NULL) {
        perror("Could not open manager details file");
        return;
    }

    // Open a temporary file for writing the modified data
    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        perror("Could not open temp file");
        fclose(file);  // Close the original file before returning
        return;
    }
    // Search for the employee by ID
    while (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "%d %s %d %f", &id, ename, &eage, &sal);

        // If this is the customer to be modified
        if (id == new_id) {
            found = 1;

            // Send current customer details to the client
            sprintf(buffer, "Manager ID: %d\nName: %s\nAge: %d\nSalary: %.2f\n", id, ename, eage,sal);
            send(sock, buffer, strlen(buffer), 0);

            recv(sock, new_ename, strlen(new_ename), 0);
            recv(sock, &new_eage, sizeof(new_eage), 0);
            recv(sock, &new_sal, sizeof(new_sal), 0);

            if (strlen(new_ename) > 0) strcpy(ename, new_ename);
            if (new_eage > 0) eage = new_eage;
            if (new_sal > 0) sal = new_sal;

            fprintf(temp,"%d %s %d %.2f\n", id, ename, eage, sal);
        } else {
            // Write unmodified customer details to the temp file
            fprintf(temp, "%s", line);
        }
    }

    fclose(file);
    fclose(temp);

    // Replace the old file with the new one
    if (found) {
        remove("managerdetails.txt");
        rename("temp.txt", "managerdetails.txt");

        char success_msg[] = "Manager details updated successfully!";
        send(sock, success_msg, strlen(success_msg), 0);
    } else {
        // Remove temp file if no customer was found
        remove("temp.txt");
        char error_msg[] = "Manager ID not found!";
        send(sock, error_msg, strlen(error_msg), 0);
    }
}
//manage user roles on client side
void manage_user_roles_client(int sock) {
    char new_role[10];
    char buffer[1024] = {0};
    int user_id;

    // Input username and new role
    printf("Enter user id: ");
    scanf("%d", &user_id);
    printf("Enter new role (ct/emp/man/ad): ");  
    scanf("%s", new_role);

    // Send username and new role to the server
    send(sock, &user_id, sizeof(user_id), 0);
    send(sock, new_role, sizeof(new_role), 0);

    // Receive response from the server
    read(sock, buffer, sizeof(buffer));
    printf("Server response: %s\n", buffer);
}
// manage user roles on server side
void manage_user_roles_server(int sock) {
    char  new_role[10], line[256];
    char stored_username[50], stored_password[50], stored_role[10];
    int id,stored_id;
    int found = 0;

    // Receive username and new role from the client (admin)
    recv(sock, &id, sizeof(id), 0);
    recv(sock, new_role, sizeof(new_role), 0);

    // Open the user details file and a temporary file to update the roles
    FILE *file = fopen("userdetails.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    if (file == NULL || temp == NULL) {
        perror("Error opening file");
        return;
    }

    // Search for the username and modify the role
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s %s %s %d", stored_role, stored_username, stored_password, &stored_id);

        if (id == stored_id) {
            found = 1;
            // Update the role in the temporary file
            fprintf(temp, "%s %s %s %d\n", new_role, stored_username, stored_password, id);
        } else {
            // Copy other users' details unchanged
            fputs(line, temp);
        }
    }

    fclose(file);
    fclose(temp);

    // Check if the username was found and update the original file
    if (found) {
        // Replace original user details file with the updated one
        remove("userdetails.txt");
        rename("temp.txt", "userdetails.txt");

        // Send success message to the client (admin)
        char success_msg[256];
        snprintf(success_msg, sizeof(success_msg), "Role for user '%d' updated to '%s' successfully.", id, new_role);
        send(sock, success_msg, strlen(success_msg), 0);
    } else {
        // If username not found, remove temp file and send an error message
        remove("temp.txt");
        char *error_msg = "Username not found!";
        send(sock, error_msg, strlen(error_msg), 0);
    }
}
//employee options displayed client side
void admin_menu(int sock) {
    int choice;
    do {
        printf("\n--------ADMIN MENU-----------\n");
        printf("1. Add new employee\n");
        printf("2. Add new manager\n");
        printf("3. Modify Customer Details\n");
        printf("4. Modify Employee Details\n");
        printf("5. Modify Manager Details\n");
        printf("6. Manager user roles\n");
        printf("7. Change Password\n");
        printf("8. Logout\n");
        printf("9. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        // Send choice to server
        send(sock, &choice, sizeof(choice), 0);

        switch (choice) {
            case 1:
                add_new_employee_client(sock);
                break;
            case 2:
                add_new_manager_client(sock);
                break;
            case 3:
                modify_customer_details_client_admin(sock);
                break;
            case 4:
                modify_employee_details_client(sock);  
                break;
            case 5:
                modify_manager_details_client(sock);
                break;
            case 6:
                manage_user_roles_client(sock);
                break;
            case 7: 
                change_password_client(sock);  // Option to change password
                break;
            case 8:
                printf("Logging out...\n");
                return;
            case 9:
                    exit(0);
            default:
                printf("Invalid choice, please try again.\n");
        }
    } while (choice != 9);
}
// admin menu option for the server
void handle_admin_request(int sock) {
    int choice;
    while (recv(sock, &choice, sizeof(choice), 0) > 0) {
        switch (choice) {
            case 1:
                    handle_new_employee_server(sock);
                    break;
            case 2:
                    handle_new_manager_server(sock);
                    break;
            case 3:
                    modify_customer_in_file_server_admin(sock);
                    break;
            case 4:
                    modify_employee_in_file_server(sock);  
                    break;
            case 5:
                    modify_manager_in_file_server(sock);
                    break;
            case 6:
                manage_user_roles_server(sock);
                break;
            case 7: 
                change_password_server(sock);  // Handle change password request
                break;
            case 8:
                printf("Customer logged out.\n");
                return;// Logout, exit the loop
              case 9 :printf("Exiting\n");
                      break;
            default:
                printf("Invalid choice received from client.\n");
        }
    }
}
#endif
