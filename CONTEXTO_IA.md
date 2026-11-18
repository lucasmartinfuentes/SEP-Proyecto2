# CONTEXTO PARA UNA IA — Proyecto 2 IEE2463 SEP

> **Cómo usar este archivo:** pégalo completo al inicio de una conversación con una IA
> (o adjúntalo) antes de pedirle código, revisiones o texto del informe. Contiene las
> reglas del curso, el hardware real, lo que ya está hecho, cómo escribimos el código
> y qué se vio y qué no se vio en clases. Si algo de aquí contradice a la IA, **manda
> este archivo**.

---

## 0. Instrucciones para la IA (léelas primero)

1. **Responde en español** (Chile). Los comentarios del código también van en español,
   **sin tildes ni ñ dentro del código** (evita problemas de encoding en Vitis).
2. **Versión obligatoria: Vivado/Vitis 2020.1.** No uses APIs, opciones de menú ni
   flujos de versiones más nuevas: nada de Vitis Unified IDE 2023+, nada de
   `xsdb`/Python de versiones nuevas, nada de nombres de drivers inventados.
3. **Procesador: ARM Cortex-A9 del PS (Zynq), BSP *standalone*, sin sistema operativo.**
   No propongas Linux, FreeRTOS, PetaLinux ni MicroBlaze a menos que se pida.
4. **Interrupciones en el Zynq = `XScuGic`** (el GIC). En el curso se enseñaron primero
   con MicroBlaze + `XIntc` (LAB07, AYUD07); en este proyecto se usa `XScuGic` como en el
   ejemplo Snake (PROY02-Ejemplo). No mezcles ambos.
5. **Usa los drivers que ya existen en `vitis_src/`** (plantilla del curso) para LCD, ADC,
   I2C y retardos. No reescribas un driver de pantalla desde cero. Las firmas reales están
   en la sección 6.
6. **No inventes registros ni macros.** Si necesitas un `XPAR_...`, sale de
   `xparameters.h` (generado desde el `.xsa`). Si no estás seguro de un nombre, dilo y
   explica cómo verificarlo (buscar en `xparameters.h`).
7. **El estilo del código importa para la nota** (30 % es código): funciones con
   argumentos de puntero, `struct`, arreglos, `#define`, comentarios en detalle de todo
   lo escrito o modificado por el grupo. Cada 3 errores de programación se descuentan
   0,75 puntos.
8. **Nada lento dentro de una interrupción** (ni LCD ni I2C): solo banderas `volatile`
   o lecturas rápidas (ADC por SPI). El trabajo pesado va en el `while(1)` del `main`.
9. **`xil_printf` no imprime `float`.** Usa enteros escalados (`%d.%02d`). Salto de línea
   `\r\n`.
10. **Código ajeno se referencia.** El curso permite usar código de internet, IA,
    ayudantías y labs, pero **todo debe quedar referenciado** (si no, es plagio por el
    Código de Honor UC). Cuando entregues código basado en algo del curso o de internet,
    indica de dónde viene, para ponerlo en el informe.
11. Prefiere **soluciones simples que se puedan mostrar funcionando en la Zybo**. El grupo
    busca buena nota con el mínimo de trabajo extra; no sobre-diseñes.
12. Si una propuesta requiere cambiar el hardware (Vivado), dilo explícitamente y da los
    comandos Tcl para agregarlos a `scripts/02_hw_avance.tcl` (el repo no guarda
    `vivado/`; el proyecto se regenera desde scripts).

---

## 1. El curso

- **IEE2463 Sistemas Electrónicos Programables**, PUC Chile, Depto. de Ingeniería
  Eléctrica, semestre **2026-2**. Profesor: Michel Rozas (2026-2). Material en GitHub:
  organización `IEE2463-SEP`, rama vigente **`2026-2`** (la `main` es histórica).
- Tema: programación de SoC **Zynq-7000** con VHDL (Vivado) y C (Vitis), protocolo AXI,
  IP Cores propios, MicroBlaze, interrupciones, debug con ILA/VIO, UART/I2C/SPI y la
  tarjeta **BoosterPack BOOSTXL-EDUMKII** de TI.
- Proyecto en **parejas**. Este es el **Proyecto 2** (el final del curso).

---

## 2. El enunciado del Proyecto 2 (resumen fiel)

