/*****************************************************************************/
//     File Name: cpld_top.v
//          Date: Oct 15th, 2004
//
//        Author: ZhangYong
//         Email: yzhang@centralitycomm.com.cn
//         Phone: 86-21-58547127*2018
//       Company: Centrality Communications Inc(ShangHai).
//
//   Description: AtlasII-EVB-MB FPGA logic
//
//
// CVS Log
// Change History:
//               $Log: not supported by cvs2svn $
//
/*****************************************************************************/

`timescale 1 ns / 10 ps
`include "cpld_defines.v"

module cpld_top	(
		x_fd,
		x_fa,
		x_fa_in,
		reboot_b,
		rst_b,
		vpll_on,
		vmem_on,
		vddpre_on,
		vddpdn_on,
		vio_on,
		vgps_on,
		led,
		flash_wp,
		x_gpio0,
		rtc_clk,
		module_id,
		main_id0,
		main_id1,
		main_id2,
		ecki,   //evb_reboot input for QA test
		edi,    //evb_rst input for QA test
		evbin2, //evb_slp_wkp input for QA test
		evbin3, //rst loop feed back
		ecko,	//rst loop out
		edo,    //evbout1
		evbout2,
		evbout3,
		x_fce0,
		x_fce1,
		x_fce2,
		x_fce3,
		x_fwe,
		x_foe,
		x_pwr_en,
		x_reset_b,
		rst_out,
		rst_in,
		dbg_ce1,
		dbg_ce0,
		dbg_mod,
		nand_cs0,
		nand_cs1,
		nand_cs2,
		nand_cs3		
			);
         
inout	[7:0]	x_fd;         
inout	[21:13]	x_fa; 
input	[12:9]	x_fa_in;
input		reboot_b;     
input		rst_b;        
output		vpll_on;      
output		vmem_on;      
output		vddpre_on;    
output		vddpdn_on;    
output		vio_on;       
output		vgps_on;      
output	[3:0]	led;          
inout		flash_wp;          
input		x_gpio0;      
input		rtc_clk;      
input	[0:2]	module_id;    
output		main_id0;	// pwr_on main board peripherals 
input		main_id1;
input		main_id2;     
input		ecki;    //evbin0     
input		edi;     //evbin1     
input		evbin2;       
input		evbin3;       
output		ecko;         
output		edo;          
output		evbout2;      
output		evbout3;      
input		x_fce0;       
input		x_fce1;       
input		x_fce2;       
input		x_fce3;       
input		x_fwe;        
input		x_foe;        
input		x_pwr_en;     
output		x_reset_b;    
inout		rst_out;      
input		rst_in;       
output		dbg_ce1;      
output		dbg_ce0;  
output		dbg_mod;    
inout		nand_cs0;     
inout		nand_cs1;     
inout		nand_cs2;     
inout		nand_cs3;    

           
wire	[7:0]	x_fd;                
wire		reboot_b;     
wire		rst_b;        
wire		vpll_on;      
wire		vmem_on;      
wire		vddpre_on;    
wire		vddpdn_on;    
wire		vio_on;       
wire		vgps_on;      
wire	[3:0]	led;          
wire		flash_wp;          
wire		x_gpio0;      
wire		rtc_clk;      
wire	[0:2]	module_id;    
wire		main_id0;	// pwr_on main board peripherals 
wire		main_id2;     
wire		ecki;    //evbin0     
wire		edi;     //evbin1     
wire		evbin2;       
wire		evbin3;       
wire		ecko;         
wire		edo;          
wire		evbout2;      
wire		evbout3;      
wire		x_fce0;       
wire		x_fce1;       
wire		x_fce2;       
wire		x_fce3;       
wire		x_fwe;        
wire		x_foe;        
wire		x_pwr_en;     
wire		x_reset_b;    
wire		rst_out;      
wire		rst_in;       
wire		dbg_ce1;      
wire		dbg_ce0;      
wire		nand_cs0;     
wire		nand_cs1;     
wire		nand_cs2;     
wire		nand_cs3;  

