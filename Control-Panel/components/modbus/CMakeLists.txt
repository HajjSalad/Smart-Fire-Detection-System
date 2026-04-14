idf_component_register(
    SRCS "modbus_master.c" "crc_16.c"            
    INCLUDE_DIRS "include" "../uart/include"
    REQUIRES driver freertos uart
)
