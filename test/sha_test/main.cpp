#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <locale.h>
#include <pwd.h>
#include <signal.h>
#include <sys/inotify.h>
#include <poll.h>
#include <dirent.h>
#include <openssl/sha.h>

#include <string>
#include <fstream>
#include <chrono>
#include <ctime> 
#include <iostream>
#include <thread>
#include <mutex>
#include <functional>
#include <vector>
#include <condition_variable>
#include <queue>
#include <sstream>
#include <atomic>
#include <map>
#include <bitset>

int main() {

	std::string tmp2 = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.138 Safari/537.36";
	unsigned char hash[SHA_DIGEST_LENGTH]; // == 20
	SHA1((const unsigned char *)tmp2.data(), strlen(tmp2.data()), hash);
	char sss[41] = {0};
	for (int i = 0; i < 20; ++i) {
		sprintf(&sss[i * 2], "%x", hash[i]);
	}
	for (int i = 0; i < 40; ++i) {
		if (sss[i] == 0) {
			sss[i] = 48;
		}
	}
	std::cout << sss << '\n';
	std::cout << "79415b55edde02fb521b23f9c69936e4b8905cd0" << '\n';
	return (0);
}
