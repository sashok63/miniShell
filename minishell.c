#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

//Clear screen
#define clear() printf("\033[2J\033[1;1H")

//Delimetr for parsing user input
#define DELIMITER " \t\r\n\a"

//Maximum input size
#define MIS 512

//Max buffer size
#define BUFSIZE 256
#define SIZE 2

//Print current dir
void dirprint(void)
{
    char cwd[128];
    getcwd(cwd, sizeof(cwd));
    printf("\nDir:%s$ ", cwd);
}

//Greating shell
void init_shell(void)
{
    clear();
    printf("\n\n\n*********************************************************\n");
    printf("\tHi, %s.\n", getenv("USER"));
    printf("\tYou use minishell at your own risk!\n");
    printf("*********************************************************\n\n");
    sleep(3);
    clear();
}

//Shell input
char *line_input(void)
{
    int c;
    int position = 0;
    int buffer_size = MIS;
    char *buffer = (char *)malloc(sizeof(char) * buffer_size);
    if (buffer == NULL)
    {
        perror("Can't allocate memory for buffer in line_input");
        exit(EXIT_FAILURE);
    }
    
    while (1)
    {
        //Read a character
        c = getchar();

        //EOF replace with /0
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            if (c == EOF && position == 0)
            {
                //Handle EOF without input
                free(buffer);
                return buffer;
            }
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;

        //Reallocate memory for buffer
        if (position >= buffer_size)
        {
            buffer_size += MIS;
            buffer = realloc(buffer, buffer_size);
            if (buffer == NULL)
            {
                perror("Can't reallocate memory for buffer in line_input");
                exit(EXIT_FAILURE);
            }
        }
    }
}

//My function copy
char *my_strdup(const char *str)
{
    size_t len = strlen(str) + 1;       //+1 for NULL termination
    char *copy = (char *)malloc(len);

    if (copy != NULL)
    {
        strcpy(copy, str);
    }
    
    return copy;
}

//Parsing line from input
char **parsing(char *str)
{
    //Token count
    char *copy = my_strdup(str);
    char *token = strtok(copy, DELIMITER);
    int token_n = 0;

    while (token != NULL)
    {
        token_n++;
        token = strtok(NULL, DELIMITER);
    }
    
    //Free memory
    free(copy);

    //Allocate memory for string
    char **parsed = (char **)malloc(sizeof(char *) * (token_n + 1));    //+1 for NULL termination
    if (parsed == NULL)
    {
        perror("Can't allocate memory for buffer in shell_line_input");
        exit(EXIT_FAILURE);
    }
    
    //Split the string and store token in array
    copy = my_strdup(str);
    token = strtok(copy, DELIMITER);
    int i = 0;

    while (token != NULL)
    {
        parsed[i] = my_strdup(token);
        token = strtok(NULL, DELIMITER);
        i++;
    }
    
    //End of array
    parsed[i] = NULL;

    //Free memory
    free(copy);

    return parsed;
    
}

//Touch file (create empty file)
int touch(char **tokens)
{
    char filename[MIS] = {0};

    strncat(filename, tokens[1], sizeof(filename));
    if(utime(filename, NULL) == -1)
    {
        if(errno == ENOENT)
        {
            if(creat(filename, 00644) == -1)
            {
                perror("Can't create file");
                return errno;
            }
        }
        //Can't update timestamp
        else
        {
            perror("Can't update the time stamp");
            return errno;
        }
    }

    return 0;
}

//Copy file
int cpfile(char **tokens)
{
    FILE *FILE_src = NULL;
    FILE *FILE_dest = NULL;

    char *src, *dest;

    if (tokens[1] == NULL || tokens[2] == NULL)
    {
        printf("Usage: cp [src] [dest]");
        return 0;
    }
    
    src = tokens[1];
    dest = tokens[2];
    
    //Open for reading
    FILE_src = fopen(src, "r");
    if (FILE_src == NULL)
    {
        perror("File src doesn't exist");
        return 1;
    }

    //Open for writing 
    FILE_dest = fopen(dest, "w");
    if (FILE_dest == NULL)
    {
        perror("File dest doesn't exist");
        fclose(FILE_src);
        return 1;
    }

    //Read and write byte by byte
    int mychar;
    while ((mychar = fgetc(FILE_src)) != EOF)
    {
        if (fputc(mychar, FILE_dest) == EOF)
        {
            perror("puts error");
            fclose(FILE_src);
            fclose(FILE_dest);
            return 1;
        }
    }
    
    fclose(FILE_src);
    fclose(FILE_dest);

    printf("Copy done!");
    
    return 0;
}

//Printing files in directory
int dirfiles(void)
{
    DIR *dir;
    struct dirent *de;

    char cwd[MIS];
    char *path = getcwd(cwd, sizeof(cwd));
    if (path == NULL)
    {
        perror("Can't allocate path for dir");
        exit(EXIT_FAILURE);
    }

    dir = opendir(path);
    while (dir)
    {
        de = readdir(dir);
        if (!de)
        {
            break;
        }
        printf("%i %s\n", de->d_type, de->d_name);
    }

    closedir(dir);

    return 0;
}

