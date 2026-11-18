# =============================================================================
#  01_crear_proyecto.tcl  --  Proyecto 2 IEE2463 (SEP)
# -----------------------------------------------------------------------------
#  Crea el proyecto de Vivado del Proyecto 2 a partir de la plantilla oficial
#  del curso (Proyecto_Base_Booster_2025, repo IEE2463-SEP/PROY02).
#
#  Que hace:
#    1. Abre la plantilla y la guarda como un proyecto NUEVO (save_project_as),
#       sin tocar la plantilla original.
#    2. Quita la referencia al .xdc que la plantilla apunta fuera del zip
#       (../SEP/Proyecto_Base_Booster_v2/...), que no existe.
#    3. Agrega nuestro constraints/Zybo-Z7-Master.xdc y nuestro ip_repo/.
#    4. Llama a 02_hw_avance.tcl (timers + interrupcion de luz).
#
#  Uso (Vivado 2020.1):
#    Tcl Console de Vivado:
#      cd C:/SEP/proyecto2
#      source scripts/01_crear_proyecto.tcl
#    o en una terminal:
#      vivado -mode batch -source scripts/01_crear_proyecto.tcl
#
#  Todo el proyecto vive en C:/SEP/proyecto2, FUERA de OneDrive: OneDrive
#  bloquea archivos mientras Vivado sintetiza y genera errores raros.
#  Mantener rutas cortas (limite de 260 caracteres de Windows).
# =============================================================================

set ROOT      [file normalize [file join [file dirname [info script]] ..]]
set PROJ_NAME P2_SEP
set PROJ_DIR  [file join $ROOT vivado]
set TPL_XPR   [file join $ROOT plantilla Proyecto_Base_Booster \
                 HW_Base_Booster_2025 Proyecto_Base_Booster_2025.xpr]
set XDC       [file join $ROOT constraints Zybo-Z7-Master.xdc]
set IP_REPO   [file join $ROOT ip_repo]

if {![file exists $TPL_XPR]} {
    error "No se encontro la plantilla en $TPL_XPR. Descomprima Proyecto_Base_Booster_2025.zip en proyecto2/plantilla/"
}
if {[file exists $PROJ_DIR]} {
    error "$PROJ_DIR ya existe. Borrelo (o cambie PROJ_DIR) antes de volver a crear el proyecto."
}

# --- 1. Plantilla -> proyecto nuevo -------------------------------------------
open_project $TPL_XPR
save_project_as $PROJ_NAME $PROJ_DIR
# Desde aqui se trabaja sobre la copia; la plantilla queda intacta.

# --- 2. Limpiar el .xdc roto de la plantilla ----------------------------------
foreach f [get_files -quiet -of_objects [get_filesets constrs_1] *.xdc] {
    puts "INFO: quitando constraint heredado de la plantilla: $f"
    remove_files -fileset constrs_1 $f
}

# --- 3. Nuestros constraints e IP repo ------------------------------------------
# add_files (sin import) => el proyecto usa directamente proyecto2/constraints,
# asi hay una sola copia del .xdc.
add_files -fileset constrs_1 -norecurse $XDC
set_property target_constrs_file $XDC [current_fileset -constrset]

set_property ip_repo_paths [list $IP_REPO] [current_project]
update_ip_catalog -rebuild

# --- 4. Hardware del avance ------------------------------------------------------
source [file join $ROOT scripts 02_hw_avance.tcl]

puts "============================================================"
puts " Proyecto creado en $PROJ_DIR/$PROJ_NAME.xpr"
puts " Siguiente paso: Generate Bitstream y luego"
puts " File > Export > Export Hardware (incluir bitstream)."
puts "============================================================"
