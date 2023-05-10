/*
REDES Y SISTEMAS DISTRIBUIDOS:
  2ª de grado de Ingeniería Informática
  Clase que atiende una petición FTP

INTEGRANTES:
  Tomás Pino Pérez              (alu0101474311)
  Jaime Alexander Sendín Dorta  (alu0101500324)
  Jorge Saiz de León            (alu0101483078)

COMANDOS IMPLEMENTADOS:
  USER  (nombre de usuario)
  PASS  (contraseña)
  SYST  (información del sistema)
  PWD   (ubicación actual)
  CWD   (cambiar de directorio)
  RNFR  (renombrar archivo)
  RNTO  (renombrar archivo)
  DELE  (eliminar archivo)
  MKD   (crear directorio)
  RMD   (eliminar directorio)
  LIST  (mostrar archivos)
  TYPE  (tipo de conexión)
  PORT  (modo activo)
  PASV  (modo pasivo)
  RETR  (descargar archivo del servidor)
  STORE (subir archivo al servidor)
  QUIT  (salir)
*/

#include "ClientConnection.h"

#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <langinfo.h>
#include <locale.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "common.h"

// Define una abreviatura para comparar el comando introducido y el definido en
// cmd, es decir si COMMNAND es igual cmd o no
#define COMMAND(cmd) strcmp(command, cmd) == 0

int connect_TCP(uint32_t address, uint16_t port) {
  struct sockaddr_in sin;
  int s{socket(AF_INET, SOCK_STREAM, 0)};

  if (s < 0) errexit("No se ha podido crear el socket: %s\n", strerror(errno));

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = address;
  sin.sin_port = htons(port);

  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    errexit("No se ha podido conectar con %s: %s\n", address, strerror(errno));

  return s;
}

ClientConnection::ClientConnection(int s, unsigned long ip) {
  // int sock = (int)(s);
  // char buffer[MAX_BUFF];

  control_socket = s, server_address = ip;

  fd = fdopen(s, "a+");

  if (fd == NULL) {
    std::cout << "Connection closed" << std::endl;
    fclose(fd);
    close(control_socket);
    ok = false;
    return;
  }

  ok = true;
  data_socket = -1;
  passive = false;
}

ClientConnection::~ClientConnection() { fclose(fd), close(control_socket); }

