/* =============================================================================
 *  main.c  --  Proyecto 2 IEE2463 SEP  --  "Flappy por voz" (nombre tentativo)
 * -----------------------------------------------------------------------------
 *  ESTADO: ESQUELETO TENTATIVO. Tiene la estructura completa, pero la logica del
 *  juego esta por hacer. Cada hueco esta marcado con  TODO(compa):  (Ctrl+F).
 *  No se ha compilado ni probado en la Zybo todavia.
 *
 *  Que ya esta escrito aca (infraestructura, no depende del juego):
 *    - Inicializacion de LCD (SPI0), ADC (SPI1) e I2C (TMP006 + OPT3001).
 *      Drivers: los de la plantilla del curso (LCD_*, ADC, I2C, Delay).
 *    - AO2: 2 AXI Timer con interrupcion + interrupcion del sensor de luz
 *      (pin INT del OPT3001 -> axi_gpio_luz -> IRQ_F2P -> GIC).
 *      Patron de AYUD07 (XTmrCtr_SetHandler) adaptado al GIC del Zynq (XScuGic),
 *      como en el ejemplo Snake (PROY02-Ejemplo).
 *    - AO1: funciones para leer/escribir registros del OPT3001 (0x01, 0x02, 0x03).
 *    - AC1/AC2: maquina de estados + structs + funciones con punteros (vacias).
 *
 *  Regla de este archivo: las interrupciones SOLO levantan banderas o leen el
 *  ADC (rapido). Todo lo lento (I2C, dibujar en el LCD) va en el while del main.
 *
 *  Hardware: ver README.md (direcciones e IDs de interrupcion).
 * ============================================================================= */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xtmrctr.h"
#include "xgpio.h"
#include "xiic.h"

#include "Delay.h"
#include "LCD_SPI.h"
#include "LCD_Driver.h"
#include "LCD_GUI.h"
#include "ADC.h"
#include "I2C.h"

/* ---------------------------------------------------------------------------
 *  Instancias que viven en los drivers de la plantilla
 * --------------------------------------------------------------------------- */
extern XGpio gpio0;          /* RS/RST de la pantalla (LCD_Driver.c)     */
extern XSpi  SpiInstance;    /* SPI 0: pantalla                          */
extern XSpi  SpiInstance1;   /* SPI 1: ADC (joystick, acelerometro, mic) */
extern XIic  iic;            /* I2C: TMP006 (0x40) y OPT3001 (0x44)      */

/* ---------------------------------------------------------------------------
 *  Macros de hardware (salen de xparameters.h, generado desde el .xsa)
 * --------------------------------------------------------------------------- */
#define GIC_DEVICE_ID        XPAR_SCUGIC_SINGLE_DEVICE_ID
#define TMR0_DEVICE_ID       XPAR_AXI_TIMER_0_DEVICE_ID
#define TMR1_DEVICE_ID       XPAR_AXI_TIMER_1_DEVICE_ID
#define GPIO_LUZ_DEVICE_ID   XPAR_AXI_GPIO_LUZ_DEVICE_ID

#define TMR0_INTR_ID         XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR     /* 61 */
#define TMR1_INTR_ID         XPAR_FABRIC_AXI_TIMER_1_INTERRUPT_INTR     /* 62 */
#define LUZ_INTR_ID          XPAR_FABRIC_AXI_GPIO_LUZ_IP2INTC_IRPT_INTR /* 63 */

#define AXI_CLK_HZ           XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ   /* 100 MHz (FCLK0) */
#define GPIO_LUZ_CANAL       1
#define GPIO_LUZ_MASK        XGPIO_IR_CH1_MASK

/* Frecuencias de los timers. Con cuenta descendente, carga = reloj / frecuencia */
#define FREC_JUEGO_HZ        30      /* Timer 0: un "tick" del juego (fisica + dibujo) */
#define FREC_SENSORES_HZ     200     /* Timer 1: muestreo del ADC (mic, acelerometro)  */
#define CARGA_TIMER(hz)      (AXI_CLK_HZ / (hz))

