/* *************************************************************************************************** */
/*                                                                                                     */
/*                                                          :::::::::   ::::::::   ::::::::      :::   */
/*  main.cpp                                               :+:    :+: :+:    :+: :+:    :+:   :+: :+:  */
/*                                                        +:+    +:+ +:+    +:+ +:+         +:+   +:+  */
/*  By: Ivan Marochkin <i.marochkin@rosalinux.ru>        +#++:++#:  +#+    +:+ +#++:++#++ +#++:++#++:  */
/*                                                      +#+    +#+ +#+    +#+        +#+ +#+     +#+   */
/*  Created: 2020/03/05 16:36:21 by Ivan Marochkin     #+#    #+# #+#    #+# #+#    #+# #+#     #+#    */
/*  Updated: 2020/03/11 15:34:28 by Ivan Marochkin    ###    ###  ########   ########  ###     ###     */
/*                                                                                                     */
/* *************************************************************************************************** */

#include "main.hpp"

std::mutex                                          g_print_mutex;
std::mutex                                          g_queue_mutex;
std::queue<std::pair<std::string, std::string>>     g_que;
std::mutex                                          g_m;
std::condition_variable                             g_cond_var;
std::atomic<bool>                                   g_notified{false};

std::atomic<int> g_i{0};

void put_log(const char *str) {

    std::lock_guard<std::mutex> lock(g_print_mutex);
    std::cout << str << '\n';
}   

int connection_for_incoming(void) {

    int listener, res;
    struct sockaddr_in addr;
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        put_log("ERROR: socket() not gave listener");
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int enable = 1;
    if ((setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) < 0) {
        put_log("ERROR: Can't reuse address");
    }
    res = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    if (res < 0) {
        put_log("ERROR: Can't bind address");
        exit(errno);
    }
    res = listen(listener, SOMAXCONN);
    if (res < 0) {
        put_log("ERROR: listen() crash");
		exit(errno);
    }
    return (listener);
}

void thread_worker(void) {

    std::map<std::string, int> path_map;
    std::map<std::string, int> agent_map;
    unsigned char path_hash[SHA_DIGEST_LENGTH];
    unsigned char agent_hash[SHA_DIGEST_LENGTH];
    char path_Xhash[41] = {0};
    char agent_Xhash[41] = {0};
    int path_count;
    int agent_count;

    while (1) {

        std::unique_lock<std::mutex> uni_lock(g_m);

        while (!g_notified) {
            g_cond_var.wait(uni_lock);
        }

        g_queue_mutex.lock();
        if (!g_que.empty()) {

            std::string path(std::get<0>(g_que.front()).data());
            std::string agent(std::get<1>(g_que.front()).data());
            g_que.pop();
            g_notified = true;
            g_cond_var.notify_one();
            g_queue_mutex.unlock();
            uni_lock.unlock();

            path_count = path_map[path];
            path_map[path] = path_count + 1;

            agent_count = agent_map[agent];
            agent_map[agent] = agent_count + 1;

            SHA1((const unsigned char *)path.data(), strlen(path.data()), path_hash);
            for (int i = 0; i < 20; ++i) {
                sprintf(&path_Xhash[i * 2], "%x", path_hash[i]);
            }
            for (int i = 0; i < 40; ++i) {
                if (path_Xhash[i] == 0) {
                    path_Xhash[i] = 48;
                }
            }

            SHA1((const unsigned char *)agent.data(), strlen(agent.data()), agent_hash);
            for (int i = 0; i < 20; ++i) {
                sprintf(&agent_Xhash[i * 2], "%x", agent_hash[i]);
            }
            for (int i = 0; i < 40; ++i) {
                if (agent_Xhash[i] == 0) {
                    agent_Xhash[i] = 48;
                }
            }

            /*<thread id> <path> <path-sha1> <path-hitcount> <user-agent>
            <user-agent-sha1> <user-agent-hitcount>*/
            std::lock_guard<std::mutex> lock(g_print_mutex);
            std::cout   << '<' << std::this_thread::get_id() << "> "    \
                        << '<' << path << "> "                          \
                        << '<' << path_Xhash << "> "                    \
                        << '<' << path_count << "> "                    \
                        << '<' << agent << "> "                         \
                        << '<' << agent_Xhash << "> "                   \
                        << '<' << agent_count << ">\n";

        } else {
            g_notified = false;
            uni_lock.unlock();
            g_queue_mutex.unlock();
        }
        // for (int i = 10000000; i; --i) {}
    }
}

std::pair<std::string, std::string> parser(char *buff) {

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
    // put_log(std::string("|" + ret1 + "|" + std::to_string(pos1) + " " + std::to_string(pos2)).data());
    pos1 = tmp.find("\nUser-Agent: ");
    if (pos1 == std::string::npos) {
        return (std::make_pair(ret1, ret2));
    }
    pos2 = tmp.find("\r\n", pos1 + 1);
    if (pos2 == std::string::npos) {
        return (std::make_pair(ret1, ret2));
    }
    ret2 = tmp.substr(pos1 + 13, pos2 - pos1 - 13);
    // put_log(std::string("|" + ret2 + "|"  + std::to_string(pos1) + " " + std::to_string(pos2)).data());
    return (std::make_pair(ret1, ret2));
}

int main() {

    int gen_sock = connection_for_incoming();
    std::vector<std::thread> vec_threads;
    std::cout << "Done " << gen_sock << '\n';
    char buff[1024];

    for (int i = 0; i < VOL_OF_THREADS; ++i) {
        vec_threads.push_back(std::thread(thread_worker));
    }

    int i = 0;
    char str_ok[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: 16\r\nConnection: close\r\n\r\n<h1>Hello</h1>\r\n";

    while (1) {

        int cur_sock = accept(gen_sock, 0, 0);
        memset(buff, 0, 1024);
        // put_log( (std::to_string(cur_sock) + " cur sock").data() );
        // int recv_res = recv(cur_sock, buff, 1024, MSG_NOSIGNAL);
        recv(cur_sock, buff, 1024, MSG_NOSIGNAL);
        std::pair<std::string, std::string> tmp = parser(buff);
        if (std::get<0>(tmp).empty() || std::get<1>(tmp).empty()) {
            send(cur_sock, "HTTP/1.1 400 Bad Request", strlen("HTTP/1.1 400 Bad Request"), MSG_NOSIGNAL);
            close(cur_sock);
            continue;
        }
        // int send_res = send(cur_sock, str_ok, strlen(str_ok), MSG_NOSIGNAL);
        send(cur_sock, str_ok, strlen(str_ok), MSG_NOSIGNAL);
        // put_log( (std::to_string(recv_res) + " " + std::to_string(send_res) + " " + std::to_string(i)).data() );
        close(cur_sock);
        std::unique_lock<std::mutex> lock(g_m);
        {
            std::unique_lock<std::mutex> qlock(g_queue_mutex);
            g_que.push(tmp);
            // put_log( (std::to_string(g_que.size()) + " " + std::to_string(i)).data() );
            g_notified = true;
            g_cond_var.notify_one();
            ++i;
        }
    }

    return (0);
}