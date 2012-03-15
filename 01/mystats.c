#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h> /* For getopt_long() */
#include <time.h> /* Needed to calculate CPU ticks per second */
#include <unistd.h> /* For sysconf() */
#include <sys/types.h>
#include <sys/wait.h> /* For sleep() */
#include <sys/dir.h> /* For opendir() and readdir() */

/* Function declarations. */
void get_load_average(int, int);
void get_memory_usage();
void get_system_info();
void get_uptime();
void get_cpu_time();
void get_context_switches();
void get_processes();
void get_disk_requests();
void get_kernel_version();
void get_cpu_info();
void get_uptime();

int main(int argc, char **argv) {
    /* This is where all the commandline options are initialized. */
    static struct option long_options[] = {
        { "load", required_argument, 0, 'l' },
        { "memory", no_argument, 0, 'm' },
        { "system", no_argument, 0, 's' },
    };

    int option_index = 0;
    int args;

    get_cpu_info();
    get_kernel_version();
    get_uptime();

    /* Executes based on command line arguments. */
    while((args = getopt_long(argc, argv, "lms", long_options, &option_index)) != -1) {
        switch(args) {
            case 'c':
                get_system_info();
                break;
            case 'h':
                printf("Usage: mystats [OPTION]...\n\n");
                printf("-m or --memory: Display the amount of memory configured into the computer and how much memory is available.");
                printf("-l or --load: Display the system's load average. Takes two integer arguments for how often and over a period.");
                printf("-s or --system: Displays extra system statistics.\n");
                break;
            case 'l':
                if(argc == 4) {
                    get_load_average(atoi(argv[2]), atoi(argv[3]));
                }
                else {
                    perror("Not enough arguments");
                    exit(-1);
                }
                break;
            case 'm':
                get_memory_usage();
                break;
            case 's':
                get_system_info();
                break;
            default:
                printf("Incorrect option!\n");
                break;
        }
    }

    return 0;
}

/* Prints out a bar graph of the system's load averages. */
void get_load_average(int rate, int period) {
    char buffer[32];
    int bytes_read;
    int i;
    int times;

    times = (period/rate);

    double one[times];
    double five[times];
    double fifteen[times];

    printf("Showing the system's load average at a rate of %d seconds during a period of %d seconds:\n", rate, period);

    for(i = rate; i < times; i += rate) {
        int load_average;
        int element;
        char lavg1[4], lavg2[4], lavg3[4];

        load_average = open("/proc/loadavg", O_RDONLY);
        memset(buffer, 0, sizeof(buffer)); bytes_read = read(load_average, buffer, sizeof(buffer));

        if(load_average < 0) {
            perror("Load average read error");
            exit(-1);
        }

        write(1, buffer, bytes_read);

        element = (i/rate)-1;
        sscanf(buffer, "%s %s %s", lavg1, lavg2, lavg3);

        /* Puts each respective load average in its proper place. */
        one[element] = atof(lavg1);
        five[element] = atof(lavg2);
        fifteen[element] = atof(lavg3);

        close(load_average);
        sleep(rate);
    }

    printf("\nBar graph of one minute statistics:\n");

    for(i = 0; i <= sizeof(one)/sizeof(double); i++) {
        int stars;
        int j;
        char star = '*';

        stars = (long)(one[i]*100);

        if(star > 0) {
            for(j = 0; j < stars; j++) {
                printf("%c", star);
            }

            printf("\n");
        }
        else {
            printf("*\n");
        }
    }

    printf("Bar graph of five minute statistics:\n");

    for(i = 0; i <= sizeof(five)/sizeof(double); i++) {
        int stars;
        int j;
        char star = '*';

        stars = (long)(five[i]*100);

        if(star > 0) {
            for(j = 0; j < stars; j++) {
                printf("%c", star);
            }

            printf("\n");
        }
        else {
            printf("*\n");
        }
    }

    printf("\nBar graph of fifteen minute statistics:\n");

    for(i = 0; i <= sizeof(fifteen)/sizeof(double); i++) {
        int stars;
        int j;
        char star = '*';

        stars = (long)(fifteen[i]*100);

        if(star > 0) {
            for(j = 0; j < stars; j++) {
                printf("%c", star);
            }

            printf("\n");
        }
        else {
            printf("*\n");
        }
    }
}

