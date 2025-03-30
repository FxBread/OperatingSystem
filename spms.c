#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include "closepipe.h"
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX_MSG 256
#define MAX_INPUT_VALUE 32
#define MAX_BOOKING 100
#define MAX_PARKING_SPACE 3
#define INVALID_INDEX -2
#define NO_PARKING_SPACE -1
#define USED 0
#define UNUSED 1
#define FAIL 0
#define SUCCESS 1

typedef struct {
    int command_type;
    char input_args[MAX_MSG];
}InputMsg;

typedef struct DistributeItem{
    int parking_space[MAX_PARKING_SPACE];
    // int battery[3];
    // int cable[3];
    // int locker[3];
    // int umbrella[3];
    // int inflation[3];
    // int valet[3];
    int facilities[6][3];
}DistributeItem;


typedef struct{
    char member_name[MAX_INPUT_VALUE];
    char booking_date[MAX_INPUT_VALUE];
    char booking_time[MAX_INPUT_VALUE];
    time_t date;  //time stamp to compare
    int parking_place;
    bool parking_need;
    float book_time_duration;
    int facilities[6];
    int command_type;
    int status;
}BookingMsg;

typedef struct{
    BookingMsg bookingMsgs[MAX_BOOKING];
    int booking_count;
    int command_type;
}BookingMsg_for_pipe;

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
pid_t child_pids[4] = {0};

