#include <wiringPiSPI.h>
#include <wiringPi.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>

using namespace std;

//定数定義
int main(void)
{
    unsigned char spi_buff[3];                      //送受信用バッファ

    //SPIチャンネル初期化
    if((wiringPiSPISetup (0, 10000000)) < 0){
        printf("wiringPiSPISetup error \n");
        return -1 ;
    }
    printf("Setup SPI...\n");

    //GPIO初期化
    if(wiringPiSetupGpio() == -1){
        printf("wiringPiSetupGpio error\n");
        return -1;
    }

    printf("Start SPI...\n");

    //通信処理
    while(1){
        spi_buff[0] = 0b00000110;
        spi_buff[1] = 0b00000000;
        spi_buff[2] = 0b00000000;

        //SPI通信実行
        wiringPiSPIDataRW(0, spi_buff, 3);             //データ送受信

        printf("%d %d %d\n", spi_buff[0], spi_buff[1], spi_buff[2]);

        //インターバル
        usleep(100000);
    }

    return 0;
}
