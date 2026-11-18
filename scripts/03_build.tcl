# =============================================================================
#  03_build.tcl  --  Proyecto 2 IEE2463 (SEP)
# -----------------------------------------------------------------------------
#  Sintetiza, implementa, genera el bitstream y exporta el hardware (.xsa con
#  bitstream incluido) para crear la plataforma en Vitis.
#
#  Uso:  vivado -mode batch -source scripts/03_build.tcl
#        (o desde la Tcl Console con el proyecto abierto: source scripts/03_build.tcl)
# =============================================================================

set ROOT     [file normalize [file join [file dirname [info script]] ..]]
set PROJ_XPR [file join $ROOT vivado P2_SEP.xpr]
set XSA      [file join $ROOT hw P2_SEP.xsa]   ;# versionado: lo usa Vitis
file mkdir [file join $ROOT hw]
set JOBS     4

if {[catch {current_project}]} {
    open_project $PROJ_XPR
}

# Regenerar los productos del block design por si hubo cambios
generate_target all [get_files DEMO.bd]

reset_run synth_1
launch_runs impl_1 -to_step write_bitstream -jobs $JOBS
wait_on_run impl_1

if {[get_property PROGRESS [get_runs impl_1]] != "100%"} {
    error "La implementacion no termino. Revise vivado/P2_SEP.runs/impl_1/runme.log"
}

# Resumen de timing: WNS negativo = el diseno no cumple timing
open_run impl_1
report_timing_summary -file [file join $ROOT vivado timing_summary.txt]
puts "WNS = [get_property SLACK [get_timing_paths -max_paths 1 -setup]] ns"

write_hw_platform -fixed -include_bit -force -file $XSA
puts "============================================================"
puts " Bitstream listo y hardware exportado en $XSA"
puts "============================================================"
