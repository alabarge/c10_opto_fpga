
library ieee;
use ieee.std_logic_1164.all; 
use std.textio.all;
use ieee.std_logic_textio.all;


package print_utilities is

--file data_file_handler : text open write_mode is "log.txt";

procedure print(f: in string; s: in string);
procedure print_sig_val(f: in string; address: std_logic_vector;data: std_logic_vector);
procedure print_sig_val(f: in string; arg: string);

end print_utilities;

package body  print_utilities  is
------------------------------------------------------------------------------------

--*******************************************
procedure print(f: in string; s: in string) is
    variable l: line;
    variable lineout: line;			   
--	file data_file_handler : text open write_mode is "log.txt";
	file data_file_handler : text open write_mode is (f & ".txt");
--******************************************    
 begin
    write(lineout, s);
    writeline(data_file_handler,lineout);
    
    write(l, s);
    writeline(output,l);
 end print;

--********************************************************************
procedure print_sig_val(f: in string; address: std_logic_vector;data: std_logic_vector)  is
  	variable tranx : line;
  	variable l : line;
	file data_file_handler : text open write_mode is (f & ".txt");
--********************************************************************  
begin
	--print to output file
	write(l,now, justified=>right,field =>10, unit=> ns );
	write(l,string'("  "));
	hwrite(l,address,justified=>left);
	write(l,string'("     "));
	hwrite(l,data);
	writeline(data_file_handler,l);
	
	write(tranx, now, justified=>right,field =>10, unit=> ns );
	write(tranx, string'("   "));
	hwrite(tranx,address,justified=>left);
	write(tranx, string'("     "));
	hwrite(tranx,data);
	writeline(output,tranx);
end print_sig_val;     

--********************************************************************
procedure print_sig_val(f: in string; arg: string)  is
  	variable tranx : line;
  	variable l : line;
	file data_file_handler : text open write_mode is (f & ".txt");
--********************************************************************  
begin
	--print to output file
	write(l, now, justified=>right,field =>10, unit=> ns );
	write(l, string'("   "));
	write(l,arg);
	writeline(data_file_handler,l);
	
	write(tranx, now, justified=>right,field =>10, unit=> ns );
	write(tranx, string'("   "));
	write(tranx,arg);
	writeline(output,tranx);
end print_sig_val;


end;
