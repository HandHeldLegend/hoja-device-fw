# FLOSS as the Bluetooth host stack instead of BTstack
set_target_properties(${PROJECT_NAME} PROPERTIES HOJA_BT_FLOSS ON)

# Test build: report timing diagnostics in SInput reports, report interval stepping 8/4/2/1 ms
target_compile_definitions(${PROJECT_NAME} PRIVATE HOJA_BT_FLOSS_DIAG=1)