La idea del proyecto debe ser **del grupo**. Se puede basar en otra fuente siempre que
se haga un **aporte sustantivo** y se diga claramente cuál fue el aporte y en qué se basó.
Se puede usar el ARM del PS, MicroBlaze o ambos.

### Actividades obligatorias (AO) — para aprobar

- **AO1:** Usar la **pantalla** de la Booster para mostrar información/figuras/imágenes
  que se actualicen dinámicamente y sean coherentes con el proyecto. Además, usar **al
  menos 3** de: **sensor de luz, sensor de temperatura, acelerómetro, micrófono**.
  Entender los registros internos de los periféricos y **validar el funcionamiento
  modificando al menos 2 registros** de algún periférico (pueden ser del mismo).
- **AO2:** Al menos **2 interrupciones generadas por timer(s) implementados en Vivado**,
  usadas para leer/escribir periféricos periódicamente. Además, **al menos una
  interrupción del sensor de temperatura o del de luz**.
- **AO3:** Crear **al menos un IP Core** que interactúe con algún periférico **y** con el
  procesador. Una **función con argumentos de punteros** para inicializar y configurar el
  hardware. Los parámetros de configuración los entrega el **usuario desde la consola del
  PC (UART)**.

### Actividades complementarias (AC) — para optar a 7,0

- **AC1:** Máquina de estados asociada al funcionamiento del proyecto.
- **AC2:** Usar estructuras, arreglos, punteros, funciones y macros.
- **AC3:** Usar **2 periféricos adicionales** a los de AO1 (no los mismos): sensor de
  luz, temperatura, acelerómetro, LED RGB, micrófono, joystick, botones.
- **AC4:** Boot de la Zybo (el código queda cargado sin subirlo cada vez).
- **AC5:** Usar **VIO e ILA** para interactuar con algún módulo y validar que la
  información enviada/recibida es correcta. **Explicar las diferencias entre VIO e ILA.**
- **AC6:** Usar el **buzzer** para tocar música coherente y complementaria al proyecto.
- **AC7 (desafío):** Usar una **memoria SD** externa y hacer algo con ella.

### Entrega final

- Un PDF en Canvas con nombre = número de grupo, que contiene solo: número de grupo,
  **link de OneDrive** con los archivos y **link de YouTube** del video.
- Archivos en el OneDrive: (1) informe PDF con el nombre del proyecto; (2) opcional, el
  video; (3) **un `.zip` con los proyectos de Vitis y Vivado exportados**, incluyendo
  los IP Cores (probar que el compañero lo pueda abrir). `readme.txt` opcional.
- Más de 3 archivos, nombres o formato incorrectos: **−1 punto**.
- Atraso: −0,1 cada 6 min las primeras 2 h, luego decae cuadráticamente hasta 1,0 a las
  12 h. Después de 12 h, nota 1,0.

### Nota final = Informe 35 % + Video 35 % + Código 30 %

**Video (5 a 15 min; si pasa de 15, nota mínima).** Nota = 1 + puntos:

| Puntos | Criterio |
| :--- | :--- |
| hasta 4 | Implementación: 3 AO + 7 AC → 4 · 3 AO + 4–6 AC → 3 · 3 AO + 1–3 AC → 2 · 2 AO + 6 AC → 2 · 2 AO + <6 AC → 1 |
| 1 | **Originalidad:** usar alguna funcionalidad de la Zybo/Zynq **no vista** en clases, labs o ayudantías, **indicando las referencias** de dónde se aprendió |
| 0,5 | Coherencia: tiene las partes pedidas, respeta los tiempos, lenguaje claro y no redundante |
| 0,5 | Calidad: editado (OBS/VSDC o similar), textos explicativos, **sin mostrar personas**: solo pantalla del PC y la Zybo en primer plano |

Partes del video: (1) diagrama de bloques en 1–3 slides, marcando **qué bloques inventó
el grupo y cuáles vienen de otras fuentes** (2–5 min); (2) lista con checks de las 10
actividades (30 s); (3) mostrar en Vivado/Vitis cada bloque y **dónde se implementó cada
una de las 10 actividades** (5–10 min); (4) funcionamiento en la Zybo (1–3 min).

