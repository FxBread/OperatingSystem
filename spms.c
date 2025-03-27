#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "closepipe.h"
#include <time.h>

#define MAX_MSG 256
#define MAX_INPUT_VALUE 32
#define MAX_BOOKING 1000

typedef struct {
    int command_type;
    char input_args[MAX_MSG];
}InputMsg;

typedef struct DistributeItem{
    int parking_space[10];
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
    bool parking_need;
    float book_time_duration;
    int facilitates[6];
    int command_type;
    int status;
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
enum State{
    PENDING=0,DONE=1,WAITING=2
};

//global variable
//pipe in header file
int process_count = 5;//number of processes
int booking_count = 0;
BookingMsg bookingMsgs[MAX_BOOKING]; //store all booking value
int bookingMsgs_count = 0;
// DistributeItem distributeItems[7][24];//resource allocate for 24x7 hour


int convert_date_to_day_index(time_t date) {
    char *temp = ctime(&date);
    char temp2[4];
    int temp3;
    strncpy(temp2, temp, 3);
    if (strcmp(temp2, "Sun") == 0) {
        temp3 = SUN;
    }else if(strcmp(temp2, "Mon") == 0) {
        temp3 = MON;
    }else if(strcmp(temp2, "Tue") == 0) {
        temp3 = TUE;
    }else if(strcmp(temp2, "Wed") == 0) {
        temp3 = WED;
    }else if(strcmp(temp2, "Thu") == 0) {
        temp3 = THU;
    }else if(strcmp(temp2, "Fri") == 0) {
        temp3 = FRI;
    }else if(strcmp(temp2, "Sat") == 0) {
        temp3 = SAT;
    }
    return temp3;
}
int convert_date_to_start_hour_index(char *booking_date) {
    char temp[3];
    int hour_index;
    strncpy(temp, booking_date, 2);
    hour_index = atoi(temp);
    return hour_index;
}
int convert_date_to_end_hour_index(int start_hour_index,float book_time_duration) {
    int hour_index;
    hour_index = start_hour_index + book_time_duration;
    return hour_index;
}
void input_process(){
    char command[MAX_MSG];
    char args[MAX_MSG];

    while(1){
        BookingMsg bookingMsg;
        //clear data of bookingmsg
        memset(&bookingMsg, 0, sizeof(BookingMsg));
        bookingMsg.status = WAITING;
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

        if (bookingMsg.command_type==0 || bookingMsg.command_type==1 ||
            bookingMsg.command_type==2 || bookingMsg.command_type==3) {
            if (count >= 4) {
                if (count > 6 && bookingMsg.command_type==0) {//addparking request
                    printf("addParking only accept no more than 2 facilities. count: %d\n",count-4);
                    break;
                }
                if (count != 6 && bookingMsg.command_type==1) {//addReservation request
                    printf("Two facilities are required for addReservation. count: %d\n",count-4);
                    break;
                }
                if (count > 7 && bookingMsg.command_type==2) {//addEvent request
                    printf("addEvent only accept no more than 3 facilities. count: %d\n",count-4);
                    break;
                }
                if (count != 5 && bookingMsg.command_type==3) {//bookEssentials request
                    printf("bookEssentials only accept 1 facilities. count: %d\n",count-4);
                    break;
                }
                char *temp = strchr(tokens[0],'-');
                if (temp!=NULL) {
                    temp= temp+1;
                    strcpy(bookingMsg.member_name,temp);
                    strcpy(bookingMsg.booking_date,tokens[1]);
                    strcpy(bookingMsg.booking_time,tokens[2]);


                    char temp2[50];
                    strcpy(temp2,tokens[1]);
                    size_t temp_size = strlen(temp2);
                    temp2[temp_size] = ' ';
                    temp2[temp_size+1] = '\0';
                    strcat(temp2, tokens[2]);//concat date and time
                    struct tm tm;
                    memset(&tm, 0, sizeof(tm));
                    strptime(temp2, "%Y-%m-%d %H:%M", &tm);//string to timestamp
                    bookingMsg.date = mktime(&tm);


                    if (bookingMsg.command_type==3) {
                        bookingMsg.parking_need=false;
                    }else {
                        bookingMsg.parking_need=true;
                    }

                }else {
                    printf("unvalid member.\n");
                    break;
                }

                if (count == 4) {

                    char *temp = strchr(tokens[3],';');
                    if (temp!=NULL) {
                        *temp = '\0';//delete ;
                        bookingMsg.book_time_duration = atof(temp);
                    }else {
                        printf("unvalid args.\n");
                        break;
                    }
                }else {
                    bookingMsg.book_time_duration = atof(tokens[3]);//convert char to float
                }

            }else{
                printf("unvalid args: %s\n",args);
                break;
            }
            if (count>4) {
                for (int i = 4; i < count; i++) {
                    char *temp = strchr(tokens[i],';');
                    if (temp!=NULL) {
                        *temp = '\0';//delete ;
                    }
                    if (strcmp(tokens[i],"battery")==0) {
                        bookingMsg.facilitates[BATTERY] = 1;
                        if (bookingMsg.command_type!=3) {//all command need a pair of item except essentialbook
                            bookingMsg.facilitates[CABLE] = 1;
                        }
                    }else if (strcmp(tokens[i],"cable")==0) {
                        bookingMsg.facilitates[CABLE] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilitates[BATTERY] = 1;
                        }
                    }else if (strcmp(tokens[i],"locker")==0) {
                        bookingMsg.facilitates[LOCKER] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilitates[UMBRELLA] = 1;
                        }
                    }else if (strcmp(tokens[i],"umbrella")==0) {
                        bookingMsg.facilitates[UMBRELLA] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilitates[LOCKER] = 1;
                        }
                    }else if (strcmp(tokens[i],"inflation")==0) {
                        bookingMsg.facilitates[INFLATION] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilitates[VALET] = 1;
                        }
                    }else if (strcmp(tokens[i],"valet")==0) {
                        bookingMsg.facilitates[VALET] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilitates[INFLATION] = 1;
                        }
                    }

                }
            }//BATTERY=0,CABLE=1,LOCKER=2,UMBRELLA=3,INFLATION=4,VALET=5
            printf("-> [PENDING]\n");
            printf("booking msg :commandtype %d name %s date %s time %s duration %f parkingneed %d",
                bookingMsg.command_type,bookingMsg.member_name,
                bookingMsg.booking_date,bookingMsg.booking_time,
                bookingMsg.book_time_duration,bookingMsg.parking_need);
            printf("booking msg :battery %d cable %d locker %d umbrella %d inflation %d valet %d\n",
                bookingMsg.facilitates[BATTERY],bookingMsg.facilitates[CABLE],bookingMsg.facilitates[LOCKER],
                bookingMsg.facilitates[UMBRELLA],bookingMsg.facilitates[INFLATION],bookingMsg.facilitates[VALET]);
            bookingMsgs[booking_count++] = bookingMsg; //store in booking array
            printf("test: booking count %d\n",booking_count);
        }



        if (bookingMsg.command_type==5) {
            char *temp;
            int status;
            if (count==1) {
                temp = tokens[0];
                printf("%s\n",temp);
            }else {
                printf("unvalid args.\n");
                break;
            }
            if (strcmp(temp,"-fcfs")==0) {
                status = FCFS;
            }else if (strcmp(temp,"-prio")==0) {
                status = PRIO;
            }else if (strcmp(temp,"-opti")==0) {
                status = OPTI;
            }else if (strcmp(temp,"-ALL")==0) {
                status = ALL;
            }else {
                printf("unvalid args.\n");
            }
            write(main_to_scheduler[PIPE_WRITE],&bookingMsgs,sizeof(bookingMsgs)); //send message to scheduler process
            write(main_to_scheduler[PIPE_WRITE],&temp,sizeof(temp));
            while (1) {
                read(scheduler_to_main[PIPE_READ],&status,sizeof(int));//receive status
                if (status==DONE) {
                    printf("-> [DONE]\n");
                    break;
                }

            }
        }

    }

}
int check_parking_valid(DistributeItem *distribute_item,int start_hour_index,int end_hour_index ) {
    for (int i = 0;i<10;i++) {
        bool parking_valid = true;
        for (int j = start_hour_index;j<end_hour_index;j++) {
            if (distribute_item[j].parking_space[i]==1) {
                parking_valid = false;
            }
        }
        if (parking_valid) {
            return i;
        }
    }
    return -1;
}
bool check_facilitates_need(int *facilitates) {
    for (int i = 0;i<6;i++) {
        if (facilitates[i]==1) {
            return true;
        }
    }
    return false;
}
void fcfs_process() {
    close_fcfs_unused_pipe();

    while (1) {
        BookingMsg recbookingMsgs[MAX_BOOKING]; //store all booking value
        DistributeItem distributeItems[7][24];//resource allocate for 24x7 hour
        read(scheduler_to_fcfs[PIPE_READ],&recbookingMsgs,sizeof(recbookingMsgs));

        for (int i = 0;i<MAX_BOOKING;i++) {//calculate booking duration time
            int day_index = convert_date_to_day_index(recbookingMsgs[i].date);//sun-sat 0-6day
            int start_hour_index = convert_date_to_start_hour_index(recbookingMsgs[i].booking_time);//time slot index 0-23
            int end_hour_index = convert_date_to_end_hour_index(start_hour_index,recbookingMsgs[i].book_time_duration);
            int parking_index;
            int facilitates_index;
            if (end_hour_index<23) {
                if (recbookingMsgs[i].parking_need==true) {
                    parking_index=check_parking_valid(distributeItems[day_index],start_hour_index,end_hour_index);

                }
                if (check_facilitates_need(recbookingMsgs[i].facilitates)) {
                    facilitates_index = check_facility_valid(distributeItems[day_index],start_hour_index,end_hour_index);
                }
            }
            if (strcmp(recbookingMsgs[i+1].member_name, "")==0) {
                break;
            }
        }

    }


}

