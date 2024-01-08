#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef struct s_cmd t_cmd;

struct s_cmd
{
    char **cmd;
    t_cmd *next;
    t_cmd *back;
};

// utils

int cmdlen(char **argc){
    int i = 0;
    while (argc[i] && strcmp(argc[i], "|") && strcmp(argc[i], ";")){
        i++;
    }
    return (i);
}

void lstadd_back(t_cmd **current, t_cmd *new){
    if(!*current){
        *current = new;
        return;
    }
    t_cmd *tmp = *current;
    while (tmp->back){
        tmp = tmp->back;
    }
        tmp->back = new;
        tmp->back->next = tmp;
    return;
}

// parce

int parce(t_cmd **lst, char **argc){
    t_cmd *new;
    int nb_cmd = cmdlen(argc);
    
    if(!(new = malloc(sizeof(t_cmd)))){
        //ERROR
    }
    if(!(new->cmd = malloc(sizeof(char *) * nb_cmd))){
        //ERROR
    }
    new->back = NULL;
    new->next = NULL;
    int tmp = nb_cmd;
    while(tmp > 0){
        tmp--;
        new->cmd[tmp] = argc[tmp];
    }
    new->cmd[nb_cmd] = NULL;
    lstadd_back(lst, new);
    lst = &new;
    return (nb_cmd);
}

// exec

void exec_cmd(t_cmd *lst, char **envp, int *fds){
    pid_t pid = 1;
    if(lst)
        pid = fork();
    if(!pid){
        if(fds[0] != -1)
            dup2(fds[0], 0);
        dup2(fds[1], 1);
        execve(lst->cmd[0], lst->cmd, envp);
        exit;
    }
}

int exec(t_cmd *lst, char **envp){
    int fds[2];

    while (lst->back){
        if(pipe(fds) < 0)
            return (1);
        exec_cmd(lst, envp, fds);
        wait(NULL);
        close(fds[0]);
        lst = lst->back;
    }
    fds[0] = -1;
    exec_cmd(lst, envp, fds);
    return (0);
}

//

int main(int argv, char **argc, char **envp){
    int index = 0;
    t_cmd *lst = NULL;

    if(argv < 1){
        // ERROR
        return (1);
    }
    *argc++;
    while(!strcmp(argc[index], ";")){
        index++;
    }
    while (argc[index]){
        while (argc[index]){
            index += parce(&lst, &argc[index]);
            if (argc[index] && !strcmp(argc[index], "|")){
                index++;
                continue;
            }
            else if (argc[index]){
                index++;
                break;
            }
        }
        exec(lst, envp);
    }
}