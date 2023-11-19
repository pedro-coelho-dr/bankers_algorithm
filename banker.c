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

    if (NUMBER_OF_RESOURCES > MAX_NUMBER_OF_RESOURCES) {
        fprintf(stderr, ":(\n");
        return 1;
    }

    // available array
    for (int i=1; i<=NUMBER_OF_RESOURCES; i++) {
        available[i - 1] = atoi(argv[i]);
    }

    // read customer.txt
    FILE *file = fopen("customer.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Fail to read customer.txt\n");
        return 1;
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        int resourceCount = 0;
        while (token != NULL) {
            if (resourceCount >= NUMBER_OF_RESOURCES) {
                fprintf(stderr, "Incompatibility between customer.txt and command line\n");
                fclose(file);
                return 1;
            }
            maximum[NUMBER_OF_CUSTOMERS][resourceCount++] = atoi(token);
            token = strtok(NULL, ",");
        }
        if (resourceCount != NUMBER_OF_RESOURCES) {
            fprintf(stderr, "Incompatibility between customer.txt and command line\n");
            fclose(file);
            return 1;
        }
        NUMBER_OF_CUSTOMERS++;
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

    char commandLine[1000];
    char command[10];
    int customer_num, resources[MAX_NUMBER_OF_RESOURCES];

    while (fgets(commandLine, sizeof(commandLine), cmdFile)) {

        if (strcmp(commandLine, "*")==0 || strcmp(commandLine, "*\n")==0 || strcmp(commandLine, "*\r\n")==0) {
            continue;
        }
        int numTokens = sscanf(commandLine, "%s %d", command, &customer_num);
        if (numTokens < 2 || (strcmp(command, "RQ") != 0 && strcmp(command, "RL") != 0)) {
            fprintf(stderr, "Incompatibility between commands.txt and command lineA\n");
            fclose(cmdFile);
            return 1;
        }

        if (customer_num<0 || customer_num>=NUMBER_OF_CUSTOMERS) {
            fprintf(stderr, "Incompatibility between commands.txt and command lineB\n");
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
                fprintf(stderr, "Incompatibility between commands.txt and command lineC\n");
                fclose(cmdFile);
                return 1;
            }
            if (lastToken != NULL) {
                fprintf(stderr, "Incompatibility between commands.txt and command lineD\n");
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

            if (i != NUMBER_OF_RESOURCES) {
                fprintf(stderr, "Incompatibility between commands.txt and command line\n");
                fclose(cmdFile);
                fclose(resultFile);
                remove("result.txt"); 
                return 1;
            }

            if (strcmp(command, "RQ") == 0) {
                request_resources(customer_num, resources, resultFile);
            } else if (strcmp(command, "RL") == 0) {
                release_resources(customer_num, resources, resultFile);
            }
        } else if (strcmp(command, "*") == 0) {
            print_state(resultFile);
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
    if (is_safe()) {
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

void release_resources(int customer_num, int release[], FILE *fptr) {
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

/* void print_state(FILE *fptr) {
    fprintf(fptr, "MAXIMUM | ALLOCATION | NEED\n");
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            fprintf(fptr, "%d ", maximum[i][j]);
        }
        fprintf(fptr, "  | "); // Three spaces before the '|'
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            fprintf(fptr, "%d ", allocation[i][j]);
        }
        fprintf(fptr, "     | "); // Six spaces before the '|', reduced by one from previous version
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            fprintf(fptr, "%d ", need[i][j]);
        }
        fprintf(fptr, "\n");
    }
    fprintf(fptr, "AVAILABLE ");
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        fprintf(fptr, "%d ", available[i]);
    }
    fprintf(fptr, "\n");
} */
/* 
void print_state(FILE *fptr) {
    const int nameLengths[3] = { strlen("MAXIMUM"), strlen("ALLOCATION"), strlen("NEED") };
    const int resourceSpacing = NUMBER_OF_RESOURCES * 2 - 1;

    int columnWidths[3] = {
        resourceSpacing > nameLengths[0] ? resourceSpacing : nameLengths[0],
        resourceSpacing > nameLengths[1] ? resourceSpacing : nameLengths[1],
        resourceSpacing
    };


    fprintf(fptr, "%-*s | %-*s | %s\n", columnWidths[0], "MAXIMUM", columnWidths[1], "ALLOCATION", "NEED");

  
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            fprintf(fptr, "%d", maximum[i][j]);
            if (j < NUMBER_OF_RESOURCES - 1) fprintf(fptr, " "); 
        }
        fprintf(fptr, "%*s", columnWidths[0] - resourceSpacing + 1, "| "); 

        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            fprintf(fptr, "%d", allocation[i][j]);
            if (j < NUMBER_OF_RESOURCES - 1) fprintf(fptr, " ");
        }
        fprintf(fptr, "%*s", columnWidths[1] - resourceSpacing + 1, "| ");

        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            fprintf(fptr, "%d", need[i][j]);
            if (j < NUMBER_OF_RESOURCES - 1) fprintf(fptr, " ");
        }
        fprintf(fptr, "\n");
    }

    // Print AVAILABLE resources
    fprintf(fptr, "AVAILABLE ");
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        fprintf(fptr, "%d ", available[i]);
    }
    fprintf(fptr, "\n");
} */