# 👋 Para el compañero: plan, estado y qué falta

> Resumen corto: **el hardware del avance (Vivado) está listo y compila**. Falta
> probarlo en la Zybo (la tienes tú) y llenar la lógica del `main.c`, que es un
> esqueleto con huecos marcados `TODO(compa):`.

---

## 1. Qué vamos a hacer

**Juego: "Flappy por voz"** (nombre tentativo). Un pajarito que esquiva tubos y
**sube cuando aplaudes o soplas** al micrófono de la Booster. Se eligió sobre
Duck Hunt porque:

- Dibujar en el LCD es lento por SPI. En Flappy solo se mueven el pájaro y unas
  franjas de los tubos, así que se redibuja muy poco por frame.
- Los sensores tienen sentido real, no son de adorno (la rúbrica pide coherencia):
  el micrófono **es** el control.
- Duck Hunt ya lo hizo un grupo en 1S24. "Controlado por voz" suma para el punto de
  originalidad.
- Riesgo: el mic tiene ruido. Por eso va con umbral + antirrebote, y un botón de respaldo.

### Cómo se cumple cada actividad (las 10)

| Act. | Qué es | Cómo lo hacemos | Estado |
| :--- | :--- | :--- | :--- |
| **AO1** | LCD dinámico + 3 sensores + modificar 2 registros | LCD = el juego. Sensores: **micrófono** (aleteo), **acelerómetro** (velocidad/inclinación), **luz** (día/noche). Registros: **límite bajo (0x02) y alto (0x03)** + config (0x01) del OPT3001 | 🟡 funciones listas, falta la lógica y probar |
| **AO2** | 2 interrupciones de timer + 1 de luz/temperatura | `axi_timer_0` = tick del juego (30 Hz), `axi_timer_1` = muestreo del ADC (200 Hz), INT del OPT3001 → `axi_gpio_luz` | 🟢 hardware listo · 🟡 handlers escritos, sin probar |
| **AO3** | IP Core propio (VHDL + AXI) configurado desde la consola (UART), con funciones de punteros | **IP de música** para el buzzer, con registros AXI (nota, tempo, volumen); los parámetros se piden por UART | 🔴 por hacer |
| AC1 | Máquina de estados | MENÚ → JUGANDO → GAME OVER (ya está el `switch`) | 🟡 esqueleto |
| AC2 | structs, arreglos, punteros, macros | `Juego`, `Pajaro`, `Tubo[]`, `Sensores` + funciones con punteros | 🟡 esqueleto |
| AC3 | 2 periféricos **distintos** de los de AO1 | **Joystick** (menú) + **temperatura** (HUD/dificultad) o **LED RGB** (vidas) | 🔴 por hacer |
| AC4 | Boot (sin PC) | `BOOT.bin` en la QSPI, jumper JP5 en QSPI (AYUD08) | 🔴 por hacer (PROG06) |
| AC5 | ILA + VIO, y explicar la diferencia | ILA sobre el SPI del ADC o el AXI del IP de música; VIO para forzar notas | 🔴 por hacer (PROG07) |
| AC6 | Música en el buzzer | Canciones de menú / juego / game over con el IP de AO3 | 🔴 por hacer (PROG08) |
| AC7 | SD (desafío) | Guardar el récord en la SD (`xilffs`) | 🔴 por hacer |

> ⚠️ **PROG06/07/08**: en esas clases llaman grupos al azar a mostrar AC4, AC5 y AC6
> respectivamente. Ir con eso avanzado.

---

## 2. Estado actual (lo que ya está hecho)

- **Vivado 2020.1** (versión obligatoria del curso).
- **Proyecto `P2_SEP`**: se genera con scripts desde la plantilla oficial
  `Proyecto_Base_Booster_2025` (PROY02). Ya trae LCD (SPI 0), ADC (SPI 1), I2C
  (TMP006 + OPT3001) y el módulo `BUZZER` de la plantilla.
- **Hardware agregado por nosotros (AO2)**: 2 × AXI Timer, AXI GPIO de 1 bit para el
  pin INT del OPT3001 (pin **U15**), y un Concat → `IRQ_F2P` del Zynq.
