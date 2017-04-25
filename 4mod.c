#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <wait.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define PROC_COUNT (9)
#define STARTING_PROC_ID (1)
#define MAX_CHILDS_COUNT (3)
#define MAX_USR_COUNT (101)

char* filename = "pids";
struct timeval time;

int pidFirstProc=0;
int countGetSignals=0;

struct proccesInfo
{
    int proc_id;
    int pid_proc;
    /* data */
};

const unsigned char CHILDS_COUNT[PROC_COUNT] =
{
	/*  0  1  2  3  4  5  6  7  8  */
    1, 1, 3, 1, 1, 1, 0, 0, 0	
};

const unsigned char GET_SIGNAL[PROC_COUNT] =
{
/*  0  1  2  3  4  5  6  7  8  */
    0, 2, 3, 0, 2, 2, 1, 1, 1
};

const unsigned char CHILDS_IDS[PROC_COUNT][MAX_CHILDS_COUNT] =
{
    {1},        /* 0 */
    {2},  		/* 1 */
    {3,4,5},     /* 2 */
    {7},        /* 3 */
    {6},        /* 4 */
    {8},        /* 5 */
    {0},        /* 6 */
    {0},        /* 7 */
    {0}         /* 8 */
};

/* Group types:
 *
 * 0 = pid;
 * 1 = parent's pgid
 * 2 = previous child group
 */
const unsigned char GROUP_TYPE[PROC_COUNT] =
{
/*  0  1  2  3  4  5  6  7  8  */
    0, 0, 0, 0, 0, 2, 0, 0, 0
};

/* Whome to send signal:
 *
 * 0 = none
 * positive = pid
 * negative = processes group
 *
 * f/ex: "-x" means, that signal is sent to the group, process with pid = x is member of
 */
const char SEND_TO[PROC_COUNT] =
{
/*  0,  1,  2,  3,  4,  5,  6,  7,  8  */
    0,  6,  1,  0,  8,  2,  7, -4,  2
};

const int SEND_SIGNALS[PROC_COUNT] =
{
    0,          /* 0 */
    SIGUSR1,    /* 1 */
    SIGUSR2,    /* 2 */
    0,          /* 3 */
    SIGUSR1,    /* 4 */
    SIGUSR1,    /* 5 */
    SIGUSR1,    /* 6 */
    SIGUSR2,    /* 7 */
    SIGUSR2     /* 8 */
};

const char RECV_SIGNALS_COUNT[2][PROC_COUNT] =
{
/*    0, 1, 2, 3, 4, 5, 6, 7, 8  */
    { 0, 0, 1, 0, 0, 0, 1, 1, 1 },  /* SIGUSR1 */
    { 0, 1, 1, 0, 1, 1, 0, 0, 0 }   /* SIGUSR2 */
};

void sig_handler(int signum);
void set_sig_handler(void (*handler)(void *), int sig_no, int flags);


char *exec_name = NULL;
void print_error_exit(const char *s_name, const char *msg, const int proc_num);

int proc_id = 0;
void forker(int curr_number, int childs_count);

void kill_wait_for_children();
void wait_for_children();

pid_t process_id_array[PROC_COUNT];

int main(int argc, char *argv[])
{
    exec_name = basename(argv[0]);
		
	FILE* f = fopen(filename, "w");
  	if(!f){
        print_error_exit(exec_name, "Error: can't open file.", 0);
    }
    fclose(f);
	
	int i = 0;
	
    for(i=0; i<PROC_COUNT; ++i){
        process_id_array[i] = 0;
    }
	
    forker(0, CHILDS_COUNT[0]);          // create processes tree
    set_sig_handler(&kill_wait_for_children, SIGTERM, 0);
    if (proc_id == 0) {                  // main process waits
        wait_for_children();
        return 0;
    }
	
	 if (proc_id==8){
     sleep(10);   
    }

    on_exit(&wait_for_children, NULL);

    if (proc_id == STARTING_PROC_ID) {                  // starter waits for all pids to be available
        while(countGetSignals<PROC_COUNT-2){}
		
        set_sig_handler(&sig_handler, 0, 0);
        sig_handler(0); // start signal-flow

    } 
    while (1) {
        pause();  // start cycle
    }

    return 0;
}   /*  main   */


void sig_handler_start(int signum) {
    countGetSignals++;
}

void get_pid_id(){
    FILE *fo = fopen(filename,"rb");
    struct proccesInfo getStruct;
    int i = 0;
    while(i<PROC_COUNT-2){
        if(!fread(&getStruct,sizeof(getStruct),1,fo)){
            print_error_exit(exec_name, "Error: can't read from file.", getpid());
        }
        process_id_array[getStruct.proc_id] = getStruct.pid_proc;
        i++;
    }
    fclose(fo);
}

void setInfoInFile(int id, pid_t pid){
    FILE *fo = fopen(filename,"ab");
    struct proccesInfo getStruct;
    getStruct.proc_id = id;
    getStruct.pid_proc = pid;
    
    if(!fwrite(&getStruct,sizeof(getStruct),1,fo)){
        print_error_exit(exec_name, "Error: can't write to file.", getpid());
    }
    fflush(fo);   
    fclose(fo);
}

void print_error_exit(const char *s_name, const char *msg, const int proc_num) {
    fprintf(stderr, "%s: %s %d\n", s_name, msg, proc_num);
    fflush(stderr);

    process_id_array[proc_num] = -1;

    exit(1);
}   /*  print_error */