///////////////////////////////////////////////////////////////
// Variable define
wire		rbt;	// reboot, active high
wire		rst_lo;	// reset loop out
wire		rst_lb = evbin3;	// reset loop back
wire	[0:3]	sw1;
wire	[0:3]	sw2;
reg	[7:0]	fd;                    
reg		fd_en;                 
wire		fce1_we = x_fce1|x_fwe;
wire		fce1_oe = x_fce1|x_foe;
wire		evb_rbt = ecki;
wire		evb_rst	= edi;
wire		evb_slp_wkp = ~evbin2;
wire		timing_rst;
reg		por_ctrl_rbt; // synthesis attribute init of por_ctrl_rbt is ¡°1'b0¡±;
wire		por_ctrl_rbt_pulse;
wire		por;	// power on reset, active high

///////////////////////////////////////////////////////////////
// Resgisters Define
reg		vgps_cm_on;
reg		flash_wp_reg;
reg		vddpdn_cfg;
reg	[7:0]	fce0_cfg;	
reg	[3:0]	fce2_cfg;	
reg	[3:0]	fce3_cfg;	
reg	[3:1] 	evb_port;	
reg	[3:0] 	led_cm;	
  
///////////////////////////////////////////////////////////////
// Powr on logic
reg	[2:0]	unit_counter;
wire		timing_clk;
reg	[7:0]	timing_counter;  // synthesis attribute init of timing_counter is ¡°8'h00¡±;

always	@(posedge rbt or posedge rtc_clk) 
    begin
    	if(rbt) unit_counter <= #1 0;
    	else	unit_counter <= unit_counter+1;
    end

assign	timing_clk = unit_counter[2];
			
