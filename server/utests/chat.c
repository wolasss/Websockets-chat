#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define SEM_USERS 0
#define SEM_ROOMS 1
#define SEM_LOGFILE 2

#define MAX_USERS 20
#define MAX_ROOMS 10

//testing 
#include "minunit.h"
 
int tests_run = 0;


struct CHATcommand {
    int commandId;
    char * name;
    char * param;
};


struct CHATcommand * CHATdecodeCommand(char* a_command, struct CHATcommand *cmd) {
    cmd = (struct CHATcommand *) malloc(sizeof(struct CHATcommand));

    int paramLen, i=1, b=0, cmdLen = strlen(a_command);

    char command[16];
    bzero(command,16);
    while(a_command[i]!=' ' && i<16) {
            command[i-1]=a_command[i];
            i++;
    }
    cmd->name = malloc(i+1);

    paramLen = cmdLen-i+1;
    if(paramLen>=0) {
        cmd->param = malloc(paramLen);
        bzero(cmd->param, paramLen);
        i++; // remove space 
        while(i<cmdLen) {
                cmd->param[b]=a_command[i];
                i++; 
                b++;
        }
    }
    strncpy(cmd->name, command, strlen(command));
    if(a_command[0]!='%') {
        if(!strcmp(command, "login")) {
            cmd->commandId = 1;
        } else if(!strcmp(command, "help")) {
            cmd->commandId = 2;
        } else if(!strcmp(command, "join")) {
            cmd->commandId = 3;
        } else if(!strcmp(command, "users")) {
            cmd->commandId = 4;
        } 
    }
    
    return cmd;
}
char * createJSON ( int * a_statusCode, char* a_sender, char* a_room, char* a_message, char* aout_reply ) {
    aout_reply = malloc(512);
    long messageAloc = strlen(a_message)+20;
    char * statusJSON = malloc(32),
            *senderJSON = malloc(64),
            *roomJSON = malloc(64),
            *messageJSON = malloc(messageAloc);

    bzero(statusJSON, 32);
    bzero(senderJSON, 64);
    bzero(roomJSON, 64);
    bzero(aout_reply,512);
    bzero(messageJSON, messageAloc);

    snprintf(statusJSON, 32, "\"status\": %d,", *a_statusCode);
    if(a_sender) {
        snprintf(senderJSON, 64, "\"sender\": \"%s\",", a_sender);
    }
    if(a_room) {
        snprintf(roomJSON, 64, "\"room\": \"%s\",", a_room);
    }
    if(a_message[0]=='[') {
        snprintf(messageJSON, messageAloc, "\"message\": %s", a_message);
    } else {
        snprintf(messageJSON, messageAloc, "\"message\": \"%s\"", a_message);
    }

    snprintf(aout_reply, 512, "{ %s %s %s %s }", statusJSON, senderJSON, roomJSON, messageJSON);
    

    free(statusJSON);
    free(senderJSON);
    free(roomJSON);
    free(messageJSON);
    return aout_reply;
}


static char * test_1() {
    struct CHATcommand * test;
    test = CHATdecodeCommand("%main room", test);
    printf("expected: main, actual: %s\n", test->name);
    mu_assert("error, %main room", !strcmp(test->name, "main"));
    return 0;
}

static char * test_2() {
    struct CHATcommand * test;
    test = CHATdecodeCommand("%main adsffdsfdsfsdfdsfsdfsdfsdfsdfdssdf", test);
    printf("expected: main, actual: %s\n", test->name);
    mu_assert("error, %main room", !strcmp(test->name, "main"));
    return 0;
}

static char * test_3() {
    struct CHATcommand * test;
    test = CHATdecodeCommand("%main :D sfdjhskjhgjkghkjdfghgdfjkd", test);
    printf("expected: main, actual: %s\n", test->name);
    mu_assert("error, %main room", !strcmp(test->name, "main"));
    return 0;
}

static char * test_4() {
    char * test;
    int status = 100;
    test = createJSON(&status, "trololo", "room1", "trolololo lololol olololo", test);
    printf("expected: main, : %s\n", test);
    mu_assert("error, json 1", !strcmp(test, "{ \"status\": 100, \"sender\": \"trololo\", \"room\": \"room1\", \"message\": \"trolololo lololol olololo\" }"));
    return 0;
}

static char * test_5() {
    char * test;
    int status = 100;
    test = createJSON(&status, "trololo", NULL, "trolololo lololol olololo", test);
    printf("expected: main, : %s\n", test);
    mu_assert("error, json 2", !strcmp(test, "{ \"status\": 100, \"sender\": \"trololo\",  \"message\": \"trolololo lololol olololo\" }"));
    return 0;
}

static char * test_6() {
    char * test;
    int status = 100;
    test = createJSON(&status, NULL, NULL, "trolololo lololol olololo", test);
    printf("expected: main, : %s\n", test);
    mu_assert("error, json 3", !strcmp(test, "{ \"status\": 100,   \"message\": \"trolololo lololol olololo\" }"));
    return 0;
}

static char * test_7() {
    char * test;
    int status = 100;
    test = createJSON(&status, NULL, NULL, "['trolololo', 'lololol', 'olololo']", test);
    printf("expected: main, : %s\n", test);
    mu_assert("error, json 4 array", !strcmp(test, "{ \"status\": 100,   \"message\": ['trolololo', 'lololol', 'olololo'] }"));
    return 0;
}

static char * all_tests() {
    mu_run_test(test_1);
    mu_run_test(test_2);
    mu_run_test(test_3);
    mu_run_test(test_4);
    mu_run_test(test_5);
    mu_run_test(test_6);
    mu_run_test(test_7);
    return 0;
}

int main(int argc, char **argv) {
    char *result = all_tests();
    if (result != 0) {
     printf("%s\n", result);
    }
    else {
     printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}