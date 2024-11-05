#include "mainHeader.hpp"

bool conectado = true;
boost::system::error_code ec = error::would_block;      //errorcode se inicializa con would_block (la operación no está terminada)
size_t length = 0;

////////////////////////////    TIMEOUT     /////////////////////////////////////////
//Esta función se activa cuando el deadline_timer alcanza el límite de tiempo sin que la lectura haya finalizado.
void onTimeout(const boost::system::error_code& error, serial_port& serial) {
    if (!error) {
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
    timer.expires_from_now(boost::posix_time::seconds(1)); //timeout 1 seg

    timer.async_wait(boost::bind(onTimeout, boost::placeholders::_1, boost::ref(serial)));  //uso bind para enlazar onTimeout con el timer. así si se llega al timeout, la función cancela la lectura

    serial.async_read_some(boost::asio::buffer(buffer, buffer_size),
                           boost::bind(onRead, boost::placeholders::_1, boost::placeholders::_2)
                           );  //la funcion onRead se llama cuando la lectura termine

    // Ejecuta la operación asíncrona en un bucle hasta que termine o se alcance el timeout
    do {
        io.run_one();   //ejecutar solo una operación asíncrona a la vez
    } while (ec == boost::asio::error::would_block && conectado);  //ec cambia de valor cuando termina una lectura o finaliza el timeout

    timer.cancel();     //cancela el timeout
    return !ec;  //devuelve true si la lectura es existosa
}
//////////////////////////////////////////////////////////////////////////////

void ingresarParametros(string& puerto, int& baudrate, int& datos, char& paridad, int& parada){
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
}

void conectarSerial(serial_port& serial, io_context& io, const string& puerto, int& baudrate, int& datos, char& paridad, int& parada){
    serial.open(puerto);
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
}

void comandos(serial_port& serial, string& comando, bool& conectado){
    cout << "\nIngrese comandos: ";
    getline(cin, comando);

    if(comando == "SALIR")
        conectado = false;

    write(serial, buffer(comando));
}

int main(){
    string puerto;
    int baudrate;
    int datos;
    char paridad;
    int parada;

    io_context io;
    serial_port serial(io);

    int intentos = 0;

    while(intentos < 3) {
        try{
            
            ingresarParametros(puerto, baudrate, datos, paridad, parada);

            conectarSerial(serial, io, puerto, baudrate, datos, paridad, parada);

            string mensaje = "Hola desde cliente!\r\n";
            write(serial, buffer(mensaje));

            cout << "\nMensaje enviado: " << mensaje;

            //"handshake"
            char read_buf[256];     //256 bytes
            boost::system::error_code error;    
            size_t len;
            deadline_timer timer(io);
                
            len = serial.read_some(boost::asio::buffer(read_buf), error);
            if (error)
                cerr << "Error de lectura: " << error.message() << endl;
            else
                cout << "Recibido: " << string(read_buf, len) << endl;
            
            string comando;
            conectado = true;

            while(conectado){

                comandos(serial, comando, conectado);
                
                //leo la respuesta con timeout 
                if (leerConTimeout(serial, timer, io, read_buf, sizeof(read_buf))) {
                    cout << "Recibido: " << string(read_buf, length) << endl;
                } else {
                    cerr << "El servidor tardó mucho en responder." << endl;
                }  
            }
        }
        catch (boost::system::system_error& e) {
            cerr << "\nError de conexión (Intento " << intentos + 1 << " de 3): " << e.what() << endl;
            intentos++;
            if (intentos >= 3) {
                cerr << "Conexión fallida después de 3 intentos." << endl;
            }
        }
        catch (out_of_range& e) {
            cerr << e.what() << endl;
        }
        catch (...) {
            cerr << "Error desconocido" << endl;
        }
    }

    return 0;
}