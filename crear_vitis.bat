@echo off
REM ==========================================================================
REM  Crea el workspace de Vitis (vitis\) desde hw\P2_SEP.xsa y compila la app
REM  con el codigo de vitis_src\. Doble clic. Tarda unos 3-5 minutos.
REM  Despues: abrir Vitis 2020.1 con el workspace <esta carpeta>\vitis.
REM ==========================================================================
setlocal
set "XIL="
for %%D in (C:\Xilinx D:\Xilinx E:\Xilinx) do if not defined XIL if exist "%%D\Vitis\2020.1\bin\xsct.bat" set "XIL=%%D"
if not defined XIL (
    echo No se encontro Vitis 2020.1 en C:\Xilinx, D:\Xilinx ni E:\Xilinx.
    pause
    exit /b 1
)
if exist "%~dp0vitis" (
    echo Ya existe la carpeta vitis\. Borrela si quiere crear el workspace de nuevo.
    pause
    exit /b 1
)
if not exist "%~dp0logs" mkdir "%~dp0logs"
cd /d "%~dp0logs"
call "%XIL%\Vitis\2020.1\bin\xsct.bat" "%~dp0scripts\04_vitis.tcl"
echo.
echo Listo. Abra Vitis 2020.1 y elija como workspace: %~dp0vitis
pause
