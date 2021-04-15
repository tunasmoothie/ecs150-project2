#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
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

std::vector<std::vector<std::vector<std::string>>> processLine(std::vector<std::string> line, const bool &mode){
	// ---------------- separate operators out -----------------
	std::vector<std::string> line_separated;
	
	for(auto str : line){
		size_t pos_r = str.find(">");
		size_t pos_p = str.find("&");
		while(pos_r != std::string::npos || pos_p != std::string::npos){
			if(pos_r < pos_p || pos_p == std::string::npos){
				std::string s = str.substr(0, pos_r);
				str = str.substr(pos_r+1, str.size());
				s.erase(std::remove_if( s.begin(), s.end(), [](char c){return std::isspace(c);}), s.end());
				if(!s.empty()) line_separated.push_back(s);
				line_separated.push_back(">");
			}
			else{
				std::string s = str.substr(0, pos_p);
				str = str.substr(pos_p+1, str.size());
				s.erase(std::remove_if( s.begin(), s.end(), [](char c){return std::isspace(c);}), s.end());
				if(!s.empty()) line_separated.push_back(s);
				line_separated.push_back("&");
			}
			
			pos_r = str.find(">");
			pos_p = str.find("&");
		}
		if(!str.empty()) line_separated.push_back(str);
	}
	
	
	// --------- divide into chunks of cmds and operators ---------
	std::vector<std::vector<std::string>> cmd_list;
	auto it1 = line_separated.begin(), it2 = line_separated.begin();
	while(it2 != line_separated.end()){
		if(*it2 == "&" || *it2 == ">"){
			if(it1 != it2){
				std::vector<std::string> cmd(it1, it2);
				cmd_list.push_back(cmd);
			}
			std::vector<std::string> op(it2, it2+1);
			cmd_list.push_back(op);
			it1 = it2+1;
		}
		it2++;
	}
	if(it1 != line_separated.end()){
		std::vector<std::string> cmd(it1, line_separated.end());
		cmd_list.push_back(cmd);
	}
	
	// ------- determine how many threads needed & store in vector -------
	std::vector<std::vector<std::vector<std::string>>> thread_list;
	auto it3 = cmd_list.begin(), it4 = cmd_list.begin();
	while(it4 != cmd_list.end()){
		if((*it4).front() == "&"){
			std::vector<std::vector<std::string>> cmd(it3, it4);
			thread_list.push_back(cmd);
			it3 = it4+1;
		}
		it4++;
	}
	std::vector<std::vector<std::string>> cmd(it3, cmd_list.end());
	thread_list.push_back(cmd);
	
	/*
	for(auto thr : thread_list){
		for(auto cmd : thr){
			for(auto str : cmd){
				std::cout << str << " ";
			}
			std::cout << " | ";
		}
		std::cout << "\n";
	}*/
	
	
	/*
	// ------------------ error detection -------------------
	if(cmd_list[0][0] == ">" || cmd_list[cmd_list.size()-1][0] == ">"){
		printError();
		if(mode == 1) exit(0);
	}
*/
	//std::cout << output[0][0] << "\n";

/*
	for(auto ch : output){
		for(auto st : ch){
			std::cout << st << "  ";
		}
		std::cout << "\n";
	}
	*/
	return thread_list;
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
		// ------------ built-in commands ------------
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
		// ------------ external commands ------------
		else{
			auto proccessed_line = processLine(input_str_vec, mode);	
			
			int child_cnt;
			for(auto cmd : proccessed_line){
				if(cmd.empty())
					continue;
				else{
					pid_t pid = fork();
					// child
					if(pid == 0){
						if(!(cmd.size() == 1 || cmd.size() == 3)){
							printError();
							exit(0);
						}
						else{
							if(cmd.size() == 3){
								if(cmd[2].size() != 1);
								else{
									int fd = open(cmd[2][0].c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
									dup2(fd, 1);
									dup2(fd, 2);
									close(fd);
									executeCmd(cmd[0], paths);
								}
							}
							else{
								executeCmd(cmd[0], paths);
							}
							printError();
							exit(0);
						}
					}
					// parent
					else{ 
						child_cnt++;
					}
				}
			}
			
			while(child_cnt != 0){
				//std::cout << "child " << wait(NULL) << "terminated\n";
				wait(NULL);
				child_cnt--;
			}
			
			/*
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
					int fd = open(processed_line[1][0].c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
					dup2(fd, 1);
					dup2(fd, 2);
					close(fd);
					executeCmd(processed_line[0], paths);
					exit(0);
				}
				else wait(NULL);
			}
			else if(type == 2){
				int child_cnt;
				
				for(auto cmd : processed_line){
					if(cmd.empty())
						continue;
					
					pid_t pid = fork();
					if(pid == 0){
						executeCmd(cmd, paths);
					}
					else{
						child_cnt++;
					}
				}
				
				while(child_cnt != 0){
					//std::cout << "child " << wait(NULL) << "terminated\n";
					wait(NULL);
					child_cnt--;
				}
			}*/
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
