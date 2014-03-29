/* Notificacion de errores - notiferror.c*/

#include <stdio.h>

/*************************************************************************
 Funcion NotifyError: función usada para la notificacion de errores
 Parámetros:
	const char *message: mensaje de error
	int error: número del error
*************************************************************************/
void NotifyError(const char *message, int error)
{
	fprintf(stderr, "%s\n", message);	
	if(error) perror("Error");	
}