int convert_date_to_day_index(time_t date) {
    char *temp = ctime(&date);
    char temp2[4];
    temp2[3] = '\0';
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
void exit_program() {
    for (int i = 0; i < 4; i++) {
        if (child_pids[i] > 0) {
            kill(child_pids[i], SIGTERM);
        }
    }
}
void add_batch(char *input_command,char *input_args) {
    char batch_command[MAX_MSG];
    char batch_args[MAX_MSG];
    strcpy(batch_command, input_command);
    strcpy(batch_args, input_args+1);
    FILE *batch_file = fopen(batch_args, "r");
    if (batch_file == NULL) {
        printf("Cannot open batch file: %s\n", batch_args);
        exit_program();
    }
    printf("Processing batch file: %s\n", batch_args);
    char line[MAX_MSG];
    int line_number = 0;
    while (fgets(line, sizeof(line), batch_file)) {
        char command[MAX_MSG];
        char args[MAX_MSG];
        line_number++;
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        int line_valid = sscanf(line, "%s %[^\n]", command, args);
        if (line_valid < 1) {
            printf("Line %d: Invalid line in batch file: %s\n", line_number, line);
        }
        BookingMsg bookingMsg;
        //clear data of bookingmsg
        memset(&bookingMsg, 0, sizeof(BookingMsg));
        //command handle
        if(strcmp(command,"addParking")==0){
            bookingMsg.command_type=0;
        }else if(strcmp(command,"addReservation")==0){
            bookingMsg.command_type=1;
        }else if(strcmp(command,"addEvent")==0){
            bookingMsg.command_type=2;
        }else if(strcmp(command,"bookEssentials")==0){
            bookingMsg.command_type=3;
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
                        bookingMsg.book_time_duration = atof(tokens[3]);
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
                        bookingMsg.facilities[BATTERY] = 1;
                        if (bookingMsg.command_type!=3) {//all command need a pair of item except essentialbook
                            bookingMsg.facilities[CABLE] = 1;
                        }
                    }else if (strcmp(tokens[i],"cable")==0) {
                        bookingMsg.facilities[CABLE] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilities[BATTERY] = 1;
                        }
                    }else if (strcmp(tokens[i],"locker")==0) {
                        bookingMsg.facilities[LOCKER] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilities[UMBRELLA] = 1;
                        }
                    }else if (strcmp(tokens[i],"umbrella")==0) {
                        bookingMsg.facilities[UMBRELLA] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilities[LOCKER] = 1;
                        }
                    }else if (strcmp(tokens[i],"inflation")==0) {
                        bookingMsg.facilities[INFLATION] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilities[VALET] = 1;
                        }
                    }else if (strcmp(tokens[i],"valet")==0) {
                        bookingMsg.facilities[VALET] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilities[INFLATION] = 1;
                        }
                    }

                }
            }//BATTERY=0,CABLE=1,LOCKER=2,UMBRELLA=3,INFLATION=4,VALET=5
            printf("-> [PENDING]\n");

            bookingMsgs[booking_count++] = bookingMsg; //store in booking array

        }


    }


}
void input_process(){
    char command[MAX_MSG];
    char args[MAX_MSG];


    while(1){
        BookingMsg bookingMsg;
        //clear data of bookingmsg
        memset(&bookingMsg, 0, sizeof(BookingMsg));
        // bookingMsg.status = WAITING;
        printf("Please enter booking:\n");

        scanf("%s", command);
        if (strcmp(command, "endProgram") != 0) {
            scanf(" %[^\n]", args);
        } else {
            args[0] = '\0';
        }
        if (strcmp(command, "addBatch") == 0) {
            add_batch(command, args);
            continue;
        }

        //command handle
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
            exit_program();
            break;

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
                        bookingMsg.book_time_duration = atof(tokens[3]);
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
                        bookingMsg.facilities[BATTERY] = 1;
                        if (bookingMsg.command_type!=3) {//all command need a pair of item except essentialbook
                            bookingMsg.facilities[CABLE] = 1;
                        }
                    }else if (strcmp(tokens[i],"cable")==0) {
                        bookingMsg.facilities[CABLE] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilities[BATTERY] = 1;
                        }
                    }else if (strcmp(tokens[i],"locker")==0) {
                        bookingMsg.facilities[LOCKER] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilities[UMBRELLA] = 1;
                        }
                    }else if (strcmp(tokens[i],"umbrella")==0) {
                        bookingMsg.facilities[UMBRELLA] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilities[LOCKER] = 1;
                        }
                    }else if (strcmp(tokens[i],"inflation")==0) {
                        bookingMsg.facilities[INFLATION] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilities[VALET] = 1;
                        }
                    }else if (strcmp(tokens[i],"valet")==0) {
                        bookingMsg.facilities[VALET] = 1;
                        if (bookingMsg.command_type!=3) {
                            bookingMsg.facilities[INFLATION] = 1;
                        }
                    }

                }
            }//BATTERY=0,CABLE=1,LOCKER=2,UMBRELLA=3,INFLATION=4,VALET=5
            printf("-> [PENDING]\n");

            bookingMsgs[booking_count++] = bookingMsg; //store in booking array

        }



        if (bookingMsg.command_type==5) {
            char *temp;
            int status;
            if (count==1) {
                temp = tokens[0];
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
            BookingMsg_for_pipe bookingMsg_for_pipe;
            memcpy(bookingMsg_for_pipe.bookingMsgs, bookingMsgs, sizeof(bookingMsgs));
            bookingMsg_for_pipe.booking_count = booking_count;
            bookingMsg_for_pipe.command_type = status;
            write(main_to_scheduler[PIPE_WRITE],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe)); //send message to scheduler process

            read(scheduler_to_main[PIPE_READ],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe));//receive status
            printf("-> [DONE]\n");


        }

    }

}
void init_pipe_value(DistributeItem distributeItems[][24]) {
    for (int i=0;i<7;i++) {
        for (int j=0;j<24;j++) {
            for (int k=0;k<MAX_PARKING_SPACE;k++) {
                distributeItems[i][j].parking_space[k] = UNUSED;
            }
            for (int l=0;l<6;l++) {
                for (int m=0;m<3;m++) {
                    distributeItems[i][j].facilities[l][m] = UNUSED;
                }
            }
        }
    }
}

