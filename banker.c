#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NUMBER_OF_CUSTOMERS 1000
#define MAX_NUMBER_OF_RESOURCES 1000

int available[MAX_NUMBER_OF_RESOURCES]; 
int maximum[MAX_NUMBER_OF_CUSTOMERS][MAX_NUMBER_OF_RESOURCES]; 
int allocation[MAX_NUMBER_OF_CUSTOMERS][MAX_NUMBER_OF_RESOURCES]; 
int need[MAX_NUMBER_OF_CUSTOMERS][MAX_NUMBER_OF_RESOURCES]; 

int NUMBER_OF_CUSTOMERS = 0;
int NUMBER_OF_RESOURCES = 0;

int request_resources(int customer_num, int request[], FILE *fptr);
void release_resources(int customer_num, int release[], FILE *fptr);
bool is_safe();
void print_state(FILE *fptr);

int main(int argc, char *argv[]) {
    NUMBER_OF_RESOURCES = argc - 1;

    // available array
    if (NUMBER_OF_RESOURCES > MAX_NUMBER_OF_RESOURCES) {
        fprintf(stderr, "[ERROR] need malloc :(\n");
        return 1;
    }
    for (int i=1; i<=NUMBER_OF_RESOURCES; i++) {
        available[i-1] = atoi(argv[i]);
    }

    // maximum array
    FILE *file = fopen("customer.txt", "r");
    if (file==NULL) {
        perror("[ERROR] to open file 'customer.txt'\n");
        return 1;
    }
    char line[100];
    while (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        for (int i=0; i<NUMBER_OF_RESOURCES && token!=NULL; i++) {
            maximum[NUMBER_OF_CUSTOMERS][i] = atoi(token);
            token = strtok(NULL, ",");
        }
        NUMBER_OF_CUSTOMERS++;
    }
    fclose(file);

    // result.txt
    FILE *resultFile = fopen("result.txt", "w");
    if (resultFile==NULL) {
        perror("[ERROR] to open file 'result.txt'");
        return 1;
    }

    // need array && allocation array
    for (int i=0; i<NUMBER_OF_CUSTOMERS; i++) {
        for (int j=0; j<NUMBER_OF_RESOURCES; j++) {
            need[i][j] = maximum[i][j];
            allocation[i][j] = 0;
        }
    }

    // commands.txt
    FILE *cmdFile = fopen("commands.txt", "r");
    if (cmdFile==NULL) {
        perror("[ERROR] to open file 'commands.txt'");
        fclose(resultFile);
        return 1;
    }

    char commandLine[1000];
    char command[10];
    int customer_num, resources[MAX_NUMBER_OF_RESOURCES];

    while (fgets(commandLine, sizeof(commandLine), cmdFile)) {
        sscanf(commandLine, "%s %d", command, &customer_num);
        
        if (strcmp(command, "RQ")==0 || strcmp(command, "RL")==0) {
            char *token = strtok(commandLine, " ");
            token = strtok(NULL, " ");

            for (int i=0; i<NUMBER_OF_RESOURCES; i++) {
                token = strtok(NULL, " ");
                resources[i] = atoi(token);
            }
            if (strcmp(command, "RQ")==0) {
                request_resources(customer_num, resources, resultFile);
            } else {
                release_resources(customer_num, resources, resultFile);
            }
        } else if (strcmp(command, "*")==0) {
            print_state(resultFile);
        } else {
            fprintf(stderr, "[ERROR] invalid command\n");
        }
    }
    fclose(cmdFile);
    fclose(resultFile);
    return 0;
}

bool is_safe() {
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
                    if (need[i][j] > work[j]) {
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



int request_resources(int customer_num, int request[], FILE *fptr) {
    // request <= need
    for (int i=0; i<NUMBER_OF_RESOURCES; i++) {
        if (request[i] > need[customer_num][i]) {
            fprintf(fptr, "The customer %d request %d %d %d was denied because exceed its maximum need\n", customer_num, request[0], request[1], request[2]);
            return -1;
        }
    }

    // request <= available
    for (int i=0; i<NUMBER_OF_RESOURCES; i++) {
        if (request[i] > available[i]) {
            fprintf(fptr, "The resources %d %d %d are not enough to costumer %d request %d %d %d \n", available[0], available[1], available[2], customer_num, request[0], request[1], request[2]);
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
    if (is_safe()) {
        // safe
        fprintf(fptr, "Allocate to customer %d the resources", customer_num);
        for (int i=0; i<NUMBER_OF_RESOURCES; i++) {
            fprintf(fptr, " %d", request[i]);
        }
        fprintf(fptr, " \n");
        return 0;
    } else {
        // not safe - restore
        for (int i=0; i<NUMBER_OF_RESOURCES; i++) {
            available[i] += request[i];
            allocation[customer_num][i] -= request[i];
            need[customer_num][i] += request[i];
        }
        fprintf(fptr, "The customer %d request %d %d %d was denied because result in an unsafe state\n", customer_num, request[0], request[1], request[2]);
        return -1;
    }
}

void release_resources(int customer_num, int release[], FILE *fptr) {
    // release <= allocation
    for (int i=0; i<NUMBER_OF_RESOURCES; i++) {
        if (release[i] > allocation[customer_num][i]) {
             fprintf(fptr, "The customer %d released %d %d %d was denied because exceed its maximum allocation\n", customer_num, release[0], release[1], release[2]);
            return;
        }
    }

    // update state
    for (int i=0; i<NUMBER_OF_RESOURCES; i++) {
        available[i] += release[i];
        allocation[customer_num][i] -= release[i];
        need[customer_num][i] += release[i];
    }

    fprintf(fptr, "Release from customer %d the resources", customer_num);
    for (int i= 0; i<NUMBER_OF_RESOURCES; i++) {
        fprintf(fptr, " %d", release[i]);
    }
    fprintf(fptr, " \n");
}

void print_state(FILE *fptr) {
    fprintf(fptr, "MAXIMUM | ALLOCATION | NEED\n");
    for (int i=0; i<NUMBER_OF_CUSTOMERS; i++) {
        fprintf(fptr, "%d %d %d   | %d %d %d      | %d %d %d \n",
            maximum[i][0], maximum[i][1], maximum[i][2], 
            allocation[i][0], allocation[i][1], allocation[i][2], 
            need[i][0], need[i][1], need[i][2]);
    }
    fprintf(fptr, "AVAILABLE %d %d %d \n", available[0], available[1], available[2]);
}
