# CONTEXT.md — Proyecto 2 IEE2463 SEP (Zybo Z7-10 + BoosterPack MKII)

Este archivo es contexto para un asistente de IA. Describe el curso, las reglas de
evaluación, el hardware real, lo que ya está construido, el código disponible para
reutilizar (con su origen, para citarlo), lo que se vio y lo que no se vio en el curso, y
el plan del avance y de la entrega final. Si una respuesta de la IA contradice este
archivo, este archivo manda.

---

## 0. Reglas para el asistente

1. **Idioma:** español de Chile. Comentarios de código en español, **sin tildes ni ñ
   dentro del código** (evita problemas de encoding en Vitis).
2. **Herramientas: Vivado y Vitis 2020.1, obligatorio.** No usar menús, comandos ni APIs
   de versiones posteriores (nada de Vitis Unified IDE 2023+, nada de flujos Python
   nuevos). Si una función de driver no existe en 2020.1, no proponerla.
3. **Procesador:** ARM Cortex-A9 del PS del Zynq, BSP *standalone*, sin sistema
   operativo. No proponer Linux, PetaLinux ni FreeRTOS. MicroBlaze solo si se pide.
4. **Interrupciones en el PS = `XScuGic`.** El curso enseñó interrupciones con MicroBlaze
   y `XIntc` (LAB07, AYUD07). Este proyecto usa `XScuGic` como el ejemplo Snake de
   PROY02. No mezclar ambos en el mismo diseño.
5. **Usar los drivers de `vitis_src/`** (plantilla del curso) para LCD, ADC, I2C y
   retardos. No escribir drivers nuevos de pantalla o ADC.
6. **No inventar** registros, macros `XPAR_*`, puertos ni nombres de IP. Los `XPAR_*`
   salen de `xparameters.h` (generado desde el `.xsa`). Si no hay certeza, decirlo e
   indicar cómo verificarlo.
7. **Estilo de código evaluado** (30 % de la nota): funciones con argumentos de puntero,
   `struct`, arreglos, `#define`, y **comentarios detallados en todo lo escrito o
   modificado por el grupo**. Cada 3 errores de programación se descuentan 0,75 puntos.
8. **Interrupciones cortas:** dentro de un handler solo banderas `volatile` o lecturas
   rápidas del ADC. Nada de LCD ni I2C en un handler.
9. `xil_printf` **no imprime `float`**: usar enteros escalados y `%d.%02d`. Salto de
   línea `\r\n`.
10. **Toda fuente externa se cita** (ver sección 4). Al entregar código o texto basado en
    algo del curso, de Xilinx, de un ejemplo, de internet o generado por IA, indicar la
    fuente exacta para el informe.
11. Preferir soluciones **simples y demostrables en la Zybo**. El objetivo del grupo es
    buena nota con el mínimo trabajo extra.
12. Si una propuesta cambia el hardware, describirla como **pasos en la GUI de Vivado**
    (el grupo edita `vivado/P2_SEP.xpr` por GUI y lo versiona). Opcionalmente, dar
    también los comandos Tcl equivalentes para `scripts/02_hw_avance.tcl` (respaldo para
    regenerar desde cero). Después de cambiar hardware: *Generate Bitstream* y *Export
    Hardware* (con bitstream) a `hw/P2_SEP.xsa`.
13. Al proponer código para `vitis_src/main.c`, **mantener los nombres existentes**
    (sección 8) y completar los bloques marcados `TODO`.

---

## 1. El curso y sus repositorios

- **IEE2463 Sistemas Electrónicos Programables**, Pontificia Universidad Católica de
  Chile, Depto. de Ingeniería Eléctrica, semestre **2026-2**. Profesor 2026-2: Michel
  Rozas. Ayudantes 2026-2: Pablo Uribe (coordinador), José Agustín Milling, Ernesto
  Ferrante, Andrés García. Comunicación oficial por Canvas; dudas como *issue* en el repo
  correspondiente.
- Material en la organización de GitHub **https://github.com/IEE2463-SEP**. La rama
  vigente es **`2026-2`** (es la rama por defecto); `main` es el semestre anterior y no
  se debe usar.
- Proyecto en parejas. Este es el **Proyecto 2**, el proyecto final.

