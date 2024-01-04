#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct s_cmd t_cmd;

struct  s_cmd
{
    int fd[2];
    char **cmd;
    t_cmd *next;
    t_cmd *back;
};

// int ft_strlen(char *str){
//     int i = 0;
//     while (str[i]){
//         i++;
//     }
//     return (i);
// }

// void lstadd(t_cmd *curent, t_cmd *new){

// }

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
    while(nb_cmd != 0){
        nb_cmd--;
        new->cmd[nb_cmd] = argv[nb_cmd];
        printf("%s\n", new->cmd[nb_cmd]);
    }
}


int main(int argv, char **argc, char **envp){
    t_cmd *cmd;

    if(argv <= 1){
        printf("Error\n");
        return(1);
    }
    int index = 1;
    while (argc[index] && !strcmp(argc[index], ";")){
        index++;
    }
    while (argc[index]){
        index += cmd_lst_create(&cmd, &argc[index]);
        return (0);
    }
    printf("Done !\nIndex = %d\n", index - 1);

}