// Esta función es la que atiende las peticiones
// Aquí es donde se implementan las acciones asociadas a los comandos.
// Véase el ejemplo del comando USER.
// Si considera que debe añadir algún otro comando siéntase libre
// de hacerlo. Asimismo, puede añadir tantos métodos auxiliares como
// sea necesario.
void ClientConnection::WaitForRequests() {
  if (!ok) return;
    // Método para limpiar la pantalla
#ifdef _WIN32
  system("clr");
#else
  system("clear");
#endif
  fprintf(fd, "220 Service ready.\n");
  printf("**************\n* USER: aldo *\n* PASS: 1234 *\n**************\n\n");

  while (!parar) {
    fscanf(fd, "%s", command);

    // Comando USER
    if (COMMAND("USER")) {
      fscanf(fd, "%s", arg), printf("(USER): %s\n", arg);

      (!strcmp(arg, "aldo")) ? fprintf(fd, "331 User name ok, need password.\n")
                             : fprintf(fd, "332 Need account for login.\n");
    }

    // Comando PASS
    else if (COMMAND("PASS")) {
      fscanf(fd, "%s", arg), printf("(PASS): %s\n", arg);

      (!strcmp(arg, "1234")) ? fprintf(fd, "230 User logged in, proceed.\n")
                             : fprintf(fd, "530 Not logged in.\n");
    }

    // SYST
    else if (COMMAND("SYST")) {
      printf("(SYST): SHOW\n"), fprintf(fd, "215 UNIX Type: L8.\n");
    }

    // Comando PWD (pwd)
    else if (COMMAND("PWD")) {
      printf("(PWD): SHOW\n");

      char path[MAX_BUFF];

      if (getcwd(path, sizeof(path)) != NULL)
        fprintf(fd, "257 \"%s\" \n", path);
    }

    // Comando CWD (cd)
    else if (COMMAND("CWD")) {
      fscanf(fd, "%s", arg), printf("(CWD): %s\n", arg);

      char path[MAX_BUFF];

      if (getcwd(path, sizeof(path)) != NULL) {
        strcat(path, "/"), strcat(path, arg);

        (chdir(path) < 0)
            ? fprintf(fd, "550 Failed to change directory.\n")
            : fprintf(fd, "250 Directory successfully changed.\n");
      }
    }

    // Comando RNFR (rename)
    else if (COMMAND("RNFR")) {
      fscanf(fd, "%s", arg), printf("(RNFR): %s\n", arg);

      fprintf(fd, "350 Requested file action pending further information.\n");
    }

    // Comando RNTO (rename)
    else if (COMMAND("RNTO")) {
      fscanf(fd, "%s", arg2), printf("(RNTO): %s\n", arg2);

      (rename(arg, arg2) < 0) ? fprintf(fd, "550 Failed to rename file.\n")
                              : fprintf(fd, "250 File successfully renamed.\n");
    }

    // Comando DELE (del)
    else if (COMMAND("DELE")) {
      fscanf(fd, "%s", arg), printf("(DELE): %s\n", arg);

      (remove(arg) < 0) ? fprintf(fd, "550 Failed to remove file.\n")
                        : fprintf(fd, "250 File successfully removed.\n");
    }

    // Comando MKD (mkdir)
    else if (COMMAND("MKD")) {
      fscanf(fd, "%s", arg), printf("(MKD): %s\n", arg);

      (mkdir(arg, 0755) < 0)
          ? fprintf(fd, "550 Failed to create directory.\n")
          : fprintf(fd, "257 Directory successfully created.\n");
    }

    // Comando RMD (rmdir)
    else if (COMMAND("RMD")) {
      fscanf(fd, "%s", arg), printf("(RMD): %s\n", arg);

      (rmdir(arg) < 0) ? fprintf(fd, "550 Failed to remove directory.\n")
                       : fprintf(fd, "250 Directory successfully removed.\n");
    }

    // Comando LIST (ls)
    else if (COMMAND("LIST")) {
      printf("(LIST): SHOW\n");
      fprintf(fd, "125 Data connection already open; transfer starting\n");

      struct sockaddr_in sa;
      socklen_t sa_len = sizeof(sa);
      char buffer[MAX_BUFF];
      std::string mostrar, ls{"ls -l"};

      ls.append(" 2>&1");

      FILE *file = popen(ls.c_str(), "r");

      if (!file) {
        fprintf(fd, "450 Requested file action not taken. File unavaible.\n");
        close(data_socket);
      }

      else {
        if (passive)
          data_socket = accept(data_socket, (struct sockaddr *)&sa, &sa_len);

        while (!feof(file))
          if (fgets(buffer, MAX_BUFF, file) != NULL) mostrar.append(buffer);

        send(data_socket, mostrar.c_str(), mostrar.size(), 0);

        fprintf(fd,
                "250 Closing data connection. Requested file action "
                "successful.\n");
        pclose(file);
        close(data_socket);
      }
    }

    // TYPE
    else if (COMMAND("TYPE")) {
      fscanf(fd, "%s", arg), printf("(TYPE): %s\n", arg);

      if (!strcmp(arg, "A"))
        fprintf(fd, "200 Switching to ASCII mode.\n");
      else if (!strcmp(arg, "I"))
        fprintf(fd, "200 Switching to Binary mode.\n");
      else if (!strcmp(arg, "L"))
        fprintf(fd, "200 Switching to Tenex mode.\n");
      else
        fprintf(fd, "501 Syntax error in parameters or arguments.\n");
    }

    // PORT
    else if (COMMAND("PORT")) {
      passive = false;

      unsigned int ip[4], port[2];

      fscanf(fd, "%d,%d,%d,%d,%d,%d", &ip[0], &ip[1], &ip[2], &ip[3], &port[0],
             &port[1]);

      uint32_t ip_addr = ip[3] << 24 | ip[2] << 16 | ip[1] << 8 | ip[0];
      uint16_t port_v = port[0] << 8 | port[1];

      data_socket = connect_TCP(ip_addr, port_v);

      fprintf(fd, "200 Okey\n");
    }

    // Comando PASV (pass)
    else if (COMMAND("PASV")) {
      passive = true;
      printf("(PASV): passive %s\n", (passive ? "on" : "off"));

      struct sockaddr_in sin, sa;
      socklen_t sa_len{sizeof(sa)};
      int s{socket(AF_INET, SOCK_STREAM, 0)};

      if (s < 0)
        errexit("No se ha podido crear el socket: %s\n", strerror(errno));

      memset(&sin, 0, sizeof(sin));
      sin.sin_family = AF_INET;
      sin.sin_addr.s_addr = server_address;
      sin.sin_port = 0;

      if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("No se ha podido hacer bind con el puerto: %s\n",
                strerror(errno));

      if (listen(s, 5) < 0) errexit("Fallo en listen: %s\n", strerror(errno));

      getsockname(s, (struct sockaddr *)&sa, &sa_len);

      fprintf(fd, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n",
              (unsigned int)(server_address & 0xff),
              (unsigned int)((server_address >> 8) & 0xff),
              (unsigned int)((server_address >> 16) & 0xff),
              (unsigned int)((server_address >> 24) & 0xff),
              (unsigned int)(sa.sin_port & 0xff),
              (unsigned int)(sa.sin_port >> 8));

      data_socket = s;
    }

    // Comando RETR (get)
    else if (COMMAND("RETR")) {
      fscanf(fd, "%s", arg);

      char path[MAX_BUFF];
      if (getcwd(path, sizeof(path)) != NULL) {
        strcat(path, "/"), strcat(path, arg);
      }

      printf("(RETR): %s\n(path): %s", arg, path);

      FILE *file = fopen(arg, "rb");

      if (!file) {
        fprintf(fd, "450 Requested file action not taken. File unavaible.\n");
        close(data_socket);
      }

      else {
        fprintf(fd, "150 File status okay; about to open data connection.\n");

        struct sockaddr_in sa;
        socklen_t sa_len = sizeof(sa);
        char buffer[MAX_BUFF];
        int n;

        if (passive)
          data_socket = accept(data_socket, (struct sockaddr *)&sa, &sa_len);

        do {
          n = fread(buffer, sizeof(char), MAX_BUFF, file);
          send(data_socket, buffer, n, 0);
        } while (n == MAX_BUFF);

        fprintf(fd,
                "226 Closing data connection. Requested file action "
                "successful.\n");
        fclose(file);
        close(data_socket);
      }
    }

    // Comando STOR (put)
    else if (COMMAND("STOR")) {
      fscanf(fd, "%s", arg), printf("(STOR): %s\n", arg);

      FILE *file = fopen(arg, "wb");

      if (!file) {
        fprintf(fd, "450 Requested file action not taken. File unavaible.\n");
        close(data_socket);
      } else {
        fprintf(fd, "150 File status okay; about to open data connection.\n");
        fflush(fd);

        struct sockaddr_in sa;
        socklen_t sa_len = sizeof(sa);
        char buffer[MAX_BUFF];
        int n;

        if (passive)
          data_socket = accept(data_socket, (struct sockaddr *)&sa, &sa_len);

        do {
          n = recv(data_socket, buffer, MAX_BUFF, 0);
          fwrite(buffer, sizeof(char), n, file);
        } while (n == MAX_BUFF);

        fprintf(fd,
                "226 Closing data connection. Requested file action "
                "successful.\n");
        fclose(file);
        close(data_socket);
      }
    }

    // QUIT
    else if (COMMAND("QUIT")) {
      printf("(QUIT): %s\n", arg);
      fprintf(fd, "221 Service closing control connection.\n");
      stop();
    }

    // Comando no implementado
    else {
      fprintf(fd, "502 Command not implemented.\n"), fflush(fd);
      printf("Comando: %s | Argumentos: %s\n", command, arg);
      printf("Error interno del servidor\n");
    }
  }

  fclose(fd);
  return;
}

void ClientConnection::stop() {
  close(data_socket), close(control_socket);
  parar = true;
}