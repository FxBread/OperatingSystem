//
// Created by 梁嘉杰 on 26/3/2025.
//

#ifndef CLOSEPIPE_H
#define CLOSEPIPE_H
#include <unistd.h>
#define PIPE_READ 0
#define PIPE_WRITE 1
int main_to_scheduler[2];// input process to scheduler
int scheduler_to_main[2];
int scheduler_to_fcfs[2];
int scheduler_to_prio[2];
int scheduler_to_opti[2];
int fcfs_to_scheduler[2];
int prio_to_scheduler[2];
int opti_to_scheduler[2];
int scheduler_to_print[2];
int print_to_scheduler[2];
void close_main_unused_pipe() {
    close(main_to_scheduler[PIPE_READ]);
    // close(main_to_scheduler[PIPE_WRITE]);
    close(scheduler_to_main[PIPE_WRITE]);
    // close(scheduler_to_main[PIPE_READ]);
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
    close(print_to_scheduler[PIPE_WRITE]);
    close(print_to_scheduler[PIPE_READ]);
}
void close_scheduler_unused_pipe() {
    close(main_to_scheduler[PIPE_WRITE]);
    // close(main_to_scheduler[PIPE_READ]);
    // close(scheduler_to_main[PIPE_WRITE]);
    close(scheduler_to_main[PIPE_READ]);
    // close(scheduler_to_fcfs[PIPE_WRITE]);
    close(scheduler_to_fcfs[PIPE_READ]);
    // close(scheduler_to_prio[PIPE_WRITE]);
    close(scheduler_to_prio[PIPE_READ]);
    // close(scheduler_to_opti[PIPE_WRITE]);
    close(scheduler_to_opti[PIPE_READ]);
    // close(scheduler_to_print[PIPE_WRITE]);
    close(scheduler_to_print[PIPE_READ]);
    close(fcfs_to_scheduler[PIPE_WRITE]);
    // close(fcfs_to_scheduler[PIPE_READ]);
    close(prio_to_scheduler[PIPE_WRITE]);
    // close(prio_to_scheduler[PIPE_READ]);
    close(opti_to_scheduler[PIPE_WRITE]);
    // close(opti_to_scheduler[PIPE_READ]);
    close(print_to_scheduler[PIPE_WRITE]);
    // close(print_to_scheduler[PIPE_READ]);
}

void close_fcfs_unused_pipe() {
    close(main_to_scheduler[PIPE_READ]);
    close(main_to_scheduler[PIPE_WRITE]);
    close(scheduler_to_main[PIPE_READ]);
    close(scheduler_to_main[PIPE_WRITE]);
    close(scheduler_to_fcfs[PIPE_WRITE]);
    // close(scheduler_to_fcfs[PIPE_READ]);
    close(scheduler_to_prio[PIPE_WRITE]);
    close(scheduler_to_prio[PIPE_READ]);
    close(scheduler_to_opti[PIPE_WRITE]);
    close(scheduler_to_opti[PIPE_READ]);
    close(scheduler_to_print[PIPE_WRITE]);
    close(scheduler_to_print[PIPE_READ]);
    // close(fcfs_to_scheduler[PIPE_WRITE]);
    close(fcfs_to_scheduler[PIPE_READ]);
    close(prio_to_scheduler[PIPE_WRITE]);
    close(prio_to_scheduler[PIPE_READ]);
    close(opti_to_scheduler[PIPE_WRITE]);
    close(opti_to_scheduler[PIPE_READ]);
    close(print_to_scheduler[PIPE_WRITE]);
    close(print_to_scheduler[PIPE_READ]);
}
void close_opti_unused_pipe() {
    close(main_to_scheduler[PIPE_READ]);
    close(main_to_scheduler[PIPE_WRITE]);
    close(scheduler_to_main[PIPE_READ]);
    close(scheduler_to_main[PIPE_WRITE]);
    close(scheduler_to_fcfs[PIPE_WRITE]);
    close(scheduler_to_fcfs[PIPE_READ]);
    close(scheduler_to_prio[PIPE_WRITE]);
    close(scheduler_to_prio[PIPE_READ]);
    close(scheduler_to_opti[PIPE_WRITE]);
    // close(scheduler_to_opti[PIPE_READ]);
    close(scheduler_to_print[PIPE_WRITE]);
    close(scheduler_to_print[PIPE_READ]);
    close(fcfs_to_scheduler[PIPE_WRITE]);
    close(fcfs_to_scheduler[PIPE_READ]);
    close(prio_to_scheduler[PIPE_WRITE]);
    close(prio_to_scheduler[PIPE_READ]);
    // close(opti_to_scheduler[PIPE_WRITE]);
    close(opti_to_scheduler[PIPE_READ]);
    close(print_to_scheduler[PIPE_WRITE]);
    close(print_to_scheduler[PIPE_READ]);
}
void close_prio_unused_pipe() {
    close(main_to_scheduler[PIPE_READ]);
    close(main_to_scheduler[PIPE_WRITE]);
    close(scheduler_to_main[PIPE_READ]);
    close(scheduler_to_main[PIPE_WRITE]);
    close(scheduler_to_fcfs[PIPE_WRITE]);
    close(scheduler_to_fcfs[PIPE_READ]);
    close(scheduler_to_prio[PIPE_WRITE]);
    // close(scheduler_to_prio[PIPE_READ]);
    close(scheduler_to_opti[PIPE_WRITE]);
    close(scheduler_to_opti[PIPE_READ]);
    close(scheduler_to_print[PIPE_WRITE]);
    close(scheduler_to_print[PIPE_READ]);
    close(fcfs_to_scheduler[PIPE_WRITE]);
    close(fcfs_to_scheduler[PIPE_READ]);
    // close(prio_to_scheduler[PIPE_WRITE]);
    close(prio_to_scheduler[PIPE_READ]);
    close(opti_to_scheduler[PIPE_WRITE]);
    close(opti_to_scheduler[PIPE_READ]);
    close(print_to_scheduler[PIPE_WRITE]);
    close(print_to_scheduler[PIPE_READ]);
}
void close_print_unused_pipe() {
    close(main_to_scheduler[PIPE_READ]);
    close(main_to_scheduler[PIPE_WRITE]);
    close(scheduler_to_main[PIPE_READ]);
    close(scheduler_to_main[PIPE_WRITE]);
    close(scheduler_to_fcfs[PIPE_WRITE]);
    close(scheduler_to_fcfs[PIPE_READ]);
    close(scheduler_to_prio[PIPE_WRITE]);
    close(scheduler_to_prio[PIPE_READ]);
    close(scheduler_to_opti[PIPE_WRITE]);
    close(scheduler_to_opti[PIPE_READ]);
    close(scheduler_to_print[PIPE_WRITE]);
    // close(scheduler_to_print[PIPE_READ]);
    close(fcfs_to_scheduler[PIPE_WRITE]);
    close(fcfs_to_scheduler[PIPE_READ]);
    close(prio_to_scheduler[PIPE_WRITE]);
    close(prio_to_scheduler[PIPE_READ]);
    close(opti_to_scheduler[PIPE_WRITE]);
    close(opti_to_scheduler[PIPE_READ]);
    // close(print_to_scheduler[PIPE_WRITE]);
    close(print_to_scheduler[PIPE_READ]);
}
#endif //CLOSEPIPE_H
