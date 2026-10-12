# FLOSS as the Bluetooth host stack instead of BTstack
hoja_use_floss(${PROJECT_NAME})

# Test build: SInput at 500 Hz over Bluetooth
target_compile_definitions(${PROJECT_NAME} PRIVATE BT_HAL_SINPUT_POLLING_RATE_MS=2)
