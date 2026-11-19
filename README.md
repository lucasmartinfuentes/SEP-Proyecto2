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
| Vivado: plantilla del curso + 2 AXI Timer + interrupción del sensor de luz | Listo. Bitstream generado, sin advertencias críticas, timing cumplido en los dos relojes (WNS +2,45 ns) |
| Hardware exportado (`hw/P2_SEP.xsa`, con bitstream) | Listo |
| Vitis: plataforma y aplicación | Listo. Compila sin errores ni warnings |
| `vitis_src/main.c` | Juego v1 completo (menú, juego, game over). El menú muestra en pantalla la evidencia de AO1 y AO2: interrupciones por segundo de cada timer, modo día/noche, contador de interrupciones de luz y los registros 0x01/0x02/0x03 del OPT3001 leídos de vuelta. Compila sin warnings |
| Pruebas en la Zybo | Pendiente: probar y ajustar los valores marcados `CALIBRAR` |
| Diagrama del avance | Listo: `docs/diagrama_avance.drawio` (falta número de grupo y exportar a PDF) |

## Actividades del enunciado

| Actividad | Implementación | Estado |
| :--- | :--- | :--- |
| AO1 · LCD + 3 sensores + 2 registros | LCD con el juego; micrófono, acelerómetro, luz y temperatura; límites 0x02/0x03 y config 0x01 del OPT3001 | Código listo, sin probar |
| AO2 · 2 timers + interrupción de luz | `axi_timer_0` (juego, 30 Hz), `axi_timer_1` (muestreo ADC, 200 Hz), INT del OPT3001 en `axi_gpio_luz` | Código listo, sin probar |
| AO3 · IP Core configurado por UART | IP de música con registros AXI para el buzzer | Pendiente |
| AC1 · Máquina de estados | Menú → juego → game over | Listo, sin probar |
| AC2 · structs, punteros, macros | `Juego`, `Pajaro`, `Tubo[]`, `Sensores` | Listo |
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
4. Ajustar en `main.c` las constantes marcadas `CALIBRAR` con esos valores.
5. En el menú, comprobar la evidencia en pantalla: `Int/s T0:~30 T1:~200`; al tapar el
   sensor, `Modo` pasa a `NOCHE`, `IntLuz` sube y `R2/R3` cambian a los límites de
   noche. Si `T0` marca más de 30, el dibujo tarda más de 33 ms por frame.
6. Grabar la demo (video de máximo 5 min) y entregar un PDF con el diagrama y el link
   público al video.

## Cómo trabajar

Requisitos: Vivado/Vitis **2020.1** (versión del curso), board files de Digilent y los
drivers del cable (guía en `docs/instalar_vivado_2020.1.md`; los drivers se instalan con
`docs/instalar_cable_drivers.bat` como administrador).

Clonar en una ruta corta y fuera de OneDrive (OneDrive bloquea archivos durante la
compilación y Windows limita las rutas a 260 caracteres), por ejemplo
`C:\SEP\proyecto2`:

```bat
git clone https://github.com/lucasmartinfuentes/SEP-Proyecto2.git C:\SEP\proyecto2
```

Los `.bat` de la raíz se usan con doble clic y buscan Xilinx en `C:\`, `D:\` o `E:\`.

### Solo código C (sin abrir Vivado)

1. Doble clic en **`crear_vitis.bat`**: crea `vitis\` desde `hw\P2_SEP.xsa` y compila.
2. Abrir Vitis 2020.1 con el workspace `<repo>\vitis`.
3. Programar: clic derecho en `P2_app` → *Run As → Launch Hardware*. Consola serial en
   el COM de la Zybo a 115200 8N1.

El código se edita en `vitis_src\` (es lo que se versiona). Lo editado dentro de Vitis
(`vitis\P2_app\src`) se copia de vuelta a `vitis_src\` antes de hacer commit.

### Cambios de hardware (Vivado, por GUI)

1. Doble clic en **`abrir_vivado.bat`** (abre `vivado\P2_SEP.xpr`; no cerrar la
   ventana negra, es la consola de Vivado).
2. Editar el block design, constraints o IP con la GUI.
3. *Generate Bitstream*.
4. *File → Export → Export Hardware* → *Include bitstream* → guardar como
   **`hw\P2_SEP.xsa`** (reemplazar).
5. Commit de `vivado\` (solo se versionan las fuentes: `.xpr`, `.bd`, wrapper, `.xci`,
   VHDL) y de `hw\P2_SEP.xsa`.
6. En Vitis, actualizar la plataforma con el `.xsa` nuevo (clic derecho en `P2_plat` →
   *Update Hardware Specification*) o borrar `vitis\` y correr `crear_vitis.bat`.

La primera vez que se abre el proyecto en otro PC, Vivado regenera los productos del
block design al compilar (los archivos generados no se versionan).

### Regenerar desde cero (respaldo)

Si `vivado\` se rompe: borrar la carpeta y hacer doble clic en `regenerar_vivado.bat`.
Recrea el proyecto desde la plantilla con `scripts\01_crear_proyecto.tcl` y
`scripts\02_hw_avance.tcl`, compila y exporta `hw\P2_SEP.xsa`. **Descarta los cambios
hechos por GUI** que no estén también en `02_hw_avance.tcl`.

## Estructura

| Ruta | Contenido |
| :--- | :--- |
| `vivado/P2_SEP.xpr` | Proyecto de Vivado (se abre con la GUI) |
| `hw/P2_SEP.xsa` | Hardware exportado con bitstream; es lo que usa Vitis |
| `vitis_src/` | Código C: drivers de la plantilla (LCD, ADC, I2C, Delay) y `main.c` |
| `constraints/` | `.xdc` de la plantilla + pin U15 (INT del OPT3001) + reloj de 125 MHz |
| `ip_repo/` | IP Cores propios (por ahora la base `BUZZER_1.0`) |
| `plantilla/` | `Proyecto_Base_Booster_2025` oficial (repo PROY02). No se edita |
| `scripts/` | Tcl de respaldo: crear proyecto (01, 02), compilar (03), Vitis (04) |
| `*.bat` | Accesos de doble clic: abrir Vivado, crear Vitis, regenerar desde cero |
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
| `BUZZER_0` | plantilla | – | Módulo RTL sin AXI (botones de la Booster → tono y volumen), reloj de 125 MHz |

Reloj de los periféricos AXI: 100 MHz (FCLK0). CPU: 667 MHz. En C se usan las macros de
`xparameters.h` (`XPAR_FABRIC_..._INTR`), no los números.

## Notas

- La plantilla de Vitis no compilaba en 2020.1. Se corrigieron dos archivos:
  `LCD_GUI.h` (declaración duplicada del arreglo `intro`) y `Delay.c` (`<unistd.h>`
  chocaba con `sleep.h`).
- El `.xdc` de la plantilla no restringía el reloj de 125 MHz (`create_clock` comentado
  y con otro nombre de puerto), así que el `BUZZER` quedaba sin análisis de timing. Se
  agregó el `create_clock`.
- Advertencias que quedan y son de la plantilla: CKLD-2 (el reloj de 125 MHz no pasa por
  un buffer global), BUFC-1 (entradas del SPI del ADC sin uso) y TIMING-18 (pines sin
  input/output delay). No afectan el funcionamiento.
- `read_opt()` de `I2C.c` reescribe la configuración del OPT3001 en cada lectura; por
  eso `main.c` usa sus propias funciones de lectura/escritura de registros.
- Los botones de la Booster están conectados al módulo `BUZZER`, no al procesador. Para
  leerlos desde C hay que pasarlos a un AXI GPIO o al IP de música.