void wait_for_children() {
    int i = CHILDS_COUNT[proc_id];
    while (i > 0) {
        wait(NULL);
        --i;
    }
}   /*  wait_for_children  */


long long current_time() {
    gettimeofday(&time, NULL);

    return time.tv_usec/1000;
}   /*  current_time  */


/*                U1, U2  */
volatile int usr_recv[2] = {0, 0};

volatile int usr_amount[2][2] =
{
/*   r, s   */
    {0, 0}, /* SIGUSR1 */
    {0, 0}  /* SIGUSR2 */
};


void kill_wait_for_children() {
    int i = 0;
	get_pid_id();
    for (i = 0; i < CHILDS_COUNT[proc_id]; ++i) {
        kill(process_id_array[CHILDS_IDS[proc_id][i]], SIGTERM);
    }

    wait_for_children();

    if (proc_id != 0){

    printf("%d %d завершил работу после %d сигнала SIGUSR1 и %d сигнала SIGUSR2\n",
    getpid(), getppid(), usr_amount[0][1], usr_amount[1][1]);
    fflush(stdout);
}
    exit(0);
}   /*  kill_wait_for_children  */


void sig_handler(int signum) {

    if (signum == SIGUSR1) {
        signum = 0;
    } else if (signum == SIGUSR2) {
        signum = 1;
    } else {
        signum = -1;
    }

    if (signum != -1) {
        ++usr_amount[signum][0];
		++usr_recv[signum];
		
        printf("%d %d %d получил %s%d текущее время %lld\n", proc_id, getpid(), getppid(), "SIGUSR", signum+1, current_time() );
		
		if (proc_id == 1) {
        if (usr_amount[0][0] + usr_amount[1][0] == MAX_USR_COUNT) {
            kill_wait_for_children();
        }
		}
	if (! ( (usr_recv[0] == RECV_SIGNALS_COUNT[0][proc_id]) &&
        (usr_recv[1] == RECV_SIGNALS_COUNT[1][proc_id]) ) ) {
 
            return;
        }
       usr_recv[0] = usr_recv[1] = 0;
	}
  char to = SEND_TO[proc_id];

    if (to != 0) {
        signum = ( (SEND_SIGNALS[proc_id] == SIGUSR1) ? 1 : 2);
        ++usr_amount[signum-1][1];
		printf("%d %d %d послал %s%d текущее время %lld\n", proc_id, getpid(), getppid(),
           "SIGUSR", signum, current_time() );
    	fflush(stdout);
    }

    if (to > 0) {
        while(process_id_array[to]==0){get_pid_id();}
        kill(process_id_array[to], SEND_SIGNALS[proc_id]);
    } else if (to < 0) {
        while(process_id_array[-to]==0){get_pid_id();}
        kill(-getpgid(process_id_array[-to]), SEND_SIGNALS[proc_id]);
    } else {
        return;
    }
}   /*  handler  */


void set_sig_handler(void (*handler)(void *), int sig_no, int flags) {
    struct sigaction sa, oldsa;             // set sighandler

    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGUSR1);
    sigaddset(&block_mask, SIGUSR2);

    sa.sa_mask = block_mask;
    sa.sa_flags = flags;
    sa.sa_handler = handler;

    if (sig_no != 0) {
        sigaction(sig_no, &sa, &oldsa);
        return;
    }
	
	
	if ((GET_SIGNAL[proc_id]&1)){
		if (sigaction(SIGUSR1, &sa, &oldsa) == -1) {
             print_error_exit(exec_name, "Нельзя задать sighandler!", proc_id);
        }
	}
	if ((GET_SIGNAL[proc_id]&2)){
		if (sigaction(SIGUSR2, &sa, &oldsa) == -1) {
             print_error_exit(exec_name, "Нельзя задать sighandler!", proc_id);
        }
	}
	
}   /*  set_sig_handler  */


void forker(int curr_number, int childs_count) {
    pid_t pid = 0;
    int i = 0;
    for (i = 0; i < childs_count; ++i) {
        int chld_id = CHILDS_IDS[curr_number][i];
        if ( (pid = fork() ) == -1) {
            print_error_exit(exec_name, "Нельзя задать fork!", chld_id);

        } else if (pid == 0) {  /*  child    */
            proc_id = chld_id;
			if (proc_id==1){
				pidFirstProc=getpid();
				set_sig_handler(&sig_handler_start,SIGUSR1,0);
			}
			else{
				set_sig_handler(&sig_handler,0,0);
			}
			setInfoInFile(proc_id,getpid());
			if(proc_id!=1){kill(pidFirstProc,SIGUSR1);}
			
            if (CHILDS_COUNT[proc_id] != 0) {
                forker(proc_id, CHILDS_COUNT[proc_id]);         // fork children
            }

            break;

        } else {    // pid != 0 (=> parent)
            static int prev_chld_grp = 0;

            int grp_type = GROUP_TYPE[chld_id];

            if (grp_type == 0) {
                if (setpgid(pid, pid) == -1) {
                    print_error_exit(exec_name, "Нельзя задать группу", chld_id);
                } else {
                    prev_chld_grp = pid;
                }

            }  else if (grp_type == 2) {
                if (setpgid(pid, prev_chld_grp) == -1) {
                    print_error_exit(exec_name, "Нельзя задать группу", chld_id);
                }
            }

        }   // parent branch

    }   // for

} /*  forker  */
