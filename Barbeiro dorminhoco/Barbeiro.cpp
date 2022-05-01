#include <thread>
#include <iostream>
#include <mutex>
#include <mutex>
#include <chrono>
#include <windows.h>
#include <condition_variable>
#include <exception>
#include <pthread.h>

using std::cin;
using std::cout;
using std::endl;

const int lugaresTotal = 3;

std::mutex mtxSeats;

int freeSeats = 3;

std::mutex m;
std::mutex status;
std::mutex barberWork;
std::condition_variable cv;

class Chairs
{
public:
    size_t tamanho;
    size_t disponivel;

    bool x = false;
    bool readyToCut = false;

    // Cadeiras disponíveis para serem coupadas por clientes e para o barbeiro dormir
    Chairs(int x, int y)
    {
        tamanho = x;
        disponivel = y;

        cout << " Tamanho: " << tamanho << " disponivel: " << disponivel << "\n";
    }

    // Função que verifica se existe lugar vago nas cadeiras e ocupa um caso tenha

    void setX(bool boolean)
    {
        x = boolean;
    }

    bool hasSpace()
    {
        cout << " Thread verificando espaco \n";
        std::unique_lock<std::mutex> lk(m); // bloqueia outras threads ao verificarem que está lock
        cv.wait(lk, [this]
                { return disponivel > 0; });
        if (disponivel == 0)
        {
            cout << " Sem espaço, matando thread \n";
            return false;
        }

        lk.unlock();
        cout << " Tem espaco --- continuando \n";
        return true;
    }

    void requisitarCadeira()
    {

        cout << " Thread Esperando \n";
        std::unique_lock<std::mutex> lk(m); // bloqueia outras threads ao verificarem que está lock
        if (disponivel > 0)
        {
            cv.wait(lk, [this]
                    { return disponivel > 0; });
            disponivel--;
            cout << " Thread requisitando\n"
                 << " Ocupando 1 --- disponivel: " << disponivel << "  ---Total: " << tamanho << endl;
            lk.unlock();
        }
        else
        {
            cout << " Sem espaço, matando thread \n";
        }
    }

    void setReadyToCut(bool state)
    {
        readyToCut = state;
    }

    bool getReadyToCut()
    {
        return readyToCut;
    }

    void finalize()
    {

        std::unique_lock<std::mutex> lk(status); // bloqueia outras threads ao verificarem que está lock
        cv.wait(lk, [this]
                { return disponivel > 0; });
        // manda sinal ao barbeiro q está esperando o corte
        setReadyToCut(true);
        cout << "Pronto para cortar: " << getReadyToCut() << "\n";
        while (x == false)
        {
            Sleep(1500);
        }
        release();
        cout << "----- Terminou o corte -------\n"
             << endl;

        x = false;
        setReadyToCut(false);
        Sleep(1500);
        lk.unlock();
    }

    // Função que libera a cadeira do barbeiro após terminar o corte
    void release()
    {
        mtxSeats.lock();
        disponivel++;
        mtxSeats.unlock();
        cv.notify_one();
    }
};

class Barber
{
public:
    bool work = false;

    Barber(){};

    void goToSleep()
    {
        while (work == false)
        {
            Sleep(1000);
        }
        cout << "Barbeiro Acordando \n";
    }

    void setWork(bool state)
    {
        std::unique_lock<std::mutex> lk(barberWork);
        cv.wait(lk, [this]
                { return lugaresTotal > 0; });

        work = state;
        lk.unlock();
    }

    bool getWork()
    {
        std::unique_lock<std::mutex> lk(barberWork);
        cv.wait(lk, [this]
                { return lugaresTotal > 0; });
        lk.unlock();
        return work;
    }
};

// Metodo do Barbeiros
void barbeiro(Chairs &chair, Barber &barber)
{
    cout << "  Barbeiro iniciando\n"
         << endl;
    int n = 100;
    while (n > 0)
    {
        if (chair.disponivel < lugaresTotal || barber.work == true)
        {

            if (chair.disponivel == lugaresTotal)
            {
                barber.setWork(false);
            }
            else
            {

                // tem q parar aqui até uma thread sinalizar para ele cortar

                // Corte de cabelo do cliente
                if (chair.getReadyToCut() == true)
                {

                    cout << "Cortando cabelo\n"
                         << "Cadeiras Desocupadas: " << chair.disponivel
                         << endl;
                    Sleep(4000);
                    cout << "Corte Finalizado\n";
                    chair.setX(true);
                }
            }
        }
        else
        { // barbeiro dorme
            cout << "Barbeiro indo dormir \n"
                 << endl;
            barber.setWork(false);
            chair.requisitarCadeira();
            cout << "Barbeiro dormiu \n\n";
            barber.goToSleep();
        }
        Sleep(2000);
        n--;
    }
}

// Metodo dos Clientes
void thread1(Chairs &chair, Barber &barber)
{
    bool barberstatus = barber.getWork();
    if (barber.getWork() == false)
    {
        cout << "Acordando barbeiro e ocupando o lugar dele\n";
        barber.setWork(true);
        chair.release();
        chair.requisitarCadeira();
    }
    else
    {
        cout << "Sentando em uma cadeira e esperando\n";
        chair.requisitarCadeira();
        Sleep(2000);
    }
    chair.finalize();
}

// void thTeste(Chairs &chair, Barber &barber)
// {
//     chair.requisitarCadeira();
// }

int main()
{
    Chairs chairs(lugaresTotal, lugaresTotal);
    Barber barber;

    std::thread b(barbeiro, std::ref(chairs), std::ref(barber));

    Sleep(3000);
    std::thread t1(thread1, std::ref(chairs), std::ref(barber));
    Sleep(1000);
    std::thread t2(thread1, std::ref(chairs), std::ref(barber));
    std::thread t3(thread1, std::ref(chairs), std::ref(barber));
    std::thread t4(thread1, std::ref(chairs), std::ref(barber));

    b.join();
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    return 0;
}