/* Gets the system's memory usage and displays on screen. */
void get_memory_usage() {
    char buffer[1024];
    int memory_usage;

    memory_usage = open("/proc/meminfo", O_RDONLY);

    if(memory_usage < 0) {
        perror("User mode time error");
        exit(-1);
    }

    memset(buffer, 0, sizeof(buffer));
    read(memory_usage, buffer, sizeof(buffer));

    char mem_total[32], mem_total1[16], mem_total2[16], mem_total3[16];
    char mem_free[32], mem_free1[16], mem_free2[16], mem_free3[16];

    memset(mem_total, 0, sizeof(mem_total));
    memset(mem_free, 0, sizeof(mem_free));

    sscanf(buffer, "%s %s %s\n%s %s %s", mem_total1, mem_total2, mem_total3, mem_free1, mem_free2, mem_free3);
    sprintf(mem_total, "%s %s %s\n", mem_total1, mem_total2, mem_total3);
    sprintf(mem_free, "%s %s %s\n", mem_free1, mem_free2, mem_free3);

    printf("The amount of memory configured into the computer:\n");
    write(1, mem_total, sizeof(mem_total));

    printf("\nThe amount of memory currently available:\n");
    write(1, mem_free, sizeof(mem_free));
}

/* Accesses all of the extra system info to be printed. */
void get_system_info() {
    printf("The amount of time that the CPU has spent in user mode, in system mode, and idle in seconds:\n");
    get_cpu_time();

    printf("The number of context switches that the kernel has performed:\n");
    get_context_switches();

    printf("The number of processes that have been created since the system was booted:\n");
    get_processes();

    printf("The number of disk requests made on the system.\n");
    get_disk_requests();

    get_uptime();
    printf("\n");
}

/* Gets the CPU user, system, and idle and displays in seconds. */
void get_cpu_time() {
    char buffer[2048];
    char output[16];
    char user_time[8];
    char nice_user_time[8];
    char system_mode_time[8];
    char idle_time[8];
    int cpu_time;
    int utime;
    int stime;
    int itime;
    int cticks;

    cpu_time = open("/proc/stat", O_RDONLY);

    if(cpu_time < 0) {
        perror("User mode time error");
        exit(-1);
    }

    memset(buffer, 0, sizeof(buffer));
    read(cpu_time, buffer, sizeof(buffer));
    sscanf(buffer, "cpu %s %s %s %s", user_time, nice_user_time, system_mode_time, idle_time);

    /* From man page of times and time */
    cticks = sysconf(_SC_CLK_TCK);
    utime = (int)strtol(user_time, NULL, 10)/cticks;
    stime = (int)strtol(system_mode_time, NULL, 10)/cticks;
    itime = (int)strtol(idle_time, NULL, 10)/cticks;

    memset(output, 0, sizeof(output));
    sprintf(output, "%d %d %d\n\n", utime, stime, itime);
    write(1, output, sizeof(output));

    close(cpu_time);
}

/* Gets the context switches and displays on screen. */
void get_context_switches() {
    char buffer[2048];
    char context_switches[16];
    char *token;
    int ctxt;
    int bytes_read;

    ctxt = open("/proc/stat", O_RDONLY);

    if(ctxt < 0) {
        perror("Get context switches error");
        exit(-1);
    }

    memset(buffer, 0, sizeof(buffer));
    memset(context_switches, 0, sizeof(context_switches));
    read(ctxt, buffer, sizeof(buffer));

    token = strtok(buffer, " \n");

    /* Goes through entire /proc/stat file and looks for context switches. */
    while(token != NULL) {
        /* Write to screen when context switches are found. */
        if(strcmp(token, "ctxt") == 0) {
            token = strtok(NULL, " \n");
            sscanf(token, "%s\n\n", context_switches);
            sprintf(context_switches, "%s\n\n", context_switches);
            write(1, context_switches, sizeof(context_switches));
        }

        token = strtok(NULL, " \n");
    }

    close(ctxt);
}

/* Get the number of processes that have been created since boot and display. */
void get_processes() {
    char buffer[2048];
    char processes[20];
    char *token;
    int procs;

    procs = open("/proc/stat", O_RDONLY);

    if(procs < 0) {
        perror("User mode time error");
        exit(-1);
    }

    memset(buffer, 0, sizeof(buffer));
    memset(processes, 0, sizeof(processes));
    read(procs, buffer, sizeof(buffer));

    token = strtok(buffer, " \n");

    /* Goes through tokenized /proc/stat file. */
    while(token != NULL) {
        /* When the number of processes have been found, write to screen. */
        if(strcmp(token, "processes") == 0) {
            token = strtok(NULL, " ");
            sscanf(token, "%s", processes);
            sprintf(processes, "%s\n\n", processes);
            write(1, processes, sizeof(processes));
        }

        token = strtok(NULL, " \n");
    }

    close(procs);
}