/* ---------------------------------------------------------------------------
 *  OPT3001 (sensor de luz) -- registros (datasheet en REF-Documentacion)
 *    0x00 Result | 0x01 Configuration | 0x02 Low Limit | 0x03 High Limit
 *  Formato de 0x00/0x02/0x03: E[15:12] exponente, R[11:0] mantisa,
 *  lux = 0.01 * 2^E * R
 * --------------------------------------------------------------------------- */
#define OPT_REG_RESULT       0x00
#define OPT_REG_CONFIG       0x01
#define OPT_REG_LIM_BAJO     0x02
#define OPT_REG_LIM_ALTO     0x03

/* TODO(compa): definir la configuracion y los umbrales de la interrupcion de luz.
 *   - OPT_CONFIG: 0xC410 es la que usa la plantilla (rango auto, 100 ms, continuo,
 *     latch). Revisar en el datasheet los bits L (latch), POL (polaridad) y FC
 *     (cuantas lecturas fuera de rango antes de disparar) y decidir.
 *   - Umbrales: medir cuantos lux marca el sensor tapado y destapado en el lab y
 *     elegir el limite bajo (para "tapar = modo noche"). Codificar en formato E/R.
 *   Estos son los "2 registros modificados" de AO1 -> mostrarlo en el video.
 *   OJO: read_opt() de I2C.c reescribe 0xC410 en la config en cada lectura. Aca no
 *   se usa read_opt(); se lee con opt3001_leer_reg(). */
#define OPT_CONFIG           0xC410
#define OPT_UMBRAL_BAJO      0x0000  /* TODO(compa) */
#define OPT_UMBRAL_ALTO      0xBFFF  /* TODO(compa) */

/* ---------------------------------------------------------------------------
 *  Parametros del juego (pantalla de la Booster: 128 x 128 px, colores RGB565)
 * --------------------------------------------------------------------------- */
#define PANTALLA_ANCHO       128
#define PANTALLA_ALTO        128
#define N_TUBOS              3
#define TUBO_ANCHO           12
#define HUECO_ALTO           40
#define PAJARO_X             30
#define PAJARO_TAM           8

/* TODO(compa): ajustar la fisica en la placa (unidades: px por tick a 30 Hz) */
#define GRAVEDAD             1
#define IMPULSO_ALETEO       (-6)
#define VELOCIDAD_TUBOS      2

#define COLOR_CIELO_DIA      CYAN
#define COLOR_CIELO_NOCHE    BLACK
#define COLOR_TUBO           GREEN
#define COLOR_PAJARO         YELLOW

/* ---------------------------------------------------------------------------
 *  Tipos (AC1: maquina de estados, AC2: structs)
 * --------------------------------------------------------------------------- */
typedef enum {
    ESTADO_MENU = 0,
    ESTADO_JUGANDO,
    ESTADO_GAME_OVER
} EstadoJuego;

typedef struct {
    int16_t y;          /* posicion actual (px)                      */
    int16_t y_ant;      /* posicion del frame anterior (para borrar) */
    int16_t vel;        /* velocidad vertical (px/tick, + = cae)     */
} Pajaro;

typedef struct {
    int16_t x;          /* borde izquierdo (px)                      */
    int16_t x_ant;
    int16_t hueco_y;    /* y donde empieza el hueco                  */
    u8      contado;    /* 1 si ya sumo punto                        */
} Tubo;

typedef struct {
    int mic;            /* ADC 10 bits                               */
    int acx, acy, acz;  /* ADC 10 bits                               */
    int joy_x, joy_y;   /* ADC 10 bits                               */
    int temp;           /* grados C (TMP006, read_tmp)               */
    u16 luz_raw;        /* registro 0x00 del OPT3001 sin procesar    */
} Sensores;

typedef struct {
    EstadoJuego estado;
    Pajaro      pajaro;
    Tubo        tubos[N_TUBOS];
    u32         puntaje;
    u32         record;
    u8          modo_noche;  /* lo cambia la interrupcion de luz */
} Juego;

