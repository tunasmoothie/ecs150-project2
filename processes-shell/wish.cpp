#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>


std::vector<std::string> parseCmd(std::string cmd){
	std::istringstream ss(cmd);
	std::vector<std::string> parsedCmd;
	std::string word;
	while(ss >> word){
		parsedCmd.push_back(word);
	}
	return parsedCmd;
}

int main(int argc, char *argv[]){
	
	std::vector<std::string> paths;
	std::string cmd;
	std::vector<std::string> cmdParsed;
	std::string wd;
	
	paths.push_back("/bin/");
	
	if(argc == 1){
		while(1){
			std::cout << "wish> ";
			std::getline(std::cin, cmd);
			cmdParsed = parseCmd(cmd);
			
			if(cmdParsed[0] == "exit"){
				exit(0);
			}
			else if(cmdParsed[0] == "path"){
				
			}
			else if(cmdParsed[0] == "cd"){
				
			}
			else{
				//std::cout << "forking...\n";
				pid_t pid = fork();
				if(pid == 0){
					for(size_t i = 0; i < paths.size(); i++){
						char *args[2];
						args[0] = (char*)(paths[i] + cmdParsed[0]).c_str();
						args[1] = NULL;
						execv(args[0], args);   // exec() will kill process if command success.
					}
					std::cout << "-wish: " + cmd + ": command not found\n";
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

