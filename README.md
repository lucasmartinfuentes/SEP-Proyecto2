# Proyecto 2 · IEE2463 SEP

Carpeta de trabajo del Proyecto 2 (Zybo Z7-10 + BoosterPack MKII).
**Vive en `C:\SEP\proyecto2`, fuera de OneDrive**: OneDrive bloquea archivos
mientras Vivado compila y causa errores raros. Para compartir con el
compañero: comprimir la carpeta (sin `vivado/` ni `vitis/`) o usar git.

## Estructura

| Carpeta | Qué contiene |
| :--- | :--- |
| `scripts/01_crear_proyecto.tcl` | Crea `vivado/P2_SEP.xpr` a partir de la plantilla oficial (sin modificarla). |
| `scripts/02_hw_avance.tcl` | Hardware agregado por el grupo para AO2: 2 AXI Timer + AXI GPIO de la interrupción de luz + Concat → `IRQ_F2P` del Zynq. |
| `constraints/Zybo-Z7-Master.xdc` | Constraints de la plantilla (LAB10) + pin **U15** = INT del OPT3001 (`interrupt_luz_tri_i`). |
| `ip_repo/` | IP Cores propios. Por ahora, el `BUZZER_1.0` de la plantilla (base del IP de música para AO3/AC6). |
| `vitis_src/` | Código C para Vitis: drivers de la plantilla (LCD, ADC, I2C, Delay) y, después, nuestro `main.c`. |
| `plantilla/` | `Proyecto_Base_Booster_2025` oficial (repo PROY02), descomprimido. No se edita. |
| `docs/` | Diagrama, guía de instalación, capturas de ILA/VIO y notas para el informe. |
| `vivado/` | **Generado** por el script 01. Se puede borrar y regenerar. |
| `vitis/` | **Generado**: workspace de Vitis. |

Material de referencia (sigue en OneDrive, `Documentos/sepproject2/`):
- `PROY02/`: enunciado, rúbrica y plantilla del informe.
- `referencias/`: repos del curso (rama `2026-2`). Ahí se revisa si algo **se vio o no** en clases (punto de originalidad).
- `PROY02-Ejemplo_snake/` y `PROY02-otrosejemplos/`: proyectos de semestres anteriores.

## Cómo generar el proyecto (Vivado 2020.1)

1. Vivado 2020.1 → **Tcl Console**:
   ```tcl
   cd C:/SEP/proyecto2
   source scripts/01_crear_proyecto.tcl
   ```
2. Revisar el block design: deben aparecer `axi_timer_0`, `axi_timer_1`, `axi_gpio_luz` y `xlconcat_0` conectados al `IRQ_F2P`.
3. **Generate Bitstream**.
4. **File → Export → Export Hardware** → *Include bitstream* → `C:\SEP\proyecto2\vivado\P2_SEP.xsa`.
5. Vitis 2020.1 → workspace `C:\SEP\proyecto2\vitis` → *Application Project* con ese `.xsa` → plantilla **Hello World** (trae `platform.c/h` y la UART como consola) → borrar `helloworld.c` → importar `vitis_src/` en `src/`. O todo de una vez con `scripts/04_vitis.tcl` (ver abajo).

> 👉 Plan, estado y pendientes: [`LEEME_COMPANERO.md`](LEEME_COMPANERO.md) · Contexto completo para una IA: [`CONTEXTO_IA.md`](CONTEXTO_IA.md)

> Si el script falla a la mitad: cerrar Vivado, borrar `vivado/` y volver a correrlo.

Sin abrir la GUI (lo que se usó para generar `vivado/` la primera vez; logs en `logs/`):

```bat
D:\Xilinx\Vivado\2020.1\bin\vivado.bat -mode batch -source C:/SEP/proyecto2/scripts/01_crear_proyecto.tcl
D:\Xilinx\Vivado\2020.1\bin\vivado.bat -mode batch -source C:/SEP/proyecto2/scripts/03_build.tcl
D:\Xilinx\Vitis\2020.1\bin\xsct.bat C:/SEP/proyecto2/scripts/04_vitis.tcl
```

`03_build.tcl` deja el bitstream y `vivado/P2_SEP.xsa` (con bitstream) listo para Vitis; `04_vitis.tcl` crea el workspace `vitis/` (plataforma + app) y compila el ELF.

### Direcciones asignadas (hardware del avance)

| Bloque | Base | IRQ_F2P (concat) | ID en GIC |
| :--- | :--- | :--- | :--- |
| `axi_timer_0` | `0x4280_0000` | In0 | 61 |
| `axi_timer_1` | `0x4281_0000` | In1 | 62 |
| `axi_gpio_luz` | `0x4121_0000` | In2 | 63 |

En C se usan las macros de `xparameters.h` (`XPAR_FABRIC_..._INTR`), no los números a mano.

## Qué cubre cada bloque (para el diagrama)

| Actividad | Dónde se cumple |
| :--- | :--- |
| AO1: LCD + 3 sensores + 2 registros | `axi_quad_spi_0` (LCD), `axi_quad_spi_1` (ADC: acelerómetro, micrófono), `axi_iic_0` (OPT3001 luz, TMP006 temperatura). Registros 0x01/0x02/0x03 del OPT3001. **Vitis** |
| AO2: 2 timers + interrupción de luz | `axi_timer_0`, `axi_timer_1`, `axi_gpio_luz`, `xlconcat_0` → `IRQ_F2P`. **Vivado** (hardware) + **Vitis** (handlers con `XScuGic`) |
| AO3: IP Core configurado por UART | `ip_repo/` (IP de música/buzzer con registros AXI). **Vivado (VHDL)** + **Vitis** (inicialización con punteros) |