/* ---------------------------------------------------------------------------
 *  Variables globales
 *  'volatile' porque las escriben las interrupciones y las lee el main.
 * --------------------------------------------------------------------------- */
static XScuGic gic;
static XTmrCtr timer0;
static XTmrCtr timer1;
static XGpio   gpio_luz;

static volatile u8 bandera_tick_juego = 0;  /* Timer 0             */
static volatile u8 bandera_evento_luz = 0;  /* INT del OPT3001     */
static volatile Sensores sensores;          /* lo llena el Timer 1 */

static Juego juego;

/* ---------------------------------------------------------------------------
 *  Prototipos
 * --------------------------------------------------------------------------- */
/* Inicializacion */
static int init_perifericos(void);
static int init_timers(XTmrCtr *t0, XTmrCtr *t1);
static int init_gpio_luz(XGpio *g);
static int init_interrupciones(XScuGic *g, XTmrCtr *t0, XTmrCtr *t1, XGpio *gl);

/* Handlers (AO2) */
static void handler_timers(void *ref, u8 n_contador);
static void handler_luz(void *ref);

/* OPT3001 (AO1: registros) */
static int  opt3001_escribir_reg(u8 reg, u16 valor);
static int  opt3001_leer_reg(u8 reg, u16 *valor);
static int  opt3001_configurar(u16 config, u16 lim_bajo, u16 lim_alto);
static u32  opt3001_a_centilux(u16 raw);

/* Sensores */
static void leer_sensores_adc(volatile Sensores *s);
static int  detectar_aleteo(const volatile Sensores *s);

/* Juego (AC1 + AC2) */
static void juego_reiniciar(Juego *j);
static void juego_actualizar(Juego *j, const volatile Sensores *s);
static int  juego_hay_choque(const Juego *j);
static void manejar_evento_luz(Juego *j);

/* Dibujo (AO1: pantalla) */
static void dibujar_menu(const Juego *j);
static void dibujar_juego(const Juego *j);
static void dibujar_game_over(const Juego *j);

/* =============================================================================
 *  main
 * ============================================================================= */
int main(void)
{
    u32 ticks = 0;

    init_platform();
    xil_printf("\r\n=== Proyecto 2 SEP: arranque ===\r\n");

    if (init_perifericos() != XST_SUCCESS)            return XST_FAILURE;
    if (init_gpio_luz(&gpio_luz) != XST_SUCCESS)      return XST_FAILURE;
    if (init_timers(&timer0, &timer1) != XST_SUCCESS) return XST_FAILURE;
    if (init_interrupciones(&gic, &timer0, &timer1, &gpio_luz) != XST_SUCCESS)
        return XST_FAILURE;

    /* AO1: escribir registros del OPT3001 y leerlos de vuelta (evidencia) */
    if (opt3001_configurar(OPT_CONFIG, OPT_UMBRAL_BAJO, OPT_UMBRAL_ALTO) != XST_SUCCESS)
        xil_printf("OPT3001: fallo la configuracion\r\n");

    /* TODO(compa): AO3 -- aca va la configuracion del IP de musica por UART
     *   (pedir tempo/volumen/cancion con scanf o inbyte y escribirlo en los
     *   registros del IP con una funcion que reciba punteros). Todavia no existe
     *   el IP con AXI: el BUZZER de la plantilla es un modulo sin AXI. */

    juego_reiniciar(&juego);
    juego.estado = ESTADO_MENU;
    dibujar_menu(&juego);

    XTmrCtr_Start(&timer0, 0);
    XTmrCtr_Start(&timer1, 0);

    while (1) {

        /* --- Evento del sensor de luz (AO2) ------------------------------- */
        if (bandera_evento_luz) {
            bandera_evento_luz = 0;
            manejar_evento_luz(&juego);
        }

        /* --- Un tick del juego (Timer 0) ---------------------------------- */
        if (!bandera_tick_juego) continue;
        bandera_tick_juego = 0;
        ticks++;

        /* I2C es lento: leer luz y temperatura solo 3 veces por segundo */
        if ((ticks % 10) == 0) {
            u16 raw;
            if (opt3001_leer_reg(OPT_REG_RESULT, &raw) == XST_SUCCESS)
                sensores.luz_raw = raw;
            sensores.temp = read_tmp();
        }

        switch (juego.estado) {   /* AC1: maquina de estados */

        case ESTADO_MENU:
            /* Empieza con un aplauso. TODO(compa): aceptar tambien joystick/boton */
            if (detectar_aleteo(&sensores)) {
                juego_reiniciar(&juego);
                juego.estado = ESTADO_JUGANDO;
                LCD_Clear(juego.modo_noche ? COLOR_CIELO_NOCHE : COLOR_CIELO_DIA);
            }
            break;

        case ESTADO_JUGANDO:
            juego_actualizar(&juego, &sensores);
            if (juego_hay_choque(&juego)) {
                if (juego.puntaje > juego.record) juego.record = juego.puntaje;
                juego.estado = ESTADO_GAME_OVER;
                dibujar_game_over(&juego);
                /* TODO(compa): AC6 cambiar la musica; AC7 guardar record en SD */
            } else {
                dibujar_juego(&juego);
            }
            break;

        case ESTADO_GAME_OVER:
            /* TODO(compa): volver al menu tras un aplauso o unos segundos */
            break;
        }
    }

    cleanup_platform();
    return 0;
}