int check_parking_valid(DistributeItem *distribute_item,int start_hour_index,int end_hour_index ) {

    for (int i = 0;i<MAX_PARKING_SPACE;i++) {
        bool parking_valid = true;
        for (int j = start_hour_index;j<end_hour_index;j++) {
            if (distribute_item[j].parking_space[i]==USED) {
                parking_valid = false;
                break;
            }
        }
        if (parking_valid) {
            return i;
        }
    }
    return NO_PARKING_SPACE;
}
bool check_facilities_need(int *facilities) {
    for (int i = 0;i<6;i++) {
        if (facilities[i]==1) {
            return true;
        }
    }
    return false;
}
bool check_facility_valid(int *facilities,DistributeItem *distribute_item,int start_hour_index,int end_hour_index) {
    bool facility_valid = false;
    int count = 0;
    int facilities_list[6];
    for (int i = 0;i<6;i++) {
        facilities_list[i] = 0;
    }
    bool next = true;
    for (int i = 0;i<6;i++) {
        if (facilities[i]==1) {
            facilities_list[count++] = i;
        }
    }
    for (int i = 0;i<count;i++) {
        bool facility_available = false;
        int facility_idx = facilities_list[i];
        for (int j = 0;j<3;j++) {
            bool slot_available = true;
            for (int k = start_hour_index;k<end_hour_index;k++) {
                if (distribute_item[k].facilities[facility_idx][j]==USED) {
                    slot_available = false;
                    break;
                }
            }
            if (slot_available) {
                facility_available = true;
                break;
            }
        }
        if (!facility_available) {
            return false;
        }
    }
    return true;

}
void set_parking_space_used(DistributeItem *distribute_item,int parking_index,int start_hour_index,int end_hour_index) {
    for (int i = start_hour_index;i<end_hour_index;i++) {
        distribute_item[i].parking_space[parking_index] = USED;
    }
}
void set_facility_used(int *facilities,DistributeItem *distribute_item,int start_hour_index,int end_hour_index) {
    for (int j = 0;j<6;j++) {
        if (facilities[j]==1) {
            for ( int k = 0;k<3;k++) {
                bool position_available = true;
                for (int i = start_hour_index;i<end_hour_index;i++) {
                    if (distribute_item[i].facilities[j][k] == USED) {
                        position_available = false;
                        break;
                    }
                }
                if (position_available) {
                    for (int i = start_hour_index; i < end_hour_index; i++) {
                        distribute_item[i].facilities[j][k] = USED;
                    }
                    return;
                }
            }
        }
    }
}
void fcfs_process() {
    while (1) {
        BookingMsg_for_pipe bookingMsg_for_pipe;
        DistributeItem distributeItems[7][24];//resource allocate for 24x7 hour
        init_pipe_value(distributeItems);//init data to be unused
        read(scheduler_to_fcfs[PIPE_READ],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe));
        BookingMsg *recbookingMsgs = bookingMsg_for_pipe.bookingMsgs; //store all booking value
        booking_count = bookingMsg_for_pipe.booking_count;
        for (int i = 0;i<booking_count;i++) {//calculate booking duration time
            int day_index = convert_date_to_day_index(recbookingMsgs[i].date);//sun-sat 0-6day
            int start_hour_index = convert_date_to_start_hour_index(recbookingMsgs[i].booking_time);//time slot index 0-23
            int end_hour_index = convert_date_to_end_hour_index(start_hour_index,recbookingMsgs[i].book_time_duration);
            int parking_index = INVALID_INDEX;
            bool facilities_valid = true;
            if (end_hour_index<24) {
                if (recbookingMsgs[i].parking_need==true) {

                    parking_index=check_parking_valid(distributeItems[day_index],start_hour_index,end_hour_index);
                }
                if (check_facilities_need(recbookingMsgs[i].facilities)&&parking_index!=NO_PARKING_SPACE) {
                    facilities_valid = check_facility_valid(recbookingMsgs[i].facilities,distributeItems[day_index],start_hour_index,end_hour_index);
                }
            }else {
                recbookingMsgs[i].status = FAIL;
                continue;
            }
            if (parking_index!=NO_PARKING_SPACE&&facilities_valid==true) {
                set_parking_space_used(distributeItems[day_index],parking_index,start_hour_index,end_hour_index);
                set_facility_used(recbookingMsgs[i].facilities,distributeItems[day_index],start_hour_index,end_hour_index);
                recbookingMsgs[i].status = SUCCESS;

            }else {

                recbookingMsgs[i].status = FAIL;
            }
        }
        memcpy(bookingMsg_for_pipe.bookingMsgs, recbookingMsgs, sizeof(recbookingMsgs));//copy recboookingMsg to bookingmsg
        write(fcfs_to_scheduler[PIPE_WRITE],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe)); //feedback to scheduler process

    }


}

