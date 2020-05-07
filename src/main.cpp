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

std::mutex              g_print_mutex;
std::queue<std::string> g_que;
std::mutex              m;
std::condition_variable cond_var;
bool                    notified = false;

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
    res = listen(listener, 0);
    if (res < 0) {
        put_log("ERROR: listen() crash");
		exit(errno);
    }
    return (listener);
}

void thread_worker(void) {

    auto myid = std::this_thread::get_id();
    std::stringstream tmp;
    tmp << "Started: " << myid;
    put_log(tmp.str().data());
    while (1) {
        std::unique_lock<std::mutex> lock(m);
        std::cout << "Check 1\n";
        std::cout << "Check 2\n";
        while (!notified) {  // loop to avoid spurious wakeups
            std::cout << "Check 3\n";
            cond_var.wait(lock);
        }
        while (!g_que.empty()) {
            std::cout << "I: " << myid;
            std::cout << "consuming " << g_que.front() << '\n';
            g_que.pop();
        }
        notified = false;
    }
}

int main() {

    int gen_sock = connection_for_incoming();
    std::vector<std::thread> vec_threads;
    std::cout << "Done " << gen_sock << '\n';
    char buff[1024];

    for (int i = 0; i < VOL_OF_THREAD; ++i) {
        vec_threads.push_back(std::thread(thread_worker));
    }

    while (1) {

        int cur_sock = accept(gen_sock, 0, 0);
        memset(buff, 0, 1024);
        int recv_res = recv(cur_sock, buff, 1024, MSG_NOSIGNAL);
        int send_res = send(cur_sock, "HTTP/1.1 200 OK", strlen("HTTP/1.1 200 OK"), MSG_NOSIGNAL);
        std::unique_lock<std::mutex> lock(m);
        g_que.push(std::string(buff));
        notified = true;
        cond_var.notify_one();
        // std::cout << "i recv: " << recv_res << '\n';
        // std::cout << "i send: " << send_res << '\n';
        close(cur_sock);
        // std::cout << buff;
    }

    return (1);
}

// #include <condition_variable>
// #include <mutex>
// #include <thread>
// #include <iostream>
// #include <queue>
// #include <chrono>
 
// int main()
// {
//     std::queue<int> produced_nums;
//     std::mutex m;
//     std::condition_variable cond_var;
//     bool done = false;
//     bool notified = false;
 
//     std::thread producer([&]() {
//         for (int i = 0; i < 5; ++i) {
//             std::this_thread::sleep_for(std::chrono::seconds(1));
//             std::unique_lock<std::mutex> lock(m);
//             std::cout << "producing " << i << '\n';
//             produced_nums.push(i);
//             notified = true;
//             cond_var.notify_one();
//         }
 
//         done = true;
//         cond_var.notify_one();
//     });
 
//     std::thread consumer([&]() {
//         std::unique_lock<std::mutex> lock(m);
//         std::cout << "Check 1\n";
//         while (!done) {
//             std::cout << "Check 2\n";
//             while (!notified) {  // loop to avoid spurious wakeups
//                 std::cout << "Check 3\n";
//                 cond_var.wait(lock);
//             }
//             while (!produced_nums.empty()) {
//                 std::cout << "consuming " << produced_nums.front() << '\n';
//                 produced_nums.pop();
//             }
//             notified = false;
//         }
//     });
 
//     producer.join();
//     consumer.join();
// }