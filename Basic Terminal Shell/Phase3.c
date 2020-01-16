//
// Created by yared on 9/4/19
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <string.h>

void ProcessForker(char **args, struct timeval *End);

int CommandToRun(char *Check);

void ShellDisplay();

void Execute(char **args, struct timeval *Start, struct timeval *End, char prompt[]);
void WaitBgProcess(int options);


void getShellCommands(char **args, int size, char *Input);

void PrintStats(struct timeval *Start, struct timeval *End);

int timeValToMili(struct timeval *t1, struct timeval *t2);

int isBackground(char Input[]);

void RunBackground(char **args, struct timeval *End, struct rusage *used);

struct Process{
    pid_t pid;
    char* Command;
    int status;
    struct timeval start;
} process;
struct Process Background [100];
int Bcount;

int main(int argc, char *argv[]) {
    char *args[32];
    struct timeval Start, End;
    while (1) {
        if (feof(stdin)) {
            printf("End of file detected!\n");
            exit(0);
        }
        // Case for no arguments, where user inputs commands directly from the terminal shell
        // When argument count > 1
        if (argc > 1) {
            gettimeofday(&Start, NULL);

            int i;

            // Copy command line arguments to argument array args
            for (i = 0; i + 1 < argc; i++) {
                int Len = strlen(argv[i + 1]) + 1;
                args[i] = malloc(sizeof(char) * Len);

                strcpy(args[i], argv[i + 1]);
                printf("Argument %d = %s\n", i, args[i]);

            }
            args[i] = NULL; // Have to terminate the command list array

            ProcessForker(args, &End);
            PrintStats(&Start, &End);
            // Printer


        } else if (argc == 1) {
            char Input[130];
            char prompt[20] = "==>";
            ShellDisplay();
            while (1) {   // Shell keeps prompting until user exits
                printf("%s", prompt);
                fgets(Input, 130, stdin);
                if (isBackground(&Input) == 0) {
                    gettimeofday(&Start, NULL);
                    getShellCommands(args, 130, Input);
                    Execute(args, &Start, &End, prompt);

                } else if (isBackground(Input) == 1) {
                    //  struct rusage used;
                    //  RunBackground(args,  &End,  &used);

                }
            }
        }
        return 0;
    }
}
void ProcessForker(char **args, struct timeval *End) {
    int i = 0;
    int ChildID = fork();
    if (ChildID < 0) {
        fprintf(stderr, "Fork Failed\n");
        exit(1);
    } else if (ChildID == 0) {
        printf("Child:   %d processing now\n", getpid());
        printf("COMMAND: ");
        while (args[i] != NULL) {
            printf("%s ", args[i]);
            i++;
        }
        if ((execvp(args[0], args) == -1)) {
            printf("Not Working\n");
        }
    } else if (ChildID > 0) {
        wait(NULL);// Wait for child process
        gettimeofday(End, NULL);
        printf("Parent:  %d processing now\n", getpid());
    }
}

void ShellDisplay() {
    printf("--------------------Welcome-------------------------\n");

    printf("You are now entering Yared's terminal\n");
    printf("Please input commands you would like to execute\n");

}

int CommandToRun(char *Check) {
    int Command;

    if (strcmp(Check, "exit") == 0) {
        Command = 0;
    } else if (strcmp(Check, "cd") == 0) {
        Command = 1;

    } else if (strcmp(Check, "set") == 0) {
        // Fix this set prompt
        Command = 2;
    } else Command = 3; // Other commands
    return Command;
}

void Execute(char **args, struct timeval *Start, struct timeval *End, char prompt[]) {
    switch (CommandToRun(args[0])) {
        case 0: // Exit Condition
            printf("COMMAND: %s\n", args[0]);
            PrintStats(Start, End);
            printf("Shell is Exiting\nGOODBYE!!!!\n");
            exit(0);

            break;
        case 1: // Change directory Condition
            chdir(args[1]);
            printf("COMMAND: %s %s\n", args[0], args[1]);

            PrintStats(Start, End);


            break;
        case 2: // Set Prompt
            if (strcmp(args[1], "prompt") == 0) {
                strcpy(prompt, args[3]);
                printf("COMMAND: %s %s %s %s", args[0], args[1], args[2], args[3]);
                printf("\nYou have successfully changed prompt to %s\n", args[3]);
            }
            PrintStats(Start, End);

            break;
        case 3:
            ProcessForker(args, End);
            PrintStats(Start, End);

            break;
        default:
            break;
            // Something here
    }
}

