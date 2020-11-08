#include "SerialA.h"

#define BUFSIZE  1024

int count = 0;
struct Statistic *threadSTAT;  //Global Statistic for thread implementation

// printer prints final statistics for files read
// information is passed in as a struct Statistic pointer
void printer(struct Statistic *stat) {
    printf("-----------------STATISTIC--------------\n\nDescription\t\tNum of count\n\n");
    printf("Bad File:\t\t%d\nDirectories:\t\t%d\nRegular Files:\t\t%d\nSpecial Files:\t\t%d\nRegular File Bytes:\t%ld\n"
           "Text Files:\t\t%d\nText File Bytes:\t%ld\n", stat->Bad, stat->Dir, stat->Reg, stat->Spe, (__intmax_t) stat->RegFB,
           stat->TextF, (__intmax_t) stat->TextFB);
 }

// retrieves systime and usertime from inputted rusage struct 
//prints the user time and sys time and threads into comma separated values
 void timesUsed(struct rusage* usage, int threads){
    getrusage(RUSAGE_SELF, usage);
     long userTime, sysTime;
     userTime = (long) usage->ru_utime.tv_sec* (1000000) +(long) (usage->ru_utime.tv_usec);
     sysTime = (long) usage->ru_stime.tv_sec * (1000000)+ (long) (usage->ru_stime.tv_usec);
      printf("%dthreads,\t%ldmicroseconds,\t%ldmicroseconds\n",threads, sysTime, userTime);


 }


// pthread start_routine
// input is FileName which is current file in buffer at time of creation
void *reader(void *args) {
    char *file = malloc(sizeof(char *) * BUFSIZE);
    pthread_mutex_lock(&m); // Lock while current thread is working on current filename
    strcpy(file, ((char *) args));   //Copy the args into the filename
    getStats(threadSTAT, (char *) args);
    pthread_mutex_unlock(&m);

}


int main(int argc, char *argv[]) {
    struct Statistic *stat1;
    char *buffer2 = malloc(sizeof(char *) * BUFSIZE);
    char *fullPath = malloc(sizeof(char *) * BUFSIZE);
    int Max, numRead, amountThr;
     amountThr =1;
    char* buffer =malloc(sizeof(char *) * BUFSIZE);;
    numRead = 0;
    struct rusage usage;

    stat1 = (struct Statistic *) malloc(sizeof(struct Statistic));
    //0 Dir, 1 Regular, 2 Special
    // Implementation for a serial server request
    if (argc < 2) {
        // Read from stdin and keep reading everyline until EOF is detected
        while (fgets(buffer, BUFSIZE, stdin) != NULL) {

            buffer[strlen(buffer) - 1] = '\0';          //Eliminate \n character in buffer
            realpath(buffer, fullPath);                   // Retrieve fullpath to use files outside of directory
            getStats(stat1, fullPath);
            numRead++;
        }
        printer(stat1);
        free(stat1);

        //Implementation for threaded part of assignment
        // If user inputs in thread next to program name goes into MultiThread mode
        // User has to put a thread amount next but defaults to 15 if none inputted
    } else if (strcmp(argv[1], "thread") == 0) {
        threadSTAT = (struct Statistic *) malloc(sizeof(struct Statistic));
        if (argc == 3) {
            Max = atoi(argv[2]);
            if (Max < 1) {
                Max = 1;
            } else if (Max > 15) {
                Max = 15;
            }
        }else {
            Max = 15;
        }
        amountThr = Max;
        pthread_t THREADS[16];
        char **Files = malloc(sizeof(char **) * (Max + 1));
        int Total = 0;
        int Id = 0;
        while (fgets(buffer2, BUFSIZE, stdin) != NULL) {
            buffer2[strlen(buffer2) - 1] = '\0';
            Total++;
            if (Id < Max) {
                Files[++Id] = malloc(sizeof(char *) * BUFSIZE);
                realpath(buffer2, Files[Id]);
                if (pthread_create(&THREADS[Id], NULL, reader, (void *) Files[Id]) == 0) {
                }
            } else if (Id == Max) {
                for (int j = 1; j <= Id; j++) {
                    if (pthread_join(THREADS[j], NULL) == 0) {
                        free(Files[j]);
                    }
                }
                Id = 1;
                Files[Id] = malloc(sizeof(char *) * BUFSIZE);
                realpath(buffer2, Files[Id]);
                pthread_create(&THREADS[Id], NULL, reader, (void *) Files[Id]);
            }
        }
        for (Id; Id > 0; Id--){
            pthread_join(THREADS[Id], NULL);
        }
        printer(threadSTAT);
    }
    free(buffer2);
    free(fullPath);
    free(buffer);
    timesUsed(&usage,amountThr);
    return 1;
}

int getModeStats(struct Statistic *stat, mode_t m) {
    if (S_ISDIR(m)) {
        stat->Dir++;
        return 0; // Directory
    }
    if (S_ISREG(m)) {
        stat->Reg++;
        return 1; //Regular
    }
    if (S_ISFIFO(m) || S_ISBLK(m) || S_ISCHR(m)) {
        stat->Spe++; //Special
        return 2;
    }
}

int isText(char *buffer) {
    char buf[BUFSIZE];
    int fdIn, cnt, i, isPrintable, Test, TestSpace;
    isPrintable = 1;
    i = 0;
    if (strcmp(buffer, "/proc/kcore") == 0) {
        return 0;
    }
    fdIn = open(buffer, O_RDONLY);

    if (fdIn < 0) {
        fprintf(stderr, "file open\n");
        exit(1);
    }
    // Copy Input to stdout

    while ((cnt = read(fdIn, buf, BUFSIZE) > 0)) {
        while (cnt > 0) {
            if ((Test = isprint(buf[i])) == 0 && (TestSpace = isspace(buf[i])) == 0) {
                isPrintable = 0; // Non printable detected
                //   printf("Test %d Test Space%d\nat buf[%d] during cnt %d\n", Test, TestSpace, i, cnt);
                }
                i++;
                cnt--;
            }
        }
        if (fdIn > 0) {
            close(fdIn);
        }

        return isPrintable;

}

void getStats(struct Statistic *stats, char *FileName) {
    struct stat *statbuf;
    statbuf = (struct stat *) malloc(sizeof(struct stat)); // System defined stat structure for getting statistics
    int Success, fileType;
    Success = stat(FileName, statbuf);
    if (Success == 0) {
       // printf("%s------%ld\n", FileName, statbuf->st_size);
        fileType = getModeStats(stats, statbuf->st_mode);
        if (fileType == 1) {
            stats->RegFB = (long) statbuf->st_size + stats->RegFB;
            if (isText(FileName)) {
                stats->TextF++;
                stats->TextFB = ((long) statbuf->st_size + (stats->TextFB));
            }
        } else if (fileType == 2) {
            stats->Spe++;
        }

    } else {
        stats->Bad++;
    //    printf("BAD---%s\n", FileName);
    }

}
