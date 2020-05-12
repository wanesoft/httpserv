
/*

    Test task for AdGuard
    "Small http-server"
    Marochkin Ivan <wanesoft@mail.ru>
    08/05/2020

*/

#ifndef __HTTPSERV_HPP__
#define __HTTPSERV_HPP__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <signal.h>
#include <fcntl.h>

#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <map>

#define VOL_OF_THREADS		5
#define HTTPSERV_BUFSIZE	1024
#define MAX_SERVERS			4

typedef std::queue<std::pair<std::string, std::string>> gen_queue;

class Httpserv {

private:
	std::mutex					_print_mutex;
	std::mutex					_queue_mutex;
	gen_queue					_que;
	std::mutex					_m;
	std::condition_variable		_cond_var;
	std::atomic<bool>			_notified{false};
	std::vector<std::thread>	_vec_threads;
	int							_general_socket;
	int							_number_of_threads;
	int							_port;

	int _create_connection(int port);
	void _put_log(const char *str);
	void _thread_worker(void);
	std::pair<std::string, std::string> _parser(char *buff);

public:
	static bool run;
	static int server_counter;
	static std::vector<Httpserv *> vec_servers;
	Httpserv(int threads, int port);
	~Httpserv(void);
	Httpserv(const Httpserv &other) = delete;
	Httpserv(const Httpserv &&other) noexcept = delete;
	Httpserv &operator=(const Httpserv &other) = delete;
	Httpserv &operator=(Httpserv &&other) noexcept = delete;
	int main_cycle();
	static void stop(int signo);
	static void stop2(int signo);
};

#endif /* __HTTPSERV_HPP__ */