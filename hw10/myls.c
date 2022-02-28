#include<stdio.h>
#include<unistd.h>
int main(int arg,char **args){
	char *argv[]={"ls","-al","/etc",(char *)0};
	char *envp[]={"PATH=/bin",0};
	execve("/bin/ls",argv,envp);
}
