#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

int NUMBER_OF_CUSTOMERS = 0;
int NUMBER_OF_RESOURCES = 0;

int request_resources(int customer_num, 
                        int request[], 
                        FILE *fptr, 
                        int available[], 
                        int allocation[][NUMBER_OF_RESOURCES], 
                        int need[][NUMBER_OF_RESOURCES]);

void release_resources(int customer_num, 
                        int release[], 
                        FILE *fptr, 
                        int available[], 
                        int allocation[][NUMBER_OF_RESOURCES], 
                        int need[][NUMBER_OF_RESOURCES]);

bool is_safe(int available[], 
            int allocation[][NUMBER_OF_RESOURCES], 
            int need[][NUMBER_OF_RESOURCES]);

void print_state(FILE *fptr, 
                    int available[], 
                    int allocation[][NUMBER_OF_RESOURCES], 
                    int need[][NUMBER_OF_RESOURCES], 
                    int maximum[][NUMBER_OF_RESOURCES]);


int main(int argc, char *argv[]) {
    NUMBER_OF_RESOURCES = argc - 1;

    // count customer
    FILE *customerFile = fopen("customer.txt", "r");
    if (customerFile == NULL) {
        fprintf(stderr, "Fail to read customer.txt\n");
        return 1;
    }
    char line2[2000];
    while (fgets(line2, sizeof(line2), customerFile)) {
        NUMBER_OF_CUSTOMERS++;
    }
    fclose(customerFile);

    // matrices
    int available[NUMBER_OF_RESOURCES]; 
    int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES]; 
    int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES]; 
    int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES]; 

    // available array
    for (int i=1; i<=NUMBER_OF_RESOURCES; i++) {
        available[i-1] = atoi(argv[i]);
    }

    // read customer.txt
    FILE *file=fopen("customer.txt", "r");
    if (file==NULL) {
        fprintf(stderr, "Fail to read customer.txt\n");
        return 1;
    }

    char line[2000];
    int customerCount = 0;
    while (fgets(line, sizeof(line), file) && customerCount < NUMBER_OF_CUSTOMERS) {
        char *token = strtok(line, ",");
        int resourceCount = 0; 
        while (token != NULL && resourceCount < NUMBER_OF_RESOURCES) {
            maximum[customerCount][resourceCount] = atoi(token);
            token = strtok(NULL, ",");
            resourceCount++;
        }
        if (resourceCount != NUMBER_OF_RESOURCES) {
            fprintf(stderr, "Incompatibility between customer.txt and command line\n");
            fclose(file);
            return 1;
        }
        customerCount++;
    }
    fclose(file);

    // need and allocation arrays
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            need[i][j] = maximum[i][j];
            allocation[i][j] = 0;
        }
    }

    // validate commands.txt
    FILE *cmdFile = fopen("commands.txt", "r");
    if (cmdFile == NULL) {
        fprintf(stderr, "Fail to read commands.txt\n");
        return 1;
    }

    char commandLine[2000];
    char command[10];
    int customer_num, resources[NUMBER_OF_RESOURCES];

    while (fgets(commandLine, sizeof(commandLine), cmdFile)) {

        if (strcmp(commandLine, "*")==0 || strcmp(commandLine, "*\n")==0 || strcmp(commandLine, "*\r\n")==0) {
            continue;
        }
        int numTokens = sscanf(commandLine, "%s %d", command, &customer_num);
        if (numTokens < 2 || (strcmp(command, "RQ") != 0 && strcmp(command, "RL") != 0)) {
            fprintf(stderr, "Incompatibility between commands.txt and command line\n");
            fclose(cmdFile);
            return 1;
        }

        if (customer_num<0 || customer_num>=NUMBER_OF_CUSTOMERS) {
            fprintf(stderr, "Incompatibility between commands.txt and command line\n");
            fclose(cmdFile);
            return 1;
        }

        if (strcmp(command, "RQ") == 0 || strcmp(command, "RL") == 0) {
            char *token = strtok(commandLine, " ");
            token = strtok(NULL, " ");

            int i = 0;
            char *lastToken = NULL;
            while ((token = strtok(NULL, " ")) != NULL) {
                if (i < NUMBER_OF_RESOURCES) {
                    resources[i++] = atoi(token);
                } else {
                    lastToken = token;
                }
            }

            if (i != NUMBER_OF_RESOURCES) {
                fprintf(stderr, "Incompatibility between commands.txt and command line\n");
                fclose(cmdFile);
                return 1;
            }
            if (lastToken != NULL) {
                fprintf(stderr, "Incompatibility between commands.txt and command line\n");
                fclose(cmdFile);
                return 1;
            }
        }
    }
    fclose(cmdFile);

    // open result.txt
    FILE *resultFile = fopen("result.txt", "w");
    if (resultFile == NULL) {
        perror("Fail to open file 'result.txt'");
        return 1;
    }

    
    // commands
    cmdFile = fopen("commands.txt", "r");
    if (cmdFile == NULL) {
        fprintf(stderr, "Fail to read commands.txt\n");
        fclose(resultFile);
        remove("result.txt");
        return 1;
    }

    while (fgets(commandLine, sizeof(commandLine), cmdFile)) {
        sscanf(commandLine, "%s %d", command, &customer_num);

        if (strcmp(command, "RQ") == 0 || strcmp(command, "RL") == 0) {
            char *token = strtok(commandLine, " ");
            token = strtok(NULL, " "); 
            int i = 0;
            while ((token = strtok(NULL, " ")) != NULL && i < NUMBER_OF_RESOURCES) {
                resources[i++] = atoi(token);
            }
            if (i!=NUMBER_OF_RESOURCES) {
                fprintf(stderr, "Incompatibility between commands.txt and command line\n");
                fclose(cmdFile);
                fclose(resultFile);
                remove("result.txt"); 
                return 1;
            }
            if (strcmp(command, "RQ") == 0) {
                request_resources(customer_num, resources, resultFile, available, allocation, need);
            } else if (strcmp(command, "RL") == 0) {
                release_resources(customer_num, resources, resultFile, available, allocation, need);
            }
        } else if (strcmp(command, "*") == 0) {
            print_state(resultFile, available, allocation, need, maximum);
        } else {
            fprintf(stderr, "[ERROR] invalid command\n");
            fclose(cmdFile);
            fclose(resultFile);
            remove("result.txt");
            return 1;
        }
    }
    fclose(cmdFile);
    fclose(resultFile);
    return 0;
}

