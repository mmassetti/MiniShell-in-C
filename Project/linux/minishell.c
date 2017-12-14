#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>  //Usada por comando_ls, comando_pwd, comando_cd
#include <fcntl.h> //Usada por comando_ls

#include <dirent.h> //Usada por comando_ls
#include <sys/stat.h>  //Usada por comando_ls, comando_mkdir

#include <fts.h> //Usada por comando_rmdir
#include <sys/types.h> //Usada por comando_rmdir
#include <sys/stat.h> //Usada por comando_rmdir
#include <errno.h> //Usada por comando_rmdir

// Variables usadas en la lectura y parseo
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

// Codigos de colores 
#define RESET_COLOR "\e[m" 
#define COLOR_VERDE "\e[32m" 
#define COLOR_AMARILLO "\e[33m"
#define COLOR_AZUL "\e[36m" 
  
// Auxiliar para comando_ls
static int filter(const struct dirent *unused);

// Prototipos de los procedimientos
char **separar_linea(char *linea);
int ejecutar(char **arg);
int cant_comandos();
void mostrar_man(char* comando);

int comando_cd(char **arg);
int comando_clear(char **arg);
int comando_cp(char **arg);
int comando_echo(char **arg);
int comando_exit(char **arg);
int comando_help(char **arg);
int comando_ls(char **arg);
int comando_man(char **arg);
int comando_mkdir(char **arg);
int comando_pwd(char **arg);
int comando_rmdir(char **arg);
int comando_rmfile(char **arg);


int (*realizar_comando[]) (char **) = { 
  &comando_cd,
  &comando_clear,
  &comando_cp, 
  &comando_echo,
  &comando_exit,
  &comando_help,
  &comando_ls,
  &comando_man,
  &comando_mkdir,
  &comando_pwd,
  &comando_rmdir,
  &comando_rmfile
}; 

char *comandos[] = {
  "cd",  
  "clear",  
  "cp",  
  "echo",  
  "exit", 
  "help",
  "ls",   
  "man",
  "mkdir", 
  "pwd",
  "rmdir",
  "rmfile"   
};


