# =============================================================================
#  04_vitis.tcl  --  Proyecto 2 IEE2463 (SEP)
# -----------------------------------------------------------------------------
#  Crea el workspace de Vitis (plataforma + aplicacion) desde vivado/P2_SEP.xsa
#  e importa el codigo de vitis_src/. Equivale a hacerlo a mano en la GUI:
#    Platform Project con el .xsa -> Application Project "Hello World" ->
#    borrar helloworld.c -> importar vitis_src/.
#  (Se usa "Hello World" y no "Empty Application" porque trae platform.c/h,
#   que los drivers de la plantilla incluyen, y deja la UART como stdin/stdout.)
#
#  Uso (terminal, no Vivado):
#    D:\Xilinx\Vitis\2020.1\bin\xsct.bat C:/SEP/proyecto2/scripts/04_vitis.tcl
#  Despues: abrir Vitis con el workspace C:\SEP\proyecto2\vitis.
#
#  OJO: vitis_src/ se COPIA al proyecto. Si editas en Vitis, copia de vuelta a
#  vitis_src/ antes de hacer commit (o edita en vitis_src/ y re-importa).
# =============================================================================

set ROOT [file normalize [file join [file dirname [info script]] ..]]
set WS   [file join $ROOT vitis]
set XSA  [file join $ROOT vivado P2_SEP.xsa]
set SRC  [file join $ROOT vitis_src]

if {![file exists $XSA]} { error "Falta $XSA. Corra antes 01_crear_proyecto.tcl y 03_build.tcl" }
if {[file exists $WS]}   { error "$WS ya existe. Borrelo antes de regenerar el workspace." }

setws $WS

platform create -name P2_plat -hw $XSA -proc ps7_cortexa9_0 -os standalone
platform generate

app create -name P2_app -platform P2_plat -domain standalone_domain -template {Hello World}
file delete -force [file join $WS P2_app src helloworld.c]
importsources -name P2_app -path $SRC

app build -name P2_app
# xsct no siempre avisa si falla la compilacion de la app: revisar el ELF.
if {![file exists [file join $WS P2_app Debug P2_app.elf]]} {
    error "No se genero P2_app.elf. Compile en Vitis (Build Project) para ver los errores."
}
puts "============================================================"
puts " Workspace listo en $WS  (ELF: P2_app/Debug/P2_app.elf)"
puts "============================================================"
