#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#include <dirent.h> //Usada por comando_cd , comando_dir y comando_mkdir

// Variables usadas en la lectura y parseo
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

// Prototipos de los procedimientos
char **separar_linea(char *linea);
int ejecutar(char **arg);
int cant_comandos();
void mostrar_help(char* comando);

int comando_cd(char **arg);
int comando_cls(char **arg);
int comando_copy(char **arg);
int comando_echo(char **arg);
int comando_exit(char **arg);
int comando_help(char **arg);
int comando_delfile(char **arg);
int comando_dir(char **arg);
int comando_mkdir(char **arg);

void cambiar_color_escritura(char* nombre_color);
void restaurar_color_escritura();

int (*realizar_comando[]) (char **) = { 
  &comando_cd,
  &comando_cls,
  &comando_copy, 
  &comando_echo,
  &comando_exit,
  &comando_help,
  &comando_delfile,
  &comando_dir,
  &comando_mkdir
}; 

char *comandos[] = {
  "cd",     
  "cls",    
  "copy",  
  "echo",  
  "exit", 
  "help", 
  "delfile",
  "dir",
  "mkdir" 
};


int main (int argc, char ** argv) {

  int finalizar = 0;
  cambiar_color_escritura("verde"); //Detalle agregado para cambiar el color del prompt
  char *prompt = "miniShell> ";
  char cmd[1024]; //Se asume que la linea de input no superara los 1024 bytes

  while (!finalizar) { //Mientras no se ejecute el comando exit 
    char *cursor;
    char ultimo_char;
    int ret; 
    int cont;

    // Imprimir el prompt
    ret = write(1, prompt, strlen(prompt));
    restaurar_color_escritura(); //Se cambia de color para todo lo que no sea el prompt

    if (!ret) { 
      finalizar = 1;
      break;
    }
    
    // Leer y parsear la entrada
    for(ret = 1, cont = 0, cursor = cmd, ultimo_char = 1; ret && (++cont < (1024-1)) && (ultimo_char != '\n');cursor++) { 
      ret = read(0, cursor, 1);
      ultimo_char = *cursor;
    } 
    *cursor = '\0';

    if (!ret) { 
      finalizar = 1;
      break;
    }

    //Ejecutar el comando requerido
    char **args= separar_linea(cmd); //Se obtiene el nombre del comando y sus argumentos
    finalizar= ejecutar(args); //Se ejecuta el comando y se guarda el retorno para ver si hay que continuar o no
    free(args);  //Se libera el espacio para args
    cambiar_color_escritura("verde"); //Detalle agregado para cambiar el color del prompt
  }

  return 0;
}


char **separar_linea(char *linea)
{
  int bufsize = LSH_TOK_BUFSIZE;
  int posicion = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "Error en la alocacion de memoria\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(linea, LSH_TOK_DELIM); //Obtenemos un puntero al primer token  
  while (token != NULL) {
    tokens[posicion] = token; // Almacenamos cada puntero en un buffer (arreglo) de tokens.
    posicion++;

    if (posicion >= bufsize) { //Si se excede el tamanio del buffer, se hace una realocacion
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "Error en la alocacion de memoria\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM); 
  }
  tokens[posicion] = NULL; //No hay mas tokens, terminamos la lista de tokens con NULL
  return tokens; //Devolvemos el arreglo de tokens, listo para ser ejecutado.
}

int ejecutar(char **arg){
    int i;

    if(arg[0]==NULL) {
      return 1; //El primer argumento es el nombre del comando, no puede ser nulo.
    }

    for(i=0; i<cant_comandos();i++){  //i va hasta la lista de comandos que tenemos.
        if(strcmp(arg[0],comandos[i])==0){ //Se fija que comando debe ser ejecutado
            return (*realizar_comando[i])(arg);  //Llama al procedimiento que ejecutara el comando
        }
    }

    printf("Error: Comando no encontrado. Puede ingresar 'help'para ver la lista de comandos disponibles\n"); //Si llegamos aca quiere decir que el comando no existe

    return 0; //retornar 0 implica que seguira la ejecucion de la mini shell.

}

int cant_comandos() {
  return sizeof(comandos) / sizeof(char *);
}
/////////////////////////////
//////Lista de comandos/////
///////////////////////////

int comando_cd(char **arg) {
 /* Uso:    cd [/ruta/a/cambiar/]         */

    if (arg[1] == NULL) { 
        //Si cd no recibe ningun argumento (en Windows), se debe imprimir el directorio actual
        char cwd[1024];
        chdir("/path/to/change/directory/to");
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);
        printf("\n");
    } else {
        if (chdir(arg[1]) != 0) {
          printf("Error: Fallo el comando 'cd'\n");
        }
    }

    return 0;
}

