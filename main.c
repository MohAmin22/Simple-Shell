#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>

char* parsed_command [50];
char string[200];
char input[200];
char temp2[200];
char export_var[200];

void sigchld_handler(int sig) {
    int m;
    waitpid(-1,&m,WNOHANG);
    FILE *log_file =fopen("/home/mohamed/history.txt", "a");
    fprintf(log_file, "Child process is terminated\n");
    fclose(log_file);
}

void register_child_signal()
{
    //sending signals when child tends to terminate
    signal(SIGCHLD, sigchld_handler);
}
void set_environment(){
//setting the environment to the current directory.
    char cwd[1000];
    chdir(getcwd(cwd, sizeof(cwd)));
}
void parse_input(char* inputArray) {
    // Iterate through the input char array and separate the strings using the space character as a delimiter
    char *token;
    int i = 0;
    token = strtok(inputArray, " ");

    while (token != NULL && i < 50) {

        // Populate the 2D char array with the separated strings
        parsed_command[i] = token;
       // strcpy(parsed_command[i], token);
        token = strtok(NULL, " ");
        i++;
    }
}

bool Is_shell_builtin() {
    if(!strcmp(parsed_command[0],"cd") || !strcmp(parsed_command[0],"echo") || !strcmp(parsed_command[0],"export"))
        return true;
    return false;
}
void execute_shell_bultin() {
    if( !strcmp(parsed_command[0],"cd") ){
        if( parsed_command[1] == NULL || !strcmp( parsed_command[1], "~" ) ){
            chdir( getenv("HOME") );
        }
        else {
            // 0 for success
            int success = !chdir(parsed_command[1]);
            if( !success ){
                // generate an error due to no success
                printf("error message: the entered path is not valid\n");
            }
        }
    }else if( !strcmp(parsed_command[0],"echo") ){
        int c = 5;
        while(input[c] != NULL){
            if(input[c] == '\"'){}
            else if(input[c] == '$'){
                // get environment variable value
                char alt[2];
                alt[0] = input[c+1];
                alt[1] = '\0';
                char* path = getenv(alt);
                printf("%s",path);
                c++;
            }
           else{
                printf("%c",input[c]);
            }
            c++;
        }
        printf("\n");
    }else if( !strcmp(parsed_command[0],"export") ){

        export_var[0]= input[7];
        export_var[1] = '\0';

       // printf("%c\n",*export_var);
        int init = 0;
        for (int counter = 10;input[counter] != '\"'; ++counter) {

            temp2[init++] = input[counter];
        }
        temp2[init]='\0';
       // printf("%s\n",temp2);
        int result = setenv(export_var, temp2, 1);
        if (result == 0) {
            //printf("MY_VAR set to %s\n", getenv("MY_VAR"));
        } else {
            printf("Error setting MY_VAR\n");
        }
    }
}
void execute_command(){
    bool IsBackEnd = false;
    if( parsed_command[1] != NULL && !strcmp(parsed_command[1],"&") ){
        IsBackEnd = true;
    }
    pid_t id = fork();
    if ( id == 0 ) {
        //so as not to cause sf
        if( parsed_command[1] != NULL && !strcmp(parsed_command[1], "&") ){
            parsed_command[1] = NULL;
        }
        // error -1
        int res = execvp(parsed_command[0], parsed_command);
        if(res < 0 && strcmp(parsed_command[0], "exit") != 0){
            printf("Error Message not found\n");
        }
        exit(0);

    }
        // wait until the user exit the opened app.
    else if ( !IsBackEnd ) {
        int status;
        waitpid(id, &status, 0);
    }
}
void evaluate(){
    char replaced [200];
    memset(replaced,'\0',sizeof(replaced));
    int rindex = 0;
    for (int counter3 = 0; counter3 < strlen(string); ++counter3) {
        if(string[counter3] == '$'){
            char alt[2];
            alt[0] = string[counter3+1];
            alt[1] = '\0';
            char* value = getenv(alt);
            for (int i = 0; value[i] != '\0' ; ++i) {
                replaced[rindex++] = value[i];
            }
            counter3++; //to jump over the variable
        }else{
            replaced[rindex++] = string[counter3];
        }
    }
    //printf("%s\n",replaced);
    strcpy(string,replaced);
}
void shell(){
    do{
        char x;
        int index = 0;

        printf("Amin's Shell => ");
        while(1){
            x = getchar();
            if(x == '\n'){
                break;
            }
            string[index++] = x;
        }
        string[index] = '\0';
        if(!strcmp(string,"exit")){
            break;
        }
        strcpy(input,string);
        //
        evaluate();
        //
        parse_input(&string);
        if(Is_shell_builtin()){
            execute_shell_bultin();
        }else{
            execute_command();
        }
        memset(string,'\0',sizeof(string));
        memset(input,'\0',sizeof(input));
        memset(export_var,'\0',sizeof(export_var));
        memset(temp2,'\0',sizeof(temp2));
        for (int counter2 = 0; counter2 < 50 ; ++counter2) {
            parsed_command[counter2] = NULL;
        }


    }while(1);
    exit(0);
}



int main() {
    // Write C code here
    register_child_signal();
    set_environment();
    shell();
    return 0;
}