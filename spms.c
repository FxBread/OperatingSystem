#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define MAX_INPUT_MSG 256

typedef struct {
    int command_type;
    char input_args[MAX_INPUT_MSG];
}InputMsg;

typedef struct DistributeItem{
    int battery[3];
    int cable[3];
    int locker[3];
    int umbrella[3];
    int inflation[3];
    int valet[3];

}DistributeItem;


typedef struct{
    char member_name[MAX_INPUT_MSG];
    char booking_date;
    char booking_time;
    time_t date;  //time stamp to compare
    int parking_place;
    int parking_need;
    float book_time_duration;
    int facilitates[6];
}BookingMsg;

enum Week{
    SUN=0,MON=1,TUE=2,WED=3,THU=4,FRI=5,SAT=6
};


//global variable
int main_to_scheduler[2];// input process to scheduler
int scheduler_to_fcfs[2];
int scheduler_to_prio[2];
int scheduler_to_opti[2];
int scheduler_to_print[2];
int process_count = 5;//number of processes
int booking_count = 0;

void input_process(){
    char command[MAX_INPUT_MSG];
    char args[MAX_INPUT_MSG];
    InputMsg msg;
    while(1){
        printf("Please enter booking:\n");
        scanf("%s %s",command,args);
        if(strcmp(command,"addParking")==0){
            msg.command_type=0;
        }else if(strcmp(command,"addReservation")==0){
            msg.command_type=1;
        }else if(strcmp(command,"addEvent")==0){
            msg.command_type=2;
        }else if(strcmp(command,"bookEssentials")==0){
            msg.command_type=3;
        }else if(strcmp(command,"addBatch")==0){
            msg.command_type=4;
        }else if(strcmp(command,"printBookings")==0){
            msg.command_type=5;
        }else if(strcmp(command,"endProgram")==0){
            msg.command_type=-1;
        }else{
            printf("unvalid command: %s\n",command);
            break;
        }

    }

}

void fork_child_create() {
    if (fork() == 0 ) {
        // fcfs process
    }else {
        printf("fork child create failed");
    }
    if (fork() == 0 ) {
        // priority process
    }else {

    }
    if (fork() == 0 ) {
        // optimize process
    }else {

    }
}

void init_system(){
    printf("~~ WELCOME TO PolyU ~~\n");
    //create pipe
    if (pipe(main_to_scheduler)<0||pipe(scheduler_to_fcfs)<0||
        pipe(scheduler_to_prio)<0||pipe(scheduler_to_opti)<0||
        pipe(scheduler_to_print)<0) {
        printf("Pipe creation error\n");
    }
    input_process();
    fork_child_create();
}


int main(int argc, char* argv[]){

    init_system();

    return 0;


}