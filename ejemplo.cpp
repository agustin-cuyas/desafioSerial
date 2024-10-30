/*
Este ejemplo escribe en el puerto COM3, si quiero leer lo que recibe usando el emulador, hacer un par con otro puerto y leer ese en tools > terminal
*/

#include <boost/asio.hpp>
#include <iostream>

using namespace std;

int main() {
    try {
        boost::asio::io_context io;

        // Cambia "COM3" al nombre del puerto correspondiente en tu sistema
        boost::asio::serial_port serial(io, "COM3");
        
        // Configuración del puerto
        serial.set_option(boost::asio::serial_port_base::baud_rate(9600));
        serial.set_option(boost::asio::serial_port_base::character_size(8));
        serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
        serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));

        // Mensaje a enviar
        string message = "Hola desde Boost.Asio!\r\n";
        
        // Escribir en el puerto
        boost::asio::write(serial, boost::asio::buffer(message));
        
        cout << "Mensaje enviado: " << message;

         // Leer respuesta del puerto
        char read_buf[256];     //256 bytes
        boost::system::error_code error;
        size_t len;

        len = serial.read_some(boost::asio::buffer(read_buf), error);

        if (error) {
            cerr << "Error de lectura: " << error.message() << endl;
        } else {
            cout << "Recibido: " << string(read_buf, len) << endl;
        }

    } 
    
    catch (boost::system::system_error& e) {
        cerr << "Error: " << e.what() << endl;
    } catch (...) {
        cerr << "Error desconocido" << endl;
    }

    return 0;
}
