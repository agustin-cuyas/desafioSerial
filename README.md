# Tercer desafío C++
## Cliente-Servidor con Comunicación Serial

- #### Descripción general del proyecto:
El desafío consiste en desarrollar dos programas, un cliente y un servidor que se comuniquen a través del protocolo serial.
En particular, el **cliente** debe poder, una vez que se conecta al servidor, enviarle comandos y recibir respuesta a estos. Además tiene que manejar los errores que puedan ocurrir y tener un tiempo de espera sin respuesta.
En el caso del **servidor**, éste lleva a cabo una máquina de estados controlada por el cliente mediante los comandos. Debe llevar un registro de los comandos recibidos y de los cambios de estado. También tiene que tener un timeout configurable, si no recibe mensajes en ese lapso se debe apagar.

- #### Proceso de desarrollo y las decisiones de diseño tomadas.
El desafío proponía una serie de librerías para gestionar la comunicación serial. Elegí Boost.Asio ya que la recomendaba para su uso con C++. No la conocía así que previamente la investigué en su página oficial https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio.html.
Para el emulador se recomendaba Virtual Serial Port Emulator así que lo descargué, su uso es muy intuitivo.
Para el desarrollo de los programas decidí hacer un mainHeader y dos archivos de clases para la máquina de estados (un .hpp y un .cpp). Elegí hacerla con clases para aprovechar el polimorfismo y que el código sea más limpio. Por último hice uso de funciones para que el main sea claro y conciso.

- #### Cómo configurar y ejecutar las aplicaciones cliente y servidor.
Lo primero que se debe hacer es encender el emulador y crear una conexión con el puerto correspondiente, en este caso el COM3.
Luego se debe ejecutar el servidor que se conectará al puerto con los parámetros correctos. Al iniciar pedirá un timeout, permanecerá encendido hasta que transcurra este tiempo sin recibir comandos.
Finalmente el cliente al ser ejecutado pedirá los parámetros para conectarse al servidor, estos deben ser:
>Puerto: COM3
>Baud rate: 9600
>Paridad: Ninguno
>Bits de datos: 8
>Bits de parada: 1

	Si se ingresa mal el puerto, se devolverá un error de conexión. Si se ingresa erroneamente el tamaño del byte el cliente recibirá un error de fuera de rango. En ambos casos se dará un nuevo intento al cliente, siendo el máximo 3.
	Una vez que la conexión fue exitosa, el cliente manda un mensaje al servidor que es respondido por este a modo de "handshake".
	A partir de ahí el cliente es capaz de enviar comandos al servidor para manejar la máquina de estados, apagar el servidor o desconectarse. Los comandos son:
	>STATUS: Solicita al servidor el estado actual de la máquina de estados.
  >START: Inicia un proceso en el servidor.
  >STOP: Detiene el proceso en el servidor.
  >SHUTDOWN: Finaliza la ejecución del servidor.
  >HELP: Muestra la lista de comandos.
  >SALIR: Desconecta al cliente del puerto, permitiendo iniciar otra conexión.
	
	En el caso que el cliente no reciba una respuesta del servidor por más de un segundo, se avisa y da a entender que probablemente éste este apagado.
	
- #### Descripción de las herramientas utilizadas.
Como se mencionó en secciones anteriores, en el desarrollo del desafío se utilizaron herramientas adicionales como son la librería Boost y el emulador VSPE. 
De la librería se pueden destacar algunas instrucciones fundamentales para la conexión y comunicación:
` boost::asio :: serial_port::open("COM3");		//abre el puerto COM3`
`boost::asio :: serial_port::set_option();	//se utilizará para configurar los otros parámetros`
`boost::asio :: serial.set_option(serial_port_base::baud_rate(9600));`
`boost::asio :: serial.set_option(serial_port_base::character_size(8));`
`boost::asio :: serial.set_option(serial_port_base::stop_bits(serial_port_base:: stop_bits::one));`
`boost::asio :: serial.set_option(serial_port_base::parity(serial_port_base:: parity::none));`
`//Para escribir en el puerto:`
` write(serial, buffer("mensaje));`
`//Para leer del puerto:`
`serial.read_some(buffer, error);`
Con respecto al emulador, su uso es sencillo:
Se debe ir a Device → Create → Connector → COM3 → Finalizar → Start Emulator
Justamente al ser tan sencillo, no es muy realista, ya que tanto servidor como cliente podrían configurarse con distinta tasa de baudios, distinta longitud de mensaje, etcétera y aún recibir bien los mensajes. Esto se debe a que el emulador hace una interpretación de lo que recibe y de esta manera puede ser leído con otra configuración.
En la realidad, si uno lee en un puerto a otra velocidad con la que se escribe, hay perdida de memoria y el mensaje no es recibido correctamente.