void getShellCommands(char **args, int size, char *Input) {
    int j = 0;
    const char s[] = " \n\t\r\f";

    char *Token = malloc(sizeof(char) * 20);

    Token = strtok(Input, s);

    while (Token != NULL) {
        int sizeArr = strlen(Token) + 1;
        args[j] = malloc(sizeof(char) * sizeArr);
        strcpy(args[j], Token);

        j++;
        Token = strtok(NULL, s);
        args[j] = malloc(sizeof(char) * 10);

    }
    free(Token);
    args[j] = NULL;


}

void PrintStats(struct timeval *Start, struct timeval *End) {
    struct rusage Child, Parent;
    struct timeval systime;
    struct timeval usertime;
    getrusage(RUSAGE_CHILDREN, &Child);
    getrusage(RUSAGE_SELF, &Parent);
    int minFltC, mjrfltP, Voluntarily, Involunt, minFltP, mjrFltC, Wall, sys, user;
    minFltC = (int) Child.ru_minflt;
    minFltP = (int) Parent.ru_minflt;
    mjrFltC = (int) Child.ru_majflt;
    mjrfltP = (int) Parent.ru_majflt;
    Voluntarily = (int) Parent.ru_nvcsw;
    Involunt = (int) Parent.ru_nivcsw;
    systime = Parent.ru_stime;
    usertime = Parent.ru_utime;
    Wall = timeValToMili(Start, End);

    sys = (int) ((systime.tv_usec) + systime.tv_sec * 1000) / 1000;
    user = (int) ((usertime.tv_usec) + usertime.tv_sec * 1000) / 1000;


    printf("Stat1: \nElasped CPU time for user: %d\nElasped CPU time for system: %d\n", user, sys);
    printf("Stat2: \nElasped wallclock time in miliseconds: %d\n", (unsigned int) Wall);
    printf("Stat3: \nProcess was prempted involuntarily %d times \n", Involunt);
    printf("Stat4: \nProcess gave up the CPU voluntarily %d time \n", Voluntarily);
    printf("Stat5: \nMajor faults occured\nChild: %d--------Parent:%d\n", mjrFltC, mjrfltP);
    printf("Stat6: \nMinor faults occured\nChild: %d--------Parent:%d\n", minFltC, minFltP);


}

int timeValToMili(struct timeval *t1, struct timeval *t2) {
    int Miliseconds;

    Miliseconds = (int) (((*t2).tv_sec - (*t1).tv_sec) * 1000000 + ((*t2).tv_usec - (*t1).tv_usec)) / 1000;

    return Miliseconds;
}

int isBackground(char Input[]) {
    int i = 0;
    while (Input[i] != NULL) {
        if (Input[i] == '&') {
            return 1;
        }
        i++;
    }
    return 0;
}

void RunBackground(char **args, struct timeval *End, struct rusage* used) {
    int *statloc;
    pid_t child_pid;
    pid_t r_pid;

        child_pid = fork();
        if(child_pid < 0){
        fprintf(stderr, "Fork Failed\n");
        exit(1);
    } else if (child_pid == 0) {
        printf("Child:   %d processing now\n", getpid());
        if ((execvp(args[0], args) == -1)) {
            printf("Not Working\n");
        }
    } else {
        WaitBgProcess(WNOHANG);
        printf("[%d] %d /*indicate background task and process id*/\n", Bcount + 1, child_pid);
        Background[Bcount].pid = child_pid;
        Background[Bcount].status = 1;
     //   Background[Bcount].command = (char *) malloc(sizeof(commands[input]));
     //   strcpy(Background[Bcount].command, commands[input]);
        Bcount++;
    }
}

void WaitBgProcess(int options){
    pid_t r_pid;
    struct rusage usage;
    r_pid = wait3(NULL, options, &usage);
    // Condition for a child id whose state has changed
    while (r_pid != 0 && r_pid != -1) {        {
            for (int i = 0; i < Bcount; i++) {
                if (r_pid == Background[i].pid && Background[i].status == 1) {
                    Background[i].status = 0;
                    printf("-- Job Complete [%d] --\n", i + 1);
                    printf("Process ID: %d\n", Background[i].pid);
                //    printStats(usage);
                }
            }
            r_pid = wait3(NULL, options, &usage);
        }

    }

    }