**Código (30 %).** Nota = 1 + 0,5 (block design ordenado y **sin bloques basura**) + 0,75
(C/VHDL **comentado en detalle**, indicando qué archivos son del grupo) + 0,75 (uso
correcto de punteros, funciones con punteros y structs; −0,75 cada 3 errores) + la misma
escala de implementación del video (hasta 4).

**Informe (35 %).** LaTeX, plantilla IEEE Access del repo PROY02 (Overleaf), **máximo 8
páginas** con referencias, formato fijo (no agregar ni quitar secciones). Nota = 1 + puntos:

| Sección | Pts | Qué piden |
| :--- | :--- | :--- |
| Arquitectura de HW y SW | 1 | Figura clara del sistema (PowerPoint/Inkscape) |
| Actividades realizadas | 1,5 | Cada AO/AC con %, descripción, bloque donde se implementó y nivel de logro. Escala: 3 AO + 7 AC → 1,5 · 3 + 4–6 → 1,0 · 3 + 1–3 → 0,5 · ≥2 AO → 0,25 |
| **Resultados** | **3** | 0,75: validar el/los IP Core con ILA, VIO o testbench · 0,75: validar el C con **debug de Vitis, capturando una interrupción** · **2,0: debug de hardware con ILA/VIO de forma simultánea a la ejecución del código en el procesador** |
| Resultados de implementación | 0,1 | Máx. 2 párrafos: problemas, cuáles se resolvieron |
| Conclusiones | 0,2 | **Cuantitativas** (con datos y números), no un resumen |
| Trabajos futuros | 0,2 | Detalles técnicos de lo que falta |

> ⚠️ Los **2 puntos** de "ILA/VIO simultáneo con el procesador" son los más caros del
> informe. Hay que planificar desde ya una captura de ILA de una transacción real (por
> ejemplo, el AXI del IP de música o el SPI del ADC) mientras corre el juego.

---

## 3. El avance (evaluación intermedia)

**NAP1 = 0,3 · Diagrama + 0,35 · AO_a + 0,35 · AO_b.**

- El grupo **elige qué 2 AO** muestra. Una AO se puede reemplazar por 2 AC (excepto AC1 y
  AC2); en ese caso cada AC se evalúa con la rúbrica de AO y se promedian.
- **Diagrama (obligatorio). Nota 7,0:** diagrama completo, hecho en software, con **los
  datos que se transmiten escritos sobre las flechas**, **colores** que indiquen dónde se
  cumple **cada AO y AC del proyecto final** (no solo las del avance), y **qué corre en
  Vivado (VHDL) y qué en Vitis (C)**. Debe dar certeza de que el proyecto es realizable con
  lo visto en el curso. Hecho a mano, como máximo 6,0.
- **Cada AO. Nota 7,0:** Vivado y Vitis funcionando, coherente con el diagrama, **mostrado
  con periféricos (pantalla preferentemente), buzzer, ILA/VIO, etc., en la Zybo**. Sin
  periféricos visibles es 6,0; incoherente con el diagrama, 5,0.
- **Avance semanal (NAPS):** en cada PROG llaman de 3 a 5 grupos al azar: **PROG06 →
  AC4**, **PROG07 → AC5**, **PROG08 → AC6**. Para 7,0 hay que mostrar el código
  funcionando con testbench/simulación y/o VIO-ILA **y** en la Zybo con switches, botones
  o LEDs. Si a un grupo nunca lo llaman, NAPS = NAP1. **Nota del avance = promedio(NAP1, NAPS).**
- Fecha del avance: según la programación del curso; el grupo debe confirmarla en Canvas.

---

## 4. Nuestro proyecto: "Flappy por voz" (nombre tentativo)

Un pájaro que avanza entre tubos; **sube cuando el jugador aplaude o sopla** al
micrófono. Inspirado en Flappy Bird (hay que referenciarlo como base); el aporte del
grupo es el control por sonido y movimiento, el día/noche por luz y la música por IP propio.