bool is_safe(int available[], 
                int allocation[][NUMBER_OF_RESOURCES], 
                int need[][NUMBER_OF_RESOURCES]) {

    // work && finish
    int work[NUMBER_OF_RESOURCES];
    bool finish[NUMBER_OF_CUSTOMERS];
    for (int i=0; i<NUMBER_OF_CUSTOMERS; i++) {
        finish[i] = false;
    }

    // work == available
    for (int i=0; i<NUMBER_OF_RESOURCES; i++) {
        work[i] = available[i];
    }

    // finish[i] == false && need[i] <= work
    while (true) {
        bool found = false;
        for (int i=0; i<NUMBER_OF_CUSTOMERS; i++) {
            if (!finish[i]) {
                bool allocate = true;
                for (int j=0; j<NUMBER_OF_RESOURCES; j++) {
                    if (need[i][j]>work[j]) {
                        allocate = false;
                        break;
                    }
                }
                if (allocate) {
                    for (int j=0; j<NUMBER_OF_RESOURCES; j++) {
                        work[j] += allocation[i][j];
                    }
                    finish[i] = true;
                    found = true;
                }
            }
        }
        if (!found) {
            break;
        }
    }

    // finish[i] == true
    for (int i=0; i<NUMBER_OF_CUSTOMERS; i++) {
        if (!finish[i]) {
            return false;
        }
    }
    return true;
}



int request_resources(int customer_num, 
                        int request[], 
                        FILE *fptr, 
                        int available[], 
                        int allocation[][NUMBER_OF_RESOURCES], 
                        int need[][NUMBER_OF_RESOURCES]) {

    // request <= need
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (request[i] > need[customer_num][i]) {
            fprintf(fptr, "The customer %d request ", customer_num);
            for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                fprintf(fptr, "%d ", request[j]);
            }
            fprintf(fptr, "was denied because exceed its maximum need\n");
            return -1;
        }
    }

    // request <= available
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (request[i] > available[i]) {
            fprintf(fptr, "The resources ");
            for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                fprintf(fptr, "%d ", available[j]);
            }
            fprintf(fptr, "are not enough to customer %d request ", customer_num);
            for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                fprintf(fptr, "%d ", request[j]);
            }
            fprintf(fptr, "\n");
            return -1;
        }
    }

    // pretend to have allocated the requested resources
    for (int i=0; i<NUMBER_OF_RESOURCES; i++) {
        available[i] -= request[i];
        allocation[customer_num][i] += request[i];
        need[customer_num][i] -= request[i];
    }

    // state is safe?
    if (is_safe(available, allocation, need)) {
        // safe
        fprintf(fptr, "Allocate to customer %d the resources ", customer_num);
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            fprintf(fptr, "%d ", request[i]);
        }
        fprintf(fptr, "\n");
        return 0;
    } else {
        // not safe - restore
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            available[i] += request[i];
            allocation[customer_num][i] -= request[i];
            need[customer_num][i] += request[i];
        }

        fprintf(fptr, "The customer %d request ", customer_num);
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            fprintf(fptr, "%d ", request[j]);
        }
        fprintf(fptr, "was denied because result in an unsafe state\n");
        return -1;
    }
}

