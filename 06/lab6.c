/* Modified demo code from Dr. Murphy's 615 website */
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

void get_current_time(int);
int set_timeout_delay(int);
void receive_data(int, siginfo_t *, void *);
void timeout();

/* Global variable to set the timeout. */
int done;
int signals[5] = { 40, 41, 42, 43, 44 };

int main(int argc, char** argv) {
    int i;
    int n;
    struct itimerval value;
    struct itimerval ovalue;
    struct sigaction sig[5];

    for(i = 0; i < 5; i++) {
        sig[i].sa_sigaction = receive_data;
        sig[i].sa_flags = SA_SIGINFO;
        sigaction(signals[i], &sig[i], NULL);
        set_timeout_delay(atoi(argv[i+1]));
    }

    /* Executes every second for 5 minutes. */
    for(i = 1; i <= 300; i++) {
        if((signal(SIGALRM, timeout)) == SIG_ERR) {
            printf("Error in setting signal handler.\n");
            exit(0);
        }

        timerclear(&value.it_interval);
        timerclear(&value.it_value);
        timerclear(&ovalue.it_interval);
        timerclear(&ovalue.it_value);

        n = setitimer(ITIMER_REAL, &value, &ovalue);

        if(n < 0 ) {
            printf("Error in clearing timer\n");
            exit(0);
        }

        /* Sets the alarm to wake up in one second. */
        value.it_value.tv_sec = 1;

        n = setitimer(ITIMER_REAL, &value, &ovalue);

        if(n < 0 ) {
            printf("Error in clearing timer\n");
            exit(0);
        }

        get_current_time(i);

        done = 0;

        while(!done);
    }

    return 0;
}

void get_current_time(int num) {
    char clock_procfs_buffer[32];
    char clock_sysfs_buffer[32];
    char* date;
    char* day;
    char* day_num;
    char* month;
    char* ms;
    char* year;
    int clock_procfs;
    int clock_sysfs;
    int current_time_procfs;
    int current_time_sysfs;
    int hours;
    int microseconds;
    int minutes;
    int seconds;
    struct timeval time;

    clock_procfs = open("/proc/myclock", O_RDONLY);
    clock_sysfs = open("/sys/kernel/myclock2/myclock2", O_RDONLY);

    gettimeofday(&time, NULL);

    if(clock_procfs < 0) {
        perror("Bad read file open of /proc/myclock");
        exit(-1);
    }

    if(clock_sysfs < 0) {
        perror("Bad read file open of /sys/kernel/myclock2/myclock2");
        exit(-1);
    }

    memset(clock_procfs_buffer, 0, sizeof(clock_procfs_buffer));
    memset(clock_sysfs_buffer, 0, sizeof(clock_sysfs_buffer));

    read(clock_procfs, clock_procfs_buffer, sizeof(clock_procfs_buffer));
    read(clock_sysfs, clock_sysfs_buffer, sizeof(clock_sysfs_buffer));

    date = ctime(&time.tv_sec);

    day = strtok(date, " ");
    month = strtok(NULL, " ");
    day_num = strtok(NULL, " ");
    strtok(NULL, " ");
    year = strtok(NULL, " ");

    ms = strtok(clock_procfs_buffer, " ");
    ms = strtok(NULL, " ");

    current_time_procfs = atoi(clock_procfs_buffer);
    current_time_sysfs = atoi(clock_sysfs_buffer);

    /* procfs */
    hours = (current_time_procfs-25200)/3600%24; /* Changes the current hour to PST. */
    minutes = current_time_procfs/60%60;
    seconds = current_time_procfs%60;
    microseconds = atoi(ms);

    printf("%s %s %s %d:%02d:%02d:%d %s", day, month, day_num, hours, minutes, seconds, microseconds, year);

    /* sysfs */
    ms = strtok(clock_sysfs_buffer, " ");
    ms = strtok(NULL, " ");

    hours = (current_time_sysfs-25200)/3600%24; /* Changes the current hour to PST. */
    minutes = current_time_sysfs/60%60;
    seconds = current_time_sysfs%60;
    microseconds = atoi(ms);

    printf("%s %s %s %d:%02d:%02d:%d %s\n", day, month, day_num, hours, minutes, seconds, microseconds, year);

    /* Prints the current time to check the time drift. */
    if(num == 300) {
        printf("%s", ctime(&time.tv_sec));
    }

    close(clock_procfs);
    close(clock_sysfs);
}

int set_timeout_delay(int delay) {
    char buffer[32];
    int delay_open;
    int delay_write;
    int delay_read;

    delay_open = open("/sys/kernel/delay/delay", O_RDWR);

    if(delay_open < 0) {
        perror("Error opening delay file");
        return -1;
    }

    sprintf(buffer, "%i", delay);

    delay_write = write(delay_open, buffer, strlen(buffer) + 1);

    if(delay_write < 0) {
        perror("Error writing to delay file");
        return -1;
    }

    delay_read = read(delay_open, &buffer[0], sizeof(buffer));

    if(delay_read < 0) {
        perror("Error reading delay file");
        return -1;
    }

    return 0;
}

void receive_data(int n, siginfo_t* info, void* unused) {
    printf("Alarm going off at %i seconds!\n\n", info->si_int);
}

/* Timeout for signal processes. */
void timeout(int arg) {
    done = 1;
}