| Act. | Implementación planificada | Dónde |
| :--- | :--- | :--- |
| AO1 | LCD = el juego + HUD. Sensores: **micrófono** (aleteo), **acelerómetro** (velocidad/inclinación), **luz** (día/noche). Registros modificados: **límite bajo 0x02 y alto 0x03** (y config 0x01) del OPT3001 | Vitis |
| AO2 | `axi_timer_0` = tick del juego 30 Hz · `axi_timer_1` = muestreo del ADC 200 Hz · INT del OPT3001 → `axi_gpio_luz` → modo noche | Vivado + Vitis |
| AO3 | IP de música propio (VHDL, AXI4-Lite) que maneja el buzzer: registros de nota/frecuencia, duración/tempo, volumen; configurado con datos pedidos por UART mediante una función con punteros | Vivado (VHDL) + Vitis |
| AC1 | MENÚ → JUGANDO → GAME OVER | Vitis |
| AC2 | `struct Juego/Pajaro/Tubo/Sensores`, arreglo de tubos, macros, funciones con punteros | Vitis |
| AC3 | Joystick (menú) + temperatura o LED RGB (no usados en AO1) | Vitis (+ Vivado si es LED RGB/botones) |
| AC4 | `BOOT.bin` en la QSPI (AYUD08) | Vitis |
| AC5 | ILA sobre una transacción (AXI del IP de música o SPI del ADC) + VIO para forzar entradas del IP | Vivado |
| AC6 | Canciones de menú / juego / game over con el IP de AO3 | VHDL + Vitis |
| AC7 | Récord guardado en la SD (`xilffs`) | Vitis (+ PS SD0 ya habilitado) |

Decisiones ya tomadas (no re-discutir salvo que se pida):

- **Flappy en vez de Duck Hunt**: menos redibujo en el LCD, sensores con sentido real
  y más originalidad (Duck Hunt ya lo hizo un grupo en 1S24).
- **2 bloques AXI Timer separados** (no los 2 contadores de un mismo bloque), así cada
  uno tiene su propia línea de interrupción y su handler.
- **Interrupción de luz por pin INT del OPT3001 → AXI GPIO con interrupción** (como en
  el ejemplo Snake), no por polling.
- Proyecto **fuera de OneDrive** (`C:\SEP\proyecto2`) y regenerable con scripts Tcl.

---

## 5. Hardware real

### Placas

- **Digilent Zybo Z7-10**: Zynq-7000 **xc7z010clg400-1**. Reloj externo de 125 MHz
  (pin K17). FCLK0 del PS para la lógica: **100 MHz**. 4 LEDs, 4 switches, botones,
  jumper **JP5** (modo de boot: JTAG / QSPI / SD). UART del PS = **UART1**
  (MIO 48–49), 115200 8N1, por el mismo micro-USB del JTAG.
- **BoosterPack BOOSTXL-EDUMKII** (TI) conectada a la Zybo con una **tarjeta adaptadora
  del curso** que agrega un **ADC SPI** (las señales analógicas de la Booster pasan por
  ese ADC). Periféricos:
  - **Pantalla TFT 128×128** color (RGB565) por SPI. El driver define `LCD_WIDTH/HEIGHT`
    en 170; usar 0–127 y verificar el área visible en la placa.
  - **Joystick** (2 ejes analógicos + botón), **acelerómetro** de 3 ejes (analógico),
    **micrófono** (analógico): todos por el ADC (SPI 1).
  - **OPT3001** (luz) y **TMP006** (temperatura) por **I2C**.
  - **Buzzer** (PWM), **LED RGB**, botones S1/S2.
- Datasheets: repo `REF-Documentacion` (copia local en `referencias/REF-Documentacion`).

### Block design `DEMO` (proyecto `P2_SEP`)

| Bloque | Origen | Base | IRQ (GIC) | Función |
| :--- | :--- | :--- | :--- | :--- |
| `processing_system7_0` | plantilla | – | – | ARM; UART1, SD0 y QSPI habilitados; `IRQ_F2P` habilitado por nosotros |
| `axi_quad_spi_0` | plantilla | (ver Address Editor) | – | Pantalla (MOSI, SCLK, CS) |
| `axi_quad_spi_1` | plantilla | (ver Address Editor) | – | ADC de la tarjeta adaptadora |
| `axi_gpio_0` | plantilla | (ver Address Editor) | – | RS y RST de la pantalla |
| `axi_iic_0` | plantilla | (ver Address Editor) | – | TMP006 `0x40`, OPT3001 `0x44` |
| `BUZZER_0` | plantilla (módulo RTL, **sin AXI**) | – | – | Buzzer con BTN0/BTN1/BTN_JY → 4 tonos y 4 volúmenes, LEDS |
| `axi_timer_0` | **grupo** | `0x4280_0000` | 61 | Tick del juego |
| `axi_timer_1` | **grupo** | `0x4281_0000` | 62 | Muestreo del ADC |
| `axi_gpio_luz` | **grupo** | `0x4121_0000` | 63 | 1 bit de entrada con interrupción: INT del OPT3001 |
| `xlconcat_0` | **grupo** | – | – | Junta las 3 interrupciones → `IRQ_F2P[2:0]` |

