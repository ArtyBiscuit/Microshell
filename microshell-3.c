#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

typedef struct s_cmd t_cmd;

struct s_cmd{
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

void cd(t_cmd *lst){
    if(!lst->cmd[1] || lst->cmd[2]){
        write(2, "error: cd: bad arguments\n", ft_strlen("error: cd: bad arguments") + 1);
        return;
    }
    if(!chdir(lst->cmd[1])){
        write(2, "error: cd: cannot change directory to ", ft_strlen("error: cd: cannot change directory to"));
        write(2, lst->cmd[1], ft_strlen(lst->cmd[1]));
        write(2, "\n", 1);
    }
}


int cmdlen(char **argv){
    int i = 0;
    while(argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";")){
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
}

void free_lst(t_cmd *lst){
    while(lst->back){
        lst = lst->back;
    }
    while(lst->next){
        lst = lst->next;
        if(lst->back->cmd)
            free(lst->back->cmd);
        free(lst->back);
    }
    if(lst->cmd)
        free(lst->cmd);
    free(lst);
}

int lst_create(t_cmd **lst, char **argv){
    t_cmd *new = NULL;
    int cmd_len = cmdlen(argv);
    if(!(new = malloc(sizeof(t_cmd)))){
        write(2, "error: fatal \n", ft_strlen("error: fatal \n"));
    }
    if(!(new->cmd = malloc(sizeof(char *) * (cmd_len + 1)))){
        write(2, "error: fatal \n", ft_strlen("error: fatal \n"));
    }
    new->next = NULL;
    new->back = NULL;
    int tmp = cmd_len;
    while(tmp){
        tmp--;
        new->cmd[tmp] = argv[tmp];
    }
    new->cmd[cmd_len] = NULL;
    lstadd_back(lst, new);
    return (cmd_len);
}

void exec_pip(t_cmd *lst, int *fds, int *fd_tmp, char **envp){
    pid_t pid = 1;

    if(!(pid = fork())){
        if(lst->back)
            dup2(fds[1], 1);
        dup2(*fd_tmp, 0);
        close(fds[0]);
        if(execve(lst->cmd[0], lst->cmd, envp)){
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
        if(pipe(fds)){
            write(2, "error: fatal \n", ft_strlen("error: fatal \n"));
        }
        if(!strcmp(lst->cmd[0], "cd"))
            cd(lst);
        else
            exec_pip(lst, fds, &fd_tmp, envp);
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

int main(int argc, char **argv, char **envp){
    t_cmd *lst = NULL;
    int index = 1;

    if(argc < 2){
        write(2, "error: fatal \n", ft_strlen("error: fatal \n"));
        return (1);
    }
    if(!strcmp(argv[argc - 1], "|"))
        return (0);
    while(argv[index]){
        while(argv[index] && !strcmp(argv[index], ";")){
            index++;
        }
        while(argv[index]){
            index += lst_create(&lst, &argv[index]);
            if(argv[index] && !strcmp(argv[index], "|")){
                index++;
                continue;
            }
            else if(argv[index]){
                index++;
                break;
            }
        }
        exec(lst, envp);
        if(lst){
            free_lst(lst);
            lst = NULL;
        }
    }   
    return (0);
}