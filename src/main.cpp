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
std::mutex              g_queue_mutex;
std::queue<std::string> g_que;
std::mutex              m;
std::condition_variable cond_var;
std::atomic<bool>       notified{false};

std::atomic<int> g_i{0};

void put_log(const char *str) {

    std::lock_guard<std::mutex> lock(g_print_mutex);
    std::cout << "[" << std::this_thread::get_id() << "] " << str << '\n';
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

    while (1) {
        // std::this_thread::sleep_for(std::chrono::seconds(1));
        std::unique_lock<std::mutex> lock(m);
        while (!notified) {  // loop to avoid spurious wakeups
            cond_var.wait(lock);
        }
        // lock.unlock();
        // while (!g_que.empty()) {
        //     {
        //         std::unique_lock<std::mutex> qlock(g_queue_mutex);
        //         g_que.pop();
        //     }
        //     std::string gaga(std::to_string(g_que.size()).data());
        //     gaga += " size of queue after pop()";
        //     put_log(gaga.data());
        //     std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // }
        // notified = false;
        g_queue_mutex.lock();
        if (!g_que.empty()) {
            g_que.pop();
            notified = true;
            cond_var.notify_one();
            g_queue_mutex.unlock();
            lock.unlock();
            std::string gaga(std::to_string(g_que.size()).data());
            gaga += " size of queue after pop()";
            put_log(gaga.data());
            // std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
            notified = false;
            lock.unlock();
            g_queue_mutex.unlock();
        }
        for (int i = 20000000; i; --i) {}
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

    int i = 0;

    while (1) {

        int cur_sock = accept(gen_sock, 0, 0);
        memset(buff, 0, 1024);
        int recv_res = recv(cur_sock, buff, 1024, MSG_NOSIGNAL);
        int send_res = send(cur_sock, "HTTP/1.1 200 OK", strlen("HTTP/1.1 200 OK"), MSG_NOSIGNAL);
        put_log( (std::to_string(recv_res) + " " + std::to_string(send_res) + " " + std::to_string(i)).data() );
        close(cur_sock);
        std::unique_lock<std::mutex> lock(m);
        {
            std::unique_lock<std::mutex> qlock(g_queue_mutex);
            g_que.push(std::string(buff));
            put_log( (std::to_string(g_que.size()) + " " + std::to_string(i)).data() );
            notified = true;
            cond_var.notify_one();
            ++i;
        }
    }

    return (1);
}

// #include <cstdlib>
// #include <iostream>

// #include <thread>
// #include <mutex>
// #include <condition_variable>
// #include <chrono>

// #define STORAGE_MIN 3
// #define STORAGE_MAX 20

// int storage = STORAGE_MIN;

// std::mutex globalMutex;
// std::condition_variable condition;

// std::mutex g_pr;

// /* Функция потока потребителя */

// void putl(std::string &str) {

//     std::lock_guard<std::mutex> lock(g_pr);
//     std::cout << std::this_thread::get_id() << " " << str << std::endl;
// }

// void consumer()
// {
// 	std::cout << "[CONSUMER] thread started" << std::endl;
// 	int toConsume = 0;
	
// 	while(true)
// 	{
// 		std::unique_lock<std::mutex> lock(globalMutex);
// 		/* Если значение общей переменной меньше максимального, 
// 		 * то поток входит в состояние ожидания сигнала о достижении
// 		 * максимума */
// 		if (storage < STORAGE_MAX)
// 		{
// 			condition.wait(lock); // Атомарно _отпускает мьютекс_ и сразу же блокирует поток
// 			toConsume = STORAGE_MIN;
// 			std::cout << "[CONSUMER] storage is maximum, consuming "
// 				      << toConsume << std::endl;
// 		}
// 		/* "Потребление" допустимого объема из значения общей 
// 		 * переменной */
// 		storage -= toConsume;
// 		std::cout << "[CONSUMER] storage = " << storage << std::endl;
// 	}
// }

// /* Функция потока производителя */
// void producer()
// {
// 	std::cout << "[PRODUCER] thread started" << std::endl;

// 	while (true)
// 	{
// 		std::this_thread::sleep_for(std::chrono::milliseconds(200));  
    
// 		std::unique_lock<std::mutex> lock(globalMutex);
// 		++storage;
// 		std::cout << "[PRODUCER] storage = " <<  storage << std::endl;
// 		/* Если значение общей переменной достигло или превысило
// 		 * максимум, поток потребитель уведомляется об этом */
// 		if (storage >= STORAGE_MAX)
// 		{
// 			std::cout << "[PRODUCER] storage maximum" << std::endl;
// 			condition.notify_one();
// 		}
// 	}
// }

// int main(int argc, char *argv[])
// {
// 	std::thread thProducer(producer);
// 	std::thread thConsumer(consumer);
//     std::thread thConsumer2(consumer);
	
// 	thProducer.join();
// 	thConsumer.join();
//     thConsumer2.join();
	
// 	return 0;
// }

// #include <condition_variable>
// #include <iostream>
// #include <random>
// #include <thread>
// #include <mutex>
// #include <queue>

// std::mutex              g_lockprint;
// std::mutex              g_lockqueue;
// std::condition_variable g_queuecheck;
// std::queue<int>         g_codes;
// bool                    g_done;
// bool                    g_notified;

// void workerFunc(int id, std::mt19937 &generator)
// {
//      // стартовое сообщение
//      {
//           std::unique_lock<std::mutex> locker(g_lockprint);
//           std::cout << "[worker " << id << "]\trunning..." << std::endl;
//      }
//      // симуляция работы
//      std::this_thread::sleep_for(std::chrono::seconds(1 + generator() % 5));
//      // симуляция ошибки
//      int errorcode = id*100+1;
//      {
//           std::unique_lock<std::mutex> locker(g_lockprint);
//           std::cout  << "[worker " << id << "]\tan error occurred: " << errorcode << std::endl;
//      }
//      // сообщаем об ошибке
//      {
//           std::unique_lock<std::mutex> locker(g_lockqueue);
//           g_codes.push(errorcode);
//           g_notified = true;
//           g_queuecheck.notify_one();
//       }
// }

// void loggerFunc()
// {
//      // стартовое сообщение
//      {
//           std::unique_lock<std::mutex> locker(g_lockprint);
//           std::cout << "[logger]\trunning..." << std::endl;
//      }
//      // до тех пор, пока не будет получен сигнал
//      while(!g_done)
//      {
//           std::unique_lock<std::mutex> locker(g_lockqueue);
//           while(!g_notified) // от ложных пробуждений
//                g_queuecheck.wait(locker);
//           // если есть ошибки в очереди, обрабатывать их
//           while(!g_codes.empty())
//           {
//                std::unique_lock<std::mutex> locker(g_lockprint);
//                std::cout << "[logger]\tprocessing error:  " << g_codes.front()  << std::endl;
//                g_codes.pop();
//           }
//           g_notified = false;
//      }
// }

// int main()
// {
//      // инициализация генератора псевдо-случайных чисел
//      std::mt19937 generator((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
//      // запуск регистратора
//      std::thread loggerThread(loggerFunc);
//      // запуск рабочих
//      std::vector<std::thread> threads;
//      for(int i = 0; i < 5; ++i)
//           threads.push_back(std::thread(workerFunc, i+1, std::ref(generator)));
//      for(auto &t: threads)
//           t.join();
//      // сообщаем регистратору о завершении и ожидаем его
//      g_done = true;
//      loggerThread.join();
//      return 0;
// }

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