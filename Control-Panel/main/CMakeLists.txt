idf_component_register(
    SRCS "main.c"             
    INCLUDE_DIRS "." "../components/uart/include" "../components/modbus/include"
    REQUIRES nvs_flash
             esp_netif
             esp_event
             modbus
)
