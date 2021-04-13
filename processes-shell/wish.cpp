#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>


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
	
	std::string cmd_raw;
	std::vector<std::string> cmd_parsed;
	std::vector<std::string> paths;
	paths.push_back("");
	paths.push_back("/bin/");
	
	// ========================================= command interpretation lambda =========================================
	auto interpretCmd = [&](){
		cmd_parsed = parseCmd(cmd_raw);
		if(cmd_parsed.size() == 0);
			else if(cmd_parsed[0] == "exit"){
				if(cmd_parsed.size() != 1)
					std::cerr << "An error has occurred" << std::endl;
				else
					exit(0);
			}
			else if(cmd_parsed[0] == "path"){
				paths.clear();
				paths.push_back("");
				for(size_t i = 1; i < cmd_parsed.size(); i++){
					if(cmd_parsed[i][0] != '/')
						cmd_parsed[i] = "/" + cmd_parsed[i];
					if(cmd_parsed[i][cmd_parsed[i].size()-1] != '/')
						cmd_parsed[i] += "/";
					char tmp[2048];
					getcwd(tmp, 2048);
					cmd_parsed[i] = tmp + cmd_parsed[i];
					paths.push_back(cmd_parsed[i]);
				}
			}
			else if(cmd_parsed[0] == "cd"){
				if(cmd_parsed.size() != 2)
					std::cerr << "An error has occurred" << std::endl;
				else if(chdir((char*)cmd_parsed[1].c_str()) == 0);
				else
					std::cerr << "An error has occurred" << std::endl;
			}
			else{
				pid_t pid = fork();
				if(pid == 0){
					char *args[cmd_parsed.size() + 1];
					
					// string to char array conversion for args
					for(size_t i = 0; i < cmd_parsed.size(); i++){
						args[i] = new char[cmd_parsed[i].size() + 1];
						std::strcpy(args[i], cmd_parsed[i].c_str());
					}
					args[cmd_parsed.size()] = NULL;
					
					for(size_t i = 0; i < paths.size(); i++){
						char cmd_path[paths[i].size() + cmd_parsed[0].size() + 1];
						std::strcpy(cmd_path, (paths[i] + cmd_parsed[0]).c_str());
						execv(cmd_path, args);   // exec() will kill process if command success.
					}
					std::cerr << "An error has occurred" << std::endl;
					exit(1);
				}	
				else{
					pid_t wait = wait(NULL);
				}
				
			}
	};
	
	// ============================================= interactive mode =============================================
	if(argc == 1){
		while(1){
			std::cout << "wish> ";
			std::getline(std::cin, cmd_raw);
			interpretCmd();
		}
	}
	// ============================================= batch mode =============================================
	else if(argc == 2){
		std::ifstream batchFile;
		batchFile.open(argv[1]);
		while(std::getline(batchFile, cmd_raw)){
			interpretCmd();
		}
		batchFile.close();
	}
	else{
		std::cerr << "An error has occured" << std::endl;
		exit(1);
	}
	exit(0);
}

