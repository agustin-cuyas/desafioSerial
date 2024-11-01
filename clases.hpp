#ifndef CLASES
#define CLASES

#include <iostream>
using namespace std;

enum class Comando{     //defino acá para que reconozca la enumeración y como clase porque es el header de clases
    HANDSHAKE,
    STATUS,
    START,
    STOP,
    SHUTDOWN,
    HELP,
    DEFAULT
};

class Estado{
    public:
        virtual void ejecutar(string&) = 0;
        virtual Estado * siguienteEstado(Comando, string&) = 0;
};

class Esperando : public Estado{
    public:
        void ejecutar(string&) override;
        Estado* siguienteEstado(Comando, string&) override;
};

class Ejecucion : public Estado{
    public:
        void ejecutar(string&) override;
        Estado* siguienteEstado(Comando, string&) override;
};

class Apagado : public Estado{
    public:
        void ejecutar(string&);
        Estado* siguienteEstado(Comando, string&) override;
};


#endif