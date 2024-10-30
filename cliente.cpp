/*
El cliente solicita los datos necesarios para establecer la conexión. Guardarlos en variables y configurar la conexión con esas variables
*/
#include <boost/asio.hpp>
#include <iostream>

using namespace std;
using namespace boost::asio;

int main(){
    string puerto;
    int baudrate;
    int datos;
    char paridad;
    int parada;

    try{
        io_context io;
        cout << "Ingrese nombre del puerto: ";
        cin >> puerto;

        cout << "Ingrese baud rate: ";
        cin >> baudrate;

        cout << "Ingrese cantidad de bits de datos: ";
        cin >> datos;

        cout << "Ingrese paridad (par, impar o ninguno): ";
        cin >> paridad;

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

        cout << "Conexión establecida" << endl;

        string message = "Hola desde Boost.Asio!\r\n";
        write(serial, buffer(message));

        cout << "Mensaje enviado: " << message << endl;

        string comando;

        while(true){
            cout << "Ingrese comandos: ";
            getline(cin,comando);

            write(serial, buffer(comando));
        }
        
    }    

    catch (boost::system::system_error& e) {
        cerr << "Error: " << e.what() << endl;
    } catch (...) {
        cerr << "Error desconocido" << endl;
    }

    return 0;
}