import os
import re
import sys

# Path to the HAL SPI header file (modify this path as needed)

if(len(sys.argv) < 2):
    print(f"Program requires root source folder path")
    exit(1)

hal_spi_header_path = sys.argv[1] + "/Drivers/STM32H5xx_HAL_Driver/Inc/stm32h5xx_hal_uart.h"

# Check if the file exists
if not os.path.exists(hal_spi_header_path):
    print(f"Error: File '{hal_spi_header_path}' not found.")
    exit(1)

# Read the content of the file
with open(hal_spi_header_path, "r") as f:
    content = f.readlines()

# Flag to check if UserData exists
user_data_exists = False
# Flag to check if we are inside the SPI_HandleTypeDef structure
inside_spi_handle = False
# Line number where SPI_HandleTypeDef starts
spi_handle_start = -1

# Regular expression to match the SPI_HandleTypeDef structure
spi_handle_regex = re.compile(r"^typedef\s+struct\s+__UART_HandleTypeDef\s*$\n")

# Iterate through the file content
for i, line in enumerate(content):
    # Check if we are inside the SPI_HandleTypeDef structure
    if spi_handle_regex.match(line):
        inside_spi_handle = True
        spi_handle_start = i
        continue

    # Check if we are inside the structure and find the UserData variable
    if inside_spi_handle:
        if "void* UserData;" in line:
            user_data_exists = True
            break
        # Check for the end of the structure
        if "}" in line and not line.strip().startswith("/*"):
            inside_spi_handle = False

# If UserData does not exist, add it
if not user_data_exists and spi_handle_start != -1:
    print("UserData not found in UART_HandleTypeDef. Adding it...")
    # Find the line to insert UserData (before the end of the structure)
    for i in range(spi_handle_start, len(content)):
        if "}" in content[i] and not content[i].strip().startswith("/*"):
            # Insert UserData before the closing brace
            content.insert(i, "  void* UserData;  /* User-defined data */\n")
            break

    # Write the modified content back to the file
    with open(hal_spi_header_path, "w") as f:
        f.writelines(content)
    print("UserData added successfully.")
else:
    if(spi_handle_start == -1):
        print("SPI_HandleTypeDef start not found. No changes made.")
    else:
        print("UserData already exists in SPI_HandleTypeDef. No changes made.")