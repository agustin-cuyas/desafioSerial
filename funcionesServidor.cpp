#include "mainHeader.hpp"
#include "clases.hpp"

void Esperando::ejecutar(string& respuesta){
    cout << "ESPERANDO, ejecute comando START." << endl;
    respuesta = "ESPERANDO, ejecute comando START.";
}

Estado* Esperando::siguienteEstado(Comando comando, string& respuesta)
{
    switch(comando){
        case Comando::START:
            respuesta = "Iniciando ejecución...";
            cout << "Estado cambiando a EJECUCION." << endl;
            return new Ejecucion();
            break;
        case Comando::STOP:
            respuesta = "El servidor ya está pausado. Ejecute START para encenderlo.";
            break;
        case Comando::SHUTDOWN:
            respuesta = "El servidor no está en ejecución. Ingrese START para encenderlo.";
            break;
    }
                
    return this;
}

void Ejecucion::ejecutar(string& respuesta)
{
    respuesta = "EJECUCION, puede ejecutar STOP o SHUTDOWN.";
}

Estado *Ejecucion::siguienteEstado(Comando comando, string& respuesta)
{
    switch(comando){
        case Comando::START:
            respuesta =  "El servidor ya fue iniciado.";
            break;
        case Comando::STOP:
            respuesta = "Deteniendo ejecución, regresando a ESPERANDO...";
            cout << "Estado cambiando a ESPERANDO." << endl;
            return new Esperando();
            break;
        case Comando::SHUTDOWN:
            respuesta = "Apagando sistema...";
            cout << "Estado cambiando a APAGADO." << endl;
            return new Apagado();
            break;
    }
    return this;
}

void Apagado::ejecutar(string& respuesta) {
    respuesta =  "APAGADO, no se aceptan más comandos.";
}

Estado* Apagado::siguienteEstado(Comando comando, string& respuesta) {
    //no se usa
    return this;
}