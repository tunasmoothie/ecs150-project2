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

void executeCmd(const std::vector<std::string> &cmd_str, const std::vector<std::string> &paths){
	char *args[cmd_str.size() + 1];
					
	// string to char array conversion for args
	for(size_t i = 0; i < cmd_str.size(); i++){
		args[i] = new char[cmd_str[i].size() + 1];
		std::strcpy(args[i], cmd_str[i].c_str());
	}
	args[cmd_str.size()] = NULL;
					
	for(size_t i = 0; i < paths.size(); i++){
		char cmd_path[paths[i].size() + cmd_str[0].size() + 1];
		std::strcpy(cmd_path, (paths[i] + cmd_str[0]).c_str());
		execv(cmd_path, args); 
		// exec() will cut subsequent error handling if command succeeds.
	}
	printError();
	exit(1);
}

std::vector<std::string> convertInputToStrVec(std::string input_raw){
	std::istringstream ss(input_raw);
	std::vector<std::string> converted;
	std::string word;
	while(ss >> word){
		converted.push_back(word);
	}
	return converted;
}

std::vector<std::vector<std::string>> processLine(std::vector<std::string> line, int &type, const bool &mode){
	std::vector<std::vector<std::string>> output;
	
	// 0 = Normal, 1 = Redirect, 2 = Parallel
	type = 0;
	
	//separate operators out
	for(size_t i = 0; i < line.size(); i++){
		size_t symbolPos = line[i].find(">");
		if(symbolPos != std::string::npos){
			type = 1;
			if(line[i].size() != 1){
				std::string tmp = line[i];
				line.pop_back();
				int k = 0;
				std::string p1 = tmp.substr(0, symbolPos);
				std::string p2 = tmp.substr(symbolPos+1, tmp.size());
				if(!p1.empty())
					line.insert(line.begin()+i+(k++), p1);
				line.insert(line.begin()+i+(k++), ">");
				if(!p2.empty())
					line.insert(line.begin()+i+(k++), p2);
			}
		}
	}
	for(size_t i = 0; i < line.size(); i++){
		size_t symbolPos = line[i].find("&");
		if(symbolPos != std::string::npos){
			type = 2;
			if(line[i].size() != 1){
				std::string tmp = line[i];
				line.pop_back();
				int k = 0;
				std::string p1 = tmp.substr(0, symbolPos);
				std::string p2 = tmp.substr(symbolPos+1, tmp.size());
				if(!p1.empty())
					line.insert(line.begin()+i+(k++), p1);
				line.insert(line.begin()+i+(k++), "&");
				if(!p2.empty())
					line.insert(line.begin()+i+(k++), p2);
			}
		}
	}
		
		
	if(type == 1){
		int opCnt;
		for(auto str : line)
			if(str == ">") opCnt++;
		if(line[0] == ">" || line[line.size()-1] == ">" || opCnt > 1){
			printError();
			if(mode == 1) exit(0);
		}
		else{
			auto it = line.begin();
			while(*it != ">") it++;
			std::vector<std::string> cmd(line.begin(), it);
			std::vector<std::string> file(it+1, it+2);
			if(it+2 != line.end()){
				printError();
				if(mode == 1) exit(0);
			}
			output.push_back(cmd);
			output.push_back(file);
		}	
	}
	else if(type == 2){
		auto it1 = line.begin(), it2 = line.begin();
		while(it2 != line.end()){
			if(*it2 == "&"){
				std::vector<std::string> cmd(it1, it2);
				output.push_back(cmd);
				it1 = it2+1;
			}
			it2++;
		}
		std::vector<std::string> cmd(it1, line.end());
		output.push_back(cmd);
	}

	return output;
}

int main(int argc, char *argv[]){
	std::string input_raw;
	std::vector<std::string> input_str_vec;
	std::vector<std::string> paths;
	paths.push_back("");
	paths.push_back("/bin/");
	
	// ========================================= command interpretation lambda =========================================
	auto interpretCmd = [&](const bool &mode){ //0 = console, 1 = batch
		input_str_vec = convertInputToStrVec(input_raw);
		if(input_str_vec.size() == 0);
		else if(input_str_vec[0] == "exit"){
			if(input_str_vec.size() != 1)
				printError();
			else
				exit(0);
		}
		// -------- built-in commands --------
		else if(input_str_vec[0] == "path"){
			paths.clear();
			paths.push_back("");
			for(size_t i = 1; i < input_str_vec.size(); i++){
				if(input_str_vec[i][0] != '/')
					input_str_vec[i] = "/" + input_str_vec[i];
				if(input_str_vec[i][input_str_vec[i].size()-1] != '/')
					input_str_vec[i] += "/";
				char tmp[2048];
				getcwd(tmp, 2048);
				input_str_vec[i] = tmp + input_str_vec[i];
				paths.push_back(input_str_vec[i]);
			}
		}
		else if(input_str_vec[0] == "cd"){
			if(input_str_vec.size() != 2)
				printError();
			else if(chdir((char*)input_str_vec[1].c_str()) == 0);
			else
				printError();
		}
		// -------- external commands --------
		else{
			// 0 = Normal, 1 = Redirect, 2 = Parallel
			int type = 0;
			auto processed_line = processLine(input_str_vec, type, mode);	
			
			if(type == 0){
				pid_t pid = fork();
				if(pid == 0){
					executeCmd(input_str_vec, paths);
				}
				else wait(NULL);
			}
			else if(type == 1){
				pid_t pid = fork();
				if(pid == 0){
					int fd = open(processed_line[1][0].c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
					dup2(fd, 1);
					dup2(fd, 2);
					close(fd);
					executeCmd(processed_line[0], paths);
					exit(0);
				}
				else wait(NULL);
			}
			else if(type == 2){
				//std::vector<pid_t> child_ids;
				int child_cnt;
				
				for(auto cmd : processed_line){
					if(cmd.empty())
						continue;
					
					pid_t pid = fork();
					if(pid == 0){
						executeCmd(cmd, paths);
					}
					else{
						//std::cout << "child id: " << pid << std::endl;
						child_cnt++;
						//child_ids.push_back(pid);
					}
				}
				
				while(child_cnt != 0){
					wait(NULL);
					child_cnt--;
				}
			}
		}
	    
	};
	
	// ============================================= interactive mode =============================================
	if(argc == 1){
		while(1){
			std::cout << "wish> ";
			std::getline(std::cin, input_raw);
			interpretCmd(0);
		}
	}
	// ============================================= batch mode =============================================
	else if(argc == 2){
		std::ifstream batFile(argv[1]);
		if(batFile.fail()){
			printError();
			exit(1);
		}
		while(std::getline(batFile, input_raw)){
			interpretCmd(0);
		}
		batFile.close();
	}
	else{
		printError();
		exit(1);
	}
	exit(0);
}
