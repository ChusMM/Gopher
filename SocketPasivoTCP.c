/* SocketPasivoTCP.c - SocketPasivoTCP */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "notiferror.h"

/******************************************************************************
Función SocketPasivoTCP: Asigna y vincula un socket usando TCP.
Parámetros:
	const char *servicio: Es el puerto o nombre del servicio.
	int longcola: Un entero que indica la longitud de la cola de espera.

Valor devuelto: en caso de éxito devuelve el descriptor del socket 
		asociado y en caso de error devuelve un entero menor que cero.
******************************************************************************/
int SocketPasivoTCP(const char *servicio, int longcola)
/*
 * Argumentos:
 *  servicio   - Nombre o puerto del servicio
 *	longcola   - Maxima longitud de la cola de espera
 */
{
	struct servent	*pse;	/* Puntero a descripcion de un servicio */
	struct sockaddr_in sin;	/* Direccion IP*/
	int	s;	/* Descriptor del socket*/

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;

    /* Obtiene puerto asociado al servicio */
	if ((pse = getservbyname(servicio, "tcp")) != NULL )
		sin.sin_port = pse->s_port;
	else if ( (sin.sin_port = htons(atoi(servicio))) == 0 ) {
		NotifyError("SocketPasivoTCP: Servicio incorrecto", 0);
		return -1;
	}

    /* direccion IP del servidor */
	sin.sin_addr.s_addr = INADDR_ANY;
		
    /* Crea el socket */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		NotifyError("SocketPasivoTCP: Imposible crear el socket", 1);
		return -2;
	}

    /* Vincular el socket local */
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) != 0) {
		NotifyError("SocketPasivoTCP: Imposible vincular el socket", 1);
		close(s);
		return -3;
	}
    /* Activar el modo pasivo del socket */
    if (listen(s, longcola) < 0) {
		NotifyError("SocketPasivoTCP: Imposible poner a la escucha el socket", 1);
		return -4;
	}
    	
  return s;
}
