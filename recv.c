#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"
#include "auxiliar.h"
#define HOST "127.0.0.1"
#define PORT 10001

msg r,t;
package mesaj;
int outputFile;
char buffer[2000][2000];
int dimMesaj[2000];
int receptionat[2000];
int totalNoMesaje = 0;
int mesajeConf = 0;
char agvZero[100];
//am setat variabilele global pentru a usura scrierea functiilor ajutatoare
void writeBuffer(){
	recv_message(&r);
	memcpy(&mesaj, r.payload, sizeof(package));
	memcpy(buffer + mesaj.index, mesaj.data, mesaj.size);
	dimMesaj[mesaj.index] = mesaj.size;
}
void writeFileName(){
	if(mesaj.index == 0){
		//index = 0 inseamna ca a receptionat numele fisierului
		char nume_fisier[100] = "recv_";
		strcat(nume_fisier, mesaj.data);
 		outputFile = open(nume_fisier, O_WRONLY | O_CREAT, 0644);
		totalNoMesaje = mesaj.size;
	}
}
void writeInFile(){
	int i;
	for(i = 1; i <= totalNoMesaje; i++){
		//printf("%d\n", dimMesaj[i]);
		write (outputFile, buffer + i, dimMesaj[i]);
	}
	//confirm primirea de mesaj catre sender
	mesaj.index = -1;
	memcpy(t.payload, &mesaj, sizeof(package));
	send_message(&t);
}
void updateReceptionat(){
	if(receptionat[mesaj.index] != 1){
		memcpy(t.payload, &mesaj, sizeof(package));
		send_message(&t);
   		printf("[%s] Sent ACK for message %d.\n", agvZero, mesaj.index);
		receptionat[mesaj.index] = 1;
	}
}

int main(int argc,char** argv){
  	
  	init(HOST,PORT);
  	sprintf(agvZero, "%s", argv[0]);
  	//incepe primirea de mesaje
  	while (1) {
    	writeBuffer();
		//copiez toate mesajele primite in buffer, in indexul mesajului
		if(receptionat[mesaj.index] == 0){
			mesajeConf++;
		}
		writeFileName();
		/*
		*daca a receptionat macar un mesaj
		*si numarul de mesaje confirmate este egal cu numarul de mesaje primite
		*scriu in fisier toate mesajele si trimit o confirmare ce indica finalul
		*/
		if(totalNoMesaje != 0 && mesajeConf == totalNoMesaje+1){
			writeInFile();
    		printf("[%s] Sent FINAL message.\n", argv[0]);
			return 0;
		}
		//daca mesajul nu a fost confirmat dar a fost primit atunci schimb valoarea in vectorul receptionat
    	updateReceptionat();
		//cand un mesaj a fost pierdut recv va rula la infinit (teoretic)
		//in sender se considera mesajul ca fiind neprimit dupa 80ms
  	}

  	return 0;
}