`IRQ_F2P[0..7]` corresponde a los IDs 61..68 del GIC. En C se usan
`XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR`, `XPAR_FABRIC_AXI_TIMER_1_INTERRUPT_INTR` y
`XPAR_FABRIC_AXI_GPIO_LUZ_IP2INTC_IRPT_INTR`.

### Pines (constraints/Zybo-Z7-Master.xdc)

| Señal | Pin | | Señal | Pin |
| :--- | :--- | --- | :--- | :--- |
| clk | K17 | | BUZZER_PWM | T14 |
| LEDS[0..3] | M14, M15, G14, D18 | | BTN0 / BTN1 / BTN_JY | T15 / P14 / R14 |
| SPI ADC (spi_rtl) | W14, Y14, T12, U12 | | I2C (iic) | V17, V18 |
| LCD MOSI / SCLK / CS | J15 / V13 / W16 | | LCD RS / RST | V12 / H15 |
| **interrupt_luz_tri_i** | **U15** | | | |

> ⚠️ **Los botones de la Booster hoy van al módulo `BUZZER`, no al procesador.** Para
> leerlos desde C hay que pasarlos a un AXI GPIO o meterlos en un IP con AXI.
>
> ⚠️ El INT del OPT3001 es **open-drain y activo en bajo**. Si en la placa la
> interrupción nunca llega, revisar el pull-up (esquemático de la Booster) y, si falta,
> poner `PULLUP true` en U15.

---

## 6. Software: drivers de la plantilla (`vitis_src/`)

Vienen del `Proyecto_Base_Booster_2025` (repo PROY02) y de AYUD10/AYUD11. **No son del
grupo**: referenciarlos así en el informe.

```c
/* ADC.h  (SPI 1)  -> valores de 10 bits */
int init_adc(XSpi *SpiInstancePtr, u16 SpiDeviceId);   /* init_adc(&SpiInstance1, SPI_DEVICE_ID_1) */
int read_joyx(); int read_joyy();
int read_acx();  int read_acy();  int read_acz();
int read_POT1(); int read_POT2(); int read_MIC();

/* I2C.h  (axi_iic_0) */
#define TMP_ADDR 0x40
#define OPT_ADDR 0x44
int init_IIC();
int read_tmp();     /* grados C enteros: registro 0x01 del TMP006 / 128 */
int read_opt();     /* OJO: escribe config 0xC410 en CADA llamada y devuelve el
                       registro 0x00 crudo (E[15:12] R[11:0]), no lux */
extern XIic iic;    /* definido en I2C.c; usar iic.BaseAddress con XIic_Send/XIic_Recv */

/* LCD_SPI.h / LCD_Driver.h / LCD_GUI.h  (SPI 0 + axi_gpio_0) */
int  XSpi_Init(XSpi *SpiPtr, u16 SpiDeviceId);           /* XSpi_Init(&SpiInstance, SPI_DEVICE_ID) */
void LCD_Init(LCD_SCAN_DIR Lcd_ScanDir);                 /* LCD_Init(SCAN_DIR_DFT) */
void LCD_Clear(COLOR Color);
void LCD_SetArealColor(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend, COLOR Color);
void GUI_DrawPoint(POINT X, POINT Y, COLOR C, DOT_PIXEL, DOT_STYLE);
void GUI_DrawLine(POINT X0, POINT Y0, POINT X1, POINT Y1, COLOR C, LINE_STYLE, DOT_PIXEL);
void GUI_DrawRectangle(POINT X0, POINT Y0, POINT X1, POINT Y1, COLOR C, DRAW_FILL, DOT_PIXEL);
void GUI_DrawCircle(POINT Xc, POINT Yc, LENGTH R, COLOR C, DRAW_FILL, DOT_PIXEL);
void GUI_Disbitmap(POINT X, POINT Y, const unsigned char *pBmp, POINT W, POINT H);
void GUI_DisString_EN(POINT X, POINT Y, const char *s, sFONT *Font, COLOR Fondo, COLOR Letra);
void GUI_DisNum(POINT X, POINT Y, int32_t n, sFONT *Font, COLOR Fondo, COLOR Letra);
/* Fuentes: Font8, Font12, Font16, Font20, Font24. Colores RGB565: WHITE BLACK BLUE RED
   GREEN CYAN YELLOW MAGENTA GRAY BROWN ... (LCD_GUI.h). DRAW_FULL / DRAW_EMPTY,
   DOT_PIXEL_1X1 ... */
extern XGpio gpio0;  extern XSpi SpiInstance;  extern XSpi SpiInstance1;

/* Delay.h */
void delay_ms(unsigned int ms);  /* tambien usleep() de "sleep.h" */
```

