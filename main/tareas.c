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

extern uint8_t LED;

extern muestreo_t Datos_muestreo;

extern mensaje_t mensaje_consola;

extern TicTocData * ticTocData;

static const char *TAG = "TAREAS "; // Para los mensajes del micro

void IRAM_ATTR leo_muestras(void *arg)
{
        inicializacion_mpu6050();
        uint8_t cont_pos_lectura;
        uint32_t contador;

        for(contador=0; contador < (CANT_BYTES_LECTURA*MUESTRAS_POR_TABLA); contador++ ) {
                Datos_muestreo.TABLA0[contador] = contador;
                Datos_muestreo.TABLA1[contador] = contador;
        }

        while (1) {

                if( xSemaphore_tomamuestra != NULL )
                {
                        if( xSemaphoreTake( xSemaphore_tomamuestra, portMAX_DELAY) == pdTRUE ) // El semaforo se libera a la frecuencia de muestreo
                        {

                                if (LED == 0) {
                                        gpio_set_level(GPIO_OUTPUT_IO_0, 1);
                                        LED=1;
                                }
                                else {
                                        LED=0;
                                        gpio_set_level(GPIO_OUTPUT_IO_0, 0);
                                }

                                /// ESTO SE DEBE PODER OPTIMIZAR, EL FOR SE DEBE PODER SACAR/////////////////////////////////////////////////////////////////////

                                #ifndef DESACTIVAR_SD
                                lee_mult_registros(ACELEROMETRO_ADDR, REG_1ER_DATO, Datos_muestreo.datos_mpu, CANT_BYTES_LECTURA); // Leo todos los registros de muestreo del acelerometro/giroscopo
                                #endif

                                if (Datos_muestreo.selec_tabla_escritura == 0) {
                                        for (cont_pos_lectura = 0; cont_pos_lectura < CANT_BYTES_LECTURA; cont_pos_lectura++ ) {
                                                Datos_muestreo.TABLA0[cont_pos_lectura + (CANT_BYTES_LECTURA * Datos_muestreo.nro_muestra)] = Datos_muestreo.datos_mpu [cont_pos_lectura]; // Agrega los bytes leidos a la tabla 0
                                        }
                                }
                                else {
                                        for (cont_pos_lectura = 0; cont_pos_lectura < CANT_BYTES_LECTURA; cont_pos_lectura++ ) {
                                                Datos_muestreo.TABLA1[cont_pos_lectura + (CANT_BYTES_LECTURA * Datos_muestreo.nro_muestra)] = Datos_muestreo.datos_mpu [cont_pos_lectura]; // Agrega los bytes leidos a la tabla 1
                                        }
                                }
                                /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                Datos_muestreo.flag_tomar_muestra = false; //Muestra leída
                                Datos_muestreo.nro_muestra++;

                                if (Datos_muestreo.nro_muestra == MUESTRAS_POR_TABLA) { // Si llené una tabla paso a la siguiente, y habilito su almacenamiento.
                                        if (Datos_muestreo.flag_tabla_llena == true) { // Si es true es porque no se grabó la tabla anterior
                                                Datos_muestreo.flag_tabla_perdida = true; // Para dar aviso de la pérdida de información.
                                        }
                                        if (Datos_muestreo.selec_tabla_escritura == 0) {
                                                Datos_muestreo.selec_tabla_escritura = 1; // Paso a escribir en la otra tabla
                                                Datos_muestreo.selec_tabla_lectura = 0; // Indico lectura en la tabla recientemente llena
                                        }
                                        else if (Datos_muestreo.selec_tabla_escritura == 1) {
                                                Datos_muestreo.selec_tabla_escritura = 0; // Paso a escribir en la otra tabla
                                                Datos_muestreo.selec_tabla_lectura = 1; // Indico lectura en la tabla recientemente llena
                                        }
                                        Datos_muestreo.flag_tabla_llena = true;
                                        Datos_muestreo.nro_muestra = 0; // Reinicio el contador de muestras
                                        xSemaphoreGive( xSemaphore_guardatabla ); // Habilito la escritura de la tabla
                                }
                        }
                }
        }
        vTaskDelete(NULL);
}

