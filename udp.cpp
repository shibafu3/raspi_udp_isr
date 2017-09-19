#include <wiringPi.h>

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include <fstream>
#include <string>
 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
static volatile int int_count = 0;
static volatile char sig = 0x00;
static volatile char sig_p = 0x00;
static volatile float steer_deg = 0;
const static signed char EncoderIndexTable[] = {0, -1, 1, 0,  1, 0, 0, -1,  -1, 0, 0, 1,  0, 1, -1, 0};
 
static void ARise(void){
	sig_p = (sig << 2) & 0x0F;
	sig |= 0x02;
 
	steer_deg += EncoderIndexTable[sig | sig_p];
//	printf("ar %d\n", sig);
}
static void AFall(void){
	sig_p = (sig << 2) & 0x0F;
	sig &= 0xFD;
 
	steer_deg += EncoderIndexTable[sig | sig_p];
//	printf("af %d\n", sig);
}
static void BRise(void){
	sig_p = (sig << 2) & 0x0F;
	sig |= 0x01;
 
	steer_deg += EncoderIndexTable[sig | sig_p];
//	printf("br %d\n", sig);
}
static void BFall(void){
	sig_p = (sig << 2) & 0x0F;
	sig &= 0xFE;
 
	steer_deg += EncoderIndexTable[sig | sig_p];
//	printf("bf %d\n", sig);
}
 
union byte2float{
	float f;
	char c[4];
};

using namespace std;

int main(void){
	printf("start...\n");
 	
	string sip;
	string sport;
	ifstream ifsip("ip.txt");
	ifstream ifsport("port.txt");
	ifsip >> sip;
	ifsport >> sport;

	printf("UDP IP : %s\n", sip.c_str());
	printf("UDP IP : %d\n", stoi(sport));

	if (wiringPiSetupGpio()){
		return 1;
	}
	pinMode(5, INPUT);  // AR
	pinMode(25, INPUT); // AF
	pinMode(6, INPUT);  // BR
	pinMode(12, INPUT); // BF
	pullUpDnControl(5, PUD_UP);
	pullUpDnControl(6, PUD_UP);
	pullUpDnControl(25, PUD_UP);
	pullUpDnControl(12, PUD_UP);

	pinMode(13, OUTPUT); // A
	pinMode(26, OUTPUT); // B
	digitalWrite(13, 0); // init
	digitalWrite(26, 0); // init

	sig += digitalRead(5) * 2;
	sig += digitalRead(6);

	wiringPiISR(5, INT_EDGE_RISING, ARise);
	wiringPiISR(25, INT_EDGE_FALLING, AFall);
	wiringPiISR(6, INT_EDGE_RISING, BRise);
	wiringPiISR(12, INT_EDGE_FALLING, BFall);
 
	int sock;
	struct sockaddr_in addr;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(stoi(sport));
	addr.sin_addr.s_addr = inet_addr(sip.c_str());
	int val = 1;
	ioctl(sock, FIONBIO, &val);
	int yes = 1;
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&yes, sizeof(yes));
 
	sleep(1);
	byte2float b2f;
	struct timespec ts = {0, 100000};
	while (1){
		digitalWrite(13, 0);
		nanosleep(&ts, NULL);
		printf("%f\n", steer_deg);
		digitalWrite(26, 0);
		nanosleep(&ts, NULL);
		printf("%f\n", steer_deg);
		digitalWrite(13, 1);
		nanosleep(&ts, NULL);
		printf("%f\n", steer_deg);
		digitalWrite(26, 1);
		nanosleep(&ts, NULL);
		printf("%f\n", steer_deg);
		digitalWrite(26, 0);
		nanosleep(&ts, NULL);
		printf("%f\n", steer_deg);
		digitalWrite(13, 0);
		nanosleep(&ts, NULL);
		printf("%f\n", steer_deg);
		digitalWrite(26, 1);
		nanosleep(&ts, NULL);
		printf("%f\n", steer_deg);
		digitalWrite(13, 1);
		nanosleep(&ts, NULL);
		printf("%f\n", steer_deg);


		b2f.f = steer_deg;
		sendto(sock, b2f.c, sizeof(b2f.c), 0, (struct sockaddr *)&addr, sizeof(addr));
	}
 
	close(sock);
	printf("exit\n");
 
	return 0;
}
