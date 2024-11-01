/*

funciones: conexion
           
El servidor debe tener un timeout configurable. Si no recibe un comando en un determinado período de tiempo, debe cerrar la conexión automáticamente.           

así como está shutdown no apaga porque solo debería funcionar en Ejecucion::siguienteEstado. Ver si se puede pasar la variable activo

el timeout anda bastante bien, solo que se cierra unicamente al recibir un mensaje luego del aviso de cierre
*/
#include "mainHeader.hpp"

atomic<bool> activo(true);  //defino como atomic para que un hilo pueda modificar la variable que está siendo leida por otro proceso.
double timeout;
std::chrono::steady_clock::time_point tiempoInicio;

DWORD WINAPI timerThread(LPVOID lpParam) {
    while (activo) {
        auto tiempoTranscurrido = std::chrono::steady_clock::now() - tiempoInicio;
        if (std::chrono::duration<double>(tiempoTranscurrido).count() >= timeout) {
            std::cout << "Timeout alcanzado. Apagando el servidor..." << std::endl;
            activo = false;
            break;
        }
        Sleep(100);  // Pequeño retardo para reducir la carga del CPU
    }
    return 0;
}


int main(){
    
    Estado* estado = new Esperando;
    cout << "Antes de conectar ingrese el timeout deseado: ";
    cin >> timeout;
    cout << "El servidor se desconectará si no recibe comandos luego de " << timeout << " segundos." << endl;
    

    try{
        io_context io;

        //steady_timer timer(io, std::chrono::seconds(static_cast<int>(timeout)));    //establece el timer en base al timeout
        
        serial_port serial(io, "COM3");
        serial.set_option(serial_port_base::baud_rate(9600));
        serial.set_option(serial_port_base::character_size(8));
        serial.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
        serial.set_option(serial_port_base::parity(serial_port_base::parity::none));

        cout << "Servidor iniciado, esperando cliente..." << endl;
        
        //atomic<bool> activo(true);  //defino como atomic para que un hilo pueda modificar la variable que está siendo leida por otro proceso.
        
        tiempoInicio = std::chrono::steady_clock::now();

        HANDLE hTimerThread = CreateThread(NULL, 0, timerThread, NULL, 0, NULL);

        if (hTimerThread == NULL) {
            std::cerr << "Error al crear el hilo del timer" << std::endl;
            return 1;
        }

        while(activo){
            char read_buf[256];     //256 bytes
            boost::system::error_code error;
            
            size_t len = serial.read_some(buffer(read_buf), error);

            if (error)
                cerr << "Error de lectura: " << error.message() << endl;
            else 
                cout << "Recibido: " << string(read_buf, len) << endl;   

            //resetear el timer al recibir comandos
            tiempoInicio = std::chrono::steady_clock::now();

            
            Comando comando = (string(read_buf, len) == "Hola desde cliente!\r\n") ? Comando::HANDSHAKE     //Comando:: porque es una enum class
                            : (string(read_buf, len) == "STATUS" ? Comando::STATUS
                            : (string(read_buf, len) == "START" ? Comando::START 
                            : (string(read_buf, len) == "STOP" ? Comando::STOP 
                            : (string(read_buf, len) == "SHUTDOWN" ? Comando::SHUTDOWN 
                            : (string(read_buf, len) == "HELP" ? Comando::HELP 
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
                    //activo = false;
                    break;
                case Comando::HELP:
                    respuesta = "Los comandos disponibles son: \nSTATUS: Solicita el estado actual de la máquina de estados. \nSTART: Inicia el proceso. \nSTOP: Detiene el proceso. \nSHUTDOWN: Apaga el servidor \nHELP: Muestra esta ayuda.";
                    break;
                case Comando::DEFAULT:
                    respuesta = "Comando desconocido. Use HELP para ver los comandos disponibles";
                    break;
            }

            write(serial, buffer(respuesta));        

        }   //while(activo)         
        // Esperar a que el hilo de timer termine
        WaitForSingleObject(hTimerThread, INFINITE);
        CloseHandle(hTimerThread);
    }    

    catch (boost::system::system_error& e) {
        cerr << "Error: " << e.what() << endl;
    } catch (...) {
        cerr << "Error desconocido" << endl;
    }

    return 0;
}