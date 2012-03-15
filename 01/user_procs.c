#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h> /* For getopt_long() */
#include <time.h> /* Needed to calculate CPU ticks per second */
#include <unistd.h> /* For sysconf() */
#include <sys/types.h>
#include <sys/dir.h> /* For opendir() and readdir() */

void get_user_proc(char*, int);
int get_user_by_uid(char*);

int main(int argc, char **argv) {
    /* This is where all the commandline options are initialized. */
    static struct option long_options[] = {
        { "long", required_argument, 0, 'l' },
        { "user", required_argument, 0, 'u' },
    };

    int option_index = 0;
    int args;

    /* Executes based on command line arguments. */
    while((args = getopt_long(argc, argv, "hul", long_options, &option_index)) != -1) {
        switch(args) {
            case 'h':
                printf("Usage: mystats [OPTION]...\n\n");
                printf("-l or --long: Displays the complete command line argument.");
                printf("-u or --user: Displays the Process ID and command name for all processes being run by the user named in the program’s command-line argument.\n");
                break;
            case 'l':
                if(argc == 3) {
                    get_user_proc(argv[2], 0);
                }
                else {
                    perror("Not enough arguments");
                    exit(-1);
                }
                break;
            case 'u':
                if(argc == 3) {
                    get_user_proc(argv[2], 1);
                }
                else {
                    perror("Not enough arguments");
                    exit(-1);
                }
                break;
            default:
                printf("Incorrect option!\n");
                break;
        }
    }

    return 0;
}

/* Edited code from http://heather.cs.ucdavis.edu/~matloff/unix.html */
void get_user_proc(char* username, int args) {
    struct direct* dir_entry;
    DIR* dir;
    char buffer[2048];
    char parse_line[64];
    char path[256];
    char pid_char[64];
    char proc_name[256];
    char proc_args[128];
    char user_proc_pid[256];
    char cmdline[256];
    char* line;
    char* token;
    int cmdline_file;
    int get_uid;
    int pid_file;
    int pid;
    int uid;

    /* Opens directory search through. */
    dir = opendir("/proc/");
    uid = get_user_by_uid(username);

    if(args == 0) {
        printf("Name, PID, Arguments\n");
    }
    else {
        printf("Name, PID\n");
    }

    while(1) {
        dir_entry = readdir(dir);

        if(dir_entry == 0) {
            break;
        }

        /* Don't search the .. and . directories. */
        if(strcmp(dir_entry->d_name, ".") != 0 || strcmp(dir_entry->d_name, "..") != 0) {
            /* Turn the folder names into an int to compare. */
            memset(pid_char, 0, sizeof(pid_char));
            sprintf(pid_char, "%s",  dir_entry->d_name);
            pid = atoi(pid_char);

            /* If the PID is actually a valid and the folder exists. */
            if(pid > 0) {
                if(args == 0) {
                    memset(proc_args, 0, sizeof(proc_args));
                    sprintf(proc_args, "/proc/%d/cmdline", pid);
                    cmdline_file = open(proc_args, O_RDONLY);
                    read(cmdline_file, cmdline, sizeof(cmdline));
                }

                memset(path, 0, sizeof(path));
                sprintf(path, "/proc/%d/status", pid);
                pid_file = open(path, O_RDONLY);
                memset(buffer, 0, sizeof(buffer));
                read(pid_file, buffer, sizeof(buffer));
                line = strtok(buffer, "\t");
                line = strtok(NULL, "\n");
                sprintf(proc_name, "%s", line);

                /* Searches through the status files. */
                while(line != NULL) {
                    if(line[0] == 'U') {
                        token = strtok(line, " \t");
                        token = strtok(NULL, " \t");
                        get_uid = atoi(token);

                        if(get_uid == uid) {
                            /* Add cmdline file info to the output. */
                            if(args == 0) {
                                memset(user_proc_pid, 0, sizeof(user_proc_pid));
                                sprintf(user_proc_pid, "%s %d %s\n", proc_name, pid, cmdline);
                            }
                            else {
                                memset(user_proc_pid, 0, sizeof(user_proc_pid));
                                sprintf(user_proc_pid, "%s %d\n", proc_name, pid);
                            }

                            write(1, user_proc_pid, sizeof(user_proc_pid));
                        }
                    }

                    line = strtok(NULL, "\n");
                }


                close(pid_file);
            }
        }
    }
}

/* Gets the UID of a given username. */
int get_user_by_uid(char* username) {
    char buffer[512];
    char* token;
    int passwd;
    int uid;

    /* This file has the UID of a given user. */
    passwd = open("/etc/passwd", O_RDONLY);

    if(passwd < 0) {
        perror("Get UID error");
        exit(-1);
    }

    memset(buffer, 0, sizeof(buffer));
    read(passwd, buffer, sizeof(buffer));

    token = strtok(buffer, ":\n");

    /* Goes through the passwd file. */
    while(token != NULL) {
        /* Finds username in the passwd and gets the UID. */
        if(strcmp(token, username) == 0) {
            token = strtok(NULL, ":\n");
            token = strtok(NULL, ":\n");
            uid = atoi(token);
        }

        token = strtok(NULL, ":\n");
    }

    close(passwd);

    return uid;
}
