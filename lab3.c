#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

long threadsMaxCount;
long currentThread;
pid_t pid;

int IsSpace(char symbol){
    return (symbol == ' ' || symbol == '\n' || symbol == '\t');
}

struct FileProcessingResult{
    int wordsCount;
    long bytesAmount;
};

struct FileProcessingResult ProcessFile(const char* path){
    char buf[1024];

    FILE *file;
    size_t readBytes;
    struct FileProcessingResult result;
    result.bytesAmount = 0;
    result.wordsCount = 0;

    int isWordEnd = 1;
    
    if ((file = fopen(path, "r")) != NULL) 
    {
        while ((readBytes = fread(buf, 1, 1024, file)) > 0)
        {
            result.bytesAmount += readBytes;
            for(int i = 0; i < readBytes; i++)
            {      
                if (IsSpace(buf[i])) 
                {
                    isWordEnd = 1;
                } 
                else 
                {
                    result.wordsCount += isWordEnd;
                    isWordEnd = 0;
                }

            }
        }

        fclose(file);
        return result;
    }
}

void ResearchDir(DIR *dir, const char* path)
{
    struct dirent *dir1;
    struct stat Stat;

    while ((dir1 = readdir(dir))!=NULL){
        if (strcmp(dir1->d_name, ".") != 0 && strcmp(dir1->d_name,"..") != 0){

            char name[768];
            strcpy(name,path);
            strcat(name,"/");
            strcat(name,dir1->d_name);

            stat(name, &Stat);
            if (S_ISDIR(Stat.st_mode) != 0) {
                DIR *newDir;

                if ((newDir = opendir(name))!=NULL){  
                    ResearchDir(newDir, name);

                    closedir(newDir);
                };
            }
            else {
                while (1){
                    if (currentThread < threadsMaxCount) 
                    {
                        currentThread++;
                        pid = fork();   

                        if (pid == 0)
                        {
                            struct FileProcessingResult result = ProcessFile(name);                        

                            printf("Thread id: %d  File name: %s  Words amount: %d  Read bytes: %li\n", 
                                    getpid(), name, result.wordsCount, result.bytesAmount);

                            exit(0);
                        }
                        
                        break;
                    }
                    else if (currentThread == threadsMaxCount)
                    {
                        int status = 0;
                        wait(&status);
                        currentThread--;  
                    }                   
                }
            }
        }
    }
}

int main(int argc, char const *argv[])
{
    if (argc == 3){
        if ((threadsMaxCount = strtol(argv[2], NULL, 10)) != 0 && threadsMaxCount > 0){
            const char* path = argv[1];
            DIR *dir;
            currentThread = 0;

            if ((dir = opendir(path)) != NULL ){
                ResearchDir(dir, path);

                for (int i = 0; i < currentThread; i++)
                {
                    int status = 0;
                    wait(&status);
                }
            }
            else {
                printf("\ndirect not exist\n");
            }           
        }
        else {
            printf("\nIncorrect input\n");
           
        }
    }
    else {
        printf("\nIncorrect num of arguments\n");
    }

    return 0;
}