/* =============================================================================
 *  Inicializacion
 * ============================================================================= */

/* LCD + ADC + I2C, igual que el main de la plantilla del curso */
static int init_perifericos(void)
{
    if (XGpio_Initialize(&gpio0, XPAR_AXI_GPIO_0_DEVICE_ID) != XST_SUCCESS) {
        xil_printf("GPIO 0 (pantalla) fallo\r\n");
        return XST_FAILURE;
    }
    if (XSpi_Init(&SpiInstance, SPI_DEVICE_ID) != XST_SUCCESS) {
        xil_printf("SPI 0 (pantalla) fallo\r\n");
        return XST_FAILURE;
    }
    if (init_adc(&SpiInstance1, SPI_DEVICE_ID_1) != XST_SUCCESS) {
        xil_printf("SPI 1 (ADC) fallo\r\n");
        return XST_FAILURE;
    }
    if (init_IIC() != XST_SUCCESS) {
        xil_printf("I2C fallo\r\n");
        return XST_FAILURE;
    }
    LCD_Init(SCAN_DIR_DFT);
    LCD_Clear(GUI_BACKGROUND);
    xil_printf("Perifericos OK\r\n");
    return XST_SUCCESS;
}

/* Dos AXI Timer separados (cada uno con su linea de interrupcion), en cuenta
 * descendente con auto-recarga: interrumpen cada CARGA_TIMER(hz) ciclos. */
static int init_timers(XTmrCtr *t0, XTmrCtr *t1)
{
    const u32 opciones = XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION | XTC_DOWN_COUNT_OPTION;

    if (XTmrCtr_Initialize(t0, TMR0_DEVICE_ID) != XST_SUCCESS) return XST_FAILURE;
    XTmrCtr_SetHandler(t0, handler_timers, t0);
    XTmrCtr_SetOptions(t0, 0, opciones);
    XTmrCtr_SetResetValue(t0, 0, CARGA_TIMER(FREC_JUEGO_HZ));

    if (XTmrCtr_Initialize(t1, TMR1_DEVICE_ID) != XST_SUCCESS) return XST_FAILURE;
    XTmrCtr_SetHandler(t1, handler_timers, t1);
    XTmrCtr_SetOptions(t1, 0, opciones);
    XTmrCtr_SetResetValue(t1, 0, CARGA_TIMER(FREC_SENSORES_HZ));

    return XST_SUCCESS;
}

/* GPIO de 1 bit, solo entrada: pin INT del OPT3001 (U15) */
static int init_gpio_luz(XGpio *g)
{
    if (XGpio_Initialize(g, GPIO_LUZ_DEVICE_ID) != XST_SUCCESS) return XST_FAILURE;
    XGpio_SetDataDirection(g, GPIO_LUZ_CANAL, 0x1);   /* 1 = entrada */
    return XST_SUCCESS;
}

