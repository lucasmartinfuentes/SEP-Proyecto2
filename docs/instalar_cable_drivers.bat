@echo off
REM ==========================================================================
REM  Instala los drivers USB-JTAG (Digilent/Xilinx) para programar la Zybo.
REM  Es el UNICO paso de la instalacion que necesita permisos de administrador.
REM  Uso: doble clic -> Windows pregunta "Permitir cambios?" -> Si.
REM ==========================================================================
net session >nul 2>&1
if %errorlevel% neq 0 (
    powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    exit /b
)
cd /d D:\Xilinx\Vivado\2020.1\data\xicom\cable_drivers\nt64
call install_drivers_wrapper.bat -log_filename D:\Xilinx\Vivado\2020.1\cable_drivers_install.log
echo.
echo Listo. Revise D:\Xilinx\Vivado\2020.1\cable_drivers_install.log si algo fallo.
pause
