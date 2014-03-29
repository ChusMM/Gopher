/* servidor gopher mas conocido como "el gofre" concurrente mixto
   Jesus Manuel Munoz Mazuecos Ingeniería técnica en informática de sistemas */
   
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include "SocketPasivoTCP.h"
#include "notiferror.h"

#define MAXLINEA    255
#define QLEN          5
#define N           100
#define ARCHIVO     '0'
#define DIRECTORIO  '1'
#define BINARIO     '9'

#define indice    "/index.txt"
#define servidor  "/serv1"

int gophermxd(int s);
void listar(char *path, int s, char opc);

/********************************************************************************
Función gophermxd: Interpretar el identificador de archivo enviado por el cliente
Parámetros: 
	int s Descriptor del socket.

Valor devuelto: 0 en caso de éxito y -1 en caso de error
*********************************************************************************/
int gophermxd(int s){

    char buf[MAXLINEA+1];
    char raiz[MAXLINEA+1]; /* = "/home/jesus/AplDistr/gophermxd/serv1"; */
    int cliStringLen;
    struct stat *bufStat;
    char tipo;
    int hijo = 0;
    int pid, estado;

    if(getcwd(raiz, sizeof(raiz)) == NULL ) { /* Obtengo la raiz del mi directorio de tarbajo */
        NotifyError("Error al obtener el direcotorio raiz", -1);
        return -1;
    }

    strncat(raiz, servidor, strlen(servidor)); /* Concatena raiz con el nombre de la carpeta del servidor que
                                                  va a ser descendiente del directorio de trabajo */

    if(strlen(raiz) > MAXLINEA) {
        NotifyError("Path del directorio raiz demasiado largo", -1);
        return -1;
    }

    cliStringLen = read(s, buf, MAXLINEA); /* recibir mensaje del cliente */

    if(cliStringLen < 0) {
        NotifyError("Error al recibir el mensaje", -1);
        return -1;
    }
      
    if(cliStringLen > MAXLINEA) {
        NotifyError("Línea demasiado larga", -1);
        return -1;
    }

    if(strncmp(buf, "\r\n", 2) == 0) { /* Si se recibe \n \r envio por defecto el fichero index del dir raiz */
        tipo = DIRECTORIO;
    }
    else {
        
        buf[cliStringLen-2] = '\0'; /* \r = \0 */
        tipo = buf[0]; /* buf[0] indentifica el tipo de archivo según los index.txt*/
        buf[0] = '/';  /* la posicion cero del buffer la sustituyo por el caracter / para poder concatenar el
                          contenido del buffer con el directorio raiz */
        strncat(raiz, buf, cliStringLen-1); /* concateno hasta el caracter \0 inclusive */
    }

    if(tipo == DIRECTORIO) { /* si se trata de un diretorio se le concatena el nombre del fichero de contenido
                             */
        strncat(raiz, indice, strlen(indice));
    }

    bufStat = (struct stat *)malloc(sizeof(struct stat));
        
    if(bufStat == NULL) {
        NotifyError("Error al asignar memoria\n", -1);
        return -1;
    }
            
    if(stat(raiz, bufStat) == -1) {
        NotifyError("Error en stat\n", -1);
        return -1;
    }

    if(bufStat -> st_mode > N) { /* Si el tamaño fichero es mayor que N el contenido será enviado por el hijo
                                 */ 
        hijo = 1;
    }
    
    switch(fork()) {

        case -1:
            
            NotifyError("Error en el fork\n", -1);
            return -1;
            break;
            
        case 0:

            if(hijo) {
                listar(raiz, s, tipo);
            }
            exit(0);
            break;
            
        default:

            pid = wait(&estado); /* Esperar a que finalize el hijo */
            
            if(!hijo) {
                listar(raiz, s, tipo);
            }
            break;
    }
    return 0;           
}
/********************************************************************************
Función listar:  enviar el contenido del archivo especificado en path al cliente.
Parámetros: 
char path: Descriptor del socket.
int s: Descriptor del soket
char opc: carácter que inidica el tipo de archivo que se va a examinar
*********************************************************************************/
void listar(char *path, int s, char opc){
    FILE *fp;
    char linea[MAXLINEA+1];

    switch(opc){

        case ARCHIVO:

            if( (fp = fopen(path, "r")) == NULL) {
                NotifyError("Error al examinar el archivo o directorio o no existe", -1);
            }

            while( fgets(linea, MAXLINEA, fp) != NULL ) {

                if(strncmp(linea, ".\r\n", 3) == 0){
                    strncat(".", linea, strlen(linea));
                }
                
                if(write(s, linea, strlen(linea)) < 0) {
                    NotifyError("Error al enviar", -1);
                }
            }

            if(write(s, ".\r\n", 3) < 0) {
                NotifyError("Error al enviar", -1);
            }
            break;
        
        case DIRECTORIO:

            if( (fp = fopen(path, "r")) == NULL) {
                NotifyError("Error al examinar el archivo o directorio o no existe", -1);
            }

            while( fgets(linea, MAXLINEA, fp) != NULL ) {
        
                if(write(s, linea, strlen(linea)) < 0) {
                    NotifyError("Error al enviar", -1);
                }
            }

            if(write(s, ".\r\n", 3) < 0) {
                close(s);
                NotifyError("Error al enviar", -1);
            }
            break;
            
        case BINARIO:

            if( (fp = fopen(path, "rb")) == NULL) {
                NotifyError("Error al examinar el archivo o directorio o no existe", -1);
            }

            while( fgets(linea, MAXLINEA, fp) != NULL ) {
        
                if(write(s, linea, strlen(linea)) < 0) {
                    NotifyError("Error al enviar", -1);
                }
            }
            break;
    }   
}

