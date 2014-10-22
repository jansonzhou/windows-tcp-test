#pragma comment(lib, "ws2_32.lib")

#include <STDIO.H>
#include <WINSOCK2.H>
#include <PROCESS.H>

void WorkThread(LPVOID lpParam)
{
	SOCKET sockSvr = (SOCKET) lpParam;
	fd_set readSet;
	int ret;
	struct timeval tv;
	char buf[5000];

	while(1){
		FD_ZERO(&readSet);
		FD_SET(sockSvr, &readSet);
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		ret = select(0, &readSet, NULL, NULL, &tv);
		if(ret == SOCKET_ERROR || ret == 0){// Error or Timeout
			printf("Select error(%d) or Timeout!\n", WSAGetLastError());
			break;
		}
		if(FD_ISSET(sockSvr, &readSet)){
			memset(buf, 0, 5000);
			ret = recv(sockSvr, buf, 5000, 0);
			if(ret == SOCKET_ERROR || ret ==0){ // Error or Peer closed
				printf("recv error (%d) or Timeout!\n", WSAGetLastError());
				break;
			}
			// printf("Socket %d recv: %\n", sockSvr, buf);
			ret = send(sockSvr, buf, strlen(buf), 0);
			if(ret == SOCKET_ERROR)
				break;
		}
	}
	closesocket(sockSvr);
}

int main(int argc, char* argv[])
{
	struct WSAData wsaData;
	SOCKET sockListen;
	SOCKET sockAccept;
	BOOL bReuseAddr = TRUE;
	struct sockaddr_in local;

	WSAStartup(WINSOCK_VERSION, &wsaData);
	sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(sockListen, SOL_SOCKET, SO_REUSEADDR, (char *)&bReuseAddr, sizeof(bReuseAddr));

	memset(&local, 0, sizeof(local));
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_family = AF_INET;
	local.sin_port = htons(1234);

	if(bind(sockListen, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR){
		printf("bind: %d\n", WSAGetLastError());
		closesocket(sockListen);
		WSACleanup();

		return -1;
	}

	if(listen(sockListen, 5000) == SOCKET_ERROR){
		printf("listen: %d\n", WSAGetLastError());
		closesocket(sockListen);
		WSACleanup();

		return -1;
	}

	while(TRUE){
		sockAccept = accept(sockListen, NULL, NULL);
		if(sockAccept == INVALID_SOCKET)
			break;
		else
			_beginthread(WorkThread, 0, (LPVOID) sockAccept);
	}

	closesocket(sockListen);
	WSACleanup();
	return 0;
}