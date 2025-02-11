
// UDP Socket 的流程，主要就 socket() -> 填位址 -> sendto()/recvfrom()
//TCP Socker connect() / listen() / accept()。

int socketfd = socket(AF_INET, SOCK_DGRAM, 0);// ipv4