- Imágenes para el LCD: script `IMG_LCD_Booster.py` (repo PROY02) convierte un `.jpg` en
  un arreglo de píxeles; se agrega en `LCD_GUI.h` y se dibuja con `GUI_Disbitmap` (AYUD11).
- El LCD es **lento por SPI**: no hacer `LCD_Clear` por frame; redibujar solo lo que
  cambió (borrar la posición anterior con el color de fondo).

### `vitis_src/main.c` (nuestro, esqueleto)

Ya contiene: inits de LCD/ADC/I2C; `init_timers()` (cuenta descendente + auto-recarga,
`XTmrCtr_SetHandler`); `init_interrupciones()` con `XScuGic` (los timers conectados a
`XTmrCtr_InterruptHandler`, la luz a `handler_luz`); `opt3001_escribir_reg()`,
`opt3001_leer_reg()`, `opt3001_configurar()` y `opt3001_a_centilux()`; structs y la
máquina de estados. Los huecos están marcados `TODO(compa):`. **Mantener esos nombres.**

### OPT3001 (resumen útil)

- Registros: `0x00` Result, `0x01` Configuration, `0x02` Low Limit, `0x03` High Limit,
  `0x7E` Manufacturer ID, `0x7F` Device ID.
- Result/límites: `E[15:12]` exponente, `R[11:0]` mantisa → **lux = 0,01 · 2^E · R**.
- Config `0xC410` (la de la plantilla): rango automático (RN=0xC), conversión de 100 ms
  (CT=0), modo continuo (M=10), latch (L=1), INT activo en bajo (POL=0), FC=00 (1 falla).
  Leer el registro de config limpia el latch/flags. El ejemplo Snake usó `0xC418`.
- Con L=1 (window/latch) el INT se activa cuando la medición sale de [bajo, alto].

### TMP006 (resumen útil)

- Dirección `0x40`. `0x01` = temperatura local (die) en 14 bits ≪ 2, 1/32 °C por LSB →
  `°C = valor/128` (lo que hace `read_tmp`). `0x02` = config (la plantilla escribe
  `0x02`, `0x80`, `0x82`). Tiene pin DRDY (data ready) que podría usarse de interrupción.

---

## 7. Repo y flujo de trabajo

```
SEP-Proyecto2/
├─ scripts/01_crear_proyecto.tcl   plantilla -> vivado/P2_SEP.xpr (quita xdc roto, agrega el nuestro)
├─ scripts/02_hw_avance.tcl        hardware del grupo (timers, gpio_luz, concat)
├─ scripts/03_build.tcl            bitstream + timing + export XSA (con bitstream)
├─ scripts/04_vitis.tcl            plataforma + app "Hello World" en vitis/ + importa vitis_src/ + build
├─ constraints/Zybo-Z7-Master.xdc
├─ ip_repo/                         IP propios (BUZZER_1.0 = plantilla AXI vacia, base para el IP de musica)
├─ vitis_src/                       C: drivers de la plantilla + main.c
├─ plantilla/                       Proyecto_Base_Booster_2025 oficial (no se edita)
├─ docs/                            guias de instalacion
├─ LEEME_COMPANERO.md               plan, estado y pendientes
└─ CONTEXTO_IA.md                   este archivo
```

- `vivado/`, `vitis/` y `logs/` **no se suben** (se regeneran). Cambios al block design →
  agregarlos como Tcl en `02_hw_avance.tcl`.
