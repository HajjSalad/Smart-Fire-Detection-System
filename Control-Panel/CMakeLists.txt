# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_spi_stm32)

target_add_binary_data(esp32_spi_stm32.elf "main/device_cert.pem" TEXT)
target_add_binary_data(esp32_spi_stm32.elf "main/private_key.pem" TEXT)
target_add_binary_data(esp32_spi_stm32.elf "main/root_ca.pem" TEXT)