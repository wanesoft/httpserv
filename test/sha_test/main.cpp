
/*

    Test task for AdGuard
    "Small http-server"
    Marochkin Ivan <wanesoft@mail.ru>
    08/05/2020

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <map>

int main() {

	std::string tmp2 = "curl/TEST.69.0";
	unsigned char hash[SHA_DIGEST_LENGTH]; // == 20
	SHA1((const unsigned char *)tmp2.data(), strlen(tmp2.data()), hash);
	char sss[41] = {0};
	// for (int i = 0; i < 20; ++i) {
	// 	sprintf(&sss[i * 2], "%x", hash[i]);
	// }
	// for (int i = 0; i < 40; ++i) {
	// 	if (sss[i] == 0) {
	// 		sss[i] = 48;
	// 	}
	// }
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++){
		// printf("%02x", hash[i]);
		sprintf(&sss[i * 2], "%02x", hash[i]);
	}
	// printf("\n");
	std::cout << sss << '\n';
	std::cout << "0d47126f9a83f4f81b3e6b3f70a130c9d195456a" << '\n';
	return (0);
}