int comando_cls(char **arg) {
    /* Uso:    cls       */

    if(arg[1] != NULL) {  
      printf("Error: El comando 'clear' no acepta ningun argumento.\n");
      return 0;
    }

    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    COORD coordScreen = { 0, 0 };    // Coordenadas nuevas para el cursor 
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi; 
    DWORD dwConSize;

    // Obtener el numero de celdas de caracteres en el buffer actual. 

   if( !GetConsoleScreenBufferInfo( hStdout, &csbi ))
   {
      return 0;
   }

   dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

   // LLenar la pantalla completa con espacios en blanco.

   if( !FillConsoleOutputCharacter( hStdout,         // Elemento 'Handle' para el buffer de pantalla de la consola
                                    (TCHAR) ' ',     // Caracter a escribir en el buffer
                                    dwConSize,       // Numero de celdas a escribir 
                                    coordScreen,     // Coordenadas de la primer celda 
                                    &cCharsWritten ))// Recibir el numero de caracteres escritos
   {
      return 0;
   }

   // Obtener el atributo de texto actual.

   if( !GetConsoleScreenBufferInfo( hStdout, &csbi ))
   {
      return 0;
   }

   // Setear los atributos del buffer.

   if( !FillConsoleOutputAttribute( hStdout,          // Elemento 'Handle' para el buffer de pantalla de la consola
                                    csbi.wAttributes, // Atributos de caracter a usar
                                    dwConSize,        // Numero de celdas a escribir 
                                    coordScreen,      // Coordenadas de la primer celda 
                                    &cCharsWritten )) // Recibir el numero de caracteres escritos
   {
      return 0;
   }

   // Poner el cursor en las coordenadas iniciales

   SetConsoleCursorPosition( hStdout, coordScreen );

   return 0;
}

int comando_copy(char **arg){
  /* Uso:   copy [nombre_archivo] [/destino/nombre_archivo_copia] */

  FILE *fuente,*destino;
  char c;

  if(arg[2]== NULL) {  //cantidad argumentos < 3
    printf("Error: Cantidad insuficiente de argumentos\n");
    return 0;
  }
  fuente=fopen(arg[1],"r"); //Se abre el archivo fuente para lectura
  destino=fopen(arg[2],"w");  //Se abre el archivo destino para escritura
  if(fuente==NULL || destino==NULL) {
    printf("Error: Fallo en el archivo fuente o destino\n");
    return 0;
  }
  while((c=fgetc(fuente))!=EOF) {  //Se lee el archivo fuente caracter a caracter
    fputc(c,destino);  //Se escribe en el archivo destino
  }
  fclose(fuente);
  fclose(destino);  //Se cierran ambos archivos

  return 0;
}

int comando_delfile(char **arg) {
  /* Uso:  delfile [nombre_archivo]     */


  if (arg[1]==NULL) {
    printf("Error: El comando 'delfile' debe tener un argumento\n");
  } else if (arg[2]!=NULL) {
    printf("Error: El comando 'delfile' no puede tener mas de un argumento\n");
  }

  if(remove(arg[1]) == -1)
        perror("Error: No se puedo eliminar el archivo");
  return 0;
}

int comando_dir(char** arg) {
  /**Uso:     dir     
    Limitaciones: -Formato de salida (agrupamiento, orden alfabetico,etc)  
                  -Funciona unicamente sin argumentos                 */
  
    DIR *dir;
    struct dirent *contenidos;     
    dir = opendir ("./");
    char* nombre;

    if (arg[1] != NULL) {
      printf("Error: El comando 'dir' no acepta ningun argumento\n");
      return 0;
    }

    if (dir != NULL)
    {
      while (contenidos = readdir (dir)) {
        nombre= contenidos->d_name;
        if (strcmp(nombre,".")!=0 && strcmp(nombre,"..")!=0) { //Se ignoran las carpetas que comiencen con '.' o '..'
          puts (nombre); 
        }
      }

    (void) closedir (dir);
  }
  else {
     printf("No se pudo abrir el directorio\n"); 
  }
  
    return 0; 
}

