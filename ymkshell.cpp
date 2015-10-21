#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <libgen.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <stdio.h>

using namespace std;

struct sigaction act;
string commendLine;
vector<string> commendSet;
int numberOfCommendSet;
// int pid = -2;

void handleCommendLine(string commendLine);
int parseCommend(string commendLine, int *previousFd, int *laterFd);
bool checkRedirction(vector<string> &argv, int &redirectionType, int argc);
// void sighandler(int signum){
// 	pid = -2;
// 	cout << endl;
// }

int main() {
	// act.sa_handler = sighandler;
	// sigemptyset(&act.sa_mask);
	// act.sa_flags = 0;
	// sigaction(SIGINT, &act, NULL);
	// pid = -2;

	while (1) {
		numberOfCommendSet = 0;
		cout << "Yanminkuan: " << get_current_dir_name() << "% ";
		getline(cin, commendLine);
		handleCommendLine(commendLine);
		commendSet.clear();
		commendSet.shrink_to_fit();
	}

	return 0;
}

void handleCommendLine(string commendLine) {

	if(commendLine == "exit")
		exit(0);
	if(commendLine == "")
		return;

	//divide commendLine if pipeline
	int lengthOfString = commendLine.size();
	for (int i=0; i < lengthOfString; i++) {
		int positionOfPipe = commendLine.find('|', 0);
		if(positionOfPipe == -1) {
			commendSet.push_back(commendLine);
			break;
		} else {
			numberOfCommendSet++;
			//push back the previous commendLine before |
			commendSet.push_back(commendLine.substr(0, positionOfPipe));
			//commendLine = line after |
			commendLine = commendLine.substr(positionOfPipe+1, commendLine.size()-positionOfPipe-1);
		}
	}

	if(numberOfCommendSet == 0) {
		//single cmdline
		parseCommend(commendLine, 0, 0);
	} else {
		//if pipeline
		int previousFd[2], laterFd[2];
		pipe(laterFd);
		parseCommend(commendSet[0], 0, laterFd);
		previousFd[0] = laterFd[0]; previousFd[1] = laterFd[1];
		for(int i=1; i < commendSet.size()-1; i++) {
			pipe(laterFd);
			parseCommend(commendSet[i], previousFd, laterFd);
			previousFd[0] = laterFd[0]; previousFd[1] = laterFd[1];
		}
		parseCommend(commendSet[commendSet.size()-1], previousFd, 0);
	}
	// pid = -2;
}

int parseCommend(string commendLine, int *previousFd, int *laterFd) {
	if (commendLine == "") {
		return 0;
	}

	int argc = 0;
	vector<string> argv;
	int quoteStart = -1, quoteEnd = -1;

	//check redirection standard, no <> or >< and parse commendLine
	int lengthOfString = commendLine.size();
	int inputCount = 0, outputCount  = 0;
	string tempArgv;
	for (int i=0; i < lengthOfString; i++) {
		if(commendLine[i] == ' ') {
			if(tempArgv != "") {
				argc++;
				argv.push_back(tempArgv);
				tempArgv="";
			}
			inputCount = outputCount = 0;
		} else if(commendLine[i] ==  '<') {
			inputCount++;
			if(inputCount > 1 || outputCount > 0) {
				cout << "Syntax Error!" << endl;
				return 1;
			}
			if(tempArgv != "") {
				argc++;
				argv.push_back(tempArgv);
			}
			tempArgv="<";
		} else if(commendLine[i] == '>') {
			outputCount++;
			if(inputCount > 0 || outputCount > 2) {
				cout << "Syntax Error!" << endl;
				return 1;
			}
			if(outputCount == 1){
				if(tempArgv != "") {
					argc++;
					argv.push_back(tempArgv);
				}
				tempArgv = ">";
			} else if(outputCount == 2) {
				tempArgv = ">>";
			}
		} else if(commendLine[i] == '"') {
			quoteStart = i;
			quoteEnd = commendLine.find('"', i+1);
			if(quoteEnd != -1) {
				if(tempArgv == "<" || tempArgv ==">" || tempArgv == ">>") {
					argc++;
					argv.push_back(tempArgv);
					tempArgv = commendLine.substr(quoteStart+1, quoteEnd-quoteStart-1);
					inputCount = outputCount = 0;
				} else {
					tempArgv += commendLine.substr(quoteStart+1, quoteEnd-quoteStart-1);
				} 
			} else {
				cout << "Syntax Error!" << endl;
				return 1;
			}
			i = quoteEnd;
		} else {
			if(inputCount > 0 || outputCount > 0) {
				if(tempArgv != "") {
					argc++;
					argv.push_back(tempArgv);
				}
				tempArgv = "";
				inputCount = outputCount = 0;
			}
			tempArgv += commendLine[i];
		}
	}
	if(tempArgv != ""){
		argc++;
		argv.push_back(tempArgv);
	}


	//cd part refer to ZYC...
	if(argv[0] == "exit") {
		argv.clear();
		argv.shrink_to_fit();
		exit(0);
	}else if(argv[0] == "cd") {
		if(argv.size() == 1) {
			argv.push_back(getenv("HOME"));
		}
		if(argv[1] == "~") {
			argv[1] = getenv("HOME");
		}
		if(chdir(argv[1].c_str()) < 0) {
			cout << "Incorrect Path!" << endl;
			return 1;
		}
	}

	//check wether invalid redirection
	int redirectionType;
	if(checkRedirction(argv, redirectionType, argc)) {
		return 1;
	} else {

	}

	return 0;
}

bool checkRedirction(vector<string> &argv, int &redirectionType, int argc) {
	redirectionType = 0;
	int inputPosition = 1024, outputPosition = 1024;
	for(int i=0; i < argc; i++) {
		for(int i=0; i < argc; i++) {
			if(argv[i] == "<") {
				inputPosition = i;
				redirectionType |= 1;
			} else if(argv[i] ==  ">") {
				redirectionType |= 2;
				redirectionType &= 3;
				outputPosition = i;
			} else if(argv[i] == ">>") {
				redirectionType |= 4;
				redirectionType &= 5;
				outputPosition = i;
			}
		}
	}
	cout << "redirectionType: " << redirectionType << endl;
	if(redirectionType > 0) {
		if(redirectionType & 1) {
			if(inputPosition + 1 == argc) {
				cout << "No Input File!" << endl;
				return 1;
			}
			int inputFd = open(argv[inputPosition+1].c_str(), O_RDONLY, S_IREAD|S_IWRITE);
			if(inputFd < 0) {
				cout << "Open Input File Error!" << endl;
				return 1;
			} else {
				dup2(inputFd, STDIN_FILENO);
			}
		}
		if((redirectionType & 2) || (redirectionType & 4)) {
			if(outputPosition + 1 == argc) {
				cout << "No Output File!" << endl;
				return 1;
			}
			if(redirectionType & 2) {
				int outputFd = open(argv[outputPosition + 1].c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
				if(outputFd < 0) {
					cout << "Open Output File Error!" << endl;
				} else {
					dup2(outputFd, STDOUT_FILENO);
				}
			}else if(redirectionType & 4) {
				int outputFd = open(argv[outputPosition + 1].c_str(), O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR);
				if(outputFd < 0) {
					cout << "Open Output File Error!" << endl;
				} else {
					dup2(outputFd, STDOUT_FILENO);
				}				
			}
		}
	}
	cout << "inputPosition: " << inputPosition << endl;
	cout << "outputPosition: " << outputPosition << endl;
	return 0;
}



















