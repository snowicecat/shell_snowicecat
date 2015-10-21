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
	}

	return 0;
}

void handleCommendLine(string commendLine) {

	if(commendLine == "exit" || commendLine == "q")
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
			commendLine = commendLine.substr(positionOfPipe+1, commendLine.size()-positionOfPipe+1);
		}
	}

	if(numberOfCommendSet == 0) {
		
	}

	// pid = -2;

}