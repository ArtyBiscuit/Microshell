#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

typedef struct s_cmd t_cmd;

struct s_cmd
{
    char **cmd;
    t_cmd *next;
    t_cmd *back;
};

int ft_strlen(char *str){
    int i = 0;
    while(str[i]){
        i++;
    }
    return (i);
}

void free_lst(t_cmd *lst){
    if (lst){
        while (lst->next){
            lst = lst->next;
        }
        t_cmd *tmp = lst;
        while(tmp->back){
            free(tmp->cmd);
            tmp = tmp->back;
            free(tmp->next);
        }
        free(tmp->cmd);
        free(tmp);
    }
    return;
}
void cd(t_cmd *lst){
    if(!lst->cmd[1] || lst->cmd[2] != NULL){
        write(2, "error: cd: bad arguments\n", ft_strlen("error: cd: bad arguments") + 1);
        return;
    }
    if(chdir(lst->cmd[1])){
        write(2, "error: cd: cannot change directory to ", ft_strlen("error: cd: cannot change directory to"));
        write(2, lst->cmd[1], ft_strlen(lst->cmd[1]));
        write(2, "\n", 1);
    };
}

void exec_pip(t_cmd *lst, char **envp, int *fds, int *fd_tmp){
    pid_t pid = 1;
    
    if(!(pid = fork())){
        if(lst->back){
            dup2(fds[1], 1);
        }
        dup2(*fd_tmp, 0);
        close(fds[0]);
        if(execve(lst->cmd[0], lst->cmd, envp) == -1){
            write(2, "error: cannot execute ", ft_strlen("error: cannot execute "));
            write(2, lst->cmd[0], ft_strlen(lst->cmd[0]));
            write(2, "\n", 1);
            close(fds[1]);
            free_lst(lst);
        }
        exit(-1);
    }
}

void exec(t_cmd *lst, char **envp){
    int fds[2];
    int fd_tmp = 0;

    while(lst){
        if(pipe(fds) < 0){
            write(2, "error: fatal \n", ft_strlen("error: fatal \n"));
            return;
        }
        if(!strcmp(lst->cmd[0], "cd"))
            cd(lst);
        else
            exec_pip(lst, envp, fds, &fd_tmp);
        wait(NULL);
        close(fds[1]);
        if(fd_tmp)
            close(fd_tmp);
        fd_tmp = fds[0];
        lst = lst->back;
    }
    close(fds[0]);
    if(fd_tmp)
        close(fd_tmp);
}

int cmdlen(char **argc){
    int i = 0;
    while(argc[i] && strcmp(argc[i], "|") && strcmp(argc[i], ";")){
        i++;
    }
    return (i);
}

void lstadd_back(t_cmd **current, t_cmd *new){
    if(!(*current)){
        *current = new;
        return;
    }
    t_cmd *tmp = *current;
    while(tmp->back){
        tmp = tmp->back;
    }
    tmp->back = new;
    new->next = tmp;
    return;
}


int lst_create(t_cmd **lst, char **argc){
    t_cmd *new = NULL;
    int cmd_size = cmdlen(argc);
    if(!(new = malloc(sizeof(t_cmd)))){
        write(2, "error: fatal \n", ft_strlen("error: fatal \n"));
    }
    new->cmd = malloc(sizeof(char *) * (cmd_size + 1));   
    if(!new->cmd){
        write(2, "error: fatal \n", ft_strlen("error: fatal \n"));
    }
    new->next = NULL;
    new->back = NULL;
    int tmp = cmd_size;
    while(tmp){
        tmp--;
        new->cmd[tmp] = argc[tmp];
    }
    new->cmd[cmd_size] = NULL;
    lstadd_back(lst, new);
    return (cmd_size);
}


int main(int argv, char **argc, char **envp){
    t_cmd *lst = NULL;
    int index = 1;
    if (argv < 1){
        write(2, "error: fatal \n", ft_strlen("error: fatal \n"));
        return (1);
    }
    if(!strcmp(argc[argv - 1], "|"))
        return (0);
    while (argc[index]){
        while (argc[index] && !strcmp(argc[index], ";")){
            index++;
        }
        while(argc[index]){
            index += lst_create(&lst, &argc[index]);
            if(argc[index] && !strcmp(argc[index], "|")){
                index++;
                continue;
            }
            else if (argc[index]){
                index++;
                break;
            }
        }
        exec(lst, envp);
        if (lst){
            free_lst(lst);
            lst = NULL;
        }
    }
}