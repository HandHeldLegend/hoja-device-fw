# FLOSS as the Bluetooth host stack instead of BTstack
hoja_use_floss(${PROJECT_NAME})

# Test build: SInput talks to a FLOSS link dongle (Pico W) instead of the PC adapter
target_compile_definitions(${PROJECT_NAME} PRIVATE HOJA_BT_FLOSS_LINK_TEST=1)