- **Compilado**: bitstream OK, timing OK (WNS +2.8 ns), `.xsa` exportado.
- **`vitis_src/main.c`**: esqueleto con la infraestructura (inits, interrupciones,
  lectura/escritura de registros del OPT3001, máquina de estados, structs). Se
  comprobó que **compila sin errores ni warnings** en Vitis 2020.1. **No se ha probado
  en la Zybo.**
- **Arreglos a la plantilla de Vitis** (no compilaba tal cual): `LCD_GUI.h` tenía
  duplicada la declaración del arreglo `intro`, y `Delay.c` incluía `<unistd.h>`, que
  choca con `sleep.h`. Ya están corregidos en `vitis_src/`.

### Direcciones y señales

| Bloque | Base | Interrupción (GIC) | Para qué |
| :--- | :--- | :--- | :--- |
| `axi_timer_0` | `0x4280_0000` | 61 | Tick del juego (30 Hz) |
| `axi_timer_1` | `0x4281_0000` | 62 | Muestreo del ADC (200 Hz) |
| `axi_gpio_luz` | `0x4121_0000` | 63 | INT del OPT3001 (pin U15) |
| `axi_quad_spi_0` | (plantilla) | – | Pantalla |
| `axi_quad_spi_1` | (plantilla) | – | ADC: joystick, acelerómetro, mic, potenciómetros |
| `axi_iic_0` | (plantilla) | – | TMP006 (`0x40`), OPT3001 (`0x44`) |
| `axi_gpio_0` | (plantilla) | – | RS/RST de la pantalla |

Reloj de la lógica (FCLK0): **100 MHz**.

---

## 3. Cómo dejarlo andando en tu PC

1. **Instalar Vivado/Vitis 2020.1**: guía en [`docs/instalar_vivado_2020.1.md`](docs/instalar_vivado_2020.1.md)
   (solo Zynq-7000, board files de Digilent y drivers del cable con
   [`docs/instalar_cable_drivers.bat`](docs/instalar_cable_drivers.bat) como admin).
2. **Clonar en una ruta corta y fuera de OneDrive**, ideal `C:\SEP\proyecto2`
   (OneDrive bloquea archivos mientras Vivado compila, y Windows tiene un límite de 260
   caracteres por ruta):
   ```bat
   git clone https://github.com/lucasmartinfuentes/SEP-Proyecto2.git C:\SEP\proyecto2
   ```
3. **Generar todo** (5 a 15 min; cambia `D:\Xilinx` si lo instalaste en otro lado):
   ```bat
   cd C:\SEP\proyecto2
   D:\Xilinx\Vivado\2020.1\bin\vivado.bat -mode batch -source scripts/01_crear_proyecto.tcl
   D:\Xilinx\Vivado\2020.1\bin\vivado.bat -mode batch -source scripts/03_build.tcl
   D:\Xilinx\Vitis\2020.1\bin\xsct.bat scripts/04_vitis.tcl
   ```
   Quedan `vivado/` (proyecto + bitstream + `.xsa`) y `vitis/` (workspace). Ninguno se sube a git.
4. **Programar**: abrir Vitis con el workspace `C:\SEP\proyecto2\vitis` → clic derecho
   en `P2_app` → *Run As → Launch Hardware (Single Application Debug)*. Eso programa la
   FPGA y carga el ELF.
5. **Consola**: terminal serial (la de Vitis o CoolTerm) en el COM de la Zybo,
   **115200 8N1**. Ahí sale lo que imprime `xil_printf`.

> ¿Algo falló a la mitad? Borra `vivado/` (o `vitis/`) y vuelve a correr el script.

---

## 4. Qué falta: lo tuyo primero (tienes la Zybo)

### 🔥 Para el avance (AO1 + AO2 + diagrama)

- [ ] **Probar la plantilla tal cual**: ¿enciende el LCD y se leen los sensores? Si
      no, el problema es de conexión/hardware, no del código nuevo.
- [ ] **Probar `main.c` en la placa** y revisar por UART:
  - [ ] Imprime `OPT3001 config=0x... lim_bajo=0x... lim_alto=0x...` con los valores
        escritos (esa es la evidencia de los "2 registros modificados").
  - [ ] Los timers corren (por ejemplo, poner un `xil_printf` cada 30 ticks).
  - [ ] Tapar el sensor de luz imprime `Interrupcion de luz`. **Si no llega nunca**:
        revisar en el esquemático de la Booster si el INT del OPT3001 tiene pull-up
        (es open-drain). Si no tiene, agregar `PULLUP true` al pin U15 en
        `constraints/Zybo-Z7-Master.xdc` y recompilar (`03_build.tcl`).
