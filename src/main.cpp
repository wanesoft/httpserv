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

void _put_log(const char *str) {

    std::cout << str << '\n';
}

int connection_for_incoming(void) {

    int listener, res;
    struct sockaddr_in addr;
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        _put_log("ERROR: socket() not gave listener");
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(443);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int enable = 1;
    if ((setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) < 0) {
        _put_log("ERROR: Can't reuse address");
    }
    res = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    if (res < 0) {
        _put_log("ERROR: Can't bind address");
        exit(errno);
    }
    res = listen(listener, 0);
    if (res < 0) {
        _put_log("ERROR: listen() crash");
		exit(errno);
    }
    return (listener);
}

int main() {

    int gen_sock = connection_for_incoming();
    std::cout << "Done " << gen_sock << '\n';
    char buff[1024];

    while (1) {

        int cur_sock = accept(gen_sock, 0, 0);
        memset(buff, 0, 1024);
        int recv_res = recv(cur_sock, buff, 1024, MSG_NOSIGNAL);
        int send_res = send(cur_sock, "ok", 2, MSG_NOSIGNAL);
        std::cout << "i recv: " << recv_res << '\n';
        std::cout << "i send: " << send_res << '\n';
        close(cur_sock);
        std::cout << buff;

    }

    return (1);
}