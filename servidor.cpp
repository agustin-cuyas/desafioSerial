#include <boost/asio.hpp>
#include <iostream>

using namespace std;
using namespace boost::asio;

int main(){
    try{
        io_context io;
        
        serial_port serial(io, "COM3");
        serial.set_option(serial_port_base::baud_rate(9600));
        serial.set_option(serial_port_base::character_size(8));
        serial.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
        serial.set_option(serial_port_base::parity(serial_port_base::parity::none));

        cout << "Servidor iniciado, esperando comandos" << endl;

        while(true){
            char read_buf[256];     //256 bytes
            boost::system::error_code error;
            
            size_t len = serial.read_some(buffer(read_buf), error);

            if (error) {
                cerr << "Error de lectura: " << error.message() << endl;
            } else {
                cout << "Recibido: " << string(read_buf, len) << endl;
            }         

            if(string(read_buf, len) == "Hola desde Boost.Asio!\r\n"){
                write(serial, buffer("Hola"));
            }
        }          
    }    

    catch (boost::system::system_error& e) {
        cerr << "Error: " << e.what() << endl;
    } catch (...) {
        cerr << "Error desconocido" << endl;
    }

    return 0;
}