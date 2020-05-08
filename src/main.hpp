/* ************************************************************************************************** */
/*                                                                                                    */
/*                                                         :::::::::   ::::::::   ::::::::      :::   */
/*  main.h                                                :+:    :+: :+:    :+: :+:    :+:   :+: :+:  */
/*                                                       +:+    +:+ +:+    +:+ +:+         +:+   +:+  */
/*  By: Ivan Marochkin <i.marochkin@rosalinux.ru>       +#++:++#:  +#+    +:+ +#++:++#++ +#++:++#++:  */
/*                                                     +#+    +#+ +#+    +#+        +#+ +#+     +#+   */
/*  Created: 2019/10/03 10:17:15 by Ivan Marochkin    #+#    #+# #+#    #+# #+#    #+# #+#     #+#    */
/*  Updated: 2019/10/03 10:17:15 by Ivan Marochkin   ###    ###  ########   ########  ###     ###     */
/*                                                                                                    */
/* ************************************************************************************************** */

#ifndef __MAIN_HPP__
#define __MAIN_HPP__

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

#define VOL_OF_THREADS		5

#endif /* __MAIN_HPP__ */