- La app de Vitis se crea con plantilla **Hello World** (trae `platform.c/h`, que los
  drivers incluyen, y deja la UART como stdin/stdout); se borra `helloworld.c`.
- Para la entrega final hay que **exportar** los proyectos de Vivado (*File → Project →
  Archive*) y de Vitis (*File → Export*) en un `.zip`.

---

## 8. Qué se vio y qué no en el curso (para el punto de originalidad y para no salirse)

### Laboratorios

| Lab | Tema | Qué se aprendió / material clave |
| :--- | :--- | :--- |
| LAB01 | Vivado Hello World | Introducción a la Zybo Z7, primer diseño VHDL |
| LAB02 | Simulación | Testbenches en Vivado |
| LAB03 | IP Core propio | *Create and Package New IP* con AXI (`AXI_LED_Driver`), `slv_reg`, re-empaquetar |
| LAB04 | AXI Traffic Generator | Transacciones AXI de prueba sin procesador (archivos `.coe`) |
| LAB05 | Logic Debug | **ILA y VIO** (product guides), Hardware Manager, triggers |
| LAB06 | MicroBlaze | Softcore con periféricos AXI |
| LAB07 | Interrupciones | MicroBlaze + **AXI Interrupt Controller (`XIntc`)** + AXI GPIO con interrupción (`Example_1_INT_GPIO.c`) |
| LAB08 | Debug de MicroBlaze | Debug de código en el procesador, JTAG |
| LAB09 | UART y multiprocesador | UART PS↔PC, MicroBlaze + PS juntos, debug de más de un procesador (`LAB09_UART.c`) |
| LAB10 | Periféricos Zybo + Booster | Proyecto base con LCD, ADC, I2C y buzzer (`Proyecto_Base_Booster`, `ADC.c`); hardware de la tarjeta adaptadora |
| LAB11 | Por definir | Actividad de avance del Proyecto 2; estudiar datasheets de luz, temperatura y ADC (conceptos, no tablas de bits) |

### Ayudantías

| Ayud. | Tema | Qué se aprendió / material clave |
| :--- | :--- | :--- |
| AYUD00 | Instalación | Vivado/Vitis 2020.1 |
| AYUD01 | Memoria en VHDL | `entity`, `architecture`, `library` |
| AYUD02 | ALU | Tipos de datos y operadores VHDL |
| AYUD03 | Mini procesador | Control unit + ALU + RAM en VHDL |
| AYUD04 | ILA / VIO | VIO en vez de switches, ILA sobre buses, **trigger condicionado** (`finish == 1`) |
| AYUD05 | AXI RGB | IP con AXI esclavo que genera **PWM** para el LED RGB; ATG con `.coe`; ILA tipo *AXI monitor* |
| AYUD06 | Xil_In32 / Xil_Out32 | Drivers propios con lectura/escritura directa de registros; IP propio con mapa `CTRL/DUTY/SW/ID`, registros de **solo lectura** |
| AYUD07 | AXI Timer | MicroBlaze + `XIntc` + AXI Timer: interrupciones periódicas (`XTmrCtr_SetHandler`, `XTC_INT_MODE_OPTION`, `XTC_AUTO_RELOAD_OPTION`), **los 2 timers de un bloque**, **modo PWM** del AXI Timer (`test_pwm.c`) |
| AYUD08 | Boot | **Debug en Vitis** (breakpoints, *Expressions*, F6); `BOOT.bin` = FSBL + bitstream + ELF, *Program Flash* `qspi-x4-single`, JP5 en QSPI |
| AYUD09 | UART | Consola serial (115200 8N1), `scanf`, `inbyte`, `outbyte`, structs/punteros en C ("máquina expendedora") |
| AYUD10 | I2C + Buzzer | AXI IIC a 100 kHz, `read_tmp`/`read_opt`, IP `BUZZER` (4 tonos/amplitudes); alarma de temperatura y theremin: **agregar puerto AXI al BUZZER** para fijar la frecuencia desde el PS; lux = 0,01·2^E·R |
| AYUD11 | SPI | 2 AXI Quad SPI (pantalla y ADC) + AXI GPIO (RS/RST); drivers del LCD; **foto propia en el LCD** con el script de Python |

### Proyectos de ejemplo disponibles

