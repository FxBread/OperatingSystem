#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define MAX_MSG 256
#define MAX_INPUT_VALUE 32
#define PIPE_READ 0
#define PIPE_WRITE 1

typedef struct {
    int command_type;
    char input_args[MAX_MSG];
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
    char member_name[MAX_INPUT_VALUE];
    char booking_date[MAX_INPUT_VALUE];
    char booking_time[MAX_INPUT_VALUE];
    time_t date;  //time stamp to compare
    int parking_place;
    int parking_need;
    float book_time_duration;
    int facilitates[6];
    int command_type;
}BookingMsg;

enum Facility{
    BATTERY=0,CABLE=1,LOCKER=2,UMBRELLA=3,INFLATION=4,VALET=5
};
enum Week{
    SUN=0,MON=1,TUE=2,WED=3,THU=4,FRI=5,SAT=6
};
enum AllocateType{
    FCFS=0,PRIO=1,OPTI=2,ALL=3
};

//global variable
int main_to_scheduler[2];// input process to scheduler
int scheduler_to_fcfs[2];
int scheduler_to_prio[2];
int scheduler_to_opti[2];
int fcfs_to_scheduler[2];
int prio_to_scheduler[2];
int opti_to_scheduler[2];
int scheduler_to_print[2];
int process_count = 5;//number of processes
int booking_count = 0;

void input_process(){
    char command[MAX_MSG];
    char args[MAX_MSG];
    BookingMsg bookingMsg;
    while(1){
        printf("Please enter booking:\n");
        scanf("%s %[^\n]",command,args);
        if(strcmp(command,"addParking")==0){
            bookingMsg.command_type=0;
        }else if(strcmp(command,"addReservation")==0){
            bookingMsg.command_type=1;
        }else if(strcmp(command,"addEvent")==0){
            bookingMsg.command_type=2;
        }else if(strcmp(command,"bookEssentials")==0){
            bookingMsg.command_type=3;
        }else if(strcmp(command,"addBatch")==0){
            bookingMsg.command_type=4;
        }else if(strcmp(command,"printBookings")==0){
            bookingMsg.command_type=5;
        }else if(strcmp(command,"endProgram")==0){
            bookingMsg.command_type=-1;
        }else{
            printf("unvalid command: %s\n",command);
            break;
        }


        char *tokens[10];
        char *token;
        int count = 0;
        token = strtok(args," ");
        while (token!=NULL && count<10) {
            tokens[count] = token;
            token = strtok(NULL," ");
            count++;
        }
        if (bookingMsg.command_type!=5&&bookingMsg.command_type!=-1) {
            if (count >= 3) {
                char *temp = strchr(tokens[0],'-');
                if (temp!=NULL) {
                    temp= temp+1;
                    strcpy(bookingMsg.member_name,temp);
                    strcpy(bookingMsg.booking_date,tokens[1]);
                    strcpy(bookingMsg.booking_time,tokens[2]);
                    bookingMsg.book_time_duration = atof(tokens[3]);
                }else {
                    printf("unvalid member.\n");
                    break;
                }

            }else{
                printf("unvalid args: %s\n",args);
                break;
            }
            if (count>=4) {
                for (int i = 4; i < count; i++) {
                    char *temp = strchr(tokens[i],';');
                    if (temp!=NULL) {
                        *temp = '\0';//delete ;
                    }
                    printf("%s\n",tokens[i]);
                    if (strcmp(tokens[i],"battery")==0) {
                        bookingMsg.facilitates[BATTERY] = 1;
                    }else if (strcmp(tokens[i],"cable")==0) {
                        bookingMsg.facilitates[CABLE] = 1;
                    }else if (strcmp(tokens[i],"locker")==0) {
                        bookingMsg.facilitates[LOCKER] = 1;
                    }else if (strcmp(tokens[i],"umbrella")==0) {
                        bookingMsg.facilitates[UMBRELLA] = 1;
                    }else if (strcmp(tokens[i],"inflation")==0) {
                        bookingMsg.facilitates[INFLATION] = 1;
                    }else if (strcmp(tokens[i],"valet")==0) {
                        bookingMsg.facilitates[VALET] = 1;
                    }

                }
            }//BATTERY=0,CABLE=1,LOCKER=2,UMBRELLA=3,INFLATION=4,VALET=5
        }

        write(main_to_scheduler[PIPE_WRITE],&bookingMsg,sizeof(BookingMsg)); //send message to scheduler process


    }

}

void scheduler_process() {
    close(main_to_scheduler[PIPE_WRITE]);
    while(1) {
        BookingMsg bookingMsg;
        read(main_to_scheduler[PIPE_READ],&bookingMsg,sizeof(BookingMsg));
        printf("member name: %s\n",bookingMsg.member_name);
        return;
    }
}
void fork_child_create() {
    if (fork() == 0 ) {
        // scheduler process
        scheduler_process();
        exit(0);
    }
    if (fork() == 0 ) {
        // fcfs process

        exit(0);
    }
    if (fork() == 0 ) {
        // priority process

        exit(0);
    }
    if (fork() == 0 ) {
        // optimize process

        exit(0);
    }
    if (fork() == 0 ) {
        // printer process
    }
}

void close_main_unused_pipe() {
    close(main_to_scheduler[PIPE_READ]);
    close(scheduler_to_fcfs[PIPE_WRITE]);
    close(scheduler_to_fcfs[PIPE_READ]);
    close(scheduler_to_prio[PIPE_WRITE]);
    close(scheduler_to_prio[PIPE_READ]);
    close(scheduler_to_opti[PIPE_WRITE]);
    close(scheduler_to_opti[PIPE_READ]);
    close(fcfs_to_scheduler[PIPE_WRITE]);
    close(fcfs_to_scheduler[PIPE_READ]);
    close(prio_to_scheduler[PIPE_WRITE]);
    close(prio_to_scheduler[PIPE_READ]);
    close(opti_to_scheduler[PIPE_WRITE]);
    close(opti_to_scheduler[PIPE_READ]);
    close(scheduler_to_print[PIPE_WRITE]);
    close(scheduler_to_print[PIPE_READ]);
}

void init_system(){
    printf("~~ WELCOME TO PolyU ~~\n");
    //create pipe
    if (pipe(main_to_scheduler)<0||pipe(scheduler_to_fcfs)<0||
    pipe(scheduler_to_prio)<0||pipe(scheduler_to_opti)<0||
    pipe(scheduler_to_fcfs)<0||pipe(scheduler_to_prio)<0||
    pipe(scheduler_to_opti)<0||pipe(scheduler_to_print)<0) {
        printf("Pipe creation error\n");
    }

    input_process();
    fork_child_create();
    close_main_unused_pipe();
}


int main(int argc, char* argv[]){

    init_system();

    return 0;


}