- [ ] **Medir valores reales** (por UART) y anotarlos aquí abajo:
  - mic en silencio / con aplauso: `____ / ____`
  - lux destapado / tapado con la mano: `____ / ____`
  - acelerómetro plano / inclinado: `____ / ____`
- [ ] Con eso, **llenar los `TODO(compa):` del `main.c`**, en este orden:
  1. `OPT_UMBRAL_BAJO/ALTO` y `OPT_CONFIG` (umbrales de la interrupción)
  2. `detectar_aleteo()` (umbral + antirrebote del mic)
  3. `juego_actualizar()` y `juego_hay_choque()` (física y choques)
  4. `dibujar_juego()` (redibujo incremental: borrar solo lo que se movió)
  5. Estados MENÚ y GAME OVER
- [ ] **Diagrama** (30 % del avance): lo hacemos aparte (no está en este repo todavía).

### Después del avance

- [ ] **AO3 + AC6: IP de música con AXI.** Hoy el `BUZZER` de la plantilla es un
      módulo RTL **sin AXI**: el procesador no le puede hablar. Hay que crear un IP
      (*Create and Package New IP*, como en LAB03/AYUD10) con registros AXI para
      frecuencia/nota, duración/tempo y volumen, y en C una función con punteros que
      lo configure con datos pedidos por UART. Referencia: el ejemplo Snake (PROY02)
      tiene `axi_musica` + `Cancion.vhd`.
- [ ] **AC3**: el joystick ya se lee por el ADC. **Los botones de la Booster (BTN0,
      BTN1, BTN_JY) hoy están conectados al módulo BUZZER, no al procesador.** Para
      usarlos desde C hay que pasarlos a un AXI GPIO (o al IP de música).
- [ ] **AC4** boot QSPI (AYUD08), **AC5** ILA/VIO (LAB05/AYUD04), **AC7** SD.
- [ ] Informe en LaTeX (plantilla en PROY02) + video de 5 a 15 min.

---

## 5. Cosas que ya sabemos que muerden

- **`read_opt()` de `I2C.c` reescribe la config `0xC410` en cada lectura** y pisaría
  nuestros umbrales/latch. El `main.c` usa `opt3001_leer_reg()` en su lugar.
- **El AXI GPIO interrumpe en cada cambio del pin** (bajada y subida). El handler ya
  filtra: solo actúa cuando el pin queda en 0 (INT activo en bajo).
- **Nada lento dentro de una interrupción**: ni LCD ni I2C. Los handlers solo
  levantan banderas o leen el ADC; el resto va en el `while` del `main`.
- **No hacer `LCD_Clear()` en cada frame**: parpadea y es lentísimo. Se borra solo lo
  que se movió, con el color del fondo.
- `xil_printf` **no imprime `float`**: usar enteros (ej. centilux) y
  `%d.%02d`. El salto de línea es `\r\n`.
- **Si cambias el Block Design en la GUI de Vivado**, ese cambio **no queda en git**
  (`vivado/` no se sube). Copia los comandos que aparecen en la *Tcl Console* a
  `scripts/02_hw_avance.tcl` para que se regenere igual.
- **Si editas el código dentro de Vitis**, cópialo de vuelta a `vitis_src/` antes del
  commit (`04_vitis.tcl` copia `vitis_src/` → `vitis/P2_app/src`).

---

## 6. Propuesta de división (conversable)

| Tú (tienes la Zybo) | Lucas |
| :--- | :--- |
| Pruebas en placa, medir sensores, llenar los `TODO(compa)` | Diagrama del avance |
| Física y dibujo del juego | IP de música (VHDL + AXI) para AO3/AC6 |
| Boot QSPI (AC4) | ILA/VIO (AC5) e informe |

Flujo git: `git pull` antes de empezar y commits chicos. **Nunca subir `vivado/`,
`vitis/` ni `logs/`** (ya están en `.gitignore`).

Contexto completo del curso (qué se vio en cada lab/ayudantía, rúbricas, límites),
pensado para pegárselo a una IA: [`CONTEXTO_IA.md`](CONTEXTO_IA.md).
