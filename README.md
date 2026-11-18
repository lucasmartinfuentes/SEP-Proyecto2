# Proyecto 2 · IEE2463 SEP

Proyecto final del curso Sistemas Electrónicos Programables (PUC, 2026-2), sobre una
Zybo Z7-10 con la BoosterPack BOOSTXL-EDUMKII.

**Idea:** un juego tipo Flappy Bird controlado por sonido. El personaje sube cuando el
micrófono detecta un aplauso o un soplido; la inclinación de la placa (acelerómetro)
cambia la velocidad y tapar el sensor de luz activa un modo noche. El buzzer toca música
generada por un IP Core propio.

## Estado

| Parte | Estado |
| :--- | :--- |
| Vivado: plantilla del curso + 2 AXI Timer + interrupción del sensor de luz | Listo. Bitstream generado, timing cumplido (WNS +2,8 ns) |
| Vitis: plataforma y aplicación | Listo. Compila sin errores ni warnings |
| `vitis_src/main.c` | Esqueleto: inicializaciones, interrupciones, registros del OPT3001 y máquina de estados. La lógica del juego está pendiente (marcada con `TODO`) |
| Pruebas en la Zybo | Pendiente |
| Diagrama del avance | Pendiente |

## Actividades del enunciado

| Actividad | Implementación | Estado |
| :--- | :--- | :--- |
| AO1 · LCD + 3 sensores + 2 registros | LCD con el juego; micrófono, acelerómetro y luz; límites 0x02/0x03 y config 0x01 del OPT3001 | Funciones listas, falta lógica y prueba |
| AO2 · 2 timers + interrupción de luz | `axi_timer_0` (juego, 30 Hz), `axi_timer_1` (muestreo ADC, 200 Hz), INT del OPT3001 en `axi_gpio_luz` | Hardware listo, handlers sin probar |
| AO3 · IP Core configurado por UART | IP de música con registros AXI para el buzzer | Pendiente |
| AC1 · Máquina de estados | Menú → juego → game over | Esqueleto |
| AC2 · structs, punteros, macros | `Juego`, `Pajaro`, `Tubo[]`, `Sensores` | Esqueleto |
| AC3 · 2 periféricos extra | Joystick + temperatura o LED RGB | Pendiente |
| AC4 · Boot | `BOOT.bin` en QSPI | Pendiente |
| AC5 · ILA y VIO | Sobre el AXI del IP de música o el SPI del ADC | Pendiente |
| AC6 · Música | Canciones de menú, juego y game over | Pendiente |
| AC7 · SD | Récord guardado en la SD | Pendiente |

## Pendiente para el avance

1. Cargar la plantilla sin cambios en la Zybo y confirmar que el LCD y los sensores responden.
2. Cargar `main.c` y revisar por UART que:
   - los registros del OPT3001 se leen de vuelta con los valores escritos;
   - los dos timers interrumpen;
   - tapar el sensor de luz genera la interrupción (si no llega nunca, revisar el
     pull-up del pin INT y, si falta, agregar `PULLUP true` a U15 en el `.xdc`).
3. Medir valores reales: micrófono en silencio y con aplauso, lux con el sensor
   destapado y tapado, acelerómetro plano e inclinado.
4. Completar los `TODO` de `main.c`: umbrales del OPT3001, detección del aplauso,
   física, choques, dibujo incremental y estados.
5. Diagrama: todas las actividades, datos sobre las flechas, un color por actividad y la
   separación Vivado/Vitis.

## Cómo generar el proyecto

Requisitos: Vivado/Vitis **2020.1** (versión del curso), board files de Digilent y los
drivers del cable (guía en `docs/instalar_vivado_2020.1.md`; los drivers se instalan con
`docs/instalar_cable_drivers.bat` como administrador).

Clonar en una ruta corta y fuera de OneDrive (OneDrive bloquea archivos durante la
compilación y Windows limita las rutas a 260 caracteres):

