
/*

    Test task for AdGuard
    "Small http-server"
    Marochkin Ivan <wanesoft@mail.ru>
    08/05/2020

*/

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

int connection_to_server(const char *str) {

    in_addr_t addr_i = 0;
    char *iter = (char *)&addr_i;
    char *tmp = strdup(str);
    for (int i = 0; tmp[i]; ++i) {
        *iter = atoi(&tmp[i]);
        while (tmp[i] && tmp[i] != '.') {
            ++i;
        }
        ++iter;
    }
    free(tmp);
    struct sockaddr_in addr;
    int res;
    int general_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (general_socket < 0) {
        perror("socket() not gave listener");
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = addr_i;
    res = connect(general_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (res < 0) {
        perror("connect() failed");
        exit(EXIT_FAILURE);
    }
    return (general_socket);
}

int main(int ac, char **av) {

	if (ac < 2) {
		std::cout << "usage ./test2 [ip-addr-serv]\n";
		return (0);
	}

	char str[] = "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: curl/TEST.69.0\r\nAccept: */*\r\n\r\n";

	for (int j = 0; j < 500; ++j) {
		pid_t ch = fork();
		if (!ch) {
			for (int i = 0; i < 1000; ++i) {
				int sent_sock = connection_to_server(av[1]);
				char buff[1024] = {0};
				int se = send(sent_sock, str, strlen(str), MSG_NOSIGNAL);
				int re = recv(sent_sock, buff, 1024, MSG_NOSIGNAL);
				// std::cout << "send: " << se << ", recv: " << re << ", " << buff << '\n';
				close(sent_sock);
			}
			exit(EXIT_SUCCESS);
		}
	}
    wait(0);
    std::cout << "done\n";

	return (0);
}