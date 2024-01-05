#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef struct s_cmd t_cmd;

struct  s_cmd
{
    char **cmd;
    int nb_cmd;
    t_cmd *next;
    t_cmd *back;
};

void	exec_cmd(char *envp[], t_cmd *cmd, int *fds)
{
	pid_t	pid;
	int		error;
	pid = 1;
	if (cmd)
		pid = fork();
	if (!pid)
	{
        if(fds[0] != -1)
            dup2(fds[0], 0);
		dup2(fds[1], 1);
		execve(cmd->cmd[0], cmd->cmd, envp);
		exit(0);
	}
}

int	exec_pipeline(t_cmd *cmd, char *envp[])
{
	int	fds[2];

	while (cmd->back)
	{
		if (pipe(fds) < 0)
			return (1);
		exec_cmd(envp, cmd, fds);
		wait(NULL);
		close(fds[1]);
		cmd = cmd->back;
	}
    exec_cmd(envp, cmd, fds);
	return (0);
}

void lstadd(t_cmd **curent, t_cmd *new){
    if(!*curent){
        *curent = new;
        return;
    }
    t_cmd *tmp = *curent;
    while(tmp->back){
        tmp = tmp->back;
    }
    tmp->back = new;
    tmp->back->next = tmp;
}

int cmdlen(char **cmd){
    int i = 0;
    while (cmd[i] && strcmp(cmd[i], "|") && strcmp(cmd[i], ";")){
        i++;
    }
    return (i);
}

int cmd_lst_create(t_cmd **cmd, char **argv){
    t_cmd *new;
    int nb_cmd = cmdlen(argv);
    if(!(new = malloc(sizeof(t_cmd)))){
        //Error
    }
    if(!(new->cmd = malloc(sizeof(char *) * nb_cmd + 1))){
        //Error
    }
    new->back = NULL;
    new->next = NULL;
    new->nb_cmd = nb_cmd;
    while(nb_cmd != 0){
        nb_cmd--;
        new->cmd[nb_cmd] = argv[nb_cmd];
    }
    lstadd(cmd, new);
    return (new->nb_cmd);
}


int main(int argv, char **argc, char **envp){
    t_cmd *cmd;

    cmd = NULL;
    if(argv <= 1){
        printf("Error\n");
        return(1);
    }
    int index = 1;
    while (argc[index] && !strcmp(argc[index], ";")){
        index++;
    }
    while (argc[index]){
        while (argc[index]){
            index += cmd_lst_create(&cmd, &argc[index]);
            if (argc[index] && !strcmp(argc[index], "|")){
                index++;
                continue;
            }
            else if(argc[index]){
                index++;
                break;
            }
        }
        exec_pipeline(cmd, envp);
    }
}