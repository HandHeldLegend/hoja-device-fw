# Define the path for .gitignore
set(GITIGNORE_FILE "${CMAKE_SOURCE_DIR}/.gitignore")

# Create or update .gitignore
if(NOT EXISTS ${GITIGNORE_FILE})
    # Create new .gitignore with common patterns
    file(WRITE ${GITIGNORE_FILE} "# Build directories\n")
    file(APPEND ${GITIGNORE_FILE} "build/\n")
    file(APPEND ${GITIGNORE_FILE} "lib/\n")
    file(APPEND ${GITIGNORE_FILE} "out/\n")
    file(APPEND ${GITIGNORE_FILE} "*/build/\n")
    file(APPEND ${GITIGNORE_FILE} "*/lib/\n")
    file(APPEND ${GITIGNORE_FILE} "*/out/\n\n")
    
    file(APPEND ${GITIGNORE_FILE} "# IDE specific files\n")
    file(APPEND ${GITIGNORE_FILE} ".vscode/\n")
    file(APPEND ${GITIGNORE_FILE} ".idea/\n")
    file(APPEND ${GITIGNORE_FILE} "*.swp\n")
    file(APPEND ${GITIGNORE_FILE} "*.swo\n\n")
    
    file(APPEND ${GITIGNORE_FILE} "# Operating System specific files\n")
    file(APPEND ${GITIGNORE_FILE} ".DS_Store\n")
    file(APPEND ${GITIGNORE_FILE} "Thumbs.db\n\n")

    message(STATUS "Created new .gitignore file at: ${GITIGNORE_FILE}")
else()
    # Check if build/ and lib/ patterns exist in .gitignore
    file(READ ${GITIGNORE_FILE} GITIGNORE_CONTENT)
    set(PATTERNS_TO_ADD "")
    
    if(NOT "${GITIGNORE_CONTENT}" MATCHES "build/")
        set(PATTERNS_TO_ADD "${PATTERNS_TO_ADD}build/\n")
    endif()
    
    if(NOT "${GITIGNORE_CONTENT}" MATCHES "lib/")
        set(PATTERNS_TO_ADD "${PATTERNS_TO_ADD}lib/\n")
    endif()
    
    if(NOT "${PATTERNS_TO_ADD}" STREQUAL "")
        file(APPEND ${GITIGNORE_FILE} "\n# Added by build script\n${PATTERNS_TO_ADD}")
        message(STATUS "Updated existing .gitignore with build/ and lib/ patterns")
    endif()
endif()

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR not provided. Please specify -DSOURCE_DIR=/path/to/source")
endif()

# Define required parameters with validation
if(NOT DEFINED UF2_SOURCE)
    message(FATAL_ERROR "UF2_SOURCE not provided. Please specify -DUF2_SOURCE=/path/to/source.uf2")
endif()

if(NOT DEFINED UF2_TARGET)
    message(FATAL_ERROR "UF2_TARGET not provided. Please specify -DUF2_TARGET=/path/to/target/directory")
endif()

if(NOT DEFINED BIN_SOURCE)
    message(FATAL_ERROR "BIN_SOURCE not provided. Please specify -DBIN_SOURCE=/path/to/source.bin")
endif()

if(NOT DEFINED BIN_TARGET)
    message(FATAL_ERROR "BIN_TARGET not provided. Please specify -DBIN_TARGET=/path/to/target/directory")
endif()

if(NOT DEFINED MANIFEST_TARGET)
    message(FATAL_ERROR "MANIFEST_TARGET not provided. Please specify -DMANIFEST_TARGET=/path/to/target/directory")
endif()

# Verify source files exist
if(NOT EXISTS ${UF2_SOURCE})
    message(FATAL_ERROR "The specified UF2 source file does not exist: ${UF2_SOURCE}")
endif()

if(NOT EXISTS ${BIN_SOURCE})
    message(FATAL_ERROR "The specified BIN source file does not exist: ${BIN_SOURCE}")
endif()

# Create target directories if they don't exist
file(MAKE_DIRECTORY ${UF2_TARGET})
file(MAKE_DIRECTORY ${BIN_TARGET})
file(MAKE_DIRECTORY ${MANIFEST_TARGET})

# Get file names without paths. TARGET_NAME (optional) publishes under another name, for devices
# still pointing at a board's old name.
if(DEFINED TARGET_NAME)
    set(UF2_FILENAME "${TARGET_NAME}.uf2")
    set(BIN_FILENAME "${TARGET_NAME}.bin")
else()
    get_filename_component(UF2_FILENAME ${UF2_SOURCE} NAME)
    get_filename_component(BIN_FILENAME ${BIN_SOURCE} NAME)
endif()

# Set full target paths
set(UF2_TARGET_FILE "${UF2_TARGET}/${UF2_FILENAME}")
set(BIN_TARGET_FILE "${BIN_TARGET}/${BIN_FILENAME}")
set(MANIFEST_JSON "${MANIFEST_TARGET}/manifest.json")

# Copy files to target locations
configure_file(${UF2_SOURCE} ${UF2_TARGET_FILE} COPYONLY)
configure_file(${BIN_SOURCE} ${BIN_TARGET_FILE} COPYONLY)

# Calculate SHA256 checksum of the BIN file
file(SHA256 ${BIN_TARGET_FILE} BIN_CHECKSUM)

# Create manifest.json with the new data
set(MANIFEST_CONTENT [=[{
  "changelog": "",
  "fw_version": %VERSION%,
  "checksum": "%CHECKSUM%"
}]=])

# Read timestamp from timestamp.h
file(READ "${SOURCE_DIR}/timestamp.h" TIMESTAMP_CONTENT)
string(REGEX MATCH "BUILD_TIMESTAMP \\(\\(uint32_t\\)([0-9]+)\\)" _ ${TIMESTAMP_CONTENT})
set(TIMESTAMP "${CMAKE_MATCH_1}")

# Replace VERSION and CHECKSUM in manifest
string(REPLACE "%VERSION%" "${TIMESTAMP}" MANIFEST_CONTENT "${MANIFEST_CONTENT}")
string(REPLACE "%CHECKSUM%" "${BIN_CHECKSUM}" MANIFEST_CONTENT "${MANIFEST_CONTENT}")

# Write manifest.json
file(WRITE ${MANIFEST_JSON} "${MANIFEST_CONTENT}")

# Output status
message(STATUS "Files processed successfully:")
message(STATUS "UF2 copied to: ${UF2_TARGET_FILE}")
message(STATUS "BIN copied to: ${BIN_TARGET_FILE}")
message(STATUS "Manifest created at: ${MANIFEST_JSON}")
message(STATUS "Generated version: ${TIMESTAMP_CONTENT}")
message(STATUS "BIN checksum: ${BIN_CHECKSUM}")
