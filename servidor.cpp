#include "mainHeader.hpp"

bool activo = true;
int timeout;
boost::system::error_code ec = error::would_block;      //errorcode se inicializa con would_block (la opercación no está terminada)
size_t length = 0;

//Esta función se activa cuando el deadline_timer alcanza el límite de tiempo sin que la lectura haya finalizado.
void onTimeout(const boost::system::error_code& error, serial_port& serial) {
    if (!error) {
        activo = false;
        serial.cancel();  //cancela la lectura
    }
}

//Esta función se llama cuando la lectura termina ya sea con exito o error
void onRead(const boost::system::error_code& error, size_t len) {
    ec = error;
    length = len;
}

bool leerConTimeout(serial_port &serial, deadline_timer &timer, io_context &io, char* buffer, size_t buffer_size) {
    ec = error::would_block;    //cada vez que se llama a la funcion reinicializo ec
    timer.expires_from_now(boost::posix_time::seconds(timeout)); //timeout configurable

    timer.async_wait(boost::bind(onTimeout, boost::placeholders::_1, boost::ref(serial)));  //uso bind para enlazar onTimeout con el timer. así si se llega al timeout, la función cancela la lectura

    serial.async_read_some(boost::asio::buffer(buffer, buffer_size),
                           boost::bind(onRead, boost::placeholders::_1, boost::placeholders::_2)
                           );  //la funcion onRead se llama cuando la lectura termine

    // Ejecuta la operación asíncrona en un bucle hasta que termine o se alcance el timeout
    do {
        io.run_one();   //ejecutar solo una operación asíncrona a la vez
    } while (ec == boost::asio::error::would_block && activo);  //ec cambia de valor cuando termina una lectura o finaliza el timeout

    timer.cancel();     //cancela el timeout
    return !ec;  //devuelve true si la lectura es existosa
}

void conectarSerial(serial_port& serial, io_context& io){
    serial.open("COM3");
    serial.set_option(serial_port_base::baud_rate(9600));
    serial.set_option(serial_port_base::character_size(8));
    serial.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
    serial.set_option(serial_port_base::parity(serial_port_base::parity::none));

    cout << "Servidor iniciado, esperando cliente..." << endl;
}

string comandosHandle(const char* read_buf, Estado* &estado){
    cout << "Recibido: " << string(read_buf, length) << endl;
    string mensaje(read_buf, strlen(read_buf));

    Comando comando = (string(read_buf, length) == "Hola desde cliente!\r\n") ? Comando::HANDSHAKE     //Comando:: porque es una enum class
                : (string(read_buf, length) == "STATUS" ? Comando::STATUS
                : (string(read_buf, length) == "START" ? Comando::START 
                : (string(read_buf, length) == "STOP" ? Comando::STOP 
                : (string(read_buf, length) == "SHUTDOWN" ? Comando::SHUTDOWN 
                : (string(read_buf, length) == "HELP" ? Comando::HELP 
                :  Comando::DEFAULT ) ) ) ) ) ;

    string respuesta;

    switch(comando){
        case Comando::HANDSHAKE:
            respuesta = "Hola desde el servidor! Cliente conectado";
            break;
        case Comando::STATUS:
            estado->ejecutar(respuesta);
            break;
        case Comando::START:
            estado = estado->siguienteEstado(comando, respuesta);
            break;
        case Comando::STOP:
            estado = estado->siguienteEstado(comando, respuesta);
            break;
        case Comando::SHUTDOWN:
            estado = estado->siguienteEstado(comando, respuesta);
            if(dynamic_cast<Apagado*>(estado))
                activo = false;
            break;
        case Comando::HELP:
            respuesta = "Los comandos disponibles son: \nSTATUS: Solicita el estado actual de la máquina de estados. \nSTART: Inicia el proceso. \nSTOP: Detiene el proceso. \nSHUTDOWN: Apaga el servidor \nHELP: Muestra esta ayuda.";
            break;
        case Comando::DEFAULT:
            respuesta = "Comando desconocido. Use HELP para ver los comandos disponibles";
            break;
    }

    return respuesta;
}

int main(){
    
    Estado* estado = new Esperando;
    cout << "Antes de conectar ingrese el timeout deseado: ";
    cin >> timeout;
    cout << "El servidor se desconectará si no recibe comandos luego de " << timeout << " segundos." << endl;    

    try{
        io_context io;
        serial_port serial(io);

        conectarSerial(serial, io);
        
        char read_buf[256];     //256 bytes
        boost::system::error_code error;      

        deadline_timer timer(io);      

        while(activo){
            
            if (leerConTimeout(serial, timer, io, read_buf, sizeof(read_buf))) {

                write(serial, buffer(comandosHandle(read_buf, estado)));

            }             
            else{
                write(serial, buffer("Timeout alcanzado. Apagando sistema..."));
                cerr << "Timeout alcanzado. Apagando el servidor..." << endl;
            }

        }
    }    

    catch (boost::system::system_error& e) {
        cerr << "Falla al establecer la conexión: " << e.what() << endl;
    } catch (...) {
        cerr << "Error desconocido" << endl;
    }

    return 0;
}