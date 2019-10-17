SET PATH=C:\Keil_v5\ARM\ARMCC\Bin;C:\ProgramData\Oracle\Java\javapath;C:\Program Files (x86)\Intel\iCLS Client\;C:\Program Files\Intel\iCLS Client\;C:\windows\system32;C:\windows;C:\windows\System32\Wbem;C:\windows\System32\WindowsPowerShell\v1.0\;C:\Program Files (x86)\ATI Technologies\ATI.ACE\Core-Static;C:\ProgramData\Lenovo\ReadyApps;C:\Program Files\TortoiseSVN\bin;C:\Program Files (x86)\AMD\ATI.ACE\Core-Static;C:\Program Files (x86)\Windows Kits\8.1\Windows Performance Toolkit\;C:\Program Files\Microsoft SQL Server\110\Tools\Binn\;C:\Program Files (x86)\Microsoft SDKs\TypeScript\1.0\;C:\Program Files\Git\cmd;C:\Program Files (x86)\ATMEL Corporation\AT91-ISP v1.13\Library\;C:\Program Files (x86)\ATMEL Corporation\AT91-ISP v1.13\sam-ba 2.9;C:\Program Files (x86)\IDM Computer Solutions\UltraEdit\
SET CPU_TYPE=STM32F103ZE
SET CPU_VENDOR=STMicroelectronics
SET UV2_TARGET=cash_provide
SET CPU_CLOCK=0x00B71B00
"C:\Keil_v5\ARM\ARMCC\Bin\ArmAsm" --Via ".\obj\startup_stm32f10x_hd._ia"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\core_cm3.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\system_stm32f10x.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\misc.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\stm32f10x_dma.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\stm32f10x_exti.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\stm32f10x_flash.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\stm32f10x_gpio.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\stm32f10x_i2c.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\stm32f10x_fsmc.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\stm32f10x_tim.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\stm32f10x_usart.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\stm32f10x_wwdg.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\stm32f10x_spi.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\stm32f10x_rcc.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\stm32f10x_dac.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\baud_adapt_lib.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\i2c_eeprom.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\parallel_to_serial.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\pin.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\serial_to_parallel.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\obj\step_motor_basic.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmLink" --Via ".\obj\cash_provide.lnp"
"C:\Keil_v5\ARM\ARMCC\Bin\fromelf.exe" ".\obj\cash_provide.axf" --i32combined --output ".\obj\cash_provide.hex"
fromelf.exe --bin -o ./obj/cash_provide.bin ./obj/cash_provide.axf