```bat
git clone https://github.com/lucasmartinfuentes/SEP-Proyecto2.git C:\SEP\proyecto2
cd C:\SEP\proyecto2
D:\Xilinx\Vivado\2020.1\bin\vivado.bat -mode batch -source scripts/01_crear_proyecto.tcl
D:\Xilinx\Vivado\2020.1\bin\vivado.bat -mode batch -source scripts/03_build.tcl
D:\Xilinx\Vitis\2020.1\bin\xsct.bat scripts/04_vitis.tcl
```

Resultado: `vivado/` (proyecto, bitstream, `P2_SEP.xsa`) y `vitis/` (workspace con la
plataforma `P2_plat` y la app `P2_app`). Ninguna de las dos carpetas se versiona.

Para programar: abrir Vitis con el workspace `C:\SEP\proyecto2\vitis`, clic derecho en
`P2_app` → *Run As → Launch Hardware*. Consola serial en el COM de la Zybo a 115200 8N1.

Si un script falla a la mitad, borrar `vivado/` o `vitis/` y volver a correrlo.

## Estructura

| Carpeta | Contenido |
| :--- | :--- |
| `scripts/01_crear_proyecto.tcl` | Crea `vivado/P2_SEP.xpr` desde la plantilla oficial (sin modificarla) |
| `scripts/02_hw_avance.tcl` | Hardware agregado: 2 AXI Timer, AXI GPIO de la interrupción de luz, Concat → `IRQ_F2P` |
| `scripts/03_build.tcl` | Síntesis, implementación, bitstream, reporte de timing y export del `.xsa` |
| `scripts/04_vitis.tcl` | Plataforma + aplicación en `vitis/`, importa `vitis_src/` y compila |
| `constraints/` | `.xdc` de la plantilla + pin U15 (INT del OPT3001) |
| `ip_repo/` | IP Cores propios (por ahora la base `BUZZER_1.0`) |
| `vitis_src/` | Código C: drivers de la plantilla (LCD, ADC, I2C, Delay) y `main.c` |
| `plantilla/` | `Proyecto_Base_Booster_2025` oficial (repo PROY02). No se edita |
| `docs/` | Guías de instalación |
| `CONTEXT.md` | Contexto completo del curso y del proyecto para usar con una IA |

## Hardware

| Bloque | Base | IRQ (GIC) | Uso |
| :--- | :--- | :--- | :--- |
| `axi_timer_0` | `0x4280_0000` | 61 | Tick del juego |
| `axi_timer_1` | `0x4281_0000` | 62 | Muestreo del ADC |
| `axi_gpio_luz` | `0x4121_0000` | 63 | INT del OPT3001 (pin U15) |
| `axi_quad_spi_0` | plantilla | – | Pantalla |
| `axi_quad_spi_1` | plantilla | – | ADC: joystick, acelerómetro, micrófono |
| `axi_iic_0` | plantilla | – | TMP006 (0x40) y OPT3001 (0x44) |
| `BUZZER_0` | plantilla | – | Módulo RTL sin AXI (botones de la Booster → tono y volumen) |

Reloj de la lógica: 100 MHz (FCLK0). En C se usan las macros de `xparameters.h`
(`XPAR_FABRIC_..._INTR`), no los números.

## Notas

- La plantilla de Vitis no compilaba en 2020.1. Se corrigieron dos archivos:
  `LCD_GUI.h` (declaración duplicada del arreglo `intro`) y `Delay.c` (`<unistd.h>`
  chocaba con `sleep.h`).
- `read_opt()` de `I2C.c` reescribe la configuración del OPT3001 en cada lectura; por
  eso `main.c` usa sus propias funciones de lectura/escritura de registros.
- Los botones de la Booster están conectados al módulo `BUZZER`, no al procesador. Para
  leerlos desde C hay que pasarlos a un AXI GPIO o al IP de música.
- Los cambios hechos al block design desde la GUI no quedan en el repo (`vivado/` no se
  versiona): hay que copiarlos como comandos Tcl a `scripts/02_hw_avance.tcl`.
- `04_vitis.tcl` copia `vitis_src/` al workspace. Lo editado dentro de Vitis se debe
  copiar de vuelta a `vitis_src/` antes de hacer commit.
