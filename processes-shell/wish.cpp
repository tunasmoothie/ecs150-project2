#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>


std::vector<std::string> parseCmd(std::string cmd_raw){
	std::istringstream ss(cmd_raw);
	std::vector<std::string> parsedCmd;
	std::string word;
	while(ss >> word){
		parsedCmd.push_back(word);
	}
	return parsedCmd;
}

int main(int argc, char *argv[]){
	
	//std::string wd = "./";
	std::string cmd_raw;
	std::vector<std::string> cmd_parsed;
	std::vector<std::string> paths;
	paths.push_back("");
	paths.push_back("/bin/");
	
	
	if(argc == 1){
		while(1){
			std::cout << "wish> ";
			std::getline(std::cin, cmd_raw);
			cmd_parsed = parseCmd(cmd_raw);
			
			if(cmd_parsed.size() == 0);
			else if(cmd_parsed[0] == "exit"){
				exit(0);
			}
			else if(cmd_parsed[0] == "path"){
				char buf[4096];
				getwd(buf);
				std::cout << buf << std::endl;
			}
			else if(cmd_parsed[0] == "cd"){
				if(cmd_parsed.size() != 2);
				else if(chdir((char*)cmd_parsed[1].c_str()) == 0){
					continue;
				}
					
				std::cerr << "An error has occurred" << std::endl;
			}
			else{
				pid_t pid = fork();
				if(pid == 0){
					char *args[cmd_parsed.size()];
					
					for(size_t i = 0; i < cmd_parsed.size(); i++){
						char *tmp = new char[cmd_parsed[i].size() + 1];
						std::strcpy(tmp, cmd_parsed[i].c_str());
						//std::cout << tmp << "   ";
						args[i] = tmp;
						//std::cout << args[i] << "\n";
					}
					std::cout << args[0] << "  " << args[1] << "  "  << args[2] << "  "  << args[2] << "  \n";
					for(size_t i = 0; i < paths.size(); i++){
						char cmd_path[paths[i].size() + cmd_parsed[0].size() + 1];
						std::strcpy(cmd_path, (paths[i] + cmd_parsed[0]).c_str());
						execv(cmd_path, args);   // exec() will kill process if command success.
					}
					std::cout << "-wish: " + cmd_raw + ": command not found\n";
					exit(1);
				}	
				wait(NULL);
			}
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

