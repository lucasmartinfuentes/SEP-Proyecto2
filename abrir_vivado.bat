@echo off
REM ==========================================================================
REM  Abre el proyecto de Vivado (vivado\P2_SEP.xpr) en la GUI. Doble clic.
REM  No cerrar la ventana negra que aparece: es la consola de Vivado.
REM ==========================================================================
setlocal
set "XIL="
for %%D in (C:\Xilinx D:\Xilinx E:\Xilinx) do if not defined XIL if exist "%%D\Vivado\2020.1\bin\vivado.bat" set "XIL=%%D"
if not defined XIL (
    echo No se encontro Vivado 2020.1 en C:\Xilinx, D:\Xilinx ni E:\Xilinx.
    pause
    exit /b 1
)
if not exist "%~dp0vivado\P2_SEP.xpr" (
    echo No existe vivado\P2_SEP.xpr. Use regenerar_vivado.bat para crearlo desde cero.
    pause
    exit /b 1
)
cd /d "%~dp0logs" 2>nul || (mkdir "%~dp0logs" & cd /d "%~dp0logs")
call "%XIL%\Vivado\2020.1\bin\vivado.bat" "%~dp0vivado\P2_SEP.xpr"
