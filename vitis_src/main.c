/* =============================================================================
 *  main.c  --  Proyecto 2 IEE2463 SEP  --  "Flappy por voz" (nombre tentativo)
 * -----------------------------------------------------------------------------
 *  ESTADO: logica del juego completa (v1). Compila en teoria, PERO NO SE HA
 *  PROBADO EN LA ZYBO. Los valores marcados con "CALIBRAR" son provisorios y hay
 *  que ajustarlos con mediciones reales (ver README, punto 3 del avance).
 *
 *  Que hace este archivo:
 *    - Inicializacion de LCD (SPI0), ADC (SPI1) e I2C (TMP006 + OPT3001).
 *      Drivers: los de la plantilla del curso (LCD_*, ADC, I2C, Delay).
 *    - AO2: 2 AXI Timer con interrupcion + interrupcion del sensor de luz
 *      (pin INT del OPT3001 -> axi_gpio_luz -> IRQ_F2P -> GIC).
 *      Patron de AYUD07 (XTmrCtr_SetHandler) adaptado al GIC del Zynq (XScuGic),
 *      como en el ejemplo Snake (PROY02-Ejemplo).
 *    - AO1: registros del OPT3001 (0x01, 0x02, 0x03) escritos y leidos de vuelta;
 *      pantalla dinamica con microfono (aleteo), acelerometro (velocidad de los
 *      tubos), luz (modo noche) y temperatura (HUD).
 *    - AC1/AC2: maquina de estados + structs + funciones con punteros.
 *
 *  Regla de este archivo: las interrupciones SOLO levantan banderas o leen el
 *  ADC (rapido). Todo lo lento (I2C, dibujar en el LCD) va en el while del main.
 *
 *  Hardware: ver README.md (direcciones e IDs de interrupcion).
 * ============================================================================= */

#include <stdio.h>
#include <stdlib.h>
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

/* Configuracion 0xC410: RN=0xC (rango automatico), CT=0 (100 ms), M=10 (continuo),
 *   L=1 (latch: el INT queda activo hasta leer el registro de config),
 *   POL=0 (INT activo en bajo), FC=00 (1 medicion fuera de rango basta).
 * OJO: read_opt() de I2C.c reescribe 0xC410 en cada lectura. Aca no se usa
 *   read_opt(); se lee con opt3001_leer_reg().
 *
 * Comparador de ventana con histeresis (la interrupcion salta cuando el resultado
 * sale de [limite bajo, limite alto]):
 *   - Modo DIA:   avisa si lux < 20  (tapar el sensor). Sin limite alto.
 *   - Modo NOCHE: avisa si lux > 60  (destapar).        Sin limite bajo.
 *   Cada vez que salta, manejar_evento_luz() cambia de ventana.
 *   Estos son los registros modificados de AO1 -> mostrarlo en el video.
 *   CALIBRAR: medir cuantos lux marca el sensor destapado y tapado (la pantalla del
 *   menu y la UART los muestran) y ajustar. Codificacion: (E << 12) | R con
 *   lux = 0.01 * 2^E * R  (R de 12 bits, o sea centilux = R << E). */
#define OPT_CONFIG              0xC410
#define OPT_UMBRAL_BAJO         0x07D0  /* modo dia:   E=0, R=2000 -> 20 lux      */
#define OPT_UMBRAL_ALTO         0xBFFF  /* modo dia:   fondo de escala (sin limite) */
#define OPT_UMBRAL_BAJO_NOCHE   0x0000  /* modo noche: sin limite bajo            */
#define OPT_UMBRAL_ALTO_NOCHE   0x1BB8  /* modo noche: E=1, R=3000 -> 60 lux      */

/* ---------------------------------------------------------------------------
 *  Parametros del juego (pantalla de la Booster: 128 x 128 px, colores RGB565)
 *  La franja superior (HUD_ALTO) es del marcador: el juego ocurre debajo.
 * --------------------------------------------------------------------------- */