- **PROY02-Ejemplo (Snake)**: Zynq PS + `XScuGic`, **2 AXI Timer + `axi_gpio_luz`
  (interrupción del OPT3001)**, IP `axi_musica` + `Cancion.vhd`, umbrales del OPT3001 en
  0x02/0x03. Es el ejemplo más parecido a nuestro hardware.
- Otros ejemplos de semestres anteriores (1S24, 2S24, 2S25) en la carpeta local
  `PROY02-otrosejemplos` (no están en este repo; incluyen un Duck Hunt de 1S24).

### Lo que NO se vio (candidatos al punto de originalidad, con referencia)

> Hay que verificar que de verdad no se vio y **citar la fuente** (app note de Xilinx,
> documentación, tutorial) en el video y el informe.

- **Tarjeta SD con `xilffs`** (FatFs) — es el AC7 "desafío", no hubo ayudantía.
- **`XScuGic` / interrupciones del PS en el Zynq** (el curso las enseñó con MicroBlaze y
  `XIntc`; el ejemplo Snake las usa, pero no hubo lab). Discutible como originalidad.
- **Timer privado del Cortex-A9 (`XScuTimer`)** o el watchdog (`XScuWdt`).
- **XADC** interno del Zynq (temperatura del chip / voltajes).
- **DMA** (AXI DMA, o el DMA del PS) para enviar bloques de píxeles a la pantalla.
- **Doble núcleo (AMP)**: el Zybo Z7-10 tiene 2 Cortex-A9; usar el segundo core.
- **Generación de números aleatorios por hardware** (LFSR en VHDL como IP).
- **Procesamiento del micrófono en VHDL** (detector de envolvente/aplauso en hardware).
- **PS GPIO/MIO** (botones de la Zybo por MIO) o **EMIO**.
- **HLS** (Vitis HLS) para generar un IP desde C.

---

## 9. Trampas conocidas (ya nos pasaron o están documentadas)

- La plantilla `.xpr` apuntaba a un `.xdc` inexistente fuera del zip; el script 01 lo
  quita y agrega el nuestro.
- Vivado 2020.1 se instala en `D:\Xilinx` en este PC; el instalador pide admin solo para
  los drivers del cable (`docs/instalar_cable_drivers.bat`).
- Rutas largas y OneDrive rompen las compilaciones de Vivado: proyecto en `C:\SEP\proyecto2`.
- Advertencias críticas esperables al crear el proyecto: `PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY`
  negativos (preset de la Zybo) y el `.xdc` faltante de la plantilla. No son errores.
- **La plantilla de Vitis no compilaba tal cual en 2020.1**; ya está arreglado en
  `vitis_src/` (documentarlo en el informe como modificación del grupo):
  1. `LCD_GUI.h`: la declaración del arreglo `intro` estaba duplicada
     (`static const int intro[128][128]= {` seguido de `static const uint16_t intro...`).
     Se borraron las 2 líneas sobrantes.
  2. `Delay.c`: `#include <unistd.h>` chocaba con `sleep.h` del BSP (`usleep`/`sleep`
     con tipos distintos). Se comentó ese include.
- `xsct` (`app build`) puede terminar "bien" aunque la app no compile: `04_vitis.tcl`
  revisa que exista el ELF.
- `read_opt()` reescribe la config del OPT3001 en cada lectura.
- El AXI GPIO interrumpe en ambos flancos: filtrar por el nivel del pin en el handler.
- `xil_printf` sin `float`; `\r\n`; `scanf(" %3s", texto)` para strings (AYUD09).
- Sin AXI en el `BUZZER` de la plantilla: la música por software requiere un IP nuevo
  (AO3).

---

## 10. Cómo pedirle cosas a la IA (ejemplos)

- "Con el contexto de CONTEXTO_IA.md, implementa `detectar_aleteo()` en
  `vitis_src/main.c` usando estos valores medidos: silencio 510±8, aplauso 900."
- "Escribe el VHDL de un IP AXI4-Lite `axi_musica` con 4 registros (CTRL, NOTA,
  DURACION, VOLUMEN) que genere el PWM del buzzer a partir de un reloj de 100 MHz, y el
  Tcl para agregarlo a `02_hw_avance.tcl`."
- "Revisa este `main.c` contra la rúbrica de código (punteros, structs, comentarios) y
  lista los errores de programación que restarían puntos."
- "Redacta la sección 'Actividades Realizadas' del informe con este estado: ..."
