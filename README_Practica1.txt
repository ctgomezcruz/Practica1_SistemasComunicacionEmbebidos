ITESO - Especialidad en Sistemas Embebidos

Practica 1

Para hacer uso correcto de los archivos es necesario tener en cuenta lo siguiente.

Del lado del servidor.
1. El archivo en Python es un servidor básico que recibe los datagramas provenientes del cliente.
   a. Revisa su integridad, mediante los CRC bytes.
   b. Desencripta los mensajes.
   c. Imprime los mensajes en la consola del servidor.
   d. Devuelve como eco los mismos datagramas que recibe: mensaje con padding encryptado + longitud del 
      mensaje sin padding + CRC bytes.
2. Para que este programa en Python funcione se requiere instalar:
   a. pycyptodome
   Y asegurarse de contar con las librerías "binascii" y "socket".

Del lado del cliente.
1. Debe tenerse instalado el proyecto de ejemplo : frdmk64f_lwip_tcpecho_freertos del SDK 
   para la tarjeta FRDMK64F de NXP.
2. Y sustituir las capetas "source" y "drivers" del proyecto de ejemplo por las carpetas en este 
   repositorio.

 
