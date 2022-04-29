#include <Windows.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

#define TAM_MAX_BUFFER 10
#define BUFFERVAZIO 0
#define BUFFERCHEIO 1

int buffer[TAM_MAX_BUFFER];
int estado = BUFFERVAZIO;
int fim = 0;
int inicio = 0;
bool sair = false;
int countElemnt = 0;
// aperte ENTER para parar a execução //
void comsumir()
{
    int item = 0;
    while (sair == false)
    {
        if (countElemnt > 0)
        {
            mtx.lock();
            std::cout << "Comsumidor consumiu elemento: " << buffer[inicio] << std::endl;
            buffer[inicio] = item;
            inicio++;
            if (inicio >= TAM_MAX_BUFFER)
            {
                inicio = 0;
            }
            countElemnt--;
            mtx.unlock();
        }
        if(countElemnt < 0){
            estado = BUFFERVAZIO;
        }
        Sleep(9);
    }
}

void produzir(int id)
{
    int item = id;
    while (sair == false)
    {
        if (estado == BUFFERVAZIO)
        {
            mtx.lock();
            buffer[fim] = item;
            std::cout << "Produtor colocou elemento: " << buffer[fim] << std::endl;
            fim++;

            if (fim >= TAM_MAX_BUFFER)
            {
                fim = 0;
            }
            item++;
            countElemnt++;
            mtx.unlock();
        }
        if(countElemnt == TAM_MAX_BUFFER){
             estado = BUFFERCHEIO;
        }
        Sleep(1);
    }
}
void parar()
{

    while (true)
    {
        if (std::cin.ignore())
        {
            sair = true;
            break;
        }
    }
}
int main()
{
    std::thread produtor;
    std::thread stop;
    std::thread consumidor;
    produtor = std::thread(produzir, 1);
    stop = std::thread(parar);
    consumidor = std::thread(comsumir);
    stop.join();
    produtor.join();
    consumidor.join();
    for (int i = 0; i < 10; i++)
    {
        std::cout << buffer[i] << std::endl;
    }
    return 0;
}