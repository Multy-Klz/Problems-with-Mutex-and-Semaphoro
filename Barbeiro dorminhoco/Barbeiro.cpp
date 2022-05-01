#include <thread>
#include <iostream>
#include <mutex>
#include <chrono>
#include <windows.h>
#include <condition_variable>
#include <exception>

using std::cin;
using std::cout;
using std::endl;

// Define quantas cadeiras vão existir no problema do barbeiro
const int lugaresTotal = 3;

// mutex para incrementar os espaços disponíveis. Aumentar a quantidade de cadeiras vagas
std::mutex mtxSeats;

// mutex para controle de ocupação das cadeiras
std::mutex m;
std::mutex status;
// mutex para sincronizar com o barbeiro
std::mutex barberWork;
// sinaliza estados globais para as threads
std::condition_variable cv;

// classe cadeira, que define  as cadeiras e os metodos que vão ser efetuados sobre as cadeiras
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

    // Sinaliza ao barbeiro para contar o cabelo
    void setX(bool boolean)
    {
        x = boolean;
    }

    // Procedimento para ocupar uma cadeira pelos clientes ou barbeiro
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

    // Variavel que define se a thread (clientes está cortando o cabelo)
    void setReadyToCut(bool state)
    {
        readyToCut = state;
    }

    // retorna o valor de readyToCut
    bool getReadyToCut()
    {
        return readyToCut;
    }

    // Metodo para terminar o corte e liberar a cadeira
    void cortarCabelo()
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

    // Função que libera a cadeira após terminar o corte
    void release()
    {
        mtxSeats.lock();
        disponivel++;
        mtxSeats.unlock();
        cv.notify_one();
    }
};

// classe do barbeiro com os métodos que ele vai fazer
class Barber
{
public:
    bool work = false;

    Barber(){};

    // função para manter ele dormindo se nenhum cliente sinalizar para ele trabalhar
    void goToSleep()
    {
        while (work == false)
        {
            Sleep(1000);
        }
        cout << "Barbeiro Acordando \n";
    }

    // atualiza o estado do barbeiro de dormindo para trabalhando
    void setWork(bool state)
    {
        std::unique_lock<std::mutex> lk(barberWork);
        cv.wait(lk, [this]
                { return lugaresTotal > 0; });

        work = state;
        lk.unlock();
    }

    // retorna o estado do barbeiro
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
        // verifica se tem cliente nas cadeiras e se seu estado é para TRABALHAR
        if (chair.disponivel < lugaresTotal || barber.work == true)
        {
            // Valida se existe clientes nas cadeiras, se não existir, ele coloca o trabalho como false para ir dormir
            if (chair.disponivel == lugaresTotal)
            {
                barber.setWork(false);
            }
            else
            {

                // Se ele estiver trabalhando, ele começa a cortar o cabelo

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
    // verifica se o barbeiro está dormindo, para sentar e acorda-lo
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
    // função onde os clientes esperam os cortes de cabelo serem finalizados
    chair.cortarCabelo();
}

// void thTeste(Chairs &chair, Barber &barber)
// {
//     chair.requisitarCadeira();
// }

int main()
{
    // cria as cadeiras e o barbeiro
    Chairs chairs(lugaresTotal, lugaresTotal);
    Barber barber;

    // cria uma thread barbeiro
    std::thread b(barbeiro, std::ref(chairs), std::ref(barber));

    Sleep(3000);
    // cria as threads clientes
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