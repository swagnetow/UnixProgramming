#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

void get_current_time();
void timeout();

int main() {
    int i, n;
    struct itimerval value, ovalue;

    if((signal(SIGALRM, timeout)) == SIG_ERR) {
        printf("Error in setting signal handler.\n");
        exit(0);
    }

	timerclear(&value.it_interval);
	timerclear(&value.it_value);
	timerclear(&ovalue.it_interval);
	timerclear(&ovalue.it_value);

    n = setitimer(ITIMER_REAL,&value,&ovalue);

	if(n < 0 ) {
		printf("Error in clearing timer\n");
		exit(0);
	}

	value.it_value.tv_sec = 1;

    n = setitimer(ITIMER_REAL,&value,&ovalue);

	if(n < 0 ) {
		printf("Error in clearing timer\n");
		exit(0);
	}

    printf("%d\n", i+1);

    while(1);

    return 0;
}

void get_current_time() {
    char clock[32];
    int clock_fd;
    int current_time;
    struct timeval time;

    clock_fd = open("/proc/myclock", O_RDONLY);

    gettimeofday(&time, NULL);

    if(clock_fd < 0) {
        perror("Bad read file open");
        exit(-1);
    }

    memset(clock, 0, sizeof(clock));
    read(clock_fd, clock, sizeof(clock));

    printf("Time according to gettimeofday():\n%s\n", ctime(&time.tv_sec));
    printf("%s \n", clock);

    char* ms;

    ms = strtok(clock, " ");
    ms = strtok(NULL, " ");

    current_time = atoi(clock);
    int hours = current_time/3600%24;
    int minutes = current_time/60%60;
    int seconds = current_time%60;
    int microseconds = atoi(ms);

    printf("%02i:%02i:%02i:%i UTC\n", hours, minutes, seconds, microseconds);

    close(clock_fd);
}

void timeout(int arg) {
    exit(0);
}