void prio_process() {
    while (1) {
        BookingMsg_for_pipe bookingMsg_for_pipe;
        DistributeItem distributeItems[7][24];//resource allocate for 24x7 hour
        init_pipe_value(distributeItems);//init data to be unused
        read(scheduler_to_prio[PIPE_READ],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe));
        BookingMsg *recbookingMsgs = bookingMsg_for_pipe.bookingMsgs; //store all booking value
        booking_count = bookingMsg_for_pipe.booking_count;
        int prio_order[] = {2,1,0,3};// event->reservation->parking->essential
        for (int order = 0;order<4;order++) {
            for (int i = 0;i<booking_count;i++) {//calculate booking duration time
                if (recbookingMsgs[i].command_type == prio_order[order]) { //Handle booking in order
                    int day_index = convert_date_to_day_index(recbookingMsgs[i].date);//sun-sat 0-6day
                    int start_hour_index = convert_date_to_start_hour_index(recbookingMsgs[i].booking_time);//time slot index 0-23
                    int end_hour_index = convert_date_to_end_hour_index(start_hour_index,recbookingMsgs[i].book_time_duration);
                    int parking_index = INVALID_INDEX;
                    bool facilities_valid = true;
                    if (end_hour_index<24) {
                        if (recbookingMsgs[i].parking_need==true) {

                            parking_index=check_parking_valid(distributeItems[day_index],start_hour_index,end_hour_index);
                        }
                        if (check_facilities_need(recbookingMsgs[i].facilities)&&parking_index!=NO_PARKING_SPACE) {
                            facilities_valid = check_facility_valid(recbookingMsgs[i].facilities,distributeItems[day_index],start_hour_index,end_hour_index);
                        }
                    }else {
                        recbookingMsgs[i].status = FAIL;
                        continue;
                    }
                    if (parking_index!=NO_PARKING_SPACE&&facilities_valid==true) {
                        set_parking_space_used(distributeItems[day_index],parking_index,start_hour_index,end_hour_index);
                        set_facility_used(recbookingMsgs[i].facilities,distributeItems[day_index],start_hour_index,end_hour_index);
                        recbookingMsgs[i].status = SUCCESS;
                    }else {
                        recbookingMsgs[i].status = FAIL;
                    }
                }

            }
        }

        memcpy(bookingMsg_for_pipe.bookingMsgs, recbookingMsgs, sizeof(recbookingMsgs));
        write(prio_to_scheduler[PIPE_WRITE],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe)); //feedback to scheduler process

    }

}
void opti_process() {
    while (1) {
        BookingMsg_for_pipe bookingMsg_for_pipe;
        DistributeItem distributeItems[7][24];//resource allocate for 24x7 hour
        init_pipe_value(distributeItems);//init data to be unused
        read(scheduler_to_opti[PIPE_READ],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe));
        BookingMsg *recbookingMsgs = bookingMsg_for_pipe.bookingMsgs; //store all booking value
        booking_count = bookingMsg_for_pipe.booking_count;
        BookingMsg temp;
        int i,j;
        for (i = 0;i<booking_count-1;i++) {//sort booking time
            for (j = 0;j<booking_count-i-1;j++) {
                if (recbookingMsgs[j].book_time_duration<recbookingMsgs[j+1].book_time_duration) {
                    temp = recbookingMsgs[j];
                    recbookingMsgs[j] = recbookingMsgs[j+1];
                    recbookingMsgs[j+1] = temp;
                }
            }
        }
        for (int i = 0;i<booking_count;i++) {//calculate booking duration time
            int day_index = convert_date_to_day_index(recbookingMsgs[i].date);//sun-sat 0-6day
            int start_hour_index = convert_date_to_start_hour_index(recbookingMsgs[i].booking_time);//time slot index 0-23
            int end_hour_index = convert_date_to_end_hour_index(start_hour_index,recbookingMsgs[i].book_time_duration);
            int parking_index = INVALID_INDEX;
            bool facilities_valid = true;
            if (end_hour_index<24) {
                if (recbookingMsgs[i].parking_need==true) {

                    parking_index=check_parking_valid(distributeItems[day_index],start_hour_index,end_hour_index);
                }
                if (check_facilities_need(recbookingMsgs[i].facilities)&&parking_index!=NO_PARKING_SPACE) {
                    facilities_valid = check_facility_valid(recbookingMsgs[i].facilities,distributeItems[day_index],start_hour_index,end_hour_index);
                }
            }else {
                recbookingMsgs[i].status = FAIL;
                continue;
            }
            if (parking_index!=NO_PARKING_SPACE&&facilities_valid==true) {
                set_parking_space_used(distributeItems[day_index],parking_index,start_hour_index,end_hour_index);
                set_facility_used(recbookingMsgs[i].facilities,distributeItems[day_index],start_hour_index,end_hour_index);
                recbookingMsgs[i].status = SUCCESS;

            }else {

                recbookingMsgs[i].status = FAIL;
            }
        }
        memcpy(bookingMsg_for_pipe.bookingMsgs, recbookingMsgs, sizeof(recbookingMsgs));//copy recboookingMsg to bookingmsg
        write(opti_to_scheduler[PIPE_WRITE],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe)); //feedback to scheduler process
    }
}

