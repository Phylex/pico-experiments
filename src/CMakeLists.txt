add_library(sd1306
        ssd1306.c
)

target_link_libraries(sd1306
        pico_stdlib
        hardware_i2c
)

# declare the main file as the source for the binary
add_executable(pico_transceiver
        main.c
)

# pull in common dependencies
target_link_libraries(pico_transceiver
        pico_stdlib
        sd1306
        hardware_i2c
        hardware_uart
)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico_transceiver)
