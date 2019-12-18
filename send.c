#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"
#include "auxiliar.h"
#define HOST "127.0.0.1"
#define PORT 10000
msg t, r;
package mesaj;
int inputFile, i = 0, j = 0;
int MessageSize = 0;
char buffer[2000][2000];
int receptionat[2000];
int dimMesaj[2000];
int totalNoMesaje = 1;
//am setat variabilele global pentru a usura scrierea functiilor ajutatoare
void writeToBuffer(){
	while((MessageSize = read(inputFile, t.payload, MSGSIZE-2*sizeof(int))) > 0){
		memcpy(buffer + totalNoMesaje, t.payload, MessageSize);
		dimMesaj[totalNoMesaje] = MessageSize;
		totalNoMesaje++;
	}
	dimMesaj[0] = totalNoMesaje;
}
//a nu se confunda cu send_message
//sendMessage simplifica alcatuirea si trimiterea mesajului
void sendMessage(){
	memcpy(mesaj.data, buffer + i, dimMesaj[i]);
	mesaj.size = dimMesaj[i];
	mesaj.index = i;
	memcpy(t.payload, &mesaj, sizeof(package));
    send_message(&t);
}

int main(int argc,char** argv){
	
  	//Calculez marimea ferestrei conform: speed*delay*1000/msgSize*8.
  	int windowSize = (atoi(argv[2]) * atoi(argv[3]) * 1000);
  	windowSize /= ((MSGSIZE-2*sizeof(int)) * 8);
  	init(HOST,PORT);

   	//dechid fisierul de input
   	inputFile = open(argv[1], O_RDONLY);
	sprintf(t.payload, "%s", argv[1]);
	memcpy(buffer, t.payload, strlen(argv[1]));

	//voi citi tot fisierul in buffer
	writeToBuffer();

	for(i = 0; i < windowSize && i <= totalNoMesaje; i++) {
		//trimite tot continutul bufferului
		//pana ajung la marimea ferestrei sau pana la numarul total de mesaje de trimis
		sendMessage();
    	printf("[%s] Sent message number %d\n", argv[0], i);
  	}

  	//incep trimiterea intregului buffer
	while(1) {
		//daca nu a fost confirmat mesajul trimis in 80ms il retrimit
    	if(recv_message_timeout(&r, 80)  == -1){
      		printf("\nTIME OUT\n");
			for(i = 0, j = 0; j < windowSize && i <= totalNoMesaje; i++, j++){
				for(;receptionat[i] == 1 && i <= totalNoMesaje; i++);
				//caut primul mesaj nereceptionat
				sendMessage();
    			printf("[%s] Sent message number %d\n", argv[0], i);
			}
    	}
		else{
			memcpy(&mesaj, r.payload, sizeof(package));
			if(mesaj.index == -1)
				return 0;
			printf("[%s] Received ACK for message %d.\n", argv[0], mesaj.index);
			//setez starea de receptionare a mesajului primis
			if(receptionat[mesaj.index] != 1)
				receptionat[mesaj.index] = 1;
			if(i == totalNoMesaje)
				i = 0;
			if(i <= totalNoMesaje){
				for(;receptionat[i] == 1; i++);
				//TRIMITE UN PACHET
				sendMessage();
    			printf("[%s] Sent message number %d\n", argv[0], i);
				i++;
			}
		}
	}
  	return 0;
}