int main (int argc, char ** argv) {

  int finalizar = 0;
  char *prompt = COLOR_VERDE "miniShell> " RESET_COLOR;
  char cmd[1024]; //Se asume que la linea de input no superara los 1024 bytes

  while (!finalizar) { //Mientras no se ejecute el comando exit 
    char *cursor;
    char ultimo_char;
    int ret; 
    int cont;


    // Imprimir el prompt
    ret = write(1, prompt, strlen(prompt));
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
    
    if(cmd[0]!='\n') { //Si no es enter
      //Ejecutar el comando requerido
      char **args= separar_linea(cmd); //Se obtiene el nombre del comando y sus argumentos
      finalizar= ejecutar(args); //Se ejecuta el comando y se guarda el retorno para ver si hay que continuar o no
      free(args);  //Se libera el espacio para args
    }
  
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

    printf("Error: Comando no encontrado.\n"); //Si llegamos aca quiere decir que el comando no existe

    return 0; //retornar 0 implica que seguira la ejecucion de la mini shell.

}

int cant_comandos() {
  return sizeof(comandos) / sizeof(char *);
}
/////////////////////////////
//////Lista de comandos/////
///////////////////////////

int comando_cd(char **arg) {
 /* Uso:    cd [/ruta/a/cambiar/]      
    Limitaciones: -Funciona unicamente si recibe un argumento.  */

    if (arg[1] == NULL) {
        printf("Error: Cantidad insuficiente de argumentos\n"); 
    } else {
        if (chdir(arg[1]) != 0) {
          printf("Error: Fallo el comando 'cd'\n");
        }
    }

    return 0;
}

int comando_clear(char **arg) {
    /* Uso:    clear       
       Limitaciones:  -Solo limpia la consola en parte. (Es una version parcial del clear) */

    if(arg[1] != NULL) {  
      printf("Error: El comando 'clear' no acepta ningun argumento.\n");
      return 0;
    }

    /* 033 es el octal del ESC, 2J es para limpiar la consola y mover el cursor hacia arriba a la izquierda
       1;1H mueve el cursor a la fila 1 columna 1 */

     printf("\033[2J\033[1;1H");
     fflush(stdout);
    
     return 0;
}

int comando_cp(char **arg){
  /* Uso:   cp [nombre_archivo] [/destino/nombre_archivo_copia] */

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

int comando_echo(char **arg){
  /*Uso:    echo  [string]    
  Limitaciones: -Esta version si recibe un argumento entre comillas igualmente imprime las comillas
                -No se contempla la funcionalidad del $ */

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

  if (arg[1] != NULL) {
    printf("Error: El comando 'help' no acepta ningun argumento\n");
    return 0;
  }

  printf("\n");
  printf("MiniShell- SO - UNS - 2017\n");
  printf("\n");
  printf("Lista de comandos disponibles:\n");

  for (i = 0; i < cant_comandos(); i++) {
    printf("  %s\n", comandos[i]);
  }

  return 0;
}

int comando_ls(char** arg) {
    /*Uso:     ls      
    Limitaciones: -Formato de salida (agrupamiento, orden alfabetico,etc)  */
                          
    struct dirent **contenidos;
    int cant_contenidos;

    if(arg[1]==NULL){ //el comando es ls , sin argumentos
        if((cant_contenidos = scandir("./", &contenidos, filter, alphasort))<0) {
          printf("Error: Fallo en el comando 'ls' \n");
        }
    }
    else if(arg[2]==NULL){  //el comando es ls + un argumento
        if((cant_contenidos = scandir(arg[1], &contenidos, filter, alphasort))<0){
          printf("Error: Fallo en el comando 'ls' \n");
        }
    }
    
    int i;
    for(i=0; i<cant_contenidos; i++){ //Se recorren las carpetas 
        char* nombre= contenidos[i]->d_name;
        if (strcmp(nombre,".")!=0 && strcmp(nombre,"..")!=0) {  //Se ignoran las carpetas que comiencen con '.' o '..'
          // Se hace un chequeo para ver si el archivo es ejecutable 
            if(!access(nombre,X_OK)) 
            { 
                int fd = -1; 
                struct stat st; 
  
                fd = open(nombre, O_RDONLY, 0); 
                if(-1 == fd) 
                { 
                    printf("\n Error: Fallo al abrir el archivo/directorio\n"); 
                    return -1; 
                } 
                 
                fstat(fd, &st); 
                // Se chequea si el directorio tiene permisos de ejecucion 
                if(S_ISDIR(st.st_mode)) 
                { 
                    // Si era un directorio, se imprime en color azul 
                    printf(COLOR_AZUL"%s     "RESET_COLOR,nombre); 
                } 
                else 
                {                                   
                    // Si era un archivo ejecutable, se imprime en color verde 
                    printf(COLOR_AMARILLO"%s     "RESET_COLOR,nombre); 
                } 
                close(fd); 
            } 
            else 
            { 
                // Si el flag de archivo 'No executable' esta encendido, se imprime en el color default
                printf("%s     ",nombre); 
            } 
        } 
        
    }
    printf("\n"); 
    
    return 0; 

}

/* Metodo auxiliar para comando_ls */
/* Una funcion necesaria para scandir. Solo retorna 1. Si se quisiera listar solamente
   ciertas cosas habria que editar esta funcion */
static int filter(const struct dirent *unused){
    return 1;
}

int comando_man(char **arg) {
  /* Uso :        man
                  man [nombre_comando]           */

  if (arg[1]==NULL) {   //Se ejecuta 'man', sin argumentos
      printf("Puede buscar informacion sobre los comandos escribiendo: \n");
      char* str = "  man [nombre_comando]\n";
      printf(COLOR_AMARILLO"%s  "RESET_COLOR,str);
      str= "Ejemplo:    man ls\n";
      printf(COLOR_AZUL"%s  "RESET_COLOR,str);
      printf("\n");
      return 0;
  } else if (arg[2]!=NULL) {  //Se ejecuta 'man' con dos o mas argumentos
      printf("Error: El comando 'man' acepta un argumento como maximo\n");
      return 0;
  } else { //Se ejecuta 'man' con un argumento
      char* nombre_comando = arg[1];
      mostrar_man(nombre_comando);
  }

  return 0;
}

void mostrar_man(char* comando) {
    if (strcmp(comando,"cd")==0) {
        printf("Comando 'cd': \n Descripcion: Cambia de directorio. \n Uso: cd [/ruta/a/cambiar]\n Limitaciones: Funciona unicamente si recibe un argumento\n");
    } else if (strcmp(comando,"clear")==0) {
        printf("Comando 'clear': \n Descripcion: Limpia la pantalla y coloca el prompt al principio de la misma. \n Uso: clear\n Limitaciones: Solo limpia la pantala en parte. (Es una version parcial del clear)\n");
    } else if (strcmp(comando,"cp")==0) {
        printf("Comando 'cp': \n Descripcion: Copia archivos en el directorio indicado. \n Uso: cp [nombre_archivo] [/destino/nombre_archivo_copia] )\n");
    } else if (strcmp(comando,"echo")==0) {
        printf("Comando 'echo': \n Descripcion: Mostrar una linea de texto/string en standard output. \n Uso: echo [string] \n Limitaciones: -Esta version si recibe un argumento entre comillas igualmente imprime las comillas -No se contempla la funcionalidad del $ \n");
    } else if (strcmp(comando,"exit")==0) {
        printf("Comando 'exit': \n Descripcion: Salir de la MiniShell. \n Uso: exit \n");
    } else if (strcmp(comando,"help")==0) {
        printf("Comando 'help': \n Descripcion: Mostrar ayuda. \n Uso: help \n");
    } else if (strcmp(comando,"ls")==0) {
        printf("Comando 'ls': \n Descripcion: Listar el contenido de un directorio. \n Uso: ls \n Limitaciones: Formato de salida (agrupamiento, orden alfabetico,etc) \n");
    } else if (strcmp(comando,"man")==0) {
        printf("Comando 'man': \n Descripcion: Mostrar un breve manual de cada comando. \n Uso: man [nombre_comando] \n");
    } else if (strcmp(comando,"mkdir")==0) {
        printf("Comando 'mkdir': \n Descripcion: Crear directorio si no existe. \n Uso: mkdir [nombre_directorio] \n");
    } else if (strcmp(comando,"pwd")==0) {
        printf("Comando 'pwd': \n Descripcion: Imprimir el directorio de trabajo actual. \n Uso: pwd \n");
    } else if (strcmp(comando,"rmdir")==0) {
        printf("Comando 'rmdir': \n Descripcion: Eliminar un directorio y los archivos que contenga. \n Uso: rmdir [nombre_directorio] \n");
    } else if (strcmp(comando,"rmfile")==0) {
        printf("Comando 'rmfile': \n Descripcion: Eliminar un archivo determinado. \n Uso: rmfile [nombre_archivo] \n");
    } else {
      printf("Error- El comando ingresado no existe\n");
    }
}

int comando_mkdir(char **arg) {

  /* Uso:   mkdir [nombre_directorio]   */

    if (arg[2] != NULL) {
      printf("Error: El comando 'mkdir' solo acepta un argumento\n");
      return 0;
    }

    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",arg[1]);
    len = strlen(tmp);
    if(tmp[len - 1] == '/') {
       tmp[len - 1] = 0;
    }
    for(p = tmp + 1; *p; p++) {
      if(*p == '/') {
       *p = 0;
        mkdir(tmp, S_IRWXU);
        *p = '/';
      }
    }
    
    mkdir(tmp, S_IRWXU);

  return 0;
}

int comando_pwd(char **arg){
    /*Uso:     pwd       */

    if(arg[1] != NULL) { 
      printf("Error: El comando 'pwd' no acepta ningun argumento.\n");
      return 0;
    }
    char cwd[1024];
    chdir("/path/to/change/directory/to");
    getcwd(cwd, sizeof(cwd));
    printf("Directorio de trabajo actual: %s\n", cwd);
    return 0;
}

int comando_rmdir(char **arg) {
  /* Uso:     rmdir [nombre_directorio]     */

    char* dir = arg[1];
    int ret = 0;
    FTS *ftsp = NULL;
    FTSENT *curr;

    if (arg[1]==NULL) {
        printf("Error: El comando 'rmdir' debe tener un argumento\n");
    } else if (arg[2]!=NULL) {
        printf("Error: El comando 'rmdir' no puede tener mas de un argumento\n");
    }

    /* Casting necesario en C porque fts_open() toma un "char * const *, en vez de "const char * const *", el 
    cual solamente esta permitido en C++. La llamada a fts_open() no modifica al argumento */
    char *files[] = { (char *) dir, NULL };

    // FTS_NOCHDIR  - Evitar cambiar el cwd,lo que podria causar un comportamiento inesperado en multithreading
    // FTS_PHYSICAL - Previene eliminar archivos por fuera del directorio especificado
    // FTS_XDEV     - No cruzar los limites del sistema de archivos
    ftsp = fts_open(files, FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV, NULL);
    if (!ftsp) {
        fprintf(stderr, "%s: Fallo en fts_open: %s\n", dir, strerror(errno));
        ret = -1;
        goto finish;
    }

    while ((curr = fts_read(ftsp))) {
        switch (curr->fts_info) {
        case FTS_NS:
        case FTS_DNR:
        case FTS_ERR:
            fprintf(stderr, "%s: Fallo en fts_read: %s\n",
                    curr->fts_accpath, strerror(curr->fts_errno));
            break;

        case FTS_DC:
        case FTS_DOT:
        case FTS_NSOK:
            // Codigo no alcanzado a menos que FTS_LOGICAL, FTS_SEEDOT, o FTS_NOSTAT hayan sido 
            //pasados a fts_open()
            break;

        case FTS_D:
            // No hacer nada. Se necesita busqueda depth-first, entonces los directorios son
            // eliminados en FTS_DP
            break;

        case FTS_DP:
        case FTS_F:
        case FTS_SL:
        case FTS_SLNONE:
        case FTS_DEFAULT:
            if (remove(curr->fts_accpath) < 0) {
                fprintf(stderr, "%s: Error: Fallo el comando 'rmdir': %s\n",
                        curr->fts_path, strerror(errno));
                ret = -1;
            }
            break;
        }
    }

      finish:
          if (ftsp) {
              fts_close(ftsp);
          }

    return ret;
      
}

int comando_rmfile(char **arg) {

  /* Uso:  rmfile [nombre_archivo]     */

  if (arg[1]==NULL) {
    printf("Error: El comando 'rmfile' debe tener un argumento\n");
  } else if (arg[2]!=NULL) {
    printf("Error: El comando 'rmfile' no puede tener mas de un argumento\n");
  }

  if(remove(arg[1]) == -1)
        perror("Error: No se puedo eliminar el archivo");
  return 0;
}
