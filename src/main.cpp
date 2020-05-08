
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

    Httpserv serv(5, 80);
    std::cout << "Starting server\n";
    std::thread t([&]() {
        serv.main_cycle();
    });
    t.join();

    return (0);
}