#define PANTALLA_ANCHO       128
#define PANTALLA_ALTO        128
#define HUD_ALTO             14
#define N_TUBOS              3
#define TUBO_ANCHO           12
#define HUECO_ALTO           40
#define MARGEN_HUECO         10      /* distancia minima del hueco al techo/suelo */
#define SEPARACION_TUBOS     (PANTALLA_ANCHO / N_TUBOS + TUBO_ANCHO)
#define PAJARO_X             30
#define PAJARO_TAM           8

/* CALIBRAR: fisica (unidades: px por tick a 30 Hz).
 * Con estos valores un aleteo sube unos 21 px (6+5+4+3+2+1). */
#define GRAVEDAD             1
#define IMPULSO_ALETEO       (-6)
#define VEL_CAIDA_MAX        5       /* limite de caida para no atravesar el suelo */
#define VELOCIDAD_TUBOS      2

/* CALIBRAR: microfono. El reposo se mide solo al arrancar (calibrar_mic), asi
 * que hay que estar en silencio al encender. MIC_UMBRAL es cuanto debe superar
 * el pico al reposo para contar como aplauso (ADC de 10 bits: 0..1023). */
#define MIC_REPOSO_DEF       512
#define MIC_UMBRAL           150
#define N_MUESTRAS_CAL       64
#define ALETEO_BLOQUEO       6       /* ticks (33 ms c/u) sin volver a detectar     */
#define ESPERA_GAME_OVER_TICKS 45    /* 1,5 s antes de aceptar un aplauso en G.O.   */

/* CALIBRAR: acelerometro. Hay que ver cual eje (acx o acy) cambia al inclinar la
 * placa de lado y cuanto vale plano. Inclinar mas que ACEL_UMBRAL cambia la
 * velocidad de los tubos. */
#define ACEL_REPOSO          512
#define ACEL_UMBRAL          60

/* 1 = imprime por UART valores de sensores en el menu y cada aleteo (calibracion) */
#define DEPURACION_UART      1

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
    int16_t x_ant;      /* x del tick anterior (para borrar)         */
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
    u8          modo_noche;    /* lo cambia la interrupcion de luz          */
    u8          redibujar;     /* 1 = repintar todo el fondo en el proximo dibujo */
    u16         ticks_estado;  /* ticks desde que entro al estado actual    */
} Juego;

/* ---------------------------------------------------------------------------
 *  Variables globales
 *  'volatile' porque las escriben las interrupciones y las lee el main.
 * --------------------------------------------------------------------------- */
static XScuGic gic;
static XTmrCtr timer0;
static XTmrCtr timer1;
static XGpio   gpio_luz;

static volatile u8  bandera_tick_juego = 0;  /* Timer 0             */
static volatile u8  bandera_evento_luz = 0;  /* INT del OPT3001     */

/* Evidencia de AO2: cuantas interrupciones llegaron de cada timer. El main las
 * imprime y las pone en 0 una vez por segundo (deben salir ~30 y ~200). */
static volatile u32 cuenta_int_t0 = 0;
static volatile u32 cuenta_int_t1 = 0;
static volatile Sensores sensores;           /* lo llena el Timer 1 */

/* Pico del microfono (|mic - reposo|) desde la ultima vez que el juego lo miro.
 * El Timer 1 (200 Hz) lo sube; detectar_aleteo() (30 Hz) lo lee y lo pone en 0.
 * Es una variable aparte (y no un campo de Sensores) porque detectar_aleteo()
 * recibe los sensores como 'const'. */
static volatile int mic_pico = 0;
static int mic_reposo = MIC_REPOSO_DEF;      /* se mide en calibrar_mic() */

static Juego juego;

/* Evidencia de AO1 y AO2 en la PANTALLA (no solo por UART). Las escribe el main
 * (no las interrupciones), por eso no son volatile. Se muestran en el menu. */
