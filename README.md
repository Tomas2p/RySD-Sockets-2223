# FTP - Servidor en el puerto 2121

## Implementación del Protocolo de Transferencia de Archivos (FTP) en C++

Práctica de Programación de Sockets. Redes y Sistemas Distribuidos - ULL

## Integrantes

    @Tomás Pino Pérez (alu0101474311)

    @Jaime Alexander Sendín Dorta (alu0101500324)

    @Jorge Saiz de León (alu0101483078)

## Ejemplo de ejecucuión

    ftp> put tests/b.txt
    local: tests/b.txt remote: tests/b.txt
    227 Entering Passive Mode (127,0,0,1,215,107).
    450 Requested file action not taken. File unavaible.
    
    ftp> pass
    Passive mode: off; fallback to active mode: off.

    ftp> ls
    200 Okey
    125 Data connection already open; transfer starting
    total 12
    -rw-rw-r-- 1 tomaspp tomaspp 76 may  4 22:16 b.txt
    -rw-rw-r-- 1 tomaspp tomaspp 76 may  4 22:34 c.txt
    -rw-rw-r-- 1 tomaspp tomaspp 76 may  4 22:23 d.txt
    250 Closing data connection. Requested file action successful.
    
    ftp> put tests/b.txt
    local: tests/b.txt remote: tests/b.txt
    200 Okey
    450 Requested file action not taken. File unavaible.

## Constructor

    $ make
    rm -f ftp_server
    g++ -std=c++17 -lpthread -Wall -o  ftp_server src/*.cpp

## Uso

En una terminal, inicia el servidor FTP:

    $ ./ftp_server
    Server Running, waiting for connection to port 2121

En una segunda terminal, inicia el comando FTP:
  
    $ ftp
    ftp> open localhost 2121

Se recomienda ir a la carpeta llamada "tests" y ejecutar todos los comandos allí, por razones de seguridad.

## Implementaciones

- Login/logout
- Upload files
- Download files
- Get system information
- Get directory location
- Change directory
- Rename files/directories
- Delete files
- Create/delete directories
- List files in directory

## Comandos

| On ftp client | Name  |        Action        |
| :-----------: | :---: | :------------------: |
|       -       | USER  |   Client username    |
|       -       | PASS  |   Cliente password   |
|       -       | SYST  |  System information  |
|      pwd      |  PWD  |  Directory location  |
|      cd       |  CWD  |   Change directory   |
|    rename     | RNFR  |     Rename file      |
|    rename     | RNTO  |     Rename file      |
|      del      | DELE  |     Delete file      |
|     mkdir     |  MKD  |    Make directory    |
|     rmdir     |  RMD  |   Remove directory   |
|      ls       | LIST  |    List directory    |
|       -       | TYPE  | Type binary or ascii |
|       -       | PORT  |     Active mode      |
|     pass      | PASV  |     Passive mode     |
|      get      | RETR  |  Get/download file   |
|      put      | STORE |   Put/upload file    |
|     quit      | QUIT  |    Disconnection     |
