# FLOSS as the Bluetooth host stack instead of BTstack
hoja_use_floss(${PROJECT_NAME})

# Test build: report timing diagnostics in SInput reports, report interval stepping 8/4/2/1 ms,
# and every radio bus transaction counted and timed
target_compile_definitions(${PROJECT_NAME} PRIVATE HOJA_BT_FLOSS_DIAG=1)
target_link_options(${PROJECT_NAME} PRIVATE
    -Wl,--wrap=cyw43_read_bytes -Wl,--wrap=cyw43_write_bytes
    -Wl,--wrap=cyw43_read_reg_u32 -Wl,--wrap=cyw43_read_reg_u16 -Wl,--wrap=cyw43_read_reg_u8
    -Wl,--wrap=cyw43_write_reg_u32 -Wl,--wrap=cyw43_write_reg_u16 -Wl,--wrap=cyw43_write_reg_u8)