static u32 int_por_seg_t0 = 0;    /* interrupciones del Timer 0 en el ultimo segundo */
static u32 int_por_seg_t1 = 0;    /* interrupciones del Timer 1 en el ultimo segundo */
static u32 cuenta_int_luz = 0;    /* interrupciones de luz desde el encendido        */
static u16 opt_leido_config = 0;  /* registros del OPT3001 leidos de vuelta          */
static u16 opt_leido_bajo   = 0;
static u16 opt_leido_alto   = 0;

/* ---------------------------------------------------------------------------
 *  Prototipos
 * --------------------------------------------------------------------------- */
/* Inicializacion */
static int  init_perifericos(void);
static int  init_timers(XTmrCtr *t0, XTmrCtr *t1);
static int  init_gpio_luz(XGpio *g);
static int  init_interrupciones(XScuGic *g, XTmrCtr *t0, XTmrCtr *t1, XGpio *gl);
static void calibrar_mic(int *reposo);

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
static int16_t hueco_aleatorio(void);
static void juego_reiniciar(Juego *j);
static void juego_actualizar(Juego *j, const volatile Sensores *s);
static int  juego_hay_choque(const Juego *j);
static void manejar_evento_luz(Juego *j);

/* Dibujo (AO1: pantalla) */
static COLOR color_cielo(const Juego *j);
static void pintar_rect(int x0, int y0, int x1, int y1, COLOR color);
static void pintar_columnas_tubo(const Tubo *t, int x0, int x1);
static void dibujar_hud(u32 puntaje, int temp);
static void dibujar_diagnostico(const volatile Sensores *s);
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

    /* Nivel de silencio del microfono (antes de arrancar los timers, para que el
     * SPI del ADC no lo use nadie mas). Hacer silencio al encender. */
    calibrar_mic(&mic_reposo);
    xil_printf("Microfono en reposo (promedio): %d\r\n", mic_reposo);

    /* TODO: AO3 -- aca va la configuracion del IP de musica por UART
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

        /* AO2: una vez por segundo (30 ticks) mostrar cuantas interrupciones llegaron
         * de cada timer. Se copian y se ponen en 0 para medir cada segundo por separado. */
        if ((ticks % FREC_JUEGO_HZ) == 0) {
            u32 n0 = cuenta_int_t0;
            u32 n1 = cuenta_int_t1;
            cuenta_int_t0 = 0;
            cuenta_int_t1 = 0;
            int_por_seg_t0 = n0;          /* para mostrarlos en la pantalla */
            int_por_seg_t1 = n1;
            xil_printf("Interrupciones/s: Timer0=%d Timer1=%d\r\n", (int)n0, (int)n1);
        }

        /* I2C es lento: leer luz y temperatura solo 3 veces por segundo */
        if ((ticks % 10) == 0) {
            u16 raw;
            if (opt3001_leer_reg(OPT_REG_RESULT, &raw) == XST_SUCCESS)
                sensores.luz_raw = raw;
            sensores.temp = read_tmp();
        }

        switch (juego.estado) {   /* AC1: maquina de estados */

        case ESTADO_MENU:
            /* Valores en vivo en el menu (AO1) y por UART (calibracion) */
            if ((ticks % 10) == 0) {
                dibujar_diagnostico(&sensores);
#if DEPURACION_UART
                xil_printf("mic=%d acx=%d acy=%d acz=%d luz_raw=0x%04x temp=%d\r\n",
                           sensores.mic, sensores.acx, sensores.acy, sensores.acz,
                           sensores.luz_raw, sensores.temp);
#endif
            }
            /* Empieza con un aplauso. TODO: aceptar tambien joystick/boton */
            if (detectar_aleteo(&sensores)) {
                /* semilla aleatoria: el valor del timer en el instante del aplauso */
                srand((unsigned int)XTmrCtr_GetValue(&timer1, 0));
                juego_reiniciar(&juego);          /* deja redibujar = 1 */
                juego.estado = ESTADO_JUGANDO;
            }
            break;

        case ESTADO_JUGANDO:
            juego_actualizar(&juego, &sensores);
            if (juego_hay_choque(&juego)) {
                if (juego.puntaje > juego.record) juego.record = juego.puntaje;
                juego.estado = ESTADO_GAME_OVER;
                juego.ticks_estado = 0;
                dibujar_game_over(&juego);
                /* TODO: AC6 cambiar la musica; AC7 guardar record en SD */
            } else {
                dibujar_juego(&juego);
                juego.redibujar = 0;              /* el fondo ya quedo pintado */
            }
            break;

        case ESTADO_GAME_OVER:
            /* Vuelve al menu con un aplauso, pero solo tras una espera corta para
             * que el mismo ruido que causo el choque no cuente. detectar_aleteo()
             * va primero para que consuma el pico del microfono en cada tick. */
            juego.ticks_estado++;
            if (detectar_aleteo(&sensores) && juego.ticks_estado > ESPERA_GAME_OVER_TICKS) {
                juego.estado = ESTADO_MENU;
                dibujar_menu(&juego);
            }
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

/* Mide el nivel de reposo del microfono: promedio de N_MUESTRAS_CAL lecturas
 * separadas 5 ms (~0,3 s). Entrega el resultado por puntero. Debe llamarse con
 * los timers detenidos (el ADC lo usa el Timer 1 despues) y en silencio. */
static void calibrar_mic(int *reposo)
{
    int i;
    long suma = 0;
    for (i = 0; i < N_MUESTRAS_CAL; i++) {
        suma += read_MIC();
        delay_ms(5);
    }
    *reposo = (int)(suma / N_MUESTRAS_CAL);
}

/* =============================================================================
 *  Handlers de interrupcion (AO2) -- cortos: banderas y ADC, nada de LCD/I2C
 * ============================================================================= */

static void handler_timers(void *ref, u8 n_contador)
{
    (void)n_contador;
    if (ref == &timer0) {
        bandera_tick_juego = 1;            /* Timer 0: tick del juego   */
        cuenta_int_t0++;
    } else if (ref == &timer1) {
        leer_sensores_adc(&sensores);      /* Timer 1: muestreo del ADC */
        cuenta_int_t1++;
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

    /* Guardar lo leido para mostrarlo en la pantalla del menu */
    opt_leido_config = c;
    opt_leido_bajo   = lb;
    opt_leido_alto   = la;
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

/* Se llama desde el Timer 1 (200 Hz). Solo ADC por SPI (rapido). Ademas guarda el
 * pico del microfono para que el juego (30 Hz) no se pierda aplausos cortos. */
static void leer_sensores_adc(volatile Sensores *s)
{
    int desv;

    s->mic   = read_MIC();
    s->acx   = read_acx();
    s->acy   = read_acy();
    s->acz   = read_acz();
    s->joy_x = read_joyx();
    s->joy_y = read_joyy();

    desv = s->mic - mic_reposo;          /* |mic - reposo| */
    if (desv < 0) desv = -desv;
    if (desv > mic_pico) mic_pico = desv;
}

/* Devuelve 1 si hubo un aplauso/soplido desde la ultima llamada.
 * Se debe llamar UNA vez por tick del juego en todos los estados: cada llamada
 * consume el pico acumulado. Antirrebote: tras detectar, ignora ALETEO_BLOQUEO
 * ticks para que un aplauso no cuente dos veces. */
static int detectar_aleteo(const volatile Sensores *s)
{
    static u8 bloqueo = 0;
    int pico = mic_pico;                 /* pico del Timer 1 desde el tick anterior */
    int desv = s->mic - mic_reposo;      /* ademas, la muestra mas reciente         */

    if (desv < 0) desv = -desv;
    if (desv > pico) pico = desv;
    mic_pico = 0;                        /* consumido */

    if (bloqueo > 0) {
        bloqueo--;
        return 0;
    }
    if (pico > MIC_UMBRAL) {
        bloqueo = ALETEO_BLOQUEO;
#if DEPURACION_UART
        xil_printf("Aleteo: pico=%d (umbral=%d)\r\n", pico, MIC_UMBRAL);
#endif
        return 1;
    }
    return 0;
}

/* =============================================================================
 *  Logica del juego (AC1 + AC2)
 * ============================================================================= */

/* Altura aleatoria del hueco: deja MARGEN_HUECO px libres arriba y abajo */
static int16_t hueco_aleatorio(void)
{
    int rango = PANTALLA_ALTO - HUD_ALTO - HUECO_ALTO - 2 * MARGEN_HUECO;
    return (int16_t)(HUD_ALTO + MARGEN_HUECO + (rand() % (rango + 1)));
}

static void juego_reiniciar(Juego *j)
{
    int i;
    j->pajaro.y     = PANTALLA_ALTO / 2;
    j->pajaro.y_ant = j->pajaro.y;
    j->pajaro.vel   = 0;
    j->puntaje      = 0;
    j->redibujar    = 1;       /* el proximo dibujo repinta todo el fondo */
    j->ticks_estado = 0;
    for (i = 0; i < N_TUBOS; i++) {
        j->tubos[i].x       = PANTALLA_ANCHO + i * SEPARACION_TUBOS;
        j->tubos[i].x_ant   = j->tubos[i].x;
        j->tubos[i].hueco_y = hueco_aleatorio();
        j->tubos[i].contado = 0;
    }
}

static void juego_actualizar(Juego *j, const volatile Sensores *s)
{
    int i;
    int vel_tubos = VELOCIDAD_TUBOS;
    int inclinacion = s->acx - ACEL_REPOSO;   /* CALIBRAR: puede ser acy */

    /* --- Pajaro: gravedad, aleteo y limites --------------------------------- */
    j->pajaro.y_ant = j->pajaro.y;
    j->pajaro.vel  += GRAVEDAD;
    if (j->pajaro.vel > VEL_CAIDA_MAX) j->pajaro.vel = VEL_CAIDA_MAX;
    if (detectar_aleteo(s)) j->pajaro.vel = IMPULSO_ALETEO;
    j->pajaro.y += j->pajaro.vel;

    if (j->pajaro.y < HUD_ALTO) {                       /* techo: no mata, frena */
        j->pajaro.y   = HUD_ALTO;
        j->pajaro.vel = 0;
    }
    if (j->pajaro.y > PANTALLA_ALTO - PAJARO_TAM)       /* suelo: lo detecta juego_hay_choque */
        j->pajaro.y = PANTALLA_ALTO - PAJARO_TAM;

    /* --- Velocidad de los tubos: acelerometro y modo noche ------------------- */
    if (inclinacion >  ACEL_UMBRAL) vel_tubos++;        /* inclinada a un lado: mas rapido */
    if (inclinacion < -ACEL_UMBRAL) vel_tubos--;        /* al otro lado: mas lento         */
    if (j->modo_noche) vel_tubos++;                     /* de noche es mas dificil         */
    if (vel_tubos < 1) vel_tubos = 1;

    /* --- Tubos: mover, reciclar y sumar puntos ------------------------------ */
    for (i = 0; i < N_TUBOS; i++) {
        Tubo *t = &j->tubos[i];

        t->x_ant = t->x;
        t->x    -= vel_tubos;

        if (t->x + TUBO_ANCHO <= 0) {                   /* salio por la izquierda */
            t->x      += N_TUBOS * SEPARACION_TUBOS;    /* vuelve detras del ultimo */
            t->hueco_y = hueco_aleatorio();
            t->contado = 0;
        }
        if (!t->contado && (t->x + TUBO_ANCHO) < PAJARO_X) {   /* paso al pajaro */
            t->contado = 1;
            j->puntaje++;
            /* TODO: AC6 un "bip" al sumar punto */
        }
    }
}

/* Devuelve 1 si el pajaro toco el suelo o un tubo. Los rangos son semiabiertos:
 * el pajaro ocupa x en [PAJARO_X, PAJARO_X+TAM) y y en [y, y+TAM). */
static int juego_hay_choque(const Juego *j)
{
    int i;
    int px0 = PAJARO_X;
    int px1 = PAJARO_X + PAJARO_TAM;
    int py0 = j->pajaro.y;
    int py1 = j->pajaro.y + PAJARO_TAM;

    if (py1 >= PANTALLA_ALTO) return 1;                       /* suelo */

    for (i = 0; i < N_TUBOS; i++) {
        const Tubo *t = &j->tubos[i];
        int tx0 = t->x;
        int tx1 = t->x + TUBO_ANCHO;
        if (px1 > tx0 && px0 < tx1) {                         /* coinciden en x */
            if (py0 < t->hueco_y || py1 > t->hueco_y + HUECO_ALTO)
                return 1;                                     /* fuera del hueco */
        }
    }
    return 0;
}

/* Lo dispara la interrupcion del OPT3001 (sensor tapado/destapado).
 * Cada evento alterna entre modo dia y noche y cambia la ventana del sensor:
 * el limite que acaba de saltar se reemplaza por el opuesto (histeresis), asi
 * el INT no se repite mientras la luz siga igual. */
static void manejar_evento_luz(Juego *j)
{
    u16 config = 0, raw = 0;
    u32 cl;

    cuenta_int_luz++;     /* evidencia de AO2 en la pantalla del menu */

    /* Leer el registro de config limpia el latch del INT (datasheet, bit L) */
    opt3001_leer_reg(OPT_REG_CONFIG, &config);
    opt3001_leer_reg(OPT_REG_RESULT, &raw);
    cl = opt3001_a_centilux(raw);
    xil_printf("Interrupcion de luz: %d.%02d lux (config=0x%04x)\r\n",
               (int)(cl / 100), (int)(cl % 100), config);

    j->modo_noche = !j->modo_noche;
    if (j->modo_noche)
        opt3001_configurar(OPT_CONFIG, OPT_UMBRAL_BAJO_NOCHE, OPT_UMBRAL_ALTO_NOCHE);
    else
        opt3001_configurar(OPT_CONFIG, OPT_UMBRAL_BAJO, OPT_UMBRAL_ALTO);

    j->redibujar = 1;     /* cambia el color del cielo: repintar todo */

    /* Si el sensor alcanzo a avisar de nuevo mientras se cambiaban los limites,
     * ese aviso es de la ventana vieja: se descarta para no volver a alternar. */
    bandera_evento_luz = 0;
}

/* =============================================================================
 *  Dibujo en el LCD (AO1)
 *  El LCD es lento por SPI: NO hacer LCD_Clear() en cada frame. Borrar solo lo
 *  que se movio (posicion anterior) con el color del fondo y dibujar lo nuevo.
 * ============================================================================= */

static COLOR color_cielo(const Juego *j)
{
    return j->modo_noche ? COLOR_CIELO_NOCHE : COLOR_CIELO_DIA;
}

/* Rectangulo relleno con coordenadas INCLUSIVAS, recortado a la pantalla.
 * Usa LCD_SetArealColor (relleno en rafaga, mucho mas rapido que GUI_DrawRectangle).
 * VERIFICAR en LCD_Driver.c: en el driver de Waveshare el final de LCD_SetArealColor
 * es EXCLUSIVO (por eso se pasa x1+1, y1+1). Si en esta version fuera inclusivo,
 * quitar los "+ 1" de abajo: es el unico lugar donde se usa. */
static void pintar_rect(int x0, int y0, int x1, int y1, COLOR color)
{
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > PANTALLA_ANCHO - 1) x1 = PANTALLA_ANCHO - 1;
    if (y1 > PANTALLA_ALTO - 1)  y1 = PANTALLA_ALTO - 1;
    if (x0 > x1 || y0 > y1) return;      /* nada visible */
    LCD_SetArealColor(x0, y0, x1 + 1, y1 + 1, color);
}

/* Pinta las columnas x0..x1 de un tubo: la parte de arriba y la de abajo del hueco */
static void pintar_columnas_tubo(const Tubo *t, int x0, int x1)
{
    pintar_rect(x0, HUD_ALTO, x1, t->hueco_y - 1, COLOR_TUBO);
    pintar_rect(x0, t->hueco_y + HUECO_ALTO, x1, PANTALLA_ALTO - 1, COLOR_TUBO);
}

/* Franja superior: puntaje y temperatura (AO1). Se repinta entera: es chica. */
static void dibujar_hud(u32 puntaje, int temp)
{
    pintar_rect(0, 0, PANTALLA_ANCHO - 1, HUD_ALTO - 1, BLACK);
    GUI_DisString_EN(2, 1, "Pts:", &Font12, BLACK, WHITE);
    GUI_DisNum(34, 1, (int32_t)puntaje, &Font12, BLACK, WHITE);
    GUI_DisNum(90, 1, (int32_t)temp, &Font12, BLACK, YELLOW);
    GUI_DisString_EN(112, 1, "C", &Font12, BLACK, YELLOW);
}

/* Valores en vivo en la parte baja del menu (AO1): microfono, acelerometro,
 * luz (lux) y temperatura. Sirve tambien para calibrar sin la UART.
 * Las lineas 4 a 7 son ademas la evidencia de AO1 y AO2 en pantalla: modo
 * dia/noche, interrupciones por segundo de cada timer, registros del OPT3001
 * leidos de vuelta y cuantas interrupciones de luz llegaron. */
static void dibujar_diagnostico(const volatile Sensores *s)
{
    char buf[26];
    u32 cl = opt3001_a_centilux(s->luz_raw);
    int inclinacion = s->acx - ACEL_REPOSO;
    const char *estado_vel = "NORM";

    if (inclinacion > ACEL_UMBRAL)       estado_vel = "RAP+";
    else if (inclinacion < -ACEL_UMBRAL) estado_vel = "LEN-";

    /* Borra la zona inferior de diagnostico (y = 60 hasta 127) */
    pintar_rect(0, 60, PANTALLA_ANCHO - 1, PANTALLA_ALTO - 1, COLOR_CIELO_DIA);

    /* Linea 1: Ejes X e Y del acelerometro */
    sprintf(buf, "X:%4d  Y:%4d", s->acx, s->acy);
    GUI_DisString_EN(2, 62, buf, &Font8, COLOR_CIELO_DIA, BLACK);

    /* Linea 2: Eje Z y velocidad resultante por inclinacion */
    sprintf(buf, "Z:%4d  Vel:%s", s->acz, estado_vel);
    GUI_DisString_EN(2, 71, buf, &Font8, COLOR_CIELO_DIA, BLACK);

    /* Linea 3: Microfono y Luz (lux) */
    sprintf(buf, "Mic:%3d L:%d.%02d", s->mic, (int)(cl / 100), (int)(cl % 100));
    GUI_DisString_EN(2, 80, buf, &Font8, COLOR_CIELO_DIA, BLACK);

    /* Linea 4: Temperatura y modo dia/noche (lo cambia la interrupcion de luz) */
    sprintf(buf, "Temp:%d C  Modo:%s", s->temp, juego.modo_noche ? "NOCHE" : "DIA");
    GUI_DisString_EN(2, 89, buf, &Font8, COLOR_CIELO_DIA, BLACK);

    /* Linea 5 (AO2): interrupciones por segundo de cada timer (~30 y ~200) */
    sprintf(buf, "Int/s T0:%d T1:%d", (int)int_por_seg_t0, (int)int_por_seg_t1);
    GUI_DisString_EN(2, 98, buf, &Font8, COLOR_CIELO_DIA, BLACK);

    /* Lineas 6 y 7 (AO1): registros 0x01, 0x02 y 0x03 del OPT3001 leidos de
     * vuelta; (AO2) cuantas interrupciones de luz llegaron */
    sprintf(buf, "R1:%04X R2:%04X", opt_leido_config, opt_leido_bajo);
    GUI_DisString_EN(2, 107, buf, &Font8, COLOR_CIELO_DIA, BLACK);

    sprintf(buf, "R3:%04X  IntLuz:%d", opt_leido_alto, (int)cuenta_int_luz);
    GUI_DisString_EN(2, 116, buf, &Font8, COLOR_CIELO_DIA, BLACK);
}

/* Las posiciones (y) estan mas arriba que en la v1 para dejar espacio a las
 * lineas nuevas del diagnostico, que ahora empieza en y = 60. */
static void dibujar_menu(const Juego *j)
{
    LCD_Clear(COLOR_CIELO_DIA);
    GUI_DisString_EN(8, 4, "FLAPPY VOZ", &Font16, COLOR_CIELO_DIA, BLACK);
    GUI_DisString_EN(14, 22, "Aplaude para", &Font12, COLOR_CIELO_DIA, BLACK);
    GUI_DisString_EN(30, 34, "empezar", &Font12, COLOR_CIELO_DIA, BLACK);
    GUI_DisString_EN(14, 46, "Record:", &Font12, COLOR_CIELO_DIA, BLACK);
    GUI_DisNum(76, 46, (int32_t)j->record, &Font12, COLOR_CIELO_DIA, BLACK);

    /* Dibuja inmediatamente los labels con los valores actuales */
    dibujar_diagnostico(&sensores);
}

/* Redibujo incremental. Si j->redibujar == 1 (inicio de partida o cambio de
 * dia/noche) repinta todo el fondo; si no, solo toca lo que se movio:
 *   - tubos: borra la franja que quedo vacia a la derecha y pinta la nueva a la
 *     izquierda (el ancho de la franja es cuanto se movio el tubo);
 *   - pajaro: borra su posicion anterior y lo pinta en la nueva;
 *   - HUD: solo cuando cambia el puntaje o la temperatura.
 * Orden: tubos -> pajaro -> HUD, para que el pajaro quede encima. */
static void dibujar_juego(const Juego *j)
{
    static u32 hud_puntaje = 0;
    static int hud_temp    = 0;
    COLOR cielo = color_cielo(j);
    int i;
    int temp = sensores.temp;

    if (j->redibujar) {
        pintar_rect(0, HUD_ALTO, PANTALLA_ANCHO - 1, PANTALLA_ALTO - 1, cielo);
        for (i = 0; i < N_TUBOS; i++) {
            const Tubo *t = &j->tubos[i];
            pintar_columnas_tubo(t, t->x, t->x + TUBO_ANCHO - 1);
        }
    } else {
        for (i = 0; i < N_TUBOS; i++) {
            const Tubo *t = &j->tubos[i];
            if (t->x > t->x_ant) {
                /* reaparecio a la derecha: borrar el tubo viejo y pintar el nuevo */
                pintar_rect(t->x_ant, HUD_ALTO, t->x_ant + TUBO_ANCHO - 1,
                            PANTALLA_ALTO - 1, cielo);
                pintar_columnas_tubo(t, t->x, t->x + TUBO_ANCHO - 1);
            } else if (t->x < t->x_ant) {
                /* movimiento normal hacia la izquierda */
                pintar_rect(t->x + TUBO_ANCHO, HUD_ALTO, t->x_ant + TUBO_ANCHO - 1,
                            PANTALLA_ALTO - 1, cielo);
                pintar_columnas_tubo(t, t->x, t->x_ant - 1);
            }
        }
        /* borrar el pajaro de su posicion anterior */
        pintar_rect(PAJARO_X, j->pajaro.y_ant,
                    PAJARO_X + PAJARO_TAM - 1, j->pajaro.y_ant + PAJARO_TAM - 1, cielo);
    }

    pintar_rect(PAJARO_X, j->pajaro.y,
                PAJARO_X + PAJARO_TAM - 1, j->pajaro.y + PAJARO_TAM - 1, COLOR_PAJARO);

    if (j->redibujar || j->puntaje != hud_puntaje || temp != hud_temp) {
        dibujar_hud(j->puntaje, temp);
        hud_puntaje = j->puntaje;
        hud_temp    = temp;
    }
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
