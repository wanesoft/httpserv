
/*

    Test task for AdGuard
    "Small http-server"
    Marochkin Ivan <wanesoft@mail.ru>
    08/05/2020

*/

#include "Httpserv.hpp"

bool Httpserv::run = true;
std::vector<Httpserv *> Httpserv::vec_servers;

void Httpserv::_put_log(const char *str) {

    /* Logging function */
    std::lock_guard<std::mutex> lock(_print_mutex);
    std::cout << str << '\n';
}   

int Httpserv::_create_connection(int port) {

    /* Here create INET socket */
    int listener, res;
    struct sockaddr_in addr;
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        _put_log("ERROR: socket() not gave listener");
        exit(errno);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Set reusable option for socket */
    int enable = 1;
    if ((setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) < 0) {
        _put_log("ERROR: Can't reuse address");
    }
    /* Try to bind */
    res = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    if (res < 0) {
        _put_log("ERROR: Can't bind address");
        exit(errno);
    }
    res = listen(listener, SOMAXCONN);
    if (res < 0) {
        _put_log("ERROR: listen() crash");
		exit(errno);
    }
    return (listener);
}

void Httpserv::_thread_worker(void) {

    std::map<std::string, int> path_map;                // map-counter for paths
    std::map<std::string, int> agent_map;               // map-counter for agents
    unsigned char path_hash[SHA_DIGEST_LENGTH];         // buffer for SHA1()
    unsigned char agent_hash[SHA_DIGEST_LENGTH];        // buffer for SHA1()
    char path_Xhash[SHA_DIGEST_LENGTH * 2 + 1] = {0};   // buffer for HEX impl sha hash
    char agent_Xhash[SHA_DIGEST_LENGTH * 2 + 1] = {0};  // buffer for HEX impl sha hash
    int path_count;                                     // current path-hitcount
    int agent_count;                                    // current agent-hitcount

    while (Httpserv::run) {

        /* Enable waiting of condition var */
        std::unique_lock<std::mutex> uni_lock(_m);
        while (Httpserv::run && !_notified) {
            _cond_var.wait(uni_lock);
        }
        /* Next lock mutex for general queue */
        _queue_mutex.lock();
        if (!_que.empty()) {
            /* Get values from queue and unlock mutexes */
            std::string path(std::get<0>(_que.front()).data());
            std::string agent(std::get<1>(_que.front()).data());
            _que.pop();
            _notified = true;
            _cond_var.notify_one();
            _queue_mutex.unlock();
            uni_lock.unlock();
            /* Get current counters from maps */
            path_count = path_map[path];
            path_map[path] = path_count + 1;
            agent_count = agent_map[agent];
            agent_map[agent] = agent_count + 1;
            /* Get hash from path and agent */
            SHA1((const unsigned char *)path.data(), strlen(path.data()), path_hash);
			for (int i = 0; i < SHA_DIGEST_LENGTH; i++){
				sprintf(&path_Xhash[i * 2], "%02x", path_hash[i]);
			}
            SHA1((const unsigned char *)agent.data(), strlen(agent.data()), agent_hash);
			for (int i = 0; i < SHA_DIGEST_LENGTH; i++){
				sprintf(&agent_Xhash[i * 2], "%02x", agent_hash[i]);
			}
            /* Lock print-mutex and print to STDOUT like this view:
            <thread id> <path> <path-sha1> <path-hitcount> <user-agent>
            <user-agent-sha1> <user-agent-hitcount> */

            // std::this_thread::sleep_for(std::chrono::milliseconds(5));

            std::lock_guard<std::mutex> lock(_print_mutex);
            std::cout   << '<' << std::this_thread::get_id() << "> "    \
                        << '<' << path << "> "                          \
                        << '<' << path_Xhash << "> "                    \
                        << '<' << path_count << "> "                    \
                        << '<' << agent << "> "                         \
                        << '<' << agent_Xhash << "> "                   \
                        << '<' << agent_count << ">\n";
            /* If general queue empty - just wait again */
        } else {
            _notified = false;
            uni_lock.unlock();
            _queue_mutex.unlock();
        }
    }
}