void release_resources(int customer_num, 
                        int release[], 
                        FILE *fptr, 
                        int available[], 
                        int allocation[][NUMBER_OF_RESOURCES], 
                        int need[][NUMBER_OF_RESOURCES]) {
    // release <= allocation
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (release[i] > allocation[customer_num][i]) {
            fprintf(fptr, "The customer %d released ", customer_num);
            for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                fprintf(fptr, "%d ", release[j]);
            }
            fprintf(fptr, "was denied because exceed its maximum allocation\n");
            return;
        }
    }

    // update state
    for (int i=0; i<NUMBER_OF_RESOURCES; i++) {
        available[i] += release[i];
        allocation[customer_num][i] -= release[i];
        need[customer_num][i] += release[i];
    }

    fprintf(fptr, "Release from customer %d the resources ", customer_num);
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        fprintf(fptr, "%d ", release[i]);
    }
    fprintf(fptr, "\n");
}

void max_digits(int matrix[][NUMBER_OF_RESOURCES], int max_digits_por_col[]) {
    for (int j=0; j<NUMBER_OF_RESOURCES; j++) {
        max_digits_por_col[j] = 0;
        for (int i=0;i<NUMBER_OF_CUSTOMERS; i++) {
            int digits = matrix[i][j] > 0 ? (int)log10(matrix[i][j]) + 1 : 1;
            if (digits > max_digits_por_col[j]) {
                max_digits_por_col[j] = digits;
            }
        }
    }
}

void print_state(FILE *fptr, 
                 int available[], 
                 int allocation[][NUMBER_OF_RESOURCES], 
                 int need[][NUMBER_OF_RESOURCES], 
                 int maximum[][NUMBER_OF_RESOURCES]) {

    int max_digits_maximum[NUMBER_OF_RESOURCES];
    int max_digits_allocation[NUMBER_OF_RESOURCES];
    int max_digits_need[NUMBER_OF_RESOURCES];

    max_digits(maximum, max_digits_maximum);
    max_digits(allocation, max_digits_allocation);
    max_digits(need, max_digits_need);

    //headers

    int total_spaces_maximum = 0, total_spaces_allocation = 0;
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        total_spaces_maximum += max_digits_maximum[i] + 1;
        total_spaces_allocation += max_digits_allocation[i] + 1;
    }
    total_spaces_maximum--;
    total_spaces_allocation--;

    int adj_total_spaces_maximum = (total_spaces_maximum > strlen("MAXIMUM") ? total_spaces_maximum : strlen("MAXIMUM")) + 1;
    int adj_total_spaces_allocation = (total_spaces_allocation > strlen("ALLOCATION") ? total_spaces_allocation : strlen("ALLOCATION")) + 1;

    fprintf(fptr, "MAXIMUM");
    for (int i = strlen("MAXIMUM"); i < total_spaces_maximum; i++) fprintf(fptr, " ");
    fprintf(fptr, " | ");

    fprintf(fptr, "ALLOCATION");
    for (int i = strlen("ALLOCATION"); i < total_spaces_allocation; i++) fprintf(fptr, " ");
    fprintf(fptr, " | NEED\n");

    //matrices

    for (int i=0; i<NUMBER_OF_CUSTOMERS; i++) {
        // maximum
        int current_space = 0;
        for (int j=0; j<NUMBER_OF_RESOURCES; j++) {
            fprintf(fptr, "%*d ", max_digits_maximum[j], maximum[i][j]);
            current_space += max_digits_maximum[j] + 1;
        }
        for (int j=current_space; j<adj_total_spaces_maximum; j++) fprintf(fptr, " ");
        fprintf(fptr, "| ");

        // allocation
        current_space = 0;
        for (int j=0; j<NUMBER_OF_RESOURCES; j++) {
            fprintf(fptr, "%*d ", max_digits_allocation[j], allocation[i][j]);
            current_space += max_digits_allocation[j] + 1;
        }
        for (int j = current_space; j<adj_total_spaces_allocation; j++) fprintf(fptr, " ");
        fprintf(fptr, "| ");

        // need
        for (int j=0; j<NUMBER_OF_RESOURCES; j++) {
            fprintf(fptr, "%*d ", max_digits_need[j], need[i][j]);
        }
        fprintf(fptr, "\n");
    }

    // available footer
    fprintf(fptr, "AVAILABLE ");
    for (int i=0; i<NUMBER_OF_RESOURCES; i++) {
        fprintf(fptr, "%d ", available[i]);
    }
    fprintf(fptr, "\n");
}