/* GIC del Zynq: conecta las 3 fuentes y habilita las excepciones del ARM */
static int init_interrupciones(XScuGic *g, XTmrCtr *t0, XTmrCtr *t1, XGpio *gl)
{
    XScuGic_Config *cfg = XScuGic_LookupConfig(GIC_DEVICE_ID);
    if (cfg == NULL) return XST_FAILURE;
    if (XScuGic_CfgInitialize(g, cfg, cfg->CpuBaseAddress) != XST_SUCCESS) return XST_FAILURE;

    /* Prioridad: menor numero = mas prioritaria. 0x3 = flanco de subida. */
    XScuGic_SetPriorityTriggerType(g, LUZ_INTR_ID,  0x98, 0x3);
    XScuGic_SetPriorityTriggerType(g, TMR0_INTR_ID, 0xA0, 0x3);
    XScuGic_SetPriorityTriggerType(g, TMR1_INTR_ID, 0xA8, 0x3);

    /* Los timers pasan por el handler del driver, que limpia la interrupcion y
     * llama a handler_timers (registrado con XTmrCtr_SetHandler). */
    if (XScuGic_Connect(g, TMR0_INTR_ID, (Xil_ExceptionHandler)XTmrCtr_InterruptHandler, t0) != XST_SUCCESS)
        return XST_FAILURE;
    if (XScuGic_Connect(g, TMR1_INTR_ID, (Xil_ExceptionHandler)XTmrCtr_InterruptHandler, t1) != XST_SUCCESS)
        return XST_FAILURE;
    if (XScuGic_Connect(g, LUZ_INTR_ID, (Xil_ExceptionHandler)handler_luz, gl) != XST_SUCCESS)
        return XST_FAILURE;

    XGpio_InterruptEnable(gl, GPIO_LUZ_MASK);
    XGpio_InterruptGlobalEnable(gl);

    XScuGic_Enable(g, TMR0_INTR_ID);
    XScuGic_Enable(g, TMR1_INTR_ID);
    XScuGic_Enable(g, LUZ_INTR_ID);

    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler, g);
    Xil_ExceptionEnable();
    return XST_SUCCESS;
}

/* =============================================================================
 *  Handlers de interrupcion (AO2) -- cortos: banderas y ADC, nada de LCD/I2C
 * ============================================================================= */

static void handler_timers(void *ref, u8 n_contador)
{
    (void)n_contador;
    if (ref == &timer0) {
        bandera_tick_juego = 1;            /* Timer 0: tick del juego   */
    } else if (ref == &timer1) {
        leer_sensores_adc(&sensores);      /* Timer 1: muestreo del ADC */
    }
}

/* El AXI GPIO interrumpe en CUALQUIER cambio del pin (bajada y subida).
 * El INT del OPT3001 es activo en bajo (POL=0): solo interesa cuando queda en 0. */
static void handler_luz(void *ref)
{
    XGpio *g = (XGpio *)ref;
    XGpio_InterruptClear(g, GPIO_LUZ_MASK);
    if ((XGpio_DiscreteRead(g, GPIO_LUZ_CANAL) & 0x1) == 0) {
        bandera_evento_luz = 1;
    }
}

/* =============================================================================
 *  OPT3001 (AO1: lectura/escritura de registros internos)
 * ============================================================================= */

static int opt3001_escribir_reg(u8 reg, u16 valor)
{
    u8 buf[3] = { reg, (u8)(valor >> 8), (u8)(valor & 0xFF) };
    return (XIic_Send(iic.BaseAddress, OPT_ADDR, buf, 3, XIIC_STOP) == 3) ? XST_SUCCESS : XST_FAILURE;
}