std::pair<std::string, std::string> Httpserv::_parser(char *buff) {
    
    /* Pars path after GET and User-agent string in buffer
    If some one not found - return EMPTY string */
    std::string tmp(buff);
    std::string ret1, ret2;
    auto pos1 = tmp.find("GET ");
    if (pos1 == std::string::npos) {
        return (std::make_pair(ret1, ret2));
    }
    auto pos2 = tmp.find(" HTTP/1.1");
    if (pos2 == std::string::npos) {
        return (std::make_pair(ret1, ret2));
    }
    ret1 = tmp.substr(pos1 + 4, pos2 - 4);
    pos1 = tmp.find("\nUser-Agent: ");
    if (pos1 == std::string::npos) {
        return (std::make_pair(ret1, ret2));
    }
    pos2 = tmp.find("\r\n", pos1 + 1);
    if (pos2 == std::string::npos) {
        return (std::make_pair(ret1, ret2));
    }
    ret2 = tmp.substr(pos1 + 13, pos2 - pos1 - 13);
    return (std::make_pair(ret1, ret2));
}

Httpserv::Httpserv(int threads, int port) {

    // Httpserv::server_counter++;
    Httpserv::vec_servers.push_back(this);
    signal(SIGINT, Httpserv::stop);
	_number_of_threads = threads;
	// Httpserv::arr_of_gen_sockets[Httpserv::server_counter] = _create_connection(port);
    // _general_socket = Httpserv::arr_of_gen_sockets[Httpserv::server_counter];
    _port = port;
    _general_socket = _create_connection(port);
}

Httpserv::~Httpserv(void) {

    for (auto &cur : _vec_threads) {
        _put_log("stooooooooooop");
        cur.join();
    }
}

int Httpserv::main_cycle() {

    /* Preparing */
    std::cout << "Server started" << '\n';
    char buff[HTTPSERV_BUFSIZE];
    /* Start threads */
    for (int i = 0; i < _number_of_threads; ++i) {
        _vec_threads.push_back(std::thread(&Httpserv::_thread_worker, this));
    }
    /* Just good-answer-string */
    char str_ok[] = "HTTP/1.1 200 OK\r\n\
					Content-Type: text/html; charset=utf-8\r\n\
					Content-Length: 16\r\nConnection: close\r\n\r\n\
					<h1>Hello</h1>\r\n";
    /* Start main cycle */
    while (Httpserv::run) {
        memset(buff, 0, HTTPSERV_BUFSIZE);
        _put_log("nachal");
        int cur_sock = accept(_general_socket, 0, 0);
        _put_log("zakoncjil");
        recv(cur_sock, buff, HTTPSERV_BUFSIZE, MSG_NOSIGNAL);
        /* If some one string is empty - send Bad request message */
        std::pair<std::string, std::string> tmp = _parser(buff);
        if (std::get<0>(tmp).empty() || std::get<1>(tmp).empty()) {
            send(cur_sock, "HTTP/1.1 400 Bad Request", strlen("HTTP/1.1 400 Bad Request"), MSG_NOSIGNAL);
            close(cur_sock);
            continue;
        }
        /* Else send 200 OK and close connection */
        send(cur_sock, str_ok, strlen(str_ok), MSG_NOSIGNAL);
        close(cur_sock);
        /* Lock queue-mutex, push new data to general queue and call to threads for work */
        std::unique_lock<std::mutex> qlock(_queue_mutex);
        _que.push(tmp);
        _notified = true;
        _cond_var.notify_one();
    }
    return (0);
}

void Httpserv::stop(int signo) {

    Httpserv::run = false;

    for (auto cur : Httpserv::vec_servers) {

        if (!cur) {
            break;
        }

        struct sockaddr_in addr;
        int res;
        int general_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (general_socket < 0) {
            perror("socket() not gave listener");
        }
        addr.sin_family = AF_INET;
        addr.sin_port = htons(cur->_port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        res = connect(general_socket, (struct sockaddr *)&addr, sizeof(addr));
        if (res < 0) {
            cur->_put_log("break accept() in main_cycle failed");
        }
        close(general_socket);

        for (int i = 0; i < cur->_number_of_threads; ++i) {
            cur->_notified = true;
            cur->_cond_var.notify_one();
        }

        cur->_put_log("Server stoped");
    } 
}