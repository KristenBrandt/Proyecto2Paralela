# Proyecto2Paralela
Proyecto 2 de Computación paralela y distribuida

Uso de OpenMPI para paralelizar un programa que descifra con fuerza bruta un
codigo cifrado con DES.

En la carpeta encrypt se encuentra el programa para encriptar un texto. Este
termina en un archivo 'cipher.txt' que luego puede ser utilizado en los
otros programas.

En la carpeta decrypt se encuentra el programa para descifrar el archivo
'cipher.txt', dado que se sabe la llave que se utilizo.

En la carpeta trykey se encuentran los archivos secuencial y paralelo para
descifrar el archivo 'cipher.txt' con fuerza bruta, dado que no se sabe la llave que se utilizo.
