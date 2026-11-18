@echo off
REM ==========================================================================
REM  Solo si se perdio o se rompio vivado\: recrea el proyecto desde la
REM  plantilla con los scripts, genera el bitstream y exporta hw\P2_SEP.xsa.
REM  Ojo: descarta los cambios hechos por GUI que no esten en los scripts.
REM  Tarda 10-15 minutos.
REM ==========================================================================
setlocal
set "XIL="
for %%D in (C:\Xilinx D:\Xilinx E:\Xilinx) do if not defined XIL if exist "%%D\Vivado\2020.1\bin\vivado.bat" set "XIL=%%D"
if not defined XIL (
    echo No se encontro Vivado 2020.1 en C:\Xilinx, D:\Xilinx ni E:\Xilinx.
    pause
    exit /b 1
)
if exist "%~dp0vivado" (
    echo Ya existe vivado\. Para regenerar desde cero, borre esa carpeta primero.
    pause
    exit /b 1
)
if not exist "%~dp0logs" mkdir "%~dp0logs"
cd /d "%~dp0logs"
call "%XIL%\Vivado\2020.1\bin\vivado.bat" -mode batch -source "%~dp0scripts\01_crear_proyecto.tcl" || goto :fallo
call "%XIL%\Vivado\2020.1\bin\vivado.bat" -mode batch -source "%~dp0scripts\03_build.tcl" || goto :fallo
echo.
echo Listo: vivado\P2_SEP.xpr y hw\P2_SEP.xsa
pause
exit /b 0
:fallo
echo.
echo Fallo. Revise los .log en la carpeta logs\.
pause
exit /b 1
