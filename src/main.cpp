
/*

    Test task for AdGuard
    "Small http-server"
    Marochkin Ivan <wanesoft@mail.ru>
    08/05/2020

*/

#include "main.hpp"

int main() {

    /*
        Httpserv Class get in 2 args
        first - number of threads
        second - port number
    */

    Httpserv serv_1(5, 80);
    Httpserv serv_2(5, 443);
    std::cout << "Starting server\n";
    std::thread t_80([&]() {
        serv_1.main_cycle();
    });
    std::thread t_443([&]() {
        serv_2.main_cycle();
    });
    t_80.join();
    t_443.join();

    return (0);
}