| Tipo | Repositorio | Tema |
| :--- | :--- | :--- |
| Lab | [LAB01-Vivado-Hello-World](https://github.com/IEE2463-SEP/LAB01-Vivado-Hello-World) | Zybo Z7 y primer diseño VHDL |
| Lab | [LAB02-Vivado-Simulation](https://github.com/IEE2463-SEP/LAB02-Vivado-Simulation) | Testbenches |
| Lab | [LAB03-Vivado-IPCore](https://github.com/IEE2463-SEP/LAB03-Vivado-IPCore) | IP Core propio con AXI |
| Lab | [LAB04-AXI-ATG](https://github.com/IEE2463-SEP/LAB04-AXI-ATG) | AXI Traffic Generator |
| Lab | [LAB05-Logic-Debug](https://github.com/IEE2463-SEP/LAB05-Logic-Debug) | ILA y VIO |
| Lab | [LAB06-Microblaze](https://github.com/IEE2463-SEP/LAB06-Microblaze) | MicroBlaze con periféricos AXI |
| Lab | [LAB07-Processor-Interrupts](https://github.com/IEE2463-SEP/LAB07-Processor-Interrupts) | Interrupciones (MicroBlaze + AXI INTC) |
| Lab | [LAB08-Microblaze-Debug](https://github.com/IEE2463-SEP/LAB08-Microblaze-Debug) | Debug de código, JTAG |
| Lab | [LAB09-UART-Multiprocesador](https://github.com/IEE2463-SEP/LAB09-UART-Multiprocesador) | UART PS↔PC, MicroBlaze + PS |
| Lab | [LAB10-Peripherals-Zybo-Booster](https://github.com/IEE2463-SEP/LAB10-Peripherals-Zybo-Booster) | Periféricos de la Booster (proyecto base) |
| Lab | [LAB11-Por-Definir](https://github.com/IEE2463-SEP/LAB11-Por-Definir) | Actividad de avance del Proyecto 2 |
| Ayud. | [AYUD00-Instalacion-Software](https://github.com/IEE2463-SEP/AYUD00-Instalacion-Software) | Instalación Vivado/Vitis 2020.1 |
| Ayud. | [AYUD01-Memoria](https://github.com/IEE2463-SEP/AYUD01-Memoria) | Memoria en VHDL |
| Ayud. | [AYUD02-ALU](https://github.com/IEE2463-SEP/AYUD02-ALU) | ALU, tipos y operadores |
| Ayud. | [AYUD03-Procesador](https://github.com/IEE2463-SEP/AYUD03-Procesador) | Mini procesador en VHDL |
| Ayud. | [AYUD04-ILA-VIO](https://github.com/IEE2463-SEP/AYUD04-ILA-VIO) | ILA y VIO |
| Ayud. | [AYUD05-AXI-RGB](https://github.com/IEE2463-SEP/AYUD05-AXI-RGB) | IP AXI con PWM para LED RGB |
| Ayud. | [AYUD06-XilIn-XilOut](https://github.com/IEE2463-SEP/AYUD06-XilIn-XilOut) | `Xil_In32`/`Xil_Out32` e IP con mapa de registros |
| Ayud. | [AYUD07-AXI-Timer](https://github.com/IEE2463-SEP/AYUD07-AXI-Timer) | AXI Timer: interrupciones y PWM |
| Ayud. | [AYUD08-Boot](https://github.com/IEE2463-SEP/AYUD08-Boot) | Debug en Vitis y boot desde QSPI |
| Ayud. | [AYUD09-UART](https://github.com/IEE2463-SEP/AYUD09-UART) | Consola UART, structs y punteros |
| Ayud. | [AYUD10-I2C-Buzzer](https://github.com/IEE2463-SEP/AYUD10-I2C-Buzzer) | I2C (TMP006, OPT3001) y buzzer |
| Ayud. | [AYUD11-SPI](https://github.com/IEE2463-SEP/AYUD11-SPI) | SPI: pantalla y ADC |
| Proy. | [PROY02](https://github.com/IEE2463-SEP/PROY02) | Enunciado, rúbrica, plantilla de informe, proyecto base, `IMG_LCD_Booster.py` |
| Proy. | [PROY02-Ejemplo](https://github.com/IEE2463-SEP/PROY02-Ejemplo) | Ejemplo Snake (el más parecido a este proyecto) |
| Ref. | [REF-Documentacion](https://github.com/IEE2463-SEP/REF-Documentacion) | Datasheets Booster, ADC, Zybo, Zynq Book, MicroBlaze |
| Ref. | [REF-Lecturas](https://github.com/IEE2463-SEP/REF-Lecturas) | Lecturas del curso (Zynq/AXI, protocolos, CAN, ADC) |
| Ref. | [REF-Programacion-C](https://github.com/IEE2463-SEP/REF-Programacion-C) | Fundamentos de C |
| Ref. | [REF-Programacion-VHDL](https://github.com/IEE2463-SEP/REF-Programacion-VHDL) | Cápsulas de VHDL |

Repositorio del grupo: **https://github.com/lucasmartinfuentes/SEP-Proyecto2**.

---

## 2. Enunciado del Proyecto 2

La idea debe ser **de autoría del grupo**. Se puede basar en otra fuente (o en el
Proyecto 1) siempre que haya un **aporte sustantivo**, y hay que **especificar claramente
cuál fue el aporte y en qué proyecto se basó**. Se puede usar el ARM del PS, MicroBlaze o
ambos. Se puede usar todo el material entregado por el profesor y los ayudantes.

### Actividades obligatorias (AO), necesarias para aprobar

- **AO1:** Usar la **pantalla** de la Booster para mostrar información, figuras o
  imágenes que se **actualicen dinámicamente** y sean coherentes con el proyecto. Usar
  **al menos 3** de: sensor de luz, sensor de temperatura, acelerómetro, micrófono.
  Entender el propósito de los registros internos de los periféricos y **validar el
  funcionamiento modificando al menos dos registros** de algún periférico (pueden ser
  del mismo).
- **AO2:** Al menos **2 interrupciones generadas por timer(s) implementados en Vivado**,
  usadas para leer/escribir periféricos periódicamente. Además, **al menos una
  interrupción del sensor de temperatura o del sensor de luz**.
- **AO3:** Al menos **un IP Core** que interactúe con algún periférico **y** con el
  procesador. Una **función con argumentos de punteros** que inicialice y configure el
  hardware. Los **parámetros de configuración los entrega el usuario desde la consola del
  PC (UART)**.

### Actividades complementarias (AC), necesarias para 7,0

- **AC1:** Máquina de estados asociada al funcionamiento del proyecto.
- **AC2:** Estructuras, arreglos, punteros, funciones y macros en el código.
- **AC3:** 2 periféricos adicionales a los de AO1 (distintos): sensor de luz,
  temperatura, acelerómetro, LED RGB, micrófono, joystick, botones.
- **AC4:** Boot de la Zybo (el código queda cargado sin subirlo en cada encendido).
- **AC5:** VIO **e** ILA para interactuar con algún módulo y validar que la información
  enviada o recibida en una transacción es correcta. **Explicar las diferencias entre VIO
  e ILA.**
- **AC6:** Buzzer tocando música coherente y complementaria al proyecto.
- **AC7 (desafío):** Usar una memoria SD externa y hacer algo con ella.

### Formato de entrega final

- PDF en Canvas, nombre = número de grupo, que contiene **solo**: número de grupo, link
  de OneDrive con los archivos y link de YouTube del video.
- En el OneDrive: (1) informe PDF con el nombre del proyecto; (2) opcional, el video;
  (3) **un `.zip` con los proyectos de Vivado y Vitis exportados**, con toda la
  información de los IP Cores (validar que el compañero lo pueda abrir). `readme.txt`
  opcional.
- Más de 3 archivos, nombres o formato incorrectos, carpeta incorrecta: **−1 punto** en
  la nota final.
- Plazo: 23:59 del día de entrega. Atraso: −0,1 por cada 6 minutos durante las primeras
  2 horas; entre 2 y 12 horas la nota decae cuadráticamente hasta 1,0; pasadas 12 horas,
  nota 1,0. Atrasos se justifican en la Dipre.

---

## 3. Evaluación

### 3.1 Avance (NAP1)

**NAP1 = 0,3 · ND + 0,35 · NAO_a + 0,35 · NAO_b**

- El diagrama (ND) es obligatorio.
- El grupo elige qué 2 AO presenta. Solo una AO se puede reemplazar por 2 AC (no AC1 ni
  AC2); cada AC se evalúa con la rúbrica de AO y se promedian.

Rúbrica del diagrama:

| Nota | Criterio |
| :--- | :--- |
| 4,0–5,0 | Diagrama coherente que cumple parcialmente; actividades y datos entre bloques no identificados |
| 6,0 | Diagrama completo, con la información entre bloques y dónde se cumple cada AO y AC, **hecho a mano** |
| **7,0** | Diagrama completo; **datos escritos sobre las flechas**; **colores** (u otro recurso) que indiquen dónde se cumple **cada AO y AC del proyecto final**; indica **qué corre en Vivado (VHDL) y qué en Vitis (C)**; **hecho en software, prolijo**; con lo visto en clases, labs y ayudantías da certeza de que es realizable |

Rúbrica de cada AO presentada:

| Nota | Criterio |
| :--- | :--- |
| 1,0 | No hizo nada, no trajo la Zybo, o el proyecto lo tiene el compañero ausente |
| 2,0 | Puede explicar cómo implementarlo, pero Vivado está en fase inicial |
| 3,0 | Proyecto a medias que compila pero no hace lo necesario |
| 4,0 | Funciona de forma incompleta; identifica el error con ILA u otro medio y trabaja en resolverlo |
| 5,0 | Funciona, pero no es coherente con el diagrama |
| 6,0 | Funciona y es coherente, pero no puede mostrar los datos: sin periféricos activos, solo en Vivado/Vitis o parcialmente en la Zybo |
| **7,0** | Vivado y Vitis funcionando, coherente con el diagrama, y **demostrado con periféricos (pantalla preferentemente), buzzer, ILA o VIO, en la Zybo Z7** |

### 3.2 Avance semanal (NAPS)

En cada jornada PROG llaman de 3 a 5 grupos al azar. Lo esperado: **PROG05** nada (se
presenta el proyecto), **PROG06 → AC4**, **PROG07 → AC5**, **PROG08 → AC6**.

| Nota | Criterio |
| :--- | :--- |
| 1,0 | Ausente al ser llamado |
| 2,0 | Presente sin avance |
| 3,0 | Código incompleto, sin simulaciones completas |
| 5,0 | Código funcionando demostrado con testbench y simulaciones (y/o VIO e ILA) |
| 7,0 | Lo anterior **y** funcionamiento en la Zybo con switches, botones y/o LEDs |

Si el grupo no fue llamado, NAPS = NAP1. **Nota del avance = promedio(NAP1, NAPS).**

### 3.3 Nota final = Informe 35 % + Video 35 % + Código 30 %

Escala de implementación (se usa en video y en código):

| Puntos | Logro mostrado en la Zybo |
| :--- | :--- |
| 4 | 3 AO + 7 AC |
| 3 | 3 AO + 4–6 AC |
| 2 | 3 AO + 1–3 AC |
| 2 | 2 AO + 6 AC |
| 1 | 2 AO + menos de 6 AC |

**Video** (entre 5 y 15 minutos; sobre 15 se evalúa con nota mínima). Nota = 1 + puntos:
implementación (hasta 4) + **originalidad 1** (una funcionalidad de la Zybo/Zynq **no
vista** en clases, labs o ayudantías, **indicando las referencias de dónde se aprendió**)
+ coherencia 0,5 (tiene las partes y los tiempos, lenguaje claro, no redundante) + calidad
0,5 (editado con OBS/VSDC o similar, textos explicativos, **no muestra personas**, solo la
pantalla del PC y la Zybo en primer plano).

Partes del video:
1. Descripción general con diagrama de bloques en 1–3 slides, **identificando qué
   bloques inventó el grupo y cuáles se tomaron de sitios web, libros, el curso, librerías
   Xilinx u otros** (2–5 min).
2. Lista con checks de cuántas de las 10 actividades se lograron (30 s).
3. En Vivado/Vitis, los bloques y sus conexiones, y **dónde se implementó cada una de las
   10 actividades**; imágenes de simulación post-implementación si aplica (5–10 min).
4. Funcionamiento final en la Zybo/Booster (1–3 min).

**Código.** Nota = 1 + 0,5 (block design con **solo los bloques necesarios**, ordenado,
sin bloques basura ni conexiones innecesarias) + 0,75 (C y VHDL **comentados en
detalle**; indicar en informe y video qué archivos son del grupo) + 0,75 (uso correcto de
punteros, funciones con argumentos de puntero y structs; −0,75 cada 3 errores de
programación) + implementación (hasta 4).

**Informe.** LaTeX, plantilla IEEE Access del repo PROY02 (usar Overleaf), **máximo 8
páginas incluyendo referencias**, formato fijo (no agregar, mezclar ni quitar
secciones). Encabezado: fecha, título, nombres y correos, si autoriza publicar el
proyecto, abstract (máx. 250 palabras), *index terms*. Nota = 1 + puntos:

| Sección | Pts | Contenido |
| :--- | :--- | :--- |
| Arquitectura de Hardware y Software | 1 | Figura clara que resuma el proyecto (PowerPoint/Inkscape) y su explicación |
| Actividades realizadas | 1,5 | Por cada AO/AC: porcentaje, descripción, bloque de la arquitectura donde se implementó y nivel de logro. 3 AO + 7 AC → 1,5 · 3 AO + 4–6 AC → 1,0 · 3 AO + 1–3 AC → 0,5 · ≥2 AO → 0,25 |
| **Resultados** | **3** | 0,75: validar el/los IP Core con ILA, VIO o testbench · 0,75: validar el C con **debug, capturando una interrupción** · **2,0: debug de hardware con VIO o ILA de forma simultánea a la ejecución del código en el procesador** |
| Resultados de implementación | 0,1 | Máximo 2 párrafos: principales problemas, cuáles se resolvieron |
| Conclusiones | 0,2 | **Cuantitativas, con datos** ("se esperaban X datos en Y ciclos, llegaron A en B porque..."); no son un resumen |
| Trabajos futuros | 0,2 | Desafíos técnicos concretos para una siguiente versión |

Los 2 puntos de "ILA/VIO simultáneo con el procesador" son la parte más cara del informe;
se planifican desde el inicio (sección 10.6).

---

## 4. Integridad académica: qué hay que citar (el curso es estricto)

El enunciado dice que **no está prohibido** usar código de internet, ChatGPT, ayudantías,
laboratorios, clases ni otra fuente, **pero cualquier código, archivo, idea de proyecto,
proyecto de Vivado u otro elemento obtenido de otra fuente debe estar debidamente
referenciado**. Lo que no tenga referencia se asume de autoría propia; si después se
encuentra en internet sin referencia, **se considera plagio**. Rigen el Código de Honor UC
y el Código de Ética IEEE.

Lista de fuentes que este proyecto ya usa o probablemente usará, y que **deben aparecer
citadas** en el informe (sección Referencias) y en el video (slide del diagrama, bloques
"no inventados por el grupo"):

| Elemento | Fuente a citar |
| :--- | :--- |
| Idea base del juego | *Flappy Bird*, Dong Nguyen (.GEARS Studios), 2013. El aporte del grupo: control por micrófono, acelerómetro y luz, música por IP propio |
| Proyecto de Vivado base (LCD, ADC, I2C, BUZZER, constraints) | `Proyecto_Base_Booster_2025`, repo PROY02 / LAB10 del curso |
| Drivers C `LCD_SPI`, `LCD_Driver`, `LCD_GUI`, fuentes `font*.c` | Plantilla del curso (AYUD11 / PROY02); `LCD_GUI.h` indica que deriva de un driver de Waveshare, citarlo también |
| `ADC.c/h`, `I2C.c/h`, `Delay.c/h` | Plantilla del curso (LAB10, AYUD10) |
| `BUZZER.vhd` | Plantilla del curso (AYUD10 / PROY02) |
| Patrón de interrupciones del AXI Timer (`XTmrCtr_SetHandler`, opciones) | AYUD07 (`Timer_Ints.c`, basado en ejemplos de Xilinx) |
| `XScuGic` + 2 AXI Timer + `axi_gpio_luz`, umbrales del OPT3001 | Ejemplo Snake, repo PROY02-Ejemplo |
| IP de música (si se basa en `axi_musica` / `Cancion.vhd`) | Ejemplo Snake, repo PROY02-Ejemplo |
| Script de imágenes `IMG_LCD_Booster.py` | Repo PROY02 |
| Plantilla AXI4-Lite generada por Vivado (`slv_reg0..3`) | Xilinx, *Create and Package New IP* (LAB03) |
| Ejemplos de drivers Xilinx usados (p. ej. `xtmrctr_intr_example.c`, `xscugic_example`, `xilffs_polled_example.c`) | Xilinx embeddedsw, versión del BSP 2020.1 |
| Datasheets (OPT3001, TMP006, ADC, Booster) | Texas Instruments y documentación del curso (REF-Documentacion) |
| Código o texto generado con IA | Indicar la herramienta y para qué se usó |

Además:

- Las **correcciones del grupo a la plantilla** (ver sección 12) son modificaciones
  propias y se declaran como tales.
- **Comentar en detalle** y **listar en el informe y en el video** qué archivos fueron
  escritos o modificados por el grupo: hoy `vitis_src/main.c`, `vitis_src/LCD_GUI.h`
  (corrección), `vitis_src/Delay.c` (corrección), `scripts/*.tcl`,
  `constraints/Zybo-Z7-Master.xdc` (pin U15) y el futuro IP de música.
- Para el **punto de originalidad** hay que usar algo **no visto** y **citar de dónde se
  aprendió** (app note, documentación de Xilinx, tutorial). Sin referencia no cuenta.

---

## 5. Plan del avance (estructura detallada)

### 5.1 Qué se presenta

- **AO1 y AO2** (las dos AO elegidas) + **diagrama**. AO3 y las AC no son necesarias
  para el avance, pero **el diagrama sí debe mostrar las 10 actividades** del proyecto
  final.
- Todo **funcionando en la Zybo**, con la pantalla como evidencia principal y la consola
  UART como evidencia secundaria.

### 5.2 Diagrama (30 %): contenido mínimo para 7,0

Hecho en software (draw.io, PowerPoint o Inkscape), con dos zonas marcadas: **PL /
Vivado (VHDL, IP)** y **PS / Vitis (C en el ARM)**, más los periféricos físicos fuera de
la Zybo.

Bloques:

| Zona | Bloques |
| :--- | :--- |
| PS / Vitis | `main` + máquina de estados (MENÚ → JUGANDO → GAME OVER), handlers de interrupción (timer 0, timer 1, luz), lógica del juego, detector de aplauso, drivers LCD/ADC/I2C, configuración del IP de música por UART, lectura/escritura en SD, GIC |
| PL / Vivado | `axi_timer_0`, `axi_timer_1`, `axi_gpio_luz`, `xlconcat_0` → `IRQ_F2P`, `axi_quad_spi_0` (LCD), `axi_quad_spi_1` (ADC), `axi_iic_0`, `axi_gpio_0` (RS/RST), **IP de música** (AO3/AC6, propio), **ILA/VIO** (AC5), interconexión AXI |
| Externos | LCD 128×128, ADC de la tarjeta adaptadora, micrófono, acelerómetro, joystick, OPT3001, TMP006, buzzer, LED RGB o botones (AC3), PC (UART), memoria QSPI (AC4), tarjeta SD (AC7) |

Datos sobre las flechas (ejemplos de rótulos):

| Desde → hacia | Rótulo |
| :--- | :--- |
| `axi_quad_spi_0` → LCD | píxeles RGB565, comandos (SPI MOSI/SCLK/CS) |
| ADC → `axi_quad_spi_1` | canal + muestra 10 bits (mic, acx/acy/acz, joystick) |
| OPT3001 ↔ `axi_iic_0` | registros 0x00 resultado, 0x01 config, 0x02/0x03 límites (I2C 100 kHz) |
| OPT3001 → `axi_gpio_luz` | INT activo en bajo (pin U15) |
| `axi_timer_0/1`, `axi_gpio_luz` → `xlconcat_0` → GIC | IRQ 61 (30 Hz), 62 (200 Hz), 63 (luz) |
| PS ↔ periféricos AXI | AXI4-Lite (M_AXI_GP0) |
| PS → IP de música | período de nota 20 bits, volumen, enable (AXI4-Lite) |
| IP de música → buzzer | PWM |
| PC ↔ PS | parámetros de música: canción, tempo, volumen (UART 115200 8N1) |
| PS ↔ SD | récord (archivo FAT) |
| QSPI → PS | `BOOT.bin` = FSBL + bitstream + ELF |
| ILA ← bus AXI del IP de música / SPI del ADC | captura de la transacción |

Colores sugeridos (una leyenda en el diagrama): AO1 azul, AO2 rojo, AO3 verde, AC1
morado, AC2 gris, AC3 naranjo, AC4 café, AC5 rosado, AC6 verde claro, AC7 amarillo. Un
bloque puede tener borde de un color y relleno de otro si cumple dos actividades.

### 5.3 Demostración en la Zybo (guion de 3–5 minutos)

1. Mostrar el diagrama y señalar AO1 y AO2.
2. En Vivado: block design con `axi_timer_0`, `axi_timer_1`, `axi_gpio_luz` y
   `xlconcat_0` → `IRQ_F2P`; Address Editor; pin U15 en el `.xdc`.
3. En Vitis: `init_timers()`, `init_interrupciones()`, `handler_timers()`,
   `handler_luz()`, `opt3001_configurar()`.
4. En la Zybo:
   - **AO1:** el LCD muestra el juego o, como mínimo, una **pantalla de diagnóstico**
     con barras/valores en vivo de micrófono, acelerómetro y lux (y temperatura). El
     personaje sube con un aplauso. Por UART se imprime
     `OPT3001 config=0x.... lim_bajo=0x.... lim_alto=0x....`: son los registros
     escritos y leídos de vuelta (los 2 registros modificados).
   - **AO2:** un contador por segundo de interrupciones de cada timer (≈30 y ≈200) en
     pantalla o UART; tapar el sensor de luz cambia el fondo a "noche" e imprime
     `Interrupcion de luz: X.XX lux`.
5. Si algo falla, mostrar con UART o ILA dónde está el error (rúbrica 4,0).

### 5.4 Checklist técnico antes del avance

- [ ] Plantilla original funcionando en la Zybo (LCD + sensores).
- [ ] `main.c` cargado: timers interrumpiendo, registros del OPT3001 leídos de vuelta.
- [ ] Interrupción de luz llegando (si no llega: pull-up del INT, ver sección 6.4).
- [ ] Valores medidos: micrófono (silencio/aplauso), lux (destapado/tapado), acelerómetro.
- [ ] Umbrales `OPT_UMBRAL_BAJO/ALTO` y detector de aplauso calibrados.
- [ ] Pantalla dinámica (juego o diagnóstico).
- [ ] Diagrama exportado en PNG/PDF.
- [ ] Llevar la Zybo, la Booster, el cable y el proyecto en el PC.

---

## 6. Hardware real

### 6.1 Placas

- **Digilent Zybo Z7-10**: Zynq-7000 **xc7z010clg400-1** (2 × Cortex-A9 + FPGA). Reloj
  externo 125 MHz en K17. **FCLK0 del PS = 100 MHz** (reloj de todos los periféricos
  AXI). 4 LEDs, 4 switches, botones, jumper **JP5** de modo de boot (JTAG / QSPI / SD).
  UART del PS = **UART1** (MIO 48–49), 115200 8N1, por el mismo micro-USB del JTAG.
  Memoria QSPI y slot microSD (SD0) disponibles en el PS.
- **BoosterPack BOOSTXL-EDUMKII** (Texas Instruments), conectada con una **tarjeta
  adaptadora del curso** que agrega un **ADC por SPI** (las señales analógicas de la
  Booster pasan por ese ADC):
  - **Pantalla TFT 128×128** color RGB565 por SPI (el driver define `LCD_WIDTH/HEIGHT`
    en 170; usar 0–127 y verificar el área visible).
  - **Joystick** (2 ejes + botón), **acelerómetro** de 3 ejes, **micrófono**: analógicos,
    vía ADC (`axi_quad_spi_1`), 10 bits.
  - **OPT3001** (luz, I2C 0x44, pin INT open-drain) y **TMP006** (temperatura, I2C 0x40).
  - **Buzzer**, **LED RGB**, botones S1/S2.

### 6.2 Block design `DEMO` del proyecto `P2_SEP`

| Bloque | Origen | Base | IRQ GIC | Función |
| :--- | :--- | :--- | :--- | :--- |
| `processing_system7_0` | plantilla (+ `IRQ_F2P` habilitado por el grupo) | – | – | ARM, UART1, SD0, QSPI, M_AXI_GP0 |
| `ps7_0_axi_periph` | plantilla | – | – | Interconexión AXI |
| `axi_quad_spi_0` | plantilla | Address Editor | – | Pantalla |
| `axi_quad_spi_1` | plantilla | Address Editor | – | ADC |
| `axi_gpio_0` | plantilla | Address Editor | – | RS y RST de la pantalla |
| `axi_iic_0` | plantilla | Address Editor | – | TMP006 y OPT3001 |
| `BUZZER_0` | plantilla, módulo RTL **sin AXI** | – | – | Tono y volumen con BTN0/BTN1, silencio con BTN_JY, LEDS |
| `axi_timer_0` | **grupo** | `0x4280_0000` | 61 | Tick del juego (30 Hz) |
| `axi_timer_1` | **grupo** | `0x4281_0000` | 62 | Muestreo del ADC (200 Hz) |
| `axi_gpio_luz` | **grupo** | `0x4121_0000` | 63 | 1 bit de entrada con interrupción: INT del OPT3001 |
| `xlconcat_0` | **grupo** | – | – | 3 interrupciones → `IRQ_F2P[2:0]` |

`IRQ_F2P[7:0]` corresponde a los IDs 61–68 del GIC (y `[15:8]` a 84–91). En C:
`XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR`, `XPAR_FABRIC_AXI_TIMER_1_INTERRUPT_INTR`,
`XPAR_FABRIC_AXI_GPIO_LUZ_IP2INTC_IRPT_INTR`.

Resultados de compilación: bitstream generado, sin advertencias críticas, timing cumplido
en `clk_fpga_0` (100 MHz) y `sys_clk_pin` (125 MHz, del `BUZZER`) con WNS +2,447 ns, `.xsa`
con bitstream exportado. ELF de la app: 145 KB de código.

### 6.3 Pines (`constraints/Zybo-Z7-Master.xdc`)

| Señal | Pin | Señal | Pin |
| :--- | :--- | :--- | :--- |
| clk | K17 | BUZZER_PWM | T14 |
| LEDS[0..3] | M14, M15, G14, D18 | BTN0 / BTN1 / BTN_JY | T15 / P14 / R14 |
| ADC `spi_rtl` | W14, Y14, T12, U12 | I2C `iic` (SCL/SDA) | V17, V18 |
| LCD MOSI / SCLK / CS | J15 / V13 / W16 | LCD RS / RST | V12 / H15 |
| **`interrupt_luz_tri_i`** | **U15** | | |

### 6.4 Limitaciones conocidas del hardware actual

- **Botones de la Booster conectados al módulo `BUZZER`**, no al procesador. Para leerlos
  en C (AC3), pasarlos a un AXI GPIO o al IP de música.
- **El `BUZZER` de la plantilla no tiene AXI:** el procesador no puede elegir la nota.
  AO3/AC6 requieren un IP nuevo (sección 10.1).
- **INT del OPT3001 open-drain, activo en bajo.** Si la interrupción nunca llega, revisar
  en el esquemático de la Booster si hay pull-up; si no, agregar al `.xdc`:
  `set_property PULLUP true [get_ports {interrupt_luz_tri_i}]` y recompilar.
- **El AXI GPIO interrumpe en cada cambio del pin** (bajada y subida): el handler filtra
  por nivel.
- El LED RGB de la Booster no está ruteado en el `.xdc` actual; si se usa para AC3, hay
  que agregar pines y un GPIO o un IP PWM (como AYUD05).

---

## 7. Software disponible (drivers de la plantilla, en `vitis_src/`)

Origen: `Proyecto_Base_Booster_2025` (PROY02), AYUD10 y AYUD11. **No son del grupo.**

```c
/* ADC.h  -- axi_quad_spi_1. Cada read hace 2 transferencias SPI y devuelve 10 bits */
int init_adc(XSpi *SpiInstancePtr, u16 SpiDeviceId); /* init_adc(&SpiInstance1, SPI_DEVICE_ID_1) */
int read_joyx(); int read_joyy();
int read_acx();  int read_acy();  int read_acz();
int read_POT1(); int read_POT2(); int read_MIC();
/* Implementacion tipica (ADC.c): */
int read_MIC(){
    u8 WriteBuffer[2] = { 0x28, 0x28 };   /* palabra de control = canal del ADC */
    u8 ReadBuffer[2]  = { 0x28, 0x28 };
    u16 MIC;
    for (int i = 0; i < 2; i++) {         /* 1a transferencia elige canal, 2a lee */
        XSpi_Transfer(&SpiInstance1, WriteBuffer, ReadBuffer, 2);
        MIC = (ReadBuffer[0] + ReadBuffer[1] * 256) / 4;
    }
    return MIC;
}

/* I2C.h  -- axi_iic_0 */
#define TMP_ADDR 0x40
#define OPT_ADDR 0x44
int init_IIC();     /* inicializa XIic y configura el TMP006 */
int read_tmp();     /* registro 0x01 del TMP006 / 128 = grados C enteros */
int read_opt();     /* OJO: escribe config 0xC410 en CADA llamada y devuelve el registro
                       0x00 crudo (E[15:12] R[11:0]), no lux */
extern XIic iic;    /* definido en I2C.c: XIic_Send/XIic_Recv(iic.BaseAddress, ...) */

/* LCD (LCD_SPI.h, LCD_Driver.h, LCD_GUI.h)  -- axi_quad_spi_0 + axi_gpio_0 */
int  XSpi_Init(XSpi *SpiPtr, u16 SpiDeviceId);           /* XSpi_Init(&SpiInstance, SPI_DEVICE_ID) */
void LCD_Init(LCD_SCAN_DIR Lcd_ScanDir);                 /* LCD_Init(SCAN_DIR_DFT) */
void LCD_Clear(COLOR Color);
void LCD_SetArealColor(POINT X0, POINT Y0, POINT X1, POINT Y1, COLOR Color);
void GUI_DrawPoint(POINT X, POINT Y, COLOR C, DOT_PIXEL Dot, DOT_STYLE Fill);
void GUI_DrawLine(POINT X0, POINT Y0, POINT X1, POINT Y1, COLOR C, LINE_STYLE S, DOT_PIXEL Dot);
void GUI_DrawRectangle(POINT X0, POINT Y0, POINT X1, POINT Y1, COLOR C, DRAW_FILL F, DOT_PIXEL Dot);
void GUI_DrawCircle(POINT Xc, POINT Yc, LENGTH R, COLOR C, DRAW_FILL F, DOT_PIXEL Dot);
void GUI_Disbitmap(POINT X, POINT Y, const unsigned char *pBmp, POINT W, POINT H);
void GUI_DisString_EN(POINT X, POINT Y, const char *s, sFONT *Font, COLOR Fondo, COLOR Letra);
void GUI_DisNum(POINT X, POINT Y, int32_t n, sFONT *Font, COLOR Fondo, COLOR Letra);
void GUI_Show(void);  void GUI_INTRO(void);              /* pantallas de bienvenida */
/* Fuentes: Font8, Font12, Font16, Font20, Font24.
   Colores RGB565: WHITE BLACK BLUE RED GREEN CYAN YELLOW MAGENTA GRAY BROWN BRED GRED GBLUE
   Enums: DRAW_FULL/DRAW_EMPTY, DOT_PIXEL_1X1..8X8, DOT_FILL_AROUND, LINE_SOLID/LINE_DOTTED */
extern XGpio gpio0;  extern XSpi SpiInstance;  extern XSpi SpiInstance1;

/* Delay.h */
void delay_ms(unsigned int ms);       /* ademas usleep()/sleep() de "sleep.h" */
```

Uso de la pantalla, igual al `main.c` de la plantilla: para refrescar un número sin
`LCD_Clear`, se reescribe el texto anterior con el color de fondo y luego el nuevo:

```c
GUI_DisString_EN(50, 30, tmp, &Font12, GUI_BACKGROUND, GUI_BACKGROUND); /* borrar */
sprintf(tmp, "%d", read_tmp());
GUI_DisString_EN(50, 30, tmp, &Font12, GUI_BACKGROUND, YELLOW);         /* escribir */
```

Imágenes: `IMG_LCD_Booster.py` (repo PROY02, requiere `numpy` y `matplotlib`) convierte
un `.jpg` en un arreglo de píxeles; se agrega en `LCD_GUI.h` y se dibuja con
`GUI_Disbitmap` (AYUD11). Un sprite de 8×8 a 16×16 cuesta poco; una imagen de 128×128
ocupa 32 KB y tarda en dibujarse.

---

## 8. Código del grupo: `vitis_src/main.c` (esqueleto actual)

Compila sin errores ni warnings en Vitis 2020.1. **No probado en la Zybo.** Los huecos
están marcados `TODO`.

Macros principales:

```c
#define FREC_JUEGO_HZ     30          /* Timer 0 */
#define FREC_SENSORES_HZ  200         /* Timer 1 */
#define CARGA_TIMER(hz)   (AXI_CLK_HZ / (hz))   /* cuenta descendente */
#define OPT_REG_RESULT 0x00  OPT_REG_CONFIG 0x01  OPT_REG_LIM_BAJO 0x02  OPT_REG_LIM_ALTO 0x03
#define OPT_CONFIG        0xC410      /* TODO: decidir latch/polaridad/fault count */
#define OPT_UMBRAL_BAJO   0x0000      /* TODO: calibrar */
#define OPT_UMBRAL_ALTO   0xBFFF      /* TODO: calibrar */
#define PANTALLA_ANCHO 128, PANTALLA_ALTO 128, N_TUBOS 3, TUBO_ANCHO 12, HUECO_ALTO 40,
        PAJARO_X 30, PAJARO_TAM 8, GRAVEDAD 1, IMPULSO_ALETEO (-6), VELOCIDAD_TUBOS 2
```

Tipos:

```c
typedef enum { ESTADO_MENU = 0, ESTADO_JUGANDO, ESTADO_GAME_OVER } EstadoJuego;
typedef struct { int16_t y, y_ant, vel; } Pajaro;
typedef struct { int16_t x, x_ant, hueco_y; u8 contado; } Tubo;
typedef struct { int mic, acx, acy, acz, joy_x, joy_y, temp; u16 luz_raw; } Sensores;
typedef struct { EstadoJuego estado; Pajaro pajaro; Tubo tubos[N_TUBOS];
                 u32 puntaje, record; u8 modo_noche; } Juego;
```

Funciones (estado):

| Función | Estado | Qué hace |
| :--- | :--- | :--- |
| `init_perifericos()` | listo | GPIO0, SPI0, ADC, I2C, `LCD_Init` |
| `init_timers(XTmrCtr*, XTmrCtr*)` | listo | 2 timers, `XTC_INT_MODE_OPTION \| XTC_AUTO_RELOAD_OPTION \| XTC_DOWN_COUNT_OPTION` |
| `init_gpio_luz(XGpio*)` | listo | canal 1 como entrada |
| `init_interrupciones(XScuGic*, ...)` | listo | prioridades, `XScuGic_Connect`, habilitación, excepciones |
| `handler_timers(void*, u8)` | listo | timer 0 → `bandera_tick_juego`; timer 1 → `leer_sensores_adc()` |
| `handler_luz(void*)` | listo | limpia la interrupción; si el pin está en 0 → `bandera_evento_luz` |
| `opt3001_escribir_reg / opt3001_leer_reg` | listo | acceso a registros por I2C |
| `opt3001_configurar(config, bajo, alto)` | listo | escribe 0x02, 0x03, 0x01 y los imprime leídos de vuelta |
| `opt3001_a_centilux(u16)` | listo | `R << E` (centilux) |
| `leer_sensores_adc(volatile Sensores*)` | listo | mic, acelerómetro, joystick |
| `detectar_aleteo(const volatile Sensores*)` | **TODO** | umbral + antirrebote del micrófono |
| `juego_reiniciar(Juego*)` | casi listo | falta altura aleatoria de los huecos |
| `juego_actualizar(Juego*, const volatile Sensores*)` | **TODO** | física y tubos |
| `juego_hay_choque(const Juego*)` | **TODO** | colisiones |
| `manejar_evento_luz(Juego*)` | parcial | lee config (limpia latch), imprime lux, alterna `modo_noche` |
| `dibujar_menu / dibujar_juego / dibujar_game_over` | parcial / **TODO** / listo | dibujo incremental pendiente |

Flujo del `main`: inits → `opt3001_configurar` → (TODO: configuración UART del IP de
música) → menú → arranca timers → `while(1)`: si hay evento de luz lo atiende; en cada tick
del juego lee luz y temperatura cada 10 ticks (I2C) y ejecuta el `switch` de estados.

---

## 9. Qué se vio en cada laboratorio y ayudantía (detalle útil)

### LAB03 — IP Core con AXI ([repo](https://github.com/IEE2463-SEP/LAB03-Vivado-IPCore))

*Tools → Create and Package New IP → Create a new AXI4 peripheral*, interfaz S00_AXI
Lite con 4 registros `slv_reg0..3`. Se editan dos archivos: el top `X_v1_0.vhd` (puertos
del usuario) y `X_v1_0_S00_AXI.vhd` (lógica). Después *Re-Package IP* y *Upgrade IP* en el
block design. Archivos: `AXI_LED_Driver_v1_0.vhd`, `AXI_LED_Driver_v1_0_S00_AXI.vhd`,
`LED_Driver.vhd`. Lectura asociada: Zynq y AXI (REF-Lecturas).

### LAB04 — AXI Traffic Generator ([repo](https://github.com/IEE2463-SEP/LAB04-AXI-ATG))

Generar transacciones AXI hacia un IP sin procesador, con archivos `.coe` (direcciones,
datos, control, máscara). Útil como testbench de hardware de un IP.

### LAB05 + AYUD04 — ILA y VIO ([LAB05](https://github.com/IEE2463-SEP/LAB05-Logic-Debug), [AYUD04](https://github.com/IEE2463-SEP/AYUD04-ILA-VIO))

- **ILA** (*Integrated Logic Analyzer*): **observa** señales internas de la FPGA en
  tiempo real y las captura según un *trigger*. No modifica nada.
- **VIO** (*Virtual Input/Output*): **lee y escribe** señales desde el PC (Hardware
  Manager), como switches y LEDs virtuales.
- AYUD04: reemplazar switches/botones por un VIO, ILA sobre los buses de datos y
  direcciones de un mini procesador, y **trigger condicionado** (`finish == 1`, pestaña
  *Probe_Ports*) para capturar solo el ciclo de interés; trigger encadenado opcional.
- Flujo: agregar los IP `ILA`/`VIO` (o *Mark Debug* en nets y *Set Up Debug*), generar
  bitstream (queda un `.ltx` de probes), *Open Hardware Manager → Program Device* con el
  `.ltx`, configurar trigger, *Run Trigger*.
- Para AXI: AYUD05 usa un ILA de tipo **AXI monitor** entre el ATG y el IP.

### LAB06 — MicroBlaze ([repo](https://github.com/IEE2463-SEP/LAB06-Microblaze))

Softcore MicroBlaze con periféricos AXI. Este proyecto no usa MicroBlaze.

### LAB07 + AYUD07 — Interrupciones y AXI Timer ([LAB07](https://github.com/IEE2463-SEP/LAB07-Processor-Interrupts), [AYUD07](https://github.com/IEE2463-SEP/AYUD07-AXI-Timer))

Se vieron con **MicroBlaze + AXI Interrupt Controller (`XIntc`)**. Patrón de AYUD07
(`Timer_Ints.c`):

```c
XTmrCtr_Initialize(TmrCtrInstancePtr, DeviceId);
XIntc_Connect(IntcInstancePtr, IntrId, (XInterruptHandler)XTmrCtr_InterruptHandler,
              (void *)TmrCtrInstancePtr);           /* el driver limpia la interrupcion */
XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
XIntc_Enable(IntcInstancePtr, IntrId);
Xil_ExceptionInit();
Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)INTC_HANDLER,
                             IntcInstancePtr);
Xil_ExceptionEnable();
XTmrCtr_SetHandler(TmrCtrInstancePtr, TimerCounterHandler, TmrCtrInstancePtr);
XTmrCtr_SetOptions(TmrCtrInstancePtr, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
XTmrCtr_SetResetValue(TmrCtrInstancePtr, 0, RESET_VALUE);
XTmrCtr_Start(TmrCtrInstancePtr, 0);

void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber) { /* cada interrupcion */ }
```

En este proyecto el mismo patrón va con `XScuGic` (`XScuGic_LookupConfig`,
`XScuGic_CfgInitialize`, `XScuGic_SetPriorityTriggerType`, `XScuGic_Connect`,
`XScuGic_Enable`, y `XScuGic_InterruptHandler` como handler de excepción), tal como en
el ejemplo Snake.

Sin `XTC_DOWN_COUNT_OPTION` el timer cuenta hacia arriba e interrumpe al desbordar:
período = (2³² − RESET_VALUE) / f_clk. Con cuenta descendente, período = RESET_VALUE /
f_clk (lo que usa `main.c`).

AYUD07 también vio:
- **Los 2 contadores de un mismo AXI Timer** (*Enable Timer 2*), LEDs a 2 Hz y 3 Hz.
  Restricción del ejercicio: las frecuencias debían salir del timer, **no de un contador
  en C**.
- **Modo PWM del AXI Timer** (`test_pwm.c`), que usa los 2 contadores (período y tiempo
  en alto):

```c
XTmrCtr_PwmDisable(TmrCtrInstancePtr);
DutyCycle = XTmrCtr_PwmConfigure(TmrCtrInstancePtr, Period, HighTime); /* en ns */
XTmrCtr_PwmEnable(TmrCtrInstancePtr);
```

### LAB08 + AYUD08 — Debug y Boot ([LAB08](https://github.com/IEE2463-SEP/LAB08-Microblaze-Debug), [AYUD08](https://github.com/IEE2463-SEP/AYUD08-Boot))

- **Debug en Vitis:** *Debug As → Launch Hardware*, breakpoints, *Step Over* (F6),
  ventana *Expressions* para ver variables. Sirve para el punto del informe "validar el C
  con debug, capturando una interrupción" (breakpoint dentro de `handler_luz` o
  `handler_timers`).
- **Boot desde QSPI (AC4):** *Create Boot Image* con **FSBL + bitstream + ELF** (en ese
  orden; el FSBL va primero porque configura el PS, carga el bitstream y luego salta a la
  aplicación); *Xilinx → Program Flash*, *Flash Type* `qspi-x4-single`; **JP5 en QSPI**;
  probar sin PC. La plataforma `P2_plat` ya genera el FSBL (`zynq_fsbl`).
- AYUD08 exigió explícitamente macros, arreglos y funciones con firma dada
  (`int LeerIntento(int anterior); void Parpadear(int patron, int veces, int semiperiodo_ms);`).
  Ese es el estilo que se espera para AC2.

### LAB09 + AYUD09 — UART ([LAB09](https://github.com/IEE2463-SEP/LAB09-UART-Multiprocesador), [AYUD09](https://github.com/IEE2463-SEP/AYUD09-UART))

- UART1 del PS a 115200 8N1; terminal de Vitis o CoolTerm. App creada desde **Hello
  World** (deja la UART como stdin/stdout).
- LAB09: driver `XUartPs` (ejemplo polled de Xilinx, `LAB09_UART.c`) y debug de
  MicroBlaze + PS a la vez.
- AYUD09: "máquina expendedora" por consola con `struct Producto`, búsqueda por código,
  menú con `inbyte()` (sin Enter), `scanf` de dos valores en una línea, `outbyte` para
  imprimir puntos. Detalles que se enseñaron:

```c
char opcion = inbyte();              /* una tecla, sin Enter; ignorar '\r' y '\n' */
scanf(" %3s", codigo);               /* string: sin & y con espacio inicial */
scanf(" %3s %d", codigo, &cantidad); /* dos valores en una linea: "B2 5" */
xil_printf("Texto\r\n");             /* sin %f */
```

### LAB10 + AYUD11 — Booster, SPI, pantalla y ADC ([LAB10](https://github.com/IEE2463-SEP/LAB10-Peripherals-Zybo-Booster), [AYUD11](https://github.com/IEE2463-SEP/AYUD11-SPI))

- 2 AXI Quad SPI (pantalla con MOSI/SCLK/CS; ADC), AXI GPIO de 2 canales para RS/RST.
  Configurar los bloques igual que el video para que los drivers funcionen sin cambios.
- Drivers `LCD_SPI`, `LCD_Driver`, `LCD_GUI`, `ADC`; `main.c` que muestra joystick,
  acelerómetro, potenciómetros, micrófono, temperatura y luz en el LCD.
- Ejercicio AYUD11: **foto propia en el LCD** con `IMG_LCD_Booster.py`.
- LAB10 cubrió también el hardware de la tarjeta adaptadora y el funcionamiento de un ADC
  (lectura 10), y la lectura de CAN (lectura 9).

### AYUD10 — I2C y buzzer ([repo](https://github.com/IEE2463-SEP/AYUD10-I2C-Buzzer))

- AXI IIC con SCL a **100 kHz**; `read_tmp()` y `read_opt()`; datasheets TMP006 y
  OPT3001 en el repo.
- IP/módulo `BUZZER` (4 tonos con BTN1, 4 amplitudes con BTN0, silencio con BTN_JY).
  Código de la plantilla (reutilizable como núcleo del IP de música):

```vhdl
-- BUZZER.vhd (plantilla del curso): PWM cuyo periodo es FRECUENCIA ciclos de reloj
process (clk) begin
  if rising_edge(clk) then
    if (BTN_JY = '1') then counter_PWM <= (others => '0');       -- silencio
    elsif (counter_PWM <= FRECUENCIA) then counter_PWM <= counter_PWM + 1;
    else counter_PWM <= (others => '0');
    end if;
  end if;
end process;
FRECUENCIA <= X"98968" when (counter_BTN1 = b"11") else X"2625A" when (counter_BTN1 = b"10") else
              X"12DDC" when (counter_BTN1 = b"01") else X"0F424";
AMPLITUD   <= shift_right(FRECUENCIA,1) when (counter_BTN0 = b"11") else  -- 50 %
              shift_right(FRECUENCIA,2) when (counter_BTN0 = b"10") else  -- 25 %
              shift_right(FRECUENCIA,3) when (counter_BTN0 = b"01") else  -- 12,5 %
              shift_right(FRECUENCIA,4);                                  -- 6,25 %
BUZZER_PWM <= '1' when (AMPLITUD >= counter_PWM) else '0';
```

- **Actividad de la ayudantía:** alarma de temperatura, **agregando un puerto AXI al
  `BUZZER`** para que la frecuencia la fije el PS (`Alertar()`; ej. 18–26 °C → 200–4000 Hz
  lineal, duty 50 %).
- **Problema propuesto:** theremin con el OPT3001: `read_opt()` en lux con
  `lux = 0,01 · 2^E · R`; reemplazar `sleep(1)` por un retardo coherente con el tiempo de
  conversión de la config `0xC410`; función `Theremin()` (5–300 lux → 2000–200 Hz inverso).
- Conclusión importante: **"IP de buzzer con AXI configurado desde el PS" ya se vio en
  AYUD10**, así que AO3 no da originalidad por sí solo; el aporte está en la secuencia
  musical, la configuración por UART y su integración con el juego.

### AYUD05 — IP AXI con PWM ([repo](https://github.com/IEE2463-SEP/AYUD05-AXI-RGB))

`RGB_driver`: IP AXI que compara registros contra un contador diente de sierra para
generar PWM en el LED RGB; ATG con `.coe`; ILA tipo AXI monitor. Lógica de usuario:

```vhdl
process(clk) begin
  if rising_edge(clk) then
    if (counter_PWM < PWM_COUNTER_MAX) then counter_PWM <= counter_PWM + 1;
    else counter_PWM <= (others => '0'); end if;
  end if;
end process;
Buff <= unsigned(slv_reg0) when (sw = b"00") else unsigned(slv_reg1) when (sw = b"01") else
        unsigned(slv_reg2) when (sw = b"10") else unsigned(slv_reg3);
```

Ejercicio: 16 colores de una tabla con LUT por switches y parpadeo; reloj de 125 MHz en
ese diseño (en este proyecto el reloj AXI es 100 MHz).

### AYUD06 — `Xil_In32` / `Xil_Out32` ([repo](https://github.com/IEE2463-SEP/AYUD06-XilIn-XilOut))

Acceso directo a registros, sin drivers:

```c
#include "xil_io.h"
Xil_Out32(XPAR_LED_DRIVER_0_S00_AXI_BASEADDR + 0,  valor0);   /* slv_reg0 */
Xil_Out32(XPAR_LED_DRIVER_0_S00_AXI_BASEADDR + 4,  valor1);   /* slv_reg1 */
u32 sw = Xil_In32(XPAR_AXI_GPIO_0_BASEADDR);                  /* GPIO canal 1, dato */
Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR + 8, sw);                  /* GPIO canal 2, dato */
```

Ejercicio: IP propio con mapa `CTRL 0x00` (EN), `DUTY 0x04` (R/G/B en 8 bits c/u), `SW
0x08` (solo lectura), `ID 0x0C` = `0x24632026` (solo lectura). Se enseñó a **modificar la
lógica de lectura generada por Vivado para que un registro sea de solo lectura** y a
verificar la dirección base leyendo `ID` al arrancar. Buen patrón para el IP de música.

### Ejemplo Snake — PROY02-Ejemplo ([repo](https://github.com/IEE2463-SEP/PROY02-Ejemplo))

El ejemplo más cercano: Zynq PS + `XScuGic`, **2 AXI Timer + `axi_gpio_luz`** (mismos
nombres que este proyecto), música con IP `axi_musica` + `Cancion.vhd`, nivel de
dificultad pedido por `scanf`, umbrales del OPT3001:

```c
u8 config3[3] = {0x01, 0xC4, 0x18};  /* config (la plantilla usa 0xC410) */
u8 config[3]  = {0x03, 0x48, 0x9A};  /* limite alto: E=4, R=0x89A -> 352,32 lux */
u8 config1[3] = {0x02, 0x1A, 0x9A};  /* limite bajo: E=1, R=0xA9A -> 54,28 lux */
XIic_Send(iic.BaseAddress, OPT_ADDR, config3, 3, XIIC_STOP);
```

(Los comentarios originales del ejemplo dicen otros valores en lux; los de arriba son el
cálculo con `lux = 0,01·2^E·R`.)

`Cancion.vhd` del Snake: ROM de 16 notas (período en ciclos de un reloj de 125 MHz, p.
ej. `x"4DD9E"` = 318 878 ciclos ≈ 392 Hz = G4) que avanza cada 0,3 s; `axi_musica` toma
esa frecuencia, usa `slv_reg0` como pausa y genera el PWM con duty 50 %. Si se reutiliza,
**citarlo** y recalcular los períodos para **100 MHz**.

Otros proyectos de semestres anteriores (1S24, 2S24, 2S25; entre ellos un Duck Hunt de
1S24) están en una carpeta local del grupo, fuera de este repo.

---

## 10. Bloques reutilizables y propuestas para lo que falta

> Las propuestas de esta sección **no están probadas**. Al usarlas, citar su base.

### 10.1 IP de música con AXI (AO3 + AC6)

Mapa de registros propuesto (AXI4-Lite, 4 registros de 32 bits, reloj de 100 MHz):

| Offset | Registro | Acceso | Descripción |
| :--- | :--- | :--- | :--- |
| `0x00` | `CTRL` | L/E | bit 0 `EN` (1 = suena) |
| `0x04` | `PERIODO` | L/E | período de la nota en ciclos de 100 MHz (20 bits; 0 = silencio) |
| `0x08` | `VOLUMEN` | L/E | 0–3: duty 50 %, 25 %, 12,5 %, 6,25 % (como el `BUZZER` de la plantilla) |
| `0x0C` | `ID` | L | constante, para verificar la dirección base (patrón AYUD06) |

Lógica de usuario en `axi_musica_v1_0_S00_AXI.vhd` (puerto nuevo `BUZZER_PWM : out std_logic`):

```vhdl
signal counter_PWM : unsigned(19 downto 0) := (others => '0');
signal periodo     : unsigned(19 downto 0);
signal amplitud    : unsigned(19 downto 0);
...
periodo <= unsigned(slv_reg1(19 downto 0));
process (S_AXI_ACLK) begin
  if rising_edge(S_AXI_ACLK) then
    if (slv_reg0(0) = '0') or (periodo = 0) then counter_PWM <= (others => '0');
    elsif (counter_PWM < periodo) then counter_PWM <= counter_PWM + 1;
    else counter_PWM <= (others => '0'); end if;
  end if;
end process;
with slv_reg2(1 downto 0) select amplitud <=
  shift_right(periodo,1) when "00", shift_right(periodo,2) when "01",
  shift_right(periodo,3) when "10", shift_right(periodo,4) when others;
BUZZER_PWM <= '1' when (slv_reg0(0) = '1') and (counter_PWM < amplitud) else '0';
```

Lado C (función con punteros, configurada por UART):

```c
typedef struct { u32 base; u8 volumen; u16 tempo_ms; u8 cancion; } Musica;

/* AO3: inicializa y configura el IP con parametros pedidos por UART */
int musica_configurar_uart(Musica *m)
{
    int v, t, c;
    xil_printf("Volumen (0-3): ");  scanf("%d", &v);
    xil_printf("Tempo ms (100-600): "); scanf("%d", &t);
    xil_printf("Cancion (0-2): ");  scanf("%d", &c);
    if (v < 0 || v > 3 || t < 100 || t > 600 || c < 0 || c > 2) return XST_FAILURE;
    m->volumen = (u8)v;  m->tempo_ms = (u16)t;  m->cancion = (u8)c;
    Xil_Out32(m->base + 0x08, m->volumen);
    Xil_Out32(m->base + 0x00, 1);
    return XST_SUCCESS;
}
void musica_tocar_nota(const Musica *m, u32 periodo) { Xil_Out32(m->base + 0x04, periodo); }
```

La secuencia de notas se puede avanzar desde el tick del timer 0 (cada `tempo_ms`), con
un arreglo de notas por canción (AC2) y una canción por estado (AC1/AC6). Así el IP solo
genera el tono y el C decide la melodía.

Períodos a 100 MHz (`periodo = 100e6 / f`; el máximo de 20 bits da f mínima ≈ 95 Hz):

| Nota | f (Hz) | Período | Hex |
| :--- | :--- | :--- | :--- |
| C4 | 261,63 | 382 219 | `0x5D50B` |
| D4 | 293,66 | 340 530 | `0x53232` |
| E4 | 329,63 | 303 370 | `0x4A10A` |
| F4 | 349,23 | 286 344 | `0x45E88` |
| G4 | 392,00 | 255 102 | `0x3E47E` |
| A4 | 440,00 | 227 273 | `0x377C9` |
| B4 | 493,88 | 202 478 | `0x316EE` |
| C5 | 523,25 | 191 113 | `0x2EA89` |
| D5 | 587,33 | 170 262 | `0x29916` |
| E5 | 659,25 | 151 688 | `0x25088` |
| G5 | 783,99 | 127 553 | `0x1F241` |
| A5 | 880,00 | 113 636 | `0x1BBE4` |

Integración en Vivado: crear el IP en `ip_repo/` (*Create and Package New IP*), agregarlo
al block design, conectar `S00_AXI` con *Run Connection Automation*, exportar
`BUZZER_PWM` al pin **T14** y **quitar** el `BUZZER_0` de la plantilla (mismo pin; además
el block design no debe tener bloques basura). Al quitar `BUZZER_0` también deja de
usarse el reloj de 125 MHz (`clk`, K17): borrar su `create_clock` y su pin del `.xdc`.
Hecho por GUI; opcionalmente reflejarlo como Tcl en `02_hw_avance.tcl`.

### 10.2 Codificar umbrales del OPT3001

```c
/* lux -> registro E[15:12] R[11:0] (lux = 0.01 * 2^E * R). Entero: centilux */
static u16 opt3001_codificar(u32 centilux)
{
    u16 e = 0;
    while (centilux > 0x0FFF && e < 11) { centilux >>= 1; e++; }
    return (u16)((e << 12) | (centilux & 0x0FFF));
}
/* Ejemplos: 5 lux -> 0x01F4 · 10 -> 0x03E8 · 20 -> 0x07D0 · 50 -> 0x19C4 ·
             100 -> 0x29C4 · 200 -> 0x39C4 */
```

Config `0xC410` = RN `0xC` (rango automático), CT 0 (100 ms), M `10` (continuo), L 1
(latch/ventana), POL 0 (INT activo en bajo), ME 0, FC `00` (1 medición fuera de rango
basta). Con latch, el INT queda activo hasta **leer el registro de configuración**.

### 10.3 Detector de aplauso (idea)

```c
#define MIC_REPOSO        512   /* medir en silencio */
#define MIC_UMBRAL        150   /* medir con aplauso */
#define ALETEO_BLOQUEO    6     /* ticks de 33 ms sin volver a detectar */
/* En leer_sensores_adc (200 Hz): guardar el maximo de |mic - MIC_REPOSO| desde el
   ultimo tick del juego. En detectar_aleteo (30 Hz): si ese maximo > MIC_UMBRAL y no hay
   bloqueo -> 1, reiniciar maximo y bloqueo. */
```

### 10.4 Redibujo incremental

- Pájaro: rectángulo con el color del fondo en `y_ant`, rectángulo del pájaro en `y`.
- Tubos: cada tick se mueven `VELOCIDAD_TUBOS` px; pintar solo la franja nueva del borde
  izquierdo y borrar la franja del borde derecho (no el tubo completo).
- HUD con `GUI_DisNum` reescribiendo solo cuando cambia el puntaje.
- Cambio día/noche: un solo `LCD_Clear` al cambiar de modo.

### 10.5 SD (AC7) — no visto en el curso: citar

En Vitis: *Board Support Package Settings* del `standalone_domain` → habilitar la
librería **`xilffs`**. SD0 ya está habilitado en el PS. Código basado en
`xilffs_polled_example.c` de Xilinx (citar):

```c
#include "ff.h"
static FATFS fs;
int record_guardar(u32 record)
{
    FIL f; UINT n; char buf[16];
    if (f_mount(&fs, "0:/", 1) != FR_OK) return XST_FAILURE;
    if (f_open(&f, "record.txt", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) return XST_FAILURE;
    int len = sprintf(buf, "%lu", (unsigned long)record);
    f_write(&f, buf, len, &n);
    f_close(&f);
    return (n == (UINT)len) ? XST_SUCCESS : XST_FAILURE;
}
```

### 10.6 ILA/VIO simultáneo con el procesador (AC5 + 2 puntos del informe)

- Poner un **System ILA** (o *Mark Debug* en la interfaz `S00_AXI` del IP de música y
  *Run Connection Automation*) para capturar las escrituras AXI a `PERIODO`/`VOLUMEN`.
- Un **VIO** con salidas hacia el IP (por ejemplo forzar `EN` o una nota de prueba) y
  entradas para leer el estado.
- Procedimiento para la captura simultánea: correr la app desde Vitis (Run o Debug), y
  en Vivado *Hardware Manager* (conectado al mismo target) armar el trigger en
  `AWADDR == offset de PERIODO` y `AWVALID == 1`; al cambiar de nota en el juego queda
  capturada la transacción. Captura de pantalla → informe.
- Explicar en informe y video: ILA solo observa (captura con trigger), VIO lee y escribe
  señales desde el PC.

### 10.7 AC3 (2 periféricos fuera de AO1)

AO1 usa micrófono, acelerómetro y luz. Opciones para AC3: **joystick** (ya legible por
ADC, para el menú) + **temperatura** (ya legible por I2C, por ejemplo cambia la dificultad
o se muestra en el HUD). Ambas sin tocar el hardware. Botones o LED RGB requieren cambios
en Vivado (sección 6.4).

---

## 11. Qué NO se vio en el curso (originalidad y citas obligatorias)

Revisión de los repositorios de labs y ayudantías de 2026-2. Lo que aparece aquí **no se
enseñó** (o solo aparece en un ejemplo de proyecto, no en clases), así que **si se usa,
se cita la fuente** de donde se aprendió; además puede valer el punto de originalidad.

| Funcionalidad | Situación | Fuente a citar si se usa |
| :--- | :--- | :--- |
| Tarjeta SD con `xilffs` (FatFs) | No vista; es el AC7 "desafío" | Xilinx `xilffs` (UG/ejemplo `xilffs_polled_example.c`), FatFs (ChaN) |
| Interrupciones del PS con `XScuGic` | No hubo lab (se vio con `XIntc`); aparece en el ejemplo Snake | Xilinx `xscugic` examples, Zynq TRM (UG585) cap. GIC; ejemplo Snake |
| Timer privado del Cortex-A9 (`XScuTimer`), watchdog (`XScuWdt`) | No visto | Xilinx `xscutimer_intr_example.c`, UG585 |
| XADC interno (temperatura del chip, voltajes) | No visto | Xilinx `xadcps` examples, UG480 |
| DMA (AXI DMA o DMA del PS) | No visto | PG021 (AXI DMA), UG585 |
| Segundo núcleo ARM (AMP) | No visto (LAB09 fue MicroBlaze + PS) | Xilinx XAPP1079 |
| PS GPIO por MIO/EMIO (botones de la Zybo) | No visto | Xilinx `xgpiops` examples, manual Zybo Z7 |
| Interrupción GPIO por cualquier flanco en AXI GPIO | Visto parcialmente (LAB07 con XIntc) | PG144 (AXI GPIO) |
| LFSR / números aleatorios en hardware | No visto | Cualquier referencia de LFSR (p. ej. XAPP052) |
| Procesamiento del micrófono en VHDL (envolvente, detector de aplauso) | No visto | Referencia del método usado |
| HLS (Vitis HLS) | No visto | UG902/UG1399 |
| Doble buffer / framebuffer en BRAM para la pantalla | No visto | Referencia del método |
| `XTC_DOWN_COUNT_OPTION` del AXI Timer | No visto explícitamente (AYUD07 usó cuenta ascendente) | PG079 (AXI Timer), driver `xtmrctr` |

Lo que **sí se vio** (no da originalidad): VHDL, testbench, IP AXI propio, ATG, ILA/VIO,
MicroBlaze, `XIntc`, AXI Timer (interrupciones, 2 contadores, PWM), debug en Vitis, boot
QSPI, `Xil_In32/Out32`, UART por consola, I2C con TMP006/OPT3001, buzzer con puerto AXI,
SPI con LCD y ADC, imágenes en el LCD.

Candidatos razonables de originalidad para este proyecto, en orden de costo: **SD con
`xilffs`** (cubre AC7 a la vez), interrupciones del OPT3001 configuradas por registros con
`XScuGic` (discutible, está en el ejemplo Snake), detector de aplauso en VHDL, `XScuTimer`.

---

## 12. Trampas conocidas

- **Plantilla de Vitis rota en 2020.1** (ya corregido en `vitis_src/`, declararlo en el
  informe como modificación del grupo):
  1. `LCD_GUI.h` declaraba dos veces el arreglo `intro`
     (`static const int intro[128][128]= {` seguido de `static const uint16_t intro...`).
     Se borraron las 2 líneas sobrantes.
  2. `Delay.c` incluía `<unistd.h>`, que choca con `sleep.h` del BSP (`usleep`/`sleep` con
     tipos distintos). Se comentó el include.
- `xsct ... app build` puede terminar sin error aunque la app no compile;
  `04_vitis.tcl` revisa que exista el ELF.
- La `.xpr` de la plantilla apuntaba a un `.xdc` inexistente fuera del zip; el script 01 lo
  quita y agrega el `.xdc` del grupo.
- Advertencias críticas esperables: `PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY_*` negativos (preset
  de la Zybo) y el `.xdc` faltante de la plantilla. No son errores.
- OneDrive y rutas largas rompen la compilación: el proyecto vive en `C:\SEP\proyecto2`.
- `read_opt()` reescribe la config del OPT3001 en cada lectura.
- `scanf` bloquea: pedir los parámetros por UART antes de arrancar los timers o en el
  estado MENÚ.
- El `.xdc` de la plantilla no restringía el reloj de 125 MHz (`create_clock` comentado y
  con el puerto `sysclk` en vez de `clk`): el `BUZZER` quedaba sin análisis de timing
  (24 registros sin reloj, TIMING-17). Ya está corregido.
- Advertencias restantes, de la plantilla e inofensivas: CKLD-2 (reloj de 125 MHz sin
  BUFG), BUFC-1 (entradas SPI del ADC sin uso), TIMING-18 (pines sin input/output delay).
- Si se edita el block design por GUI, `regenerar_vivado.bat` (scripts 01/02) ya no
  reproduce ese diseño salvo que los cambios se copien a `02_hw_avance.tcl`.

---

## 13. Repo y flujo de trabajo

```
SEP-Proyecto2/
├─ README.md                       estado, plan y cómo trabajar
├─ CONTEXT.md                      este archivo
├─ vivado/P2_SEP.xpr               proyecto de Vivado (se edita por GUI; se versionan sus fuentes)
├─ hw/P2_SEP.xsa                   hardware exportado con bitstream (lo usa Vitis)
├─ abrir_vivado.bat                doble clic: abre el proyecto en la GUI
├─ crear_vitis.bat                 doble clic: workspace vitis/ desde hw/P2_SEP.xsa + build
├─ regenerar_vivado.bat            respaldo: recrea vivado/ desde la plantilla con scripts
├─ scripts/01_crear_proyecto.tcl   plantilla -> vivado/P2_SEP.xpr (solo para regenerar)
├─ scripts/02_hw_avance.tcl        hardware del grupo (timers, gpio_luz, concat)
├─ scripts/03_build.tcl            bitstream + timing + export a hw/P2_SEP.xsa
├─ scripts/04_vitis.tcl            plataforma + app Hello World + importa vitis_src/ + build
├─ constraints/Zybo-Z7-Master.xdc
├─ ip_repo/                        IP propios (BUZZER_1.0: plantilla AXI vacía)
├─ vitis_src/                      C: drivers de la plantilla + main.c
├─ plantilla/                      Proyecto_Base_Booster_2025 oficial (no se edita)
└─ docs/                           guías de instalación
```

Flujo normal (GUI, sin scripts):

- Hardware: `abrir_vivado.bat` → editar → *Generate Bitstream* → *File → Export → Export
  Hardware* (*Include bitstream*) a `hw/P2_SEP.xsa` → commit de `vivado/` y `hw/`.
- Software: `crear_vitis.bat` (o en Vitis: *Platform Project* con `hw/P2_SEP.xsa`, app
  *Hello World*, borrar `helloworld.c`, importar `vitis_src/`). Si cambia el `.xsa`:
  clic derecho en `P2_plat` → *Update Hardware Specification*.

- De `vivado/` se versionan solo las fuentes (`.xpr`, `.bd`, wrapper, `.xci`, VHDL);
  Vivado regenera lo demás. `vitis/` y `logs/` no se versionan; el código C vive en
  `vitis_src/`.
- La app se crea desde la plantilla **Hello World** (trae `platform.c/h`, que los drivers
  incluyen) y se borra `helloworld.c`.
- Entrega final: *File → Project → Archive* en Vivado y *File → Export* en Vitis, en un
  `.zip` con los IP incluidos.

---

## 14. Ejemplos de pedidos

- "Implementa `detectar_aleteo()` en `vitis_src/main.c` con estos valores medidos:
  silencio 505–520, aplauso hasta 900."
- "Escribe el VHDL del IP `axi_musica` según la sección 10.1 y los comandos Tcl para
  agregarlo a `02_hw_avance.tcl`, reemplazando `BUZZER_0`."
- "Revisa `main.c` contra la rúbrica de código (punteros, structs, comentarios) y lista
  lo que restaría puntos."
- "Redacta la sección 'Actividades Realizadas' del informe con este estado: ..., citando
  las fuentes de la sección 4."