void scheduler_process() {
    int status;
    BookingMsg_for_pipe bookingMsg_fcfs_saved;
    BookingMsg_for_pipe bookingMsg_prio_saved;
    BookingMsg_for_pipe bookingMsg_opti_saved;
    while(1) {
        BookingMsg_for_pipe bookingMsg_for_pipe;
        read(main_to_scheduler[PIPE_READ],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe));
        //processing scheduler function
        if (bookingMsg_for_pipe.command_type==FCFS) {
            write(scheduler_to_fcfs[PIPE_WRITE],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe));
            read(fcfs_to_scheduler[PIPE_READ],&bookingMsg_fcfs_saved,sizeof(bookingMsg_fcfs_saved));//receive fcfs result
        }else if (bookingMsg_for_pipe.command_type==PRIO) {
            write(scheduler_to_prio[PIPE_WRITE],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe));
            read(prio_to_scheduler[PIPE_READ],&bookingMsg_prio_saved,sizeof(bookingMsg_prio_saved));//receive prio result
        }else if (bookingMsg_for_pipe.command_type==OPTI) {
            write(scheduler_to_opti[PIPE_WRITE],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe));
            read(opti_to_scheduler[PIPE_READ],&bookingMsg_opti_saved,sizeof(bookingMsg_opti_saved));//receive opti result
        }else if (bookingMsg_for_pipe.command_type==ALL) {
            write(scheduler_to_fcfs[PIPE_WRITE],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe));
            read(fcfs_to_scheduler[PIPE_READ],&bookingMsg_fcfs_saved,sizeof(bookingMsg_fcfs_saved));//receive fcfs result
            write(scheduler_to_prio[PIPE_WRITE],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe));
            read(prio_to_scheduler[PIPE_READ],&bookingMsg_prio_saved,sizeof(bookingMsg_prio_saved));//receive prio result
            write(scheduler_to_opti[PIPE_WRITE],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe));
            read(opti_to_scheduler[PIPE_READ],&bookingMsg_opti_saved,sizeof(bookingMsg_opti_saved));//receive opti result
        }
        if (bookingMsg_for_pipe.command_type==FCFS) {
            write(scheduler_to_print[PIPE_WRITE],&bookingMsg_fcfs_saved,sizeof(bookingMsg_fcfs_saved)); //send to print process
            read(print_to_scheduler[PIPE_READ],&bookingMsg_fcfs_saved,sizeof(bookingMsg_fcfs_saved)); //receive from print process
        }else if (bookingMsg_for_pipe.command_type==PRIO) {
            write(scheduler_to_print[PIPE_WRITE],&bookingMsg_prio_saved,sizeof(bookingMsg_prio_saved)); //send to print process
            read(print_to_scheduler[PIPE_READ],&bookingMsg_prio_saved,sizeof(bookingMsg_prio_saved)); //receive from print process
        }else if (bookingMsg_for_pipe.command_type==OPTI) {
            write(scheduler_to_print[PIPE_WRITE],&bookingMsg_opti_saved,sizeof(bookingMsg_opti_saved)); //send to print process
            read(print_to_scheduler[PIPE_READ],&bookingMsg_opti_saved,sizeof(bookingMsg_opti_saved)); //receive from print process
        }else if (bookingMsg_for_pipe.command_type==ALL) {
            printf("*** Parking Booking Manager – Summary Report ***\n");
            printf("Performance:\n");
            printf("  For FCFS:\n");
            write(scheduler_to_print[PIPE_WRITE],&bookingMsg_fcfs_saved,sizeof(bookingMsg_fcfs_saved)); //send to print process
            read(print_to_scheduler[PIPE_READ],&bookingMsg_fcfs_saved,sizeof(bookingMsg_fcfs_saved)); //receive from print process
            printf("  For PRIO:\n");
            write(scheduler_to_print[PIPE_WRITE],&bookingMsg_prio_saved,sizeof(bookingMsg_prio_saved)); //send to print process
            read(print_to_scheduler[PIPE_READ],&bookingMsg_prio_saved,sizeof(bookingMsg_prio_saved)); //receive from print process
            printf("  For OPTI:\n");
            write(scheduler_to_print[PIPE_WRITE],&bookingMsg_opti_saved,sizeof(bookingMsg_opti_saved)); //send to print process
            read(print_to_scheduler[PIPE_READ],&bookingMsg_opti_saved,sizeof(bookingMsg_opti_saved)); //receive from print process
        }


        write(scheduler_to_main[PIPE_WRITE],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe)); //feedback to parent

    }
}