void scheduler_process() {
    close_scheduler_unused_pipe();
    int status;

    while(1) {
        BookingMsg recbookingMsgs[MAX_BOOKING]; //store all booking value
        read(main_to_scheduler[PIPE_READ],&recbookingMsgs,sizeof(recbookingMsgs));
        read(main_to_scheduler[PIPE_READ],&status,sizeof(int));//receive scheduler type
        printf("%d\n",status);
        //processing scheduler function
        if (status==FCFS) {

            write(scheduler_to_fcfs[PIPE_WRITE],&recbookingMsgs,sizeof(recbookingMsgs));
        }

        status = DONE;
        write(scheduler_to_main[PIPE_WRITE],&status,sizeof(int)); //feedback to parent

    }
}

void fork_child_create() {
    if (fork() == 0 ) {
        // scheduler process
        scheduler_process();
        // exit(0);
    }
    if (fork() == 0 ) {
        // fcfs process
        fcfs_process();
        // exit(0);
    }
    // if (fork() == 0 ) {
    //     // priority process
    //
    //     exit(0);
    // }
    // if (fork() == 0 ) {
    //     // optimize process
    //
    //     exit(0);
    // }
    // if (fork() == 0 ) {
    //     // printer process
    // }
}


void init_system(){
    printf("~~ WELCOME TO PolyU ~~\n");
    //create pipe
    if (pipe(main_to_scheduler)<0||pipe(scheduler_to_main)<0||
        pipe(scheduler_to_fcfs)<0||pipe(scheduler_to_prio)<0||
        pipe(scheduler_to_opti)<0||pipe(fcfs_to_scheduler)<0||
        pipe(prio_to_scheduler)<0||pipe(opti_to_scheduler)<0||
        pipe(scheduler_to_print)<0||pipe(print_to_scheduler)<0) {
        printf("Pipe creation error\n");
    }
    fork_child_create();
    close_main_unused_pipe();
    input_process();

}


int main(int argc, char* argv[]){

    init_system();

    return 0;


}