static int opt3001_leer_reg(u8 reg, u16 *valor)
{
    u8 rx[2];
    if (XIic_Send(iic.BaseAddress, OPT_ADDR, &reg, 1, XIIC_REPEATED_START) != 1) return XST_FAILURE;
    if (XIic_Recv(iic.BaseAddress, OPT_ADDR, rx, 2, XIIC_STOP) != 2)            return XST_FAILURE;
    *valor = ((u16)rx[0] << 8) | rx[1];
    return XST_SUCCESS;
}

/* Escribe config + umbrales y los lee de vuelta por UART (evidencia para AO1) */
static int opt3001_configurar(u16 config, u16 lim_bajo, u16 lim_alto)
{
    u16 c = 0, lb = 0, la = 0;
    if (opt3001_escribir_reg(OPT_REG_LIM_BAJO, lim_bajo) != XST_SUCCESS) return XST_FAILURE;
    if (opt3001_escribir_reg(OPT_REG_LIM_ALTO, lim_alto) != XST_SUCCESS) return XST_FAILURE;
    if (opt3001_escribir_reg(OPT_REG_CONFIG,   config)   != XST_SUCCESS) return XST_FAILURE;

    opt3001_leer_reg(OPT_REG_CONFIG,   &c);
    opt3001_leer_reg(OPT_REG_LIM_BAJO, &lb);
    opt3001_leer_reg(OPT_REG_LIM_ALTO, &la);
    xil_printf("OPT3001 config=0x%04x  lim_bajo=0x%04x  lim_alto=0x%04x\r\n", c, lb, la);
    return XST_SUCCESS;
}

/* lux = 0.01 * 2^E * R  ->  en centilux (entero) = R << E.
 * xil_printf no imprime float: mostrar como  cl/100 "." cl%100 */
static u32 opt3001_a_centilux(u16 raw)
{
    return ((u32)(raw & 0x0FFF)) << ((raw >> 12) & 0xF);
}

/* =============================================================================
 *  Sensores
 * ============================================================================= */

/* Se llama desde el Timer 1. Solo ADC por SPI (rapido). */
static void leer_sensores_adc(volatile Sensores *s)
{
    s->mic   = read_MIC();
    s->acx   = read_acx();
    s->acy   = read_acy();
    s->acz   = read_acz();
    s->joy_x = read_joyx();
    s->joy_y = read_joyy();
}

/* Devuelve 1 si hubo un aplauso/soplido en este tick. */
static int detectar_aleteo(const volatile Sensores *s)
{
    /* TODO(compa): deteccion del aplauso con el microfono.
     *   - Medir en la placa el valor del mic en silencio (~mitad de escala, 512?)
     *     y con un aplauso (imprimirlo por UART).
     *   - Umbral sobre |mic - reposo| + antirrebote (ignorar N ticks despues de un
     *     aleteo) para que un aplauso no cuente dos veces.
     *   - Respaldo: aceptar tambien un boton (ver LEEME_COMPANERO.md: hoy los
     *     botones van al modulo BUZZER, no al procesador).
     *   Ojo: el Timer 1 muestrea a 200 Hz pero esto se llama a 30 Hz; si se pierden
     *   aplausos, guardar el maximo del mic en leer_sensores_adc() y resetearlo aca. */
    (void)s;
    return 0;
}

/* =============================================================================
 *  Logica del juego (AC1 + AC2)
 * ============================================================================= */

static void juego_reiniciar(Juego *j)
{
    int i;
    j->pajaro.y     = PANTALLA_ALTO / 2;
    j->pajaro.y_ant = j->pajaro.y;
    j->pajaro.vel   = 0;
    j->puntaje      = 0;
    for (i = 0; i < N_TUBOS; i++) {
        j->tubos[i].x       = PANTALLA_ANCHO + i * (PANTALLA_ANCHO / N_TUBOS + TUBO_ANCHO);
        j->tubos[i].x_ant   = j->tubos[i].x;
        j->tubos[i].hueco_y = 30;   /* TODO(compa): altura aleatoria (rand()) */
        j->tubos[i].contado = 0;
    }
}