void print_process() {
    while(1) {
        BookingMsg_for_pipe bookingMsg_for_pipe;
        read(scheduler_to_print[PIPE_READ],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe));
        BookingMsg *recbookingMsgs = bookingMsg_for_pipe.bookingMsgs;
        int booking_count = bookingMsg_for_pipe.booking_count;

        char *command_type;
        if (bookingMsg_for_pipe.command_type==FCFS) {
            command_type = "FCFS";
        }else if (bookingMsg_for_pipe.command_type==PRIO) {
            command_type = "Priority";
        }else if (bookingMsg_for_pipe.command_type==OPTI) {
            command_type = "Optimized";
        }else if (bookingMsg_for_pipe.command_type==ALL) {
            command_type = "ALL";
        }
        char* member_names[] = {
            "member_A",
            "member_B",
            "member_C",
            "member_D",
            "member_E",
        };
        //processing print function
        //parking booking -summary
        if (strcmp(command_type,"ALL")==0) {
            int total_number_booking = booking_count;
            int total_number_assigned = 0;
            int total_number_rejected = 0;
            float total_parking_time_slot = 0.0; //24x7 = 168
            float total_facility_time_slot[6];  //one facility time slot 24x7x3 = 504
            for (int i = 0; i < 6; i++) {
                total_facility_time_slot[i] = 0.0;
            }
            for (int i = 0; i < booking_count; i++) {
                if (recbookingMsgs[i].status==SUCCESS) {
                    total_number_assigned++;
                    total_parking_time_slot = recbookingMsgs[i].book_time_duration+total_parking_time_slot;
                }else if (recbookingMsgs[i].status==FAIL) {
                    total_number_rejected++;
                }
            }
            for (int i = 0; i < booking_count; i++) {
                if (recbookingMsgs[i].status==SUCCESS) {
                    for (int j = 0; j < 6; j++) {
                        if (recbookingMsgs[i].facilities[j]==1) {
                            total_facility_time_slot[j] = total_facility_time_slot[j]+recbookingMsgs[i].book_time_duration;
                        }
                    }
                }
            }
            for (int i = 0; i < 6; i++) {
                if (total_facility_time_slot[i]!=0) {
                    total_facility_time_slot[i] = total_facility_time_slot[i]/504.0;
                }

            }
            float percent_total_number_assigned = total_number_assigned*1.0/total_number_booking;
            float percent_total_number_rejected = total_number_rejected*1.0/total_number_booking;
            float percent_total_parking_time = total_parking_time_slot*1.0/(7*24*MAX_PARKING_SPACE);
            printf("     Total Number of Bookings Received: %d\n",total_number_booking);
            printf("           Number of Bookings Assigned: %d (%.1f%%)\n",total_number_assigned,percent_total_number_assigned*100);
            printf("           Number of Bookings Rejected: %d (%.1f%%)\n",total_number_rejected,percent_total_number_rejected*100);
            printf("     Utilization of Time Slot:\n");
            printf("           Battery:   - %.1f%%\n",total_facility_time_slot[0]*100);
            printf("           Cable:   - %.1f%%\n",total_facility_time_slot[1]*100);
            printf("           Locker:   - %.1f%%\n",total_facility_time_slot[2]*100);
            printf("           Umbrella:   - %.1f%%\n",total_facility_time_slot[3]*100);
            printf("           Inflation:   - %.1f%%\n",total_facility_time_slot[4]*100);
            printf("           Valet:   - %.1f%%\n",total_facility_time_slot[5]*100);
            printf("           Parking:   - %.1f%%\n",percent_total_parking_time*100);
        }


        //parking booking -accepted
        if (strcmp(command_type,"ALL")!=0) {
            printf("*** Parking Booking - ACCEPTED / %s ***\n", command_type);

            for (int member = 0; member<5;member++) {
                printf("%s has the following bookings:\n", member_names[member]);
                printf("Date         Start     End     Type       Device\n");
                printf("===========================================================================\n");

                for (int i = 0; i < booking_count; i++) {
                    if (strcmp(recbookingMsgs[i].member_name, member_names[member]) != 0) {continue;}
                    if (recbookingMsgs[i].status==FAIL) {continue;}
                    int start_time = convert_date_to_start_hour_index(recbookingMsgs[i].booking_time);
                    int end_time = convert_date_to_end_hour_index(start_time,recbookingMsgs[i].book_time_duration);
                    char str_start_time[6];
                    char str_end_time[6];
                    char *booking_type;
                    if (recbookingMsgs[i].command_type == 0) {
                        booking_type = "Parking";
                    }else if (recbookingMsgs[i].command_type == 1) {
                        booking_type = "Reservation";
                    }else if (recbookingMsgs[i].command_type == 2) {
                        booking_type = "Event";
                    }else if (recbookingMsgs[i].command_type == 3) {
                        booking_type = "*";
                    }
                    sprintf(str_start_time, "%02d:00", start_time);
                    sprintf(str_end_time, "%02d:00", end_time);


                    printf("%s   %s     %s   %-12s ", recbookingMsgs[i].booking_date, str_start_time, str_end_time, booking_type);
                    bool facilities_valid = false;
                    char* facility_names[] = {
                        "BATTERY",
                        "CABLE",
                        "LOCKER",
                        "UMBRELLA",
                        "INFLATION",
                        "VALET"
                    };
                    for (int i2 = 0; i2 < 6; i2++) {
                        if (recbookingMsgs[i].facilities[i2]==1) {
                            facilities_valid = true;
                            printf("%-20s \n                                            ", facility_names[i2]);
                        }
                    }
                    if (facilities_valid==false) {
                        printf("- ");
                    }
                    printf("\n");
                }
                printf("\n");
            }
            printf("   - End -\n\n\n");

            //parking booking -REJECTED
            printf("*** Parking Booking - REJECTED / %s ***\n", command_type);
            for (int member = 0; member<5;member++) {
                printf("%s has the following bookings:\n\n", member_names[member]);
                printf("Date         Start     End     Type       Device\n");
                printf("===========================================================================\n");
                for (int i = 0; i < booking_count; i++) {
                    if (strcmp(recbookingMsgs[i].member_name, member_names[member]) != 0) {continue;}
                    if (recbookingMsgs[i].status==SUCCESS) {continue;}
                    int start_time = convert_date_to_start_hour_index(recbookingMsgs[i].booking_time);
                    int end_time = convert_date_to_end_hour_index(start_time,recbookingMsgs[i].book_time_duration);
                    char str_start_time[6];
                    char str_end_time[6];
                    char *booking_type;
                    if (recbookingMsgs[i].command_type == 0) {
                        booking_type = "Parking";
                    }else if (recbookingMsgs[i].command_type == 1) {
                        booking_type = "Reservation";
                    }else if (recbookingMsgs[i].command_type == 2) {
                        booking_type = "Event";
                    }else if (recbookingMsgs[i].command_type == 3) {
                        booking_type = "*";
                    }
                    sprintf(str_start_time, "%02d:00", start_time);
                    sprintf(str_end_time, "%02d:00", end_time);

                    printf("%s   %s     %s   %-12s ", recbookingMsgs[i].booking_date, str_start_time, str_end_time, booking_type);
                    bool facilities_valid = false;
                    char* facility_names[] = {
                        "BATTERY",
                        "CABLE",
                        "LOCKER",
                        "UMBRELLA",
                        "INFLATION",
                        "VALET"
                    };
                    for (int i2 = 0; i2 < 6; i2++) {
                        if (recbookingMsgs[i].facilities[i2]==1) {
                            facilities_valid = true;
                            printf("%-20s \n                                            ", facility_names[i2]);
                        }
                    }
                    if (facilities_valid==false) {
                        printf("- ");
                    }
                    printf("\n");
                }
                printf("\n");
            }
        }
        printf("   - End -\n\n\n");
        write(print_to_scheduler[PIPE_WRITE],&bookingMsg_for_pipe,sizeof(bookingMsg_for_pipe)); //send to scheduler process
    }
}
void fork_child_create() {
    pid_t pid;
    pid = fork();
    if (pid == 0 ) {
        // scheduler process
        close_scheduler_unused_pipe();
        scheduler_process();
        exit(0);
    }else {
        child_pids[0] = pid;
    }
    pid = fork();
    if (pid == 0 ) {
        // fcfs process
        close_fcfs_unused_pipe();
        fcfs_process();
        exit(0);
    }else {
        child_pids[1] = pid;
    }
    pid = fork();
    if (pid == 0 ) {
        // priority process
        close_prio_unused_pipe();
        prio_process();
        exit(0);
    }else {
        child_pids[2] = pid;
    }
    pid = fork();
    if (pid == 0 ) {
        // optimize process
        close_opti_unused_pipe();
        opti_process();
        exit(0);
    }else {
        child_pids[2] = pid;
    }
    pid = fork();
    if (pid == 0 ) {
        // printer process
        close_print_unused_pipe();
        print_process();
        exit(0);
    }else {
        child_pids[3] = pid;
    }
    close_main_unused_pipe();
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

    input_process();

}


int main(int argc, char* argv[]){

    init_system();

    return 0;


}