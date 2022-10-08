idf_component_register(SRCS "main.c" "wifi.c" "tictoc/microtime.c"  "tictoc/daemon.c" "tictoc/sic.c" "tictoc/linearfit.c" "tictoc/circularOrderedArray.c" "tictoc/halfSampleMode.c"
                       "funciones_nodo/tareas.c" "funciones_nodo/file_server.c" "funciones_nodo/acelerometroI2C.c" "funciones_nodo/GPIO.c" "funciones_nodo/timer_muestreo.c" "funciones_nodo/sd_card.c" "funciones_nodo/mqtt.c" 
                    INCLUDE_DIRS "tictoc" "funciones_nodo"
                    EMBED_FILES "funciones_nodo/favicon.ico" "funciones_nodo/upload_script.html")
