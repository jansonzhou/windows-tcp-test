#pragma comment(lib, "ws2_32.lib")

#include <STDIO.H>
#include <WINSOCK2.H>
#include <PROCESS.H>

#define IP_ADDRESS    "10.189.58.101"
//#define IP_ADDRESS    "10.101.170.42"
#define PORT          1234
#define MAX_REQUEST   1000000
#define MAX_CONN      10
#define BUFFER        "1234567890"

char *address = IP_ADDRESS;
int port = PORT;
int req_count = MAX_REQUEST;
int conn_count = MAX_CONN;

void WorkThread(LPVOID lpParam)
{
	SOCKET sockClient;
	struct sockaddr_in local;
	int ret = 0;
	char buf[5000];

	sockClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockClient == INVALID_SOCKET) {
		conn_count--;
		return;
	}

	memset(&local, 0, sizeof(local));
	local.sin_addr.s_addr = inet_addr(address);
	local.sin_family = AF_INET;
	local.sin_port = htons(port);

	if(connect(sockClient, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR) {
		printf("connect: %d\n", WSAGetLastError());
		closesocket(sockClient);
		conn_count--;
		return;
	}

	while (req_count > 0) {
		ret = send(sockClient, BUFFER, (int)strlen(BUFFER), 0);
		if (ret == SOCKET_ERROR) {
			printf("send error (%d)!\n", WSAGetLastError());
			break;
		}

		memset(buf, 0, 5000);
		ret = recv(sockClient, buf, 5000, 0);
		if (ret == SOCKET_ERROR || ret == 0){ // Error or Peer closed
			printf("recv error (%d) or Timeout!\n", WSAGetLastError());
			break;
		}
		req_count--;
	}

	closesocket(sockClient);
	conn_count--;
}

static void
print_usage()
{
	printf("Usage:\n");
	printf("tcp_client.exe <ip addr> <port> <req count> <conn count>\n");
}

int main(int argc, char* argv[])
{
	struct WSAData wsaData;
	int count = 0;

	if (argc != 5) {
		print_usage();
		return 1;
	}

	address = argv[1];
	port = atoi(argv[2]);
	req_count = atoi(argv[3]);
	conn_count = atoi(argv[4]);

	WSAStartup(WINSOCK_VERSION, &wsaData);

	while(count < conn_count){
		_beginthread(WorkThread, 0, NULL);
		count++;
	}

	while(conn_count != 0) {
		Sleep(1000);
	}

	WSACleanup();
	return 0;
}