# =============================================================================
#  02_hw_avance.tcl  --  Proyecto 2 IEE2463 (SEP)
# -----------------------------------------------------------------------------
#  Hardware que el GRUPO agrega sobre la plantilla del curso (AO2):
#
#    axi_timer_0  --IRQ--\
#    axi_timer_1  --IRQ---> xlconcat_0 --(dout[2:0])--> PS7 IRQ_F2P
#    axi_gpio_luz --IRQ--/        (In0, In1, In2)
#        ^
#        +-- pin U15 (JD) <- INT del OPT3001 (sensor de luz de la Booster)
#
#  - 2 AXI Timer independientes => 2 interrupciones por timer (AO2).
#  - AXI GPIO de 1 bit, solo entrada, con interrupcion => interrupcion del
#    sensor de luz (AO2). Sus umbrales se programan por I2C en los registros
#    0x01/0x02/0x03 del OPT3001 (eso ademas cumple "modificar 2 registros" de AO1).
#
#  Se llama desde 01_crear_proyecto.tcl, pero tambien se puede correr solo con
#  el proyecto ya abierto:  source scripts/02_hw_avance.tcl
# =============================================================================

set BD_FILE [get_files DEMO.bd]
open_bd_design $BD_FILE

if {[llength [get_bd_cells -quiet axi_timer_0]] > 0} {
    puts "INFO: el hardware del avance ya existe en el block design; no se hace nada."
    return
}

# --- PS7: habilitar interrupciones desde la logica programable (IRQ_F2P) ----------
set_property -dict [list \
    CONFIG.PCW_USE_FABRIC_INTERRUPT {1} \
    CONFIG.PCW_IRQ_F2P_INTR {1} \
] [get_bd_cells processing_system7_0]

# --- Bloques nuevos ------------------------------------------------------------
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_timer:2.0 axi_timer_0
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_timer:2.0 axi_timer_1

create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_luz
set_property -dict [list \
    CONFIG.C_GPIO_WIDTH {1} \
    CONFIG.C_ALL_INPUTS {1} \
    CONFIG.C_INTERRUPT_PRESENT {1} \
] [get_bd_cells axi_gpio_luz]

create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_0
set_property CONFIG.NUM_PORTS {3} [get_bd_cells xlconcat_0]

# --- Conexion AXI al procesador (reutiliza el interconnect de la plantilla) -----
foreach slave {axi_timer_0/S_AXI axi_timer_1/S_AXI axi_gpio_luz/S_AXI} {
    apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config [list \
        Clk_master {Auto} Clk_slave {Auto} Clk_xbar {Auto} \
        Master {/processing_system7_0/M_AXI_GP0} Slave "/$slave" \
        ddr_seg {Auto} intc_ip {/ps7_0_axi_periph} master_apm {0} \
    ] [get_bd_intf_pins $slave]
}

# --- Pin externo de la interrupcion de luz --------------------------------------
# El puerto queda como "interrupt_luz_tri_i", igual que en constraints/Zybo-Z7-Master.xdc
create_bd_intf_port -mode Master -vlnv xilinx.com:interface:gpio_rtl:1.0 interrupt_luz
connect_bd_intf_net [get_bd_intf_ports interrupt_luz] [get_bd_intf_pins axi_gpio_luz/GPIO]

# --- Interrupciones -> Concat -> PS7 ---------------------------------------------
connect_bd_net [get_bd_pins axi_timer_0/interrupt]    [get_bd_pins xlconcat_0/In0]
connect_bd_net [get_bd_pins axi_timer_1/interrupt]    [get_bd_pins xlconcat_0/In1]
connect_bd_net [get_bd_pins axi_gpio_luz/ip2intc_irpt] [get_bd_pins xlconcat_0/In2]
connect_bd_net [get_bd_pins xlconcat_0/dout]          [get_bd_pins processing_system7_0/IRQ_F2P]

# --- Direcciones, validacion y wrapper -----------------------------------------
assign_bd_address
regenerate_bd_layout
validate_bd_design
save_bd_design

set wrapper [make_wrapper -files $BD_FILE -top -force]
if {[llength [get_files -quiet $wrapper]] == 0} {
    add_files -norecurse $wrapper
}
set_property top DEMO_wrapper [current_fileset]
update_compile_order -fileset sources_1

puts "INFO: hardware del avance agregado (2 AXI Timer + GPIO luz + Concat -> IRQ_F2P)."
