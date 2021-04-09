#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

/*
std::vector<std::string> parseCmd(std::string cmd){
	std::vector<std::string> str;
	return str;
}*/

int main(int argc, char *argv[]){
	
	std::vector<std::string> paths;
	std::string wd;
	
	//std::cout << "startup\n";
	
	if(argc == 1){
		while(1){
			std::cout << "wish> ";
			
			std::string cmd;
			std::getline(std::cin, cmd);
			//cmd = parseCmd(cmd)[0];
			
			if(cmd == "exit"){
				exit(0);
			}
			else if(cmd == "path"){
				
			}
			else if(cmd == "ls"){
				pid_t pid = fork();
				if(pid == 0){
					char *args[2];
					args[0] = (char*)"/bin/ls";
					args[1] = NULL;
					execv(args[0], args);
					exit(1);
				}	
				wait(NULL);
			}
			
			
			//std::cout << cmd << std::endl; 
		}
	}
	else if(argc == 2){
		
	}
	else{
		std::cerr << "An error has occured" << std::endl;
		exit(1);
	}

	exit(0);
}