/*******************************************************************************
Función main:  crea el socket TCP acepta clientes concurrentemente y llama al            
               servicio gopher.
Parámetros: 
argv[1]: Número de puerto.
*******************************************************************************/

int main(int argc, char *argv[]){

    char    *servicio;       /* servicio */
    
    struct sockaddr_in fsin; /* direccion del cliente */
    int msock;               /* socket maestro*/
    fd_set  rfds;            /* conjunto de descriptores de lectura */
    fd_set  afds;            /* conjunto de descriptores activos */
    unsigned int    alen;    /* longitud de la direccion */
    int fd, nfds;
    
    switch (argc) {
        
        case 2:
            
            servicio = argv[1];
            break;

        default:
            
            NotifyError("Uso: <puerto> \n", 0);
            return 1;
    }
    
    msock = SocketPasivoTCP(servicio, QLEN);

    nfds = getdtablesize(); /* Número máximo de ficheros que un proceso puede tener abiertos */
    FD_ZERO(&afds);         /* vacia el conjunto de descriptores activos */
    FD_SET(msock, &afds);   /* añade el descriptor msock al conjunto de descriptores archivos */

    while (1) {
        memcpy(&rfds, &afds, sizeof(rfds)); /* volcar el conjunto de descriptores activos en el conjunto de
                                               descriptores de lectura */ 

        if (select(nfds, &rfds, NULL, NULL, NULL) < 0){ /* Comprueba si se ha producido algun descriptor de
                                                           lectura ha cambiado de estado */
            NotifyError("gophermxd: Error en select\n", -1);
            return -1;
        }
        
        if (FD_ISSET(msock, &rfds)) { /* si ha cambiado de estado el descriptor de lectura msock */
            int ssock;

            alen = sizeof(fsin);
            ssock = accept(msock, (struct sockaddr *)&fsin, &alen); /* aceptar conexion */

            if (ssock < 0) {
                NotifyError("gophermxd: Error en accept\n", -1);
                return -1;
            }

            FD_SET(ssock, &afds); /* el socket esclavo se añade al conjunto de descriptores activos */
        }

        for (fd=0; fd<nfds; ++fd)
            if (fd != msock && FD_ISSET(fd, &rfds)) /* si uno de los descriptores cambió de estado y no es
                                                       msock */
                if (gophermxd(fd) == 0) {  /* llamada al servicio */
                    close(fd);             /* cierra la conexion */
                    FD_CLR(fd, &afds);     /* el descriptor se elimina del conjunto de descriptores activos */
                }
    }
}