- #### Cualquier desafío técnico enfrentado y cómo fue resuelto.
La primera traba del desafío fue lograr incluir la librería. Al descargarla se baja todo el paquete Boost, un archivo bastante pesado. Boost/asio es una librería del tipo header-only, que significa que solo contiene templates y funciones en linea, por lo que al incluirla especificando la ruta, la librería tiraba error ya que no reconocía ninguno de los includes que esta contenía. Estos errores no se solucionaban incluyendo todas las rutas a estos headers en el código. Con ayuda de mis compañeros encontramos en la página de Boost un Get Started que detallaba su vinculación con Visual Studio: https://www.boost.org/doc/libs/1_86_0/more/getting_started/windows.html .
Lamentablemente este tutorial está totalmente desactualizado (menciona una versión del 2005), por lo que tuve que reversionarlo para el  software actual. A continuación están los 3 puntos fundamentales para poder untilizar la librería:
>- Configurar c_cpp_properties.json: Ctrl+Shift+P → C/C++: Edit Configurations (JSON)
Se debe generar este archivo e incluir la ruta exacta al paquete boost. Por ejemplo: "C:/Users/ususario/boost_1_86_0"
>- Configurar tasks.json: Ctrl+Shift+P → Tasks: Configure Default Build Task
Acá también se debe incluir la ruta anterior junto con "-I" y "-lsw2_32". Esto se usará al momento de compilar el código.
>- Comando para compilar:
El comando usado debe pasar los parámtros mencionados anteriormente, para enlazar de forma correcta el programa con la librería:
`g++ -std=c++17 -I "C:/Users/usuario/Documents/boost_1_86_0" main.cpp -o main -lws2_32`

	Solucionado esto, la librería se puede incluir de esta manera:
	`#include <boost/asio.hpp>`

	El siguiente problema fue hacer funcional los timeout.
	El primer acercamiento fue crear un hilo que en paralelo corra un contador que una vez alcanzado el límite corte la ejecución del main. Esto funcionaba parcialmente, ya que la instrucción vista  `serial.read_some()` bloquea el programa, es decir que una vez alcanzada esa linea no continua hasta leer algo en el buffer. Entonces si se alcanzaba el timeout, el servidor no se apagaba hasta recibir un nuevo mensaje del cliente.
	La solución fue hacer una lectura sincrónica, la cual si se alcanza el timeout se corta. 
	Esta implementación hace uso de la función `boost::bind()` que se encuentra en la librería `boost/bind/bind.hpp`. La función es similar a `bind`de C++, se le pasa como argumento una función y una serie de parámetros que se enlazan a esta.
	Generadas una serie de funciones, se modifica la lectura. Esta solo se hace dentro del timeout, una vez superado no se lee más y, en el caso del servidor éste se apaga, y para el cliente se avisa que el servidor tardó mucho en responder.
	En el código el manejo del timer está resaltado para remarcar la solución que más tiempo me llegó encontrar.
	

- #### Posibles mejoras u optimizaciones futuras.
Considero que el código cumple bien las consignas del desafío, se agregaron comandos para facilitarle la experiencia al usuario y se tuvieron en cuenta varios casos particulares. Podría añadirse más validaciones al momento del ingreso de parámetros, aunque para este caso particular no hay diferencia ya que como se vió el emulador permite prácticamente cualquier configuración. Como optimización se podría separar las funciones en un archivo aparte para que quede aún más prolijo el main.
