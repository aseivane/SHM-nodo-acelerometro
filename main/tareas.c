/** \file	tareas.c
 *  \brief	Contiene tareas varias del programa
 *  Autor: Ramiro Alonso
 *  Versión: 1
 *	Contiene las funciones de manejo e inicializacion de los GPIOs
 */

#include "main.h"
#include "tareas.h"

extern FILE *f_samples;
extern SemaphoreHandle_t xSemaphore_tomamuestra;
extern SemaphoreHandle_t xSemaphore_guardatabla;

extern bool flag_tomar_muestra;
extern bool flag_muestra_perdida;
extern bool flag_tabla_llena;
extern bool flag_tabla_perdida;
extern uint8_t selec_tabla_escritura;
extern uint8_t selec_tabla_lectura;
extern uint32_t nro_muestra;
extern uint8_t LED;
extern uint8_t nro_tabla;
extern uint32_t nro_archivo;


extern uint8_t TABLA0[LONG_TABLAS];
extern uint8_t TABLA1[LONG_TABLAS];
extern uint8_t datos [CANT_BYTES_LECTURA];  // Lugar donde guardo los datos leidos del mpu


static const char *TAG = "TAREAS"; // Para los mensajes del micro


void leo_muestras(void *arg)
{
    inicializacion_mpu6050();
    uint8_t cont_pos_lectura;
    uint32_t contador;

    for(contador=0;contador < (CANT_BYTES_LECTURA*MUESTRAS_POR_TABLA);  contador++ ){
      TABLA0[contador] = contador;
    }

    while (1) {
          if( xSemaphore_tomamuestra != NULL )
          {
              if( xSemaphoreTake( xSemaphore_tomamuestra, 0) == pdTRUE ) // El semaforo se libera a la frecuencia de muestreo
              {
            /// ESTO SE DEBE PODER OPTIMIZAR, EL FOR SE DEBE PODER SACAR/////////////////////////////////////////////////////////////////////
                  lee_mult_registros(ACELEROMETRO_ADDR, REG_1ER_DATO, datos, CANT_BYTES_LECTURA); // Leo todos los registros de muestreo del acelerometro/giroscopo
                  if (selec_tabla_escritura == 0){
                    for (cont_pos_lectura = 0; cont_pos_lectura < CANT_BYTES_LECTURA; cont_pos_lectura ++ ){
                      TABLA0[cont_pos_lectura + (CANT_BYTES_LECTURA * nro_muestra)] = datos [cont_pos_lectura]; // Agrega los bytes leidos a la tabla 0
                    }
                  }
                  else {
                    for (cont_pos_lectura = 0; cont_pos_lectura < CANT_BYTES_LECTURA; cont_pos_lectura ++ ){
                      TABLA1[cont_pos_lectura + (CANT_BYTES_LECTURA * nro_muestra)] = datos [cont_pos_lectura]; // Agrega los bytes leidos a la tabla 1
                    }
                  }
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                  flag_tomar_muestra = false;  //Muestra leída
                  nro_muestra ++;

                  if (nro_muestra == MUESTRAS_POR_TABLA){  // Si llené una tabla paso a la siguiente, y habilito su almacenamiento.
                    if (flag_tabla_llena == true){ // Si es true es porque no se grabó la tabla anterior
                      flag_tabla_perdida = true; // Para dar aviso de la pérdida de información.
                      }
                      if (selec_tabla_escritura == 0){
                        selec_tabla_escritura = 1;  // Paso a escribir en la otra tabla
                        selec_tabla_lectura = 0;    // Indico lectura en la tabla recientemente llena
                        }
                      else if (selec_tabla_escritura == 1){
                        selec_tabla_escritura = 0;  // Paso a escribir en la otra tabla
                        selec_tabla_lectura = 1;    // Indico lectura en la tabla recientemente llena
                        }
                    flag_tabla_llena = true;
                    nro_muestra = 0; // Reinicio el contador de muestras
                    xSemaphoreGive( xSemaphore_guardatabla ); // Habilito la escritura de la tabla
                    }
              }
          }
    }
    vTaskDelete(NULL);
}

void guarda_datos(void *arg)
{
  //uint32_t cont_pos_lectura;
  char archivo[40];  // Para guardar el nombre del archivo

  while (1){

    if( xSemaphore_guardatabla != NULL ) { //Chequea que el semáforo esté inicializado

        if( xSemaphoreTake( xSemaphore_guardatabla, 0 ) == pdTRUE ) //Si se guardó una tabla de datos se libera el semáforo
          {

            if (LED == 0){
                gpio_set_level(GPIO_OUTPUT_IO_0, 1);
                LED=1;
            }
            else {
                LED=0;
                gpio_set_level(GPIO_OUTPUT_IO_0, 0);
            }

          if (nro_tabla == 0){ // Inicio un archivo nuevo
            sprintf(archivo, MOUNT_POINT"/datos%d.txt", nro_archivo );
            nro_archivo++; // El proxímo archivo tendrá otro número
            //       FILE *f_samples = fopen(archivo, "a");
            f_samples = fopen(archivo, "w");  // Abro un archivo nuevo
            if (f_samples == NULL) {
                ESP_LOGE(TAG, "Failed to open file for writing");
                printf("Error al abrir el archivo");
            }
          }
          nro_tabla ++;

//fwrite(TABLA0, sizeof(uint8_t), sizeof(TABLA0), f_samples);


              if (selec_tabla_lectura == 0){
                   fwrite(TABLA0, sizeof(uint8_t), sizeof(TABLA0), f_samples);
   //                printf ("Tabla 0, Guardada. %s \n", archivo);
              }
              if (selec_tabla_lectura == 1){
                   fwrite(TABLA1, sizeof(uint8_t), sizeof(TABLA1), f_samples);
   //                printf ("Tabla 1, Guardada. %s \n", archivo);
                }

//printf("FIN fwrite(): \n");

//              fsync(f_samples);
             fflush(f_samples);
             flag_tabla_llena = false;

           if (nro_tabla > (TABLAS_POR_ARCHIVO-1)){ // Porque llené el archivo
             fclose(f_samples);
             nro_tabla = 0;
           }

        }
    }


  } //while(1)
  vTaskDelete(NULL);
}


void muestra_info(void *arg)
{
  while(1){
    if (flag_muestra_perdida == true){
      printf("Muestra perdida \n");
      flag_muestra_perdida = false;
    }
    if (flag_tabla_perdida == true){
      printf("Tabla perdida \n");
      flag_tabla_perdida = false;
    }
    vTaskDelay(1);

  }
  vTaskDelete(NULL); // Nunca se va a ejecutar
}
