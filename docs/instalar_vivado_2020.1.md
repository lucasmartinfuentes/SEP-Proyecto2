# Instalar Vivado + Vitis 2020.1

Resumen de la guía oficial del curso (`referencias/AYUD00-Instalacion-Software/How_to_Install_Vitis.pdf`
y su [video](https://www.youtube.com/watch?v=cnd4IJH-XsY&list=PLACEUah7BCQvPCO8X1JKmvREXUQkfqjQA)),
adaptado a este PC. **Tiene que ser la versión 2020.1**: los ayudantes revisan con esa versión.

En este PC hay **136 GB libres en D:** y solo 41 GB en C:, así que se instala en `D:\Xilinx`.
(En `D:\VIVADO` y `C:\vivado` hay restos de una instalación anterior que ya no tiene Vivado; se pueden borrar.)

1. **Descargar** (requiere cuenta AMD/Xilinx y llenar el formulario de exportación):
   [archivo de descargas de Vitis](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vitis/archive-vitis.html)
   → **2020.1** → *Xilinx Unified Installer 2020.1: **All OS installer Single-File Download***
   (`Xilinx_Unified_2020.1_0602_1208.tar.gz`, unos 35.5 GB). Guárdenlo en `D:\`.
2. **Descomprimir** en `D:\` con 7-Zip (dos pasos: `.tar.gz` → `.tar` → carpeta).
   Después pueden borrar el `.tar.gz` y el `.tar` para recuperar espacio.
3. Ejecutar `D:\Xilinx_Unified_2020.1_0602_1208\xsetup.exe`. Si dice que no hay internet o que hay una versión más nueva: **Ignore / Continue**.
4. Aceptar las licencias (*I Agree* en las tres).
5. Producto: **Vitis** (incluye Vivado).
6. Componentes:
   - **Desmarcar *System Generator for DSP*** (da errores con MATLAB).
   - En *Devices* basta con **Zynq-7000** (la Zybo Z7-10 es `xc7z010clg400-1`). Desmarcar el resto ahorra muchos GB.
   - Dejar marcado **Install Cable Drivers** (necesario para programar la Zybo por USB).
7. Ruta de instalación: `D:\Xilinx`.
8. Instalar (2 a 6 horas).
9. **Board files de Digilent** (opcional para este proyecto, porque la plantilla usa el part y no la board, pero lo pide la guía):
   descargar <https://github.com/Digilent/vivado-boards/archive/master.zip> y copiar el contenido de
   `vivado-boards-master/new/board_files/` en `D:\Xilinx\Vivado\2020.1\data\boards\board_files\`.
10. Probar: abrir *Vivado 2020.1* desde el menú Inicio, y conectar la Zybo con el jumper JP5 en **JTAG**.

Después de instalar, seguir `C:SEPproyecto2README.md` → *Cómo generar el proyecto*.