//Handle built in commands
int builtin_cmd(char **tokens)
{
    if (strcmp(tokens[0], "help") == 0)
    {
        printf("There is not place for playing around\n");
        printf("You can use commands like:\n");
        printf("help, exit, cd, mkdir, dir, clear, cp, touch!\n");
        printf("And I think you can use external commands to!\n");
        printf("Take your time here");
        return 1;
    }
    else if (strcmp(tokens[0], "hello") == 0)
    {
        printf("Whatsup dude %s", getenv("USER"));
        return 1;
    }
    else if (strcmp(tokens[0], "exit") == 0)
    {
        printf("Bye DUDE\n");
        exit(EXIT_SUCCESS);
    }
    else if (strcmp(tokens[0], "cd") == 0)
    {
        int i = chdir(tokens[1]);
        if (i != 0)
        {
            printf("There is no direction like %s", tokens[1]);
        }
        return 1;
    }
    else if (strcmp(tokens[0], "mkdir") == 0)
    {
        int i = mkdir(tokens[1], 0777);
        if (i != 0)
        {
            fprintf(stderr, "Can't create path for mkdir: %s", tokens[1]);
        }
        return 1;
    }
    else if (strcmp(tokens[0], "dir") == 0)
    {
        int i = dirfiles();
        if (i != 0)
        {
            fprintf(stderr, "Somthing went wrong in dir");
        }
        return 1;
    }
    else if (strcmp(tokens[0], "clear") == 0)
    {
        clear();
        return 1;
    }
    else if (strcmp(tokens[0], "cp") == 0)
    {
        int i = cpfile(tokens);
        if (i != 0)
        {
            fprintf(stderr, "Somthing went wrong in cp");
        } 
        return 1;  
    }
    else if (strcmp(tokens[0], "touch") == 0)
    {
        int i = touch(tokens);
        if (i != 0)
        {
            fprintf(stderr, "Somthing went wrong in touch");
        }
        return 1;
    }
    
    return 0;
}

//Execute external commands
int external_cmd(char **tokens)
{
    pid_t pid = fork();
    
    if (pid == -1)
    {
        fprintf(stderr, "fork error in external_cmd");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        if (execvp(tokens[0], tokens) == -1)
        {
            fprintf(stderr, "execvp external_cmd");
            free(tokens);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        int status;
        pid_t wpid = waitpid(pid, &status, 0);
        if (wpid == -1)
        {
            perror("waitpid error in external_cmd");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

//Handle commands with pipe
int pipe_cmd(char **tokens)
{
    //Parse for to pipe
    int pipe_pos = 0;
    while (tokens[pipe_pos] != NULL)
    {
        if (strcmp(tokens[pipe_pos], "|") == 0)
        {
            break;
        }
        pipe_pos++;    
    }
    
    if (tokens[pipe_pos] == NULL || tokens[pipe_pos + 1] == NULL)
    {
        fprintf(stderr, "Invalid use of pipe");
        exit(EXIT_FAILURE);
    }
    
    tokens[pipe_pos] = NULL;    //Splite to two commands

    int pipe_fd[2];
    pid_t child_pid;

    //Create a pipe
    if (pipe(pipe_fd) == -1)
    {
        fprintf(stderr, "Pipe error");
        exit(EXIT_FAILURE);
    }
    
    //Fork a child process
    if ((child_pid = fork()) == -1)
    {
        fprintf(stderr, "Fork error");
        exit(EXIT_FAILURE);
    }
    
    //Child process
    if (child_pid == 0)
    {
        //Close writing
        close(pipe_fd[1]);

        //Redirect stdin to read end of pipe
        dup2(pipe_fd[0], STDIN_FILENO);

        //Close the read end of the pipe
        close(pipe_fd[0]);
        
        //Execute the second command
        execvp(tokens[pipe_pos + 1], &tokens[pipe_pos + 1]);

        //If execvp fails
        fprintf(stderr, "execvp child");
        exit(EXIT_FAILURE);
    }
    else    //Parent process
    {
        //Close the read end of the pipe
        close(pipe_fd[0]);

        pid_t first_child_pid = fork();

        if (first_child_pid == -1)
        {
            fprintf(stderr, "Fork error 1");
            exit(EXIT_FAILURE);
        }

        if (first_child_pid == 0)
        {
            //Redir to write EOP
            dup2(pipe_fd[1], STDOUT_FILENO);

            //Close write to EOP
            close(pipe_fd[1]);  

            //Execute first command
            execvp(tokens[0], &tokens[0]);

            //If execvp fails
            fprintf(stderr, "execvp first child");
            exit(EXIT_FAILURE);
        }
        else
        {
            //Wait for the first child
            waitpid(first_child_pid, NULL, 0);

            //Close write in the parent
            close(pipe_fd[1]);

            //Wait for second child
            waitpid(child_pid, NULL, 0);

            //Go back
            return 0;
        }
    }

    fprintf(stderr, "Somthing went wrong in pipe_cmd");
    return 0;
}

//Process user input
int shell_process(char *str)
{
    //Parsing the input
    char **tokens = parsing(str);

    //Check for pipe
    int pipe_present = 0;
    int i = 0;
    while (tokens[i] != NULL)
    {
        if (strcmp(tokens[i], "|") == 0)
        {
            pipe_present = 1;
            break;
        }
        i++;
    }

    if (pipe_present)
    {
        pipe_cmd(tokens);
    }
    else
    {
        int is_builtin = builtin_cmd(tokens);
        if (!is_builtin)
        {
            external_cmd(tokens);
        }
    }
    
    return 0;
}

int main()
{
    init_shell();

    //Main loop
    while (1)
    {
        //Printing direction
        dirprint();

        //Get user input
        char *str = line_input();

        //Process input
        if (shell_process(str) != 0)
        {
            fprintf(stderr, "Process failed");
        }

        //Memory free
        free(str);
    }

    return 0;
}