/* Gets the number of disk requests made on the system and displays. */
void get_disk_requests() {
    char buffer[2048];
    char* line;
    char* token;
    char disk_name[8];
    char disk_reads[16];
    char disk_writes[16];
    char disk_rw[16];
    int d_reads;
    int d_writes;
    int d_rw;
    int bytes_read;
    int diskstats;

    diskstats = open("/proc/diskstats", O_RDONLY);

    if(diskstats < 0) {
        perror("Diskstats info read error");
        exit(-1);
    }

    memset(buffer, 0, sizeof(buffer));
    bytes_read = read(diskstats, buffer, sizeof(buffer));
    line = strtok(buffer, " \n");

    /* Goes through tokenized /proc/diskstats */
    while(line != NULL) {
        /* This finds a hard disk. */
        if(line[0] == 's' && line[1] == 'd') {
            memset(disk_name, 0, sizeof(disk_name));
            strcpy(disk_name, line);
            line = strtok(NULL, " \n");

            memset(disk_reads, 0, sizeof(disk_reads));
            strcpy(disk_reads, line);
            line = strtok(NULL, " \n");
            line = strtok(NULL, " \n");
            line = strtok(NULL, " \n");
            line = strtok(NULL, " \n");

            memset(disk_writes, 0, sizeof(disk_writes));
            strcpy(disk_writes, line);

            /* Add up all of the reads and writes as total number of requests. */
            d_reads = atoi(disk_reads);
            d_writes = atoi(disk_writes);
            d_rw = d_reads + d_writes;

            memset(disk_rw, 0, sizeof(disk_rw));
            sprintf(disk_rw, "%s: %d\n", disk_name, d_rw);

            write(1, disk_rw, sizeof(disk_rw));
        }

        line = strtok(NULL, " \n");
    }

    close(diskstats);
}

/* Gets and displays CPU type, model, and number of CPUs. */
void get_cpu_info() {
    char buffer[1024];
    char cpu_type[32];
    char cpu_model[64];
    char cpu_num[32];
    int bytes_read;
    int cpu_fd;
    char* token;
    int i;

    cpu_fd = open("/proc/cpuinfo", O_RDONLY);

    if(cpu_fd < 0) {
        perror("CPU info read error");
        exit(-1);
    }

    printf("CPU type, model and number of processors:\n");

    memset(buffer, 0, sizeof(buffer));
    bytes_read = read(cpu_fd, buffer, sizeof(buffer));
    token = strtok(buffer, "\n");
    token = strtok(NULL, "\n");

    /* CPU type */
    memset(cpu_type, 0, sizeof(cpu_type));
    strcpy(cpu_type, token);
    sprintf(cpu_type, "%s\n", cpu_type);
    write(1, cpu_type, sizeof(cpu_type));
    token = strtok(NULL, "\n");
    token = strtok(NULL, "\n");
    token = strtok(NULL, "\n");

    /* CPU model */
    memset(cpu_model, 0, sizeof(cpu_model));
    strcpy(cpu_model, token);
    sprintf(cpu_model, "%s\n", cpu_model);
    write(1, cpu_model, sizeof(cpu_model));
    token = strtok(NULL, "\n");
    token = strtok(NULL, "\n");
    token = strtok(NULL, "\n");
    token = strtok(NULL, "\n");
    token = strtok(NULL, "\n");
    token = strtok(NULL, "\n");
    token = strtok(NULL, "\n");
    token = strtok(NULL, "\n");

    /* Number of CPUs */
    memset(cpu_num, 0, sizeof(cpu_num));
    strcpy(cpu_num, token);
    sprintf(cpu_num, "%s\n\n", cpu_num);
    write(1, cpu_num, sizeof(cpu_num));

    close(cpu_fd);
}

/* Gets kernel version and displays onto screen. */
void get_kernel_version() {
    char buffer[1024];
    int bytes_read;
    int kernel_ver_fd;

    kernel_ver_fd = open("/proc/version", O_RDONLY);

    if(kernel_ver_fd < 0) {
        perror("Kernel info read error");
        exit(-1);
    }

    printf("Kernel version:\n");
    bytes_read = read(kernel_ver_fd, buffer, sizeof(buffer));
    write(1, buffer, bytes_read);

    close(kernel_ver_fd);
}

/* Gets system uptime and display on screen. */
void get_uptime() {
    char buffer[1024];
    int bytes_read;
    int uptime_fd;

    uptime_fd = open("/proc/uptime", O_RDONLY);

    if(uptime_fd < 0) {
        perror("Uptime info read error");
        exit(-1);
    }

    printf("\nAmount of time since the system was last booted (in dd:hh:mm:ss format):\n");
    memset(buffer, 0, sizeof(buffer));
    bytes_read = read(uptime_fd, buffer, sizeof(buffer));
    sscanf(buffer, "%s", buffer);

    /* Various time conversions to get the proper uptime. */
    int time = atoi(buffer);
    int days = time/86400;
    int hours = time/3600%24;
    int minutes = time/60%60;
    int seconds = time%60;
    char time_output[11];

    memset(buffer, 0, sizeof(buffer));
    sprintf(time_output, "%02d:%02d:%02d:%02d\n", days, hours, minutes, seconds);
    write(1, time_output, sizeof(time_output));

    printf("\n\n");

    close(uptime_fd);
}
