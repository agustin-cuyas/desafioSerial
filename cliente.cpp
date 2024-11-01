/*
El cliente solicita los datos necesarios para establecer la conexión. Guardarlos en variables y configurar la conexión con esas variables

STATUS: Solicita al servidor el estado actual de la máquina de estados.
START: Inicia un proceso en el servidor.
STOP: Detiene el proceso en el servidor.
SHUTDOWN: Finaliza la ejecución del servidor. (terminar programa servidor)

HELP:
SALIR: terminar programa cliente

Desconexión inesperada. (por ejemplo cortar el emulador)
Tiempo de espera sin respuesta.

funciones: ingresarParametros
           establecerConexion
           que el while se corte cuando diga SALIR
*/

#include "mainHeader.hpp"

int main(){
    string puerto;
    int baudrate;
    int datos;
    char paridad;
    int parada;

    io_context io;

    int intentos = 0;

    while(intentos < 3) {
        try{
            cout << "\nPara conectarse a un servidor, proporcione los siguientes parámetros:\n"
                 << "Ingrese nombre del puerto: ";
            cin >> puerto;
            
            cout << "Ingrese baud rate: ";
            cin >> baudrate;

            cout << "Ingrese cantidad de bits de datos: ";
            cin >> datos;

            cout << "Ingrese paridad (P: par, I: impar o N: ninguno): ";
            cin >> paridad;
            paridad = tolower(paridad);

            cout << "Ingrese cantidad de bits de parada (1 o 2): ";
            cin >> parada;

            serial_port serial(io, puerto);

            serial.set_option(serial_port_base::baud_rate(baudrate));
            serial.set_option(serial_port_base::character_size(datos));

            switch(paridad){
                case 'p':
                    serial.set_option(serial_port_base::parity(serial_port_base::parity::even));
                    break;
                case 'i':
                    serial.set_option(serial_port_base::parity(serial_port_base::parity::odd));
                    break;
                case 'n':
                    serial.set_option(serial_port_base::parity(serial_port_base::parity::none));
                    break;
            }   

            switch(parada){
                case 1:
                    serial.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
                    break;
                case 2:
                    serial.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::two));
                    break;
            }

            cin.ignore();
            system("cls");
        

            cout << "\nConexión establecida" << endl;

            string mensaje = "Hola desde cliente!\r\n";
            write(serial, buffer(mensaje));

            cout << "\nMensaje enviado: " << mensaje;

            //"handshake"
            char read_buf[256];     //256 bytes
            boost::system::error_code error;    
            size_t len;

            len = serial.read_some(boost::asio::buffer(read_buf), error);
            if (error)
                cerr << "Error de lectura: " << error.message() << endl;
            else
                cout << "Recibido: " << string(read_buf, len) << endl;            
            
            string comando;
            bool activo = true;

            while(activo){
                cout << "\nIngrese comandos: ";
                getline(cin, comando);

                if(comando == "SALIR")
                    activo = false;

                write(serial, buffer(comando));

                //leo la respuesta                
                len = serial.read_some(boost::asio::buffer(read_buf), error);
                if (error)
                    cerr << "Error de lectura: " << error.message() << endl;
                else
                    cout << "Recibido: " << string(read_buf, len) << endl;  
            }
        }
        catch (boost::system::system_error& e) {
            cerr << "\nError de conexión (Intento " << intentos + 1 << " de 3): " << e.what() << endl;
            intentos++;
            if (intentos >= 3) {
                cerr << "Conexión fallida después de 3 intentos." << endl;
            }
        }
        catch (...) {
            cerr << "Error desconocido" << endl;
        }
    }

    return 0;
}