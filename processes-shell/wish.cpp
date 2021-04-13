#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>


std::vector<std::string> convertInputToStr(std::string input_raw){
	std::istringstream ss(input_raw);
	std::vector<std::string> converted;
	std::string word;
	while(ss >> word){
		converted.push_back(word);
	}
	return converted;
}

std::vector<std::vector<std::string>> parseLine(std::vector<std::string>){
	std::vector<std::vector<std::string>> parsedLine;
	return parsedLine;
}

int main(int argc, char *argv[]){
	
	std::string input_raw;
	std::vector<std::string> input_str;
	std::vector<std::string> paths;
	paths.push_back("");
	paths.push_back("/bin/");
	
	// ========================================= command interpretation lambda =========================================
	auto interpretCmd = [&](){
		input_str = convertInputToStr(input_raw);
		if(input_str.size() == 0);
			else if(input_str[0] == "exit"){
				if(input_str.size() != 1)
					std::cerr << "An error has occurred" << std::endl;
				else
					exit(0);
			}
			else if(input_str[0] == "path"){
				paths.clear();
				paths.push_back("");
				for(size_t i = 1; i < input_str.size(); i++){
					if(input_str[i][0] != '/')
						input_str[i] = "/" + input_str[i];
					if(input_str[i][input_str[i].size()-1] != '/')
						input_str[i] += "/";
					char tmp[2048];
					getcwd(tmp, 2048);
					input_str[i] = tmp + input_str[i];
					paths.push_back(input_str[i]);
				}
			}
			else if(input_str[0] == "cd"){
				if(input_str.size() != 2)
					std::cerr << "An error has occurred" << std::endl;
				else if(chdir((char*)input_str[1].c_str()) == 0);
				else
					std::cerr << "An error has occurred" << std::endl;
			}
			else{
				pid_t pid = fork();
				if(pid == 0){
					char *args[input_str.size() + 1];
					
					// string to char array conversion for args
					for(size_t i = 0; i < input_str.size(); i++){
						args[i] = new char[input_str[i].size() + 1];
						std::strcpy(args[i], input_str[i].c_str());
					}
					args[input_str.size()] = NULL;
					
					for(size_t i = 0; i < paths.size(); i++){
						char cmd_path[paths[i].size() + input_str[0].size() + 1];
						std::strcpy(cmd_path, (paths[i] + input_str[0]).c_str());
						execv(cmd_path, args);   // exec() will kill process if command success.
					}
					std::cerr << "An error has occurred" << std::endl;
					exit(1);
				}	
				else{
					wait(NULL);
				}
				
			}
	};
	
	// ============================================= interactive mode =============================================
	if(argc == 1){
		while(1){
			std::cout << "wish> ";
			std::getline(std::cin, input_raw);
			interpretCmd();
		}
	}
	// ============================================= batch mode =============================================
	else if(argc == 2){
		std::ifstream batchFile;
		batchFile.open(argv[1]);
		while(std::getline(batchFile, input_raw)){
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

