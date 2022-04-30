#include <Windows.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <string>


std::mutex mtx;
std::mutex mutex;
int lendo = 0;
bool sair = false;
int count = 0;

void ler_dados(void){
    int tempo=100;
    std::cout << "Lendo os dados ... Leitores lendo: " << lendo << std::endl;
    Sleep(tempo);

}

void usar_dados(){
    int tempo=1000; 
    std::cout << "Usando dados... " << std::endl;
    Sleep(tempo);
}
void esperando(){
    int tempo=500;
   
    std::cout << "Esperando ... " << std::endl;
    Sleep(tempo);
}
void escrever_dados(){
    int tempo=500;
    
    std::cout << "Escrevendo...  " << std::endl;
    Sleep(tempo);
}

void leitor(){
    while(!sair){
        mtx.lock();
        lendo++;
        if(lendo==1){
            mutex.lock();
        }

        mtx.unlock();
        ler_dados();
        mtx.lock();
        lendo--;

        if(lendo==0){
            mutex.unlock();
        }

        mtx.unlock();
        usar_dados();
    }
}

void escritor(){
    while(!sair){
        esperando();
        mutex.lock();
        escrever_dados();
        count++;
        mutex.unlock();
    }

}
void stop(){
     while(!sair){
         if(count >=10){
             sair = true;
         }
     }
 }
int main(){
    std::thread leitores[6];
    std::thread escritores[3];
    std::thread sair;
    sair = std::thread(stop);
    for(int i=0;i<6;i++){
        leitores[i] = std::thread(leitor);
    }
    for(int i=0;i<3;i++){
        escritores[i] = std::thread(escritor);
    }
    for(int i=0;i<6;i++){
        leitores[i].join();
    }
    for(int i=0;i<3;i++){
        escritores[i].join();
    }
    sair.join();

    return 0;
}