void IRAM_ATTR guarda_datos(void *arg)
{
        //uint32_t cont_pos_lectura;
        char archivo[40]; // Para guardar el nombre del archivo

        while (1) {
                if( xSemaphore_guardatabla != NULL ) { //Chequea que el semáforo esté inicializado

                        if( xSemaphoreTake( xSemaphore_guardatabla, portMAX_DELAY ) == pdTRUE ) //Si se guardó una tabla de datos se libera el semáforo
                        {
                                // if (LED == 0) {
                                //         gpio_set_level(GPIO_OUTPUT_IO_0, 1);
                                //         LED=1;
                                // }
                                // else {
                                //         LED=0;
                                //         gpio_set_level(GPIO_OUTPUT_IO_0, 0);
                                // }

                                if (Datos_muestreo.nro_tabla == 0) { // Inicio un archivo nuevo
                                        sprintf(archivo, MOUNT_POINT "/M-%d-D-%d.txt", Datos_muestreo.nro_muestreo, Datos_muestreo.nro_archivo );
                                        Datos_muestreo.nro_archivo++; // El proxímo archivo tendrá otro número
                                        //       FILE *f_samples = fopen(archivo, "a");
                                        f_samples = fopen(archivo, "w"); // Abro un archivo nuevo

                                        if (f_samples == NULL) {
                                                ESP_LOGE(TAG, "Failed to open file for writing");
                                                printf("Error al abrir el archivo");
                                        }
                                        else {

              #ifdef ARCHIVOS_CON_ENCABEZADO
                                                // char ticToctimestamp[100];
                                                // int64_t ttTime = ticTocTime(ticTocData);
                                                // microsToTimestamp(ttTime, ticToctimestamp);
                                                char timestamp[64];

                                                //      TicTocData * ticTocData = malloc(sizeof(TicTocData));     /* ALGORITMO DE SINCRONISMO*/
                                                int64_t ttTime = ticTocTime(ticTocData);
                                                microsToTimestamp(ttTime, timestamp);
                                                fprintf(f_samples,"Timestamp: %s)\n", timestamp);

                                                //    free(ticTocData);

              #endif

                                        }

                                }
                                Datos_muestreo.nro_tabla++;  // Aumento contador de tablas guardadas

//fwrite(Datos_muestreo.TABLA0, sizeof(uint8_t), sizeof(Datos_muestreo.TABLA0), f_samples);

                                if (f_samples == NULL) {
                                        ESP_LOGE(TAG, "EL ARCHIVO NO ESTA ABIERTO. ERROR EN LA TARJETA");
                                }
                                else{
                                        if (Datos_muestreo.selec_tabla_lectura == 0) {
                                                fwrite(Datos_muestreo.TABLA0, sizeof(uint8_t), sizeof(Datos_muestreo.TABLA0), f_samples);
                                        }
                                        else {
                                                fwrite(Datos_muestreo.TABLA1, sizeof(uint8_t), sizeof(Datos_muestreo.TABLA1), f_samples);
                                        }
                                        fflush(f_samples); // Vacio el buffer
                                        Datos_muestreo.flag_tabla_llena = false;

                                        if (Datos_muestreo.nro_tabla > (TABLAS_POR_ARCHIVO-1)) { // Porque grabé en el archivo todas las tablas que tenia que guardar
                                                fclose(f_samples);
                                                Datos_muestreo.nro_tabla = 0;  // Reinicio el contador de tablas en archivo
                                        }

                                }

                        }
                }


        } //while(1)
        vTaskDelete(NULL);
}


void muestra_info(void *arg)
{
        while(1) {


                if (mensaje_consola.mensaje_nuevo == true) {
                        mensaje_consola.mensaje_nuevo=false;
                        printf(mensaje_consola.mensaje);
                        Datos_muestreo.flag_muestra_perdida = false;
                }

                if (Datos_muestreo.flag_muestra_perdida == true) {
                        printf("Muestra perdida \n");
                        Datos_muestreo.flag_muestra_perdida = false;
                }
                if (Datos_muestreo.flag_tabla_perdida == true) {
                        printf("Tabla perdida \n");
                        Datos_muestreo.flag_tabla_perdida = false;
                }


#ifdef MUESTRA_DATOS_SINCRONIZACION
                char ticToctimestamp[64];
                char timestamp[64];
                microsToTimestamp(epochInMicros(), timestamp);
                if(!ticTocReady(ticTocData)) {
                        printf("Waiting for ticTocTo be ready. (systemTime: %s)\n", timestamp);
                } else {
                        int64_t ttTime = ticTocTime(ticTocData);
                        microsToTimestamp(ttTime, ticToctimestamp);
                        printf("TicTocTime %s (%lld) (systemTime: %s)\n",ticToctimestamp,  ttTime, timestamp);
                }
#endif

#ifdef MUESTRA_ESTADISTICAS_CPU
                static char cBuffer[ 512 ];
                vTaskGetRunTimeStats( cBuffer );
                printf("Estadisticas: \n");
                printf(cBuffer);
#endif

                printf("Duracion muestreo = %d \n",Datos_muestreo.duracion_muestreo);
                printf("Contador de segundos = %d \n",Datos_muestreo.contador_segundos);




                vTaskDelay(10000 / portTICK_PERIOD_MS);

        }
        vTaskDelete(NULL); // Nunca se va a ejecutar
}
