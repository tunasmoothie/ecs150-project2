#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

void printError(){
	std::cerr << "An error has occurred" << std::endl;
}

std::vector<std::string> convertInputToStr(std::string input_raw){
	std::istringstream ss(input_raw);
	std::vector<std::string> converted;
	std::string word;
	while(ss >> word){
		converted.push_back(word);
	}
	return converted;
}

std::vector<std::vector<std::string>> parseLine(std::vector<std::string> line, int &mode){
	
	std::vector<std::vector<std::string>> output;
	// 0 = Normal, 1 = Redirect, 2 = Parallel
	mode = 0;
	
	//separate operators out
	for(size_t i = 0; i < line.size(); i++){
		size_t symbolPos = line[i].find(">");
		if(symbolPos != std::string::npos){
			mode = (mode == 1) ? (printError(), exit(0), 0) : 1;
			if(line[i].size() != 1){
				line.pop_back();
				int k = 0;
				std::string p1 = line[i].substr(0, symbolPos);
				std::string p2 = line[i].substr(symbolPos+1, line[i].size());
				if(!p1.empty())
					line.insert(line.begin()+i+(k++), p1);
				line.insert(line.begin()+i+(k++), ">");
				if(!p2.empty())
					line.insert(line.begin()+i+(k++), p2);
				i += k;
			}
		}
	}
		
	if(mode == 1){
		if(line[0] == ">" || line[line.size()-1] == ">"){
			printError();
			exit(0);
		}
		
		auto it = line.begin();
		while(*it != ">") it++;
		std::vector<std::string> cmd(line.begin(), it);
		std::vector<std::string> file(it+1, it+2);
		if(it+2 != line.end()){
			printError();
			exit(0);
		}
		
		output.push_back(cmd);
		output.push_back(file);
	}
	

	return output;
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
				printError();
			else
				exit(0);
		}
		// -------- built-in commands --------
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
				printError();
			else if(chdir((char*)input_str[1].c_str()) == 0);
			else
				printError();
		}
		// -------- external commands --------
		else{
			// 0 = Normal, 1 = Redirect, 2 = Parallel
			int mode = 0;
			parseLine(input_str, mode);	
			
			/*
			for(auto a : commands){
				for(auto b : a){
					std::cout << b << "  ";
				}
				std::cout << std::endl;
			}*/
			
			pid_t pid = fork();
			if(pid == 0){
				if(mode == 0){
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
						execv(cmd_path, args); 
						// exec() will cut subsequent error handling if command succeeds.
					}
					printError();
					exit(1);
				}	
				else if(mode == 1){
					/*
					int fd = open(commands[0][2].c_str(), O_RDWR | O_CREAT);
					dup2(fd, 1);
					dup2(fd, 2);
					//close(fd);
					
					char *args[commands[0].size() + 1];
					// string to char array conversion for args
					for(size_t i = 0; i < commands[0].size(); i++){
						args[i] = new char[commands[0][i].size() + 1];
						std::strcpy(args[i], commands[0][i].c_str());
					}
					args[input_str.size()] = NULL;
					for(size_t i = 0; i < paths.size(); i++){
						char cmd_path[paths[i].size() + input_str[0].size() + 1];
						std::strcpy(cmd_path, (paths[i] + input_str[0]).c_str());
						execv(cmd_path, args);  
						// exec() will cut subsequent error handling if command succeeds.
					}
					printError();
					exit(1);*/
				}
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