int comando_echo(char **arg){
  /*Uso:    echo  [string]    
  Limitaciones: -Esta version si recibe un argumento entre comillas igualmente imprime las comillas
                -No se contempla la funcionalidad del $ */


  if (arg[1]==NULL) { //Si el comando se ejecuta sin argumentos
    printf("ECHO esta activado\n");
  }

  int i;
  int k=0;
  while(arg[++k]){ // Se recorre argumento por argumento 
    for(i=0;i<strlen(arg[k]);i++){ //Se imprime caracter a caracter
       printf("%c",arg[k][i]);
    }
    printf(" ");   // Se imprime el espacio entre los distintos argumentos
  }
  printf("\n"); // Se imprime un salto de linea al final del ultimo argumento

  return 0;
}

int comando_exit(char **arg){
  /*Uso:     exit       */

  if (arg[1] != NULL) {
    printf("Error: El comando 'exit' no acepta ningun argumento\n");
    return 0;
  }

  return 1;
}


int comando_help(char **arg) {
  /*Uso:     help       */

  int i;

  if (arg[1] != NULL) { //se ingresa help con un argumento
    char* comando= arg[1];
    if (strcmp(comando,"cd")==0) {
        printf("Comando 'cd': \n Descripcion: Muestra el nombre del directorio actual o cambia de directorio. \n Uso: cd [/ruta/a/cambiar]\n");
    } else if (strcmp(comando,"cls")==0) {
        printf("Comando 'cls': \n Descripcion: Limpia la pantalla y coloca el prompt al principio de la misma. \n Uso: clear\n");
    } else if (strcmp(comando,"copy")==0) {
        printf("Comando 'copy': \n Descripcion: Copia archivos en el directorio indicado. \n Uso: cp [nombre_archivo] [/destino/nombre_archivo_copia] )\n");
    } else if (strcmp(comando,"echo")==0) {
        printf("Comando 'echo': \n Descripcion: Muestra mensajes o activa y desactiva el eco del comando. \n Uso: echo [string] \n Limitaciones: -Esta version si recibe un argumento entre comillas igualmente imprime las comillas -No se contempla la funcionalidad del $ \n");
    } else if (strcmp(comando,"exit")==0) {
        printf("Comando 'exit': \n Descripcion: Salir de la MiniShell. \n Uso: exit \n");
    } else if (strcmp(comando,"help")==0) {
        printf("Comando 'help': \n Descripcion: Mostrar ayuda. \n Uso: help , help [nombre_comando] \n");
    } else if (strcmp(comando,"delfile")==0) {
        printf("Comando 'delfile': \n Descripcion: Eliminar un archivo determinado. \n Uso: delfile [nombre_archivo] \n");
    } else if (strcmp(comando,"mkdir")==0) {
        printf("Comando 'mkdir': \n Descripcion: Crear directorio si no existe. \n Uso: mkdir [nombre_directorio] \n");
    } else if (strcmp(comando,"dir")==0) {
        printf("Comando 'dir': \n Descripcion: Listar el contenido de un directorio. \n Uso: dir \n Limitaciones: Formato de salida (agrupamiento, orden alfabetico,etc) \n");
    } else {
      printf("Error- El comando ingresado no existe\n");
    }

    return 0;
  }

  printf("\n");
  printf("MiniShell- SO - UNS - 2017\n");
  printf("\n");
  printf("Lista de comandos disponibles:\n");

  for (i = 0; i < cant_comandos(); i++) {
    printf("  %s\n", comandos[i]);
  }

  printf("\nPuede ingresar help [nombre_comando] para ver informacion sobre un comando determinado\n");

  return 0;
}


int comando_mkdir(char **arg) {

  /* Uso:   mkdir [nombre_directorio]   */

    if (arg[2] != NULL) {
      printf("Error: El comando 'mkdir' solo acepta un argumento\n");
      return 0;
    }
    _mkdir(arg[1]);
    
  return 0;
}

void cambiar_color_escritura(char* color) {
  //Cambia color del prompt
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD saved_attributes;

    /* Guardar atributos actuales */
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    saved_attributes = consoleInfo.wAttributes;

    if (strcmp(color,"verde")==0) {
      SetConsoleTextAttribute(hConsole,FOREGROUND_GREEN);
    }
}

void restaurar_color_escritura() {
  //Cambia color de todo lo que no sea el prompt
      HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
      WORD saved_attributes= consoleInfo.wAttributes;
      SetConsoleTextAttribute(hConsole,saved_attributes);
}