static void juego_actualizar(Juego *j, const volatile Sensores *s)
{
    /* TODO(compa): fisica del pajaro
     *   - guardar y_ant, sumar GRAVEDAD a vel; si detectar_aleteo(s) -> vel = IMPULSO_ALETEO
     *   - limitar y a [0, PANTALLA_ALTO - PAJARO_TAM]
     *   - (opcional) acelerometro: inclinar la placa cambia VELOCIDAD_TUBOS
     *
     * TODO(compa): tubos
     *   - mover x hacia la izquierda; si sale de la pantalla, reaparece a la
     *     derecha con hueco nuevo y contado = 0
     *   - si el tubo paso al pajaro y !contado -> puntaje++ (y un "bip")       */
    (void)j;
    (void)s;
}

static int juego_hay_choque(const Juego *j)
{
    /* TODO(compa): choque con suelo/techo o con un tubo fuera del hueco.
     *   Comparar el cuadrado del pajaro (PAJARO_X, y, PAJARO_TAM) con cada tubo. */
    (void)j;
    return 0;
}

/* Lo dispara la interrupcion del OPT3001 (sensor tapado/destapado) */
static void manejar_evento_luz(Juego *j)
{
    u16 config = 0, raw = 0;
    /* Leer el registro de config limpia el latch del INT (datasheet, bit L) */
    opt3001_leer_reg(OPT_REG_CONFIG, &config);
    opt3001_leer_reg(OPT_REG_RESULT, &raw);
    u32 cl = opt3001_a_centilux(raw);
    xil_printf("Interrupcion de luz: %d.%02d lux (config=0x%04x)\r\n",
               (int)(cl / 100), (int)(cl % 100), config);

    /* TODO(compa): que hace en el juego. Idea: modo noche (cambia el color del
     *   cielo y sube la dificultad), o pausa. Redibujar el fondo completo. */
    j->modo_noche = !j->modo_noche;
}

/* =============================================================================
 *  Dibujo en el LCD (AO1)
 *  El LCD es lento por SPI: NO hacer LCD_Clear() en cada frame. Borrar solo lo
 *  que se movio (posicion anterior) con el color del fondo y dibujar lo nuevo.
 * ============================================================================= */

static void dibujar_menu(const Juego *j)
{
    LCD_Clear(COLOR_CIELO_DIA);
    GUI_DisString_EN(20, 40, "FLAPPY VOZ", &Font16, COLOR_CIELO_DIA, BLACK);
    GUI_DisString_EN(14, 70, "Aplaude para", &Font12, COLOR_CIELO_DIA, BLACK);
    GUI_DisString_EN(30, 84, "empezar", &Font12, COLOR_CIELO_DIA, BLACK);
    /* TODO(compa): mostrar el record (j->record) y temperatura/luz (AO1) */
    (void)j;
}

static void dibujar_juego(const Juego *j)
{
    /* TODO(compa): redibujo incremental
     *   1. pajaro: cuadrado color fondo en (PAJARO_X, y_ant) y luego COLOR_PAJARO en y
     *      GUI_DrawRectangle(x0, y0, x1, y1, color, DRAW_FULL, DOT_PIXEL_1X1);
     *   2. tubos: borrar solo la franja de VELOCIDAD_TUBOS px que quedo a la derecha
     *      y pintar la franja nueva a la izquierda (no el tubo entero)
     *   3. HUD: puntaje arriba (GUI_DisNum) y, si se quiere, temperatura (AO1) */
    (void)j;
}

static void dibujar_game_over(const Juego *j)
{
    LCD_Clear(BLACK);
    GUI_DisString_EN(20, 40, "GAME OVER", &Font16, BLACK, RED);
    GUI_DisString_EN(20, 70, "Puntaje:", &Font12, BLACK, WHITE);
    GUI_DisNum(90, 70, (int32_t)j->puntaje, &Font12, BLACK, WHITE);
    GUI_DisString_EN(20, 86, "Record:", &Font12, BLACK, WHITE);
    GUI_DisNum(90, 86, (int32_t)j->record, &Font12, BLACK, WHITE);
}