always	@(posedge rbt or posedge timing_clk) 
    begin
    	if(rbt) timing_counter <= #1 8'h00;
    	else if(timing_counter!=8'hff) timing_counter <= #1 timing_counter+1;
    end

//
//no power on logic
//

// synthesis attribute init of vio_on is ¡°1'b1¡±;
// synthesis attribute init of vpll_on is ¡°1'b1¡±;
// synthesis attribute init of vmem_on is ¡°1'b1¡±;
// synthesis attribute init of vddpre_on is ¡°1'b1¡±;
// synthesis attribute init of vddpdn_on is ¡°1'b1¡±;
// synthesis attribute init of main_id0 is ¡°1'b1¡±;
// synthesis attribute init of timing_rst is ¡°1'b0¡±;

assign	 vio_on	 = 	1'b1;
assign	 vpll_on = 	1'b1;
assign	 vmem_on = 	1'b1;
assign	 vddpre_on =  1'b1;
assign	 vddpdn_on =  vddpdn_cfg ? x_pwr_en : 1'b1;
assign	 main_id0 = 	1'b1;
assign	 timing_rst =  1'b0;

//
// end "no power on logic"
//

// synthesis attribute init of por_ctrl_rbt is ¡°1'b0¡±;
/*

//  power on sequence logic
always	@(posedge rtc_clk)
    begin
	 vio_on	 <= #1	(timing_counter < `VDDIO_ON_TIMING)? 1'b0 : 1'b1;
	 vpll_on <= #1	(timing_counter < `VPLL_ON_TIMING)? 1'b0 : 1'b1;
	 vmem_on <= #1	(timing_counter < `VMEM_ON_TIMING)? 1'b0 : 1'b1;
	 vddpre_on <= #1 (timing_counter < `VDDPRE_ON_TIMING)? 1'b0 : 1'b1;
	 vddpdn_on <= #1 vddpdn_cfg ? x_pwr_en :((timing_counter < `VDDPDN_ON_TIMING)? 1'b0 : 1'b1);
	 main_id0 <= #1	(timing_counter < `MB_PWR_ON_TIMING)? 1'b0 : 1'b1;
	 timing_rst <= #1 (timing_counter < `RST_TIMING)? 1'b1 : 1'b0;
    end
  */

///////////////////////////////////////////////////////////////
// Power on, Reboot, Reset logic

//Reboot
assign	rbt = evb_rbt|(~reboot_b)|por_ctrl_rbt_pulse;

// Power on reset
assign	por = (timing_counter==8'h02)? 1'b1 :1'b0;

// Reset
wire	rst_wo_rst_out;	// reset without rst_out
assign	rst_wo_rst_out	= evb_rst|(~rst_b)|(~rst_in)|timing_rst;
assign	rst_out 	= rst_wo_rst_out? 1'b0 : 1'bz;
assign	rst_lo		= rst_wo_rst_out|(~rst_out);
assign	x_reset_b 	= ~rst_lo;
assign	ecko 		= ~rst_lo;

//
reg	pre_por_ctrl_rbt; // synthesis attribute init of pre_por_ctrl_rbt is ¡°1'b0¡±;
always	@(posedge rtc_clk) pre_por_ctrl_rbt <= #1 por_ctrl_rbt; 
assign 	por_ctrl_rbt_pulse = pre_por_ctrl_rbt&(pre_por_ctrl_rbt==1'b0);

  	
///////////////////////////////////////////////////////////////           
// Registers reset and write    
always	@(posedge por or posedge fce1_we)
    begin             
	if(por) 
	begin
		vgps_cm_on		<= #1 `VGPS_CM_ON;
		flash_wp_reg		<= #1 `FLASH_WP;
		vddpdn_cfg		<= #1 `VDDPDN_CFG;
		fce2_cfg		<= #1 `FCE2_CFG;	
		fce3_cfg		<= #1 `FCE3_CFG;
		evb_port		<= #1 `EVB_PORT;			
	        led_cm			<= #1 `LED_CM;
	        por_ctrl_rbt		<= #1 1'b0;
	end             
	else  
	case ({x_fa[14:13],x_fa_in[12:9], 1'b0}) 
		`POR_CTRL_ADDR:		por_ctrl_rbt <=#1 x_fd[0];
		`SYS_CTRL_ADDR:		begin vgps_cm_on<= #1  x_fd[0]; flash_wp_reg<= #1 x_fd[4]; vddpdn_cfg<= #1 x_fd[5]; end
		`FCE2_CFG_ADDR:		fce2_cfg	<= #1  x_fd[3:0];
		`FCE3_CFG_ADDR:		fce3_cfg	<= #1  x_fd[3:0];
		`EVB_PORT_ADDR:		evb_port	<= #1  x_fd[3:1];
		`LED_CM_ADDR:		led_cm		<= #1  x_fd[3:0];
		default: 		;
	endcase  
    end         

// Registers Read
always	@(negedge fce1_oe)
    begin              
	case ({x_fa[14:13],x_fa_in[12:9], 1'b0}) 
		`SYS_CTRL_ADDR:		begin	fd_en<= #1 ~main_id2; 
						fd[0]<= #1 vgps_cm_on; 
						fd[4]<= #1 flash_wp_reg;
						fd[5]<= #1 vddpdn_cfg; 
					end	
		`FCE0_CFG_ADDR:		begin fd_en<= #1 ~main_id2; fd[7:0]<= #1 fce0_cfg; end	
		`FCE2_CFG_ADDR:		begin fd_en<= #1 ~main_id2; fd[3:0]<= #1 fce2_cfg; end
		`FCE3_CFG_ADDR:		begin fd_en<= #1 ~main_id2; fd[3:0]<= #1 fce3_cfg; end	
		default: 		fd_en<= #1 1'b0; 
	endcase  
    end    
         

///////////////////////////////////////////////////////////////
// x_fa output
assign	x_fa[21:18] =rst_lb ? 	((sw1[0:1]==2'b00)? 4'b0000:
				((sw1[0:1]==2'b01)? 4'b0110:
				((sw1[0:1]==2'b10)? 4'b1010:
				((sw1[0:1]==2'b11)? 4'b1110: 
				4'b0000)))) : 4'hz;
assign	x_fa[17]    =rst_lb ? 	((sw2[0:1]==2'b01)? 1'b1:1'b0):1'bz; 
assign	x_fa[16]    =rst_lb ? 	1'b1: 1'bz; 
assign	x_fa[15]    =rst_lb ? 	sw2[3]: 1'bz;
assign	x_fa[14:13] =rst_lb ?	sw1[2:3]: 2'bzz;
//assign	x_fa[12:9]  = 4'hz;

///////////////////////////////////////////////////////////////
// x_fd output
assign	x_fd[7:0]   = ((!rst_lb)&&fd_en&&(!(x_fce1||x_foe)))? fd[7:0]:8'hzz;

/////////////////////////////////////////////////////////////////////
// Other Outputs

assign	vgps_on	= vgps_cm_on;
assign	flash_wp = rst_lb ? 1'bz : flash_wp_reg;

 /*
assign	nand_cs0 = rst_lb ? 1'bz : ((fce0_cfg[3:1]==3'b000)? x_fce0:
				   ((fce2_cfg[3:0]==4'h0)? x_fce2:
				   ((fce3_cfg[3:0]==4'h0)? x_fce3: 1'b1)));
assign	nand_cs1 = rst_lb ? 1'bz : ((fce0_cfg[3:1]==3'b001)? x_fce0:
				   ((fce2_cfg[3:0]==4'h1)? x_fce2:
				   ((fce3_cfg[3:0]==4'h1)? x_fce3 :1'b1)));
assign	nand_cs2 = rst_lb ? 1'bz : ((fce0_cfg[3:1]==3'b010)? x_fce0:
				   ((fce2_cfg[3:0]==4'h2)? x_fce2:
				   ((fce3_cfg[3:0]==4'h2)? x_fce3: 1'b1)));
assign	nand_cs3 = rst_lb ? 1'bz : ((fce0_cfg[3:1]==3'b011)? x_fce0:
				   ((fce2_cfg[3:0]==4'h3)? x_fce2:
				   ((fce3_cfg[3:0]==4'h3)? x_fce3: 1'b1)));
*/
assign	nand_cs0 = rst_lb ? 1'bz : ((fce0_cfg[3:1]==3'b000)? x_fce0: ((fce2_cfg[3:0]==4'h0)? x_fce2: 1'b1));
assign	nand_cs1 = rst_lb ? 1'bz : ((fce0_cfg[3:1]==3'b001)? x_fce0: ((fce2_cfg[3:0]==4'h1)? x_fce2: 1'b1));
assign	nand_cs2 = rst_lb ? 1'bz : ((fce0_cfg[3:1]==3'b010)? x_fce0: ((fce2_cfg[3:0]==4'h2)? x_fce2: 1'b1));
assign	nand_cs3 = rst_lb ? 1'bz : ((fce0_cfg[3:1]==3'b011)? x_fce0: ((fce2_cfg[3:0]==4'h3)? x_fce2: 1'b1));

assign	dbg_ce0 = rst_lb ? 1'b1 : ((fce0_cfg[3:1]==3'b111)? x_fce0:
				   ((fce2_cfg[3:0]==4'h8)? x_fce2:
				   ((fce3_cfg[3:0]==4'h8)? x_fce3: 1'b1)));
assign	dbg_ce1 = x_fce1|({x_fa[14:13],x_fa_in[12]}!=3'b000);
assign	dbg_mod	= main_id1;

assign	edo	= evb_port[1];
assign	evbout2	= evb_port[2];
assign	evbout3	= evb_port[3];

assign	led[3:0]= led_cm[3:0];
//assign	x_gpio0	= evb_slp_wkp;

/////////////////////////////////////////////////////////////////////
// Misc Logic

// SW1 and SW2
assign	sw1[0:3] = {module_id[0:2], flash_wp};
assign	sw2[0:3] = {nand_cs0, nand_cs1, nand_cs2, nand_cs3};

always @(negedge rst_lo) 
	fce0_cfg[7:0] <= #1 {sw1[0:3], sw2[0:3]};

endmodule
