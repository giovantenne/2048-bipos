@echo off
@setlocal EnableDelayedExpansion enableextensions
@cls
@if not defined COMPILER @SET COMPILER=GNUArmEmbeddedGCC
@cd %~dp0

:: выделяем наименование папки из пути скрипта - это название эльфа
@SET PROGRAM_NAME=%~dp0
@for /D %%a in ("%PROGRAM_NAME:~0,-1%.txt") do @SET PROGRAM_NAME=%%~na
	
:: Путь к компилятору
@SET BASE_PATH=d:\Dev\AmazfitBip_FW\soft\Patch\GNUArmEmbeddedGCC
@SET LIBRARY_PATH="!BASE_PATH!\arm-none-eabi\lib\thumb\v7e-m+fp\hard"
@SET LD_OPT=-lm -lc
@SET GCC_OPT=-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fno-math-errno 
@SET EABI=arm-none-eabi
@SET COMPILERPATH=%BASE_PATH%\bin
@SET LIB_BIP="..\libbip\libbip.a"
@SET GCC_OPT=%GCC_OPT% -I "..\libbip"
@SET GCC_OPT=%GCC_OPT% -DFW_VERSION=%TERGET_FW_VERSION_GCC% -DLIB_BIP_H=\"..\/libbip\/libbip.h\"
@SET PATH=%PATH%;%COMPILERPATH%
@SET LD_LIBRARY_PATH=%LIBRARY_PATH%

@SET AS=%EABI%-as
@SET LD=%EABI%-ld
@SET OBJCOPY=%EABI%-objcopy
@SET GCC=%EABI%-gcc
@SET NM=%EABI%-nm
@SET GCC_OPT=%GCC_OPT% -c -Os -Wa,-R -Wall -fpie -pie -fpic -mthumb -mlittle-endian  
@SET GCC_OPT=%GCC_OPT% -ffunction-sections -fdata-sections
@SET LD_OPT=%LD_OPT% -L%LIBRARY_PATH% -EL -N -Os --cref 
@SET LD_OPT=%LD_OPT% -pie
@SET LD_OPT=%LD_OPT% --gc-sections 

if exist label.txt (
set /p LABEL=< label.txt
) else (
SET LABEL = %PROGRAM_NAME%
)



@call :echoColor 0F "====================================" 1
@call :echoColor 0F "Наименование проекта: "
@echo %PROGRAM_NAME%
::@call :echoColor 0F "Отображаемое имя: "
::@echo %LABEL%
@call :echoColor 0F "Компилятор: "
@echo %COMPILER%
@call :echoColor 0F "Базовый путь: "
@echo %BASE_PATH%

@call :echoColor 0F "====================================" 1
@echo.	

@call :echoColor 0F "Начинаем сборку..." 1
@SET PARTNAME=%PROGRAM_NAME%
@call :echoColor 0B "Компиляция "
@call :echoColor 0E "%PARTNAME%" 1

@SET n=1
@for  %%f in (*.c) do ( 
@	SET FILES_TO_COMPILE=!FILES_TO_COMPILE! %%~nf.o
@call :EchoN "%n%.	%%~nf.c"
!GCC! !GCC_OPT! -o %%~nf.o %%~nf.c
@if errorlevel 1 goto :error
@call :echoColor 0A "...OK" 1
@SET /a n=n+1)
@SET /a n=n-1
@call :echoColor 0B "Итого: "
@call :echoColor 0E "%n%" 1

@call :echoColor 0B "Сборка..."
%LD% -Map %PARTNAME%.map -o %PROGRAM_NAME%.elf %FILES_TO_COMPILE% %LD_OPT% %LIB_BIP%
@if errorlevel 1 goto :error
::@call :echoColor 0B "."

if exist label.txt (
%OBJCOPY%  %PROGRAM_NAME%.elf --add-section .elf.label=label.txt
)

@call :EchoN "%PROGRAM_NAME%%" > name.txt
%OBJCOPY%  %PROGRAM_NAME%.elf --add-section .elf.name=name.txt
if exist name.txt del name.txt
@if errorlevel 1 goto :error

if exist asset.res (
@echo "Added asset.res"
%OBJCOPY%  %PROGRAM_NAME%.elf --add-section .elf.resources=asset.res
)

if exist settings.bin (
@echo "Added settings.bin"
%OBJCOPY%  %PROGRAM_NAME%.elf --add-section .elf.settings=settings.bin
)
::@call :echoColor 0B "."

@call :echoColor 0A "OK" 1
@call :echoColor 0B "Сборка окончена." 1


:done_

@call :echoColor 0A "Готово." 1 
pause 
@goto :EOF

:error
@call :echoColor 4e ОШИБКА! 1
@endlocal & @SET ERROR=ERROR
@pause
@goto :EOF

::===========================================================
:: A function prints text in first parameter without CRLF 
:EchoN
    
@    <nul set /p strTemp=%~1
@    exit /b 0
::===========================================================
:: A function to convert Decimal to Hexadecimal
:: you need to pass the Decimal as first parameter
:: and return it in the second
:DecToHex
@set LOOKUP=0123456789ABCDEF & set HEXSTR= & set PREFIX=0x

@if "%1" EQU "" set "%2=0"&Goto :exit_

@set /a A=%1 || exit /b 1
@if !A! LSS 0 set /a A=0xfffffff + !A! + 1 & set PREFIX=f
:loop
@set /a B=!A! %% 16 & set /a A=!A! / 16
@set HEXSTR=!LOOKUP:~%B%,1!%HEXSTR%
@if %A% GTR 0 Goto :loop
@set "%2=%PREFIX%%HEXSTR%"
@exit /b 0
::===========================================================
::	Вывод заданной строки заданным цветом
::	3 параметр если не пустой задает перевод строки
::  0 = Черный 	8 = Серый
::  1 = Синий 	9 = Светло-синий
::  2 = Зеленый A = Светло-зеленый
::  3 = Голубой B = Светло-голубой
::  4 = Красный C = Светло-красный
::  5 = Лиловый D = Светло-лиловый
::  6 = Желтый 	E = Светло-желтый
::  7 = Белый 	F = Ярко-белый
:echoColor [Color] [Text] [\n]
 @ if not defined BS for /F "tokens=1 delims=#" %%i in ('"prompt #$H#& echo on& for %%j in (.) do rem"') do set "BS=%%i"
 @ if not exist foo set /p .=.<nul>foo
 @ set "regex=%~2" !
 @ set "regex=%regex:"="%"
 @ findstr /a:%1 /prc:"\." "%regex%\..\foo" nul
 @ set /p .=%BS%%BS%%BS%%BS%%BS%%BS%%BS%%BS%%BS%<nul
 @ if "%3" neq "" echo.
 @exit /b 0
::===========================================================
====================================================
