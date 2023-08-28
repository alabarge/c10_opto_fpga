library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_textio.all;
library std;
use std.textio.all;

package PCK_tb IS	

	CONSTANT logfilename	: string := "tb.log";    -- log file
        FILE output : text open write_mode is logfilename;
	signal   observe_1      : std_logic;
	
   function f_stdtochar (
      std		: in	std_logic
      ) return character;

   function f_swapvec ( 
	   std		: in	std_logic_vector
	) return std_logic_vector;	

   function f_stdvtostr ( 
	   std		: in	std_logic_vector
	) return string;	

   function f_stdvtostrswap ( 
	   std		: in	std_logic_vector
	) return string;	

   function f_stdvtostrhex ( 
	   std		: in	std_logic_vector
	) return string;	

   function f_stdtostr ( 
	   std		: in	std_logic
	) return string;	

   function f_inttostr (
      int		: in	integer
      ) return string;

   FUNCTION f_sinttostr (
      int		: IN	integer
      ) RETURN string;

end PCK_tb;

package body PCK_tb IS
	
	-----------------------------------------------------------------------------
	-- CONVERSION FUCTIONS						      						   --
	-----------------------------------------------------------------------------
	
   -----------------------------------------------------------------------------
   -- Function name: stdtochar                                                --
   -- Function:      Converts a std_logic to a character                      --
   -----------------------------------------------------------------------------
   function f_stdtochar (
      std	: in	std_logic
      ) return character IS

   begin
      case std IS
         when '0'    => return '0';
         when '1'    => return '1';
         when 'X'    => return 'X';
         when 'Z'    => return 'Z';
         when 'W'    => return 'W';
         when 'L'    => return 'L';
         when 'H'    => return 'H';
         when others => return 'U';
      end case;
   end f_stdtochar;


	-----------------------------------------------------------------------------
	-- Function name: swapvec                                                 --
	-- Function:      Reverses order of significance of a std_logic_vector    --
	-----------------------------------------------------------------------------
	function f_swapvec (
		std	: in	std_logic_vector
	  ) return std_logic_vector IS

	  	variable tmpvec : std_logic_vector(std'LENGTH-1 downto 0);
		begin
			tmpvec := (others => '0');
			for i in 0 to (std'LENGTH-1)  loop
				tmpvec(std'LENGTH-i-1) := std(i);
			end loop;
		return tmpvec;
	end f_swapvec;				 
	
	-----------------------------------------------------------------------------
	-- Function name: stdvtostr                                                 --
	-- Function:      Converts a std_logic_vector to a binary string            --
	-----------------------------------------------------------------------------
	function f_stdvtostr (
		std	: in	std_logic_vector
	  ) return string IS

	  	variable tmpstr : string(1 to std'LENGTH);
		begin
			tmpstr := (others => '0');
			for i in std'REVERSE_range  loop
				case std(i) IS
					when '0'    => tmpstr(i+1) := '0';
					when '1'    => tmpstr(i+1) := '1';
					when 'X'    => tmpstr(i+1) := 'X';
					when 'Z'    => tmpstr(i+1) := 'Z';
					when 'W'    => tmpstr(i+1) := 'W';
					when 'L'    => tmpstr(i+1) := 'L';
					when 'H'    => tmpstr(i+1) := 'H';
					when others => tmpstr(i+1) := 'U';
				end case;
			end loop;
		return tmpstr(1 TO std'LENGTH);
	end f_stdvtostr;	   
	
	-----------------------------------------------------------------------------
	-- Function name: stdvtostrswap                                               --
	-- Function:      Converts a std_logic_vector to a reversed binary string            --
	-----------------------------------------------------------------------------
	function f_stdvtostrswap (
		std	: in	std_logic_vector
	  ) return string IS
	  	variable tmpstr : string(1 to std'LENGTH);
		begin
			tmpstr := (others => '0');
			for i in std'range  loop
				case std(i) IS
					when '0'    => tmpstr(i+1) := '0';
					when '1'    => tmpstr(i+1) := '1';
					when 'X'    => tmpstr(i+1) := 'X';
					when 'Z'    => tmpstr(i+1) := 'Z';
					when 'W'    => tmpstr(i+1) := 'W';
					when 'L'    => tmpstr(i+1) := 'L';
					when 'H'    => tmpstr(i+1) := 'H';
					when others => tmpstr(i+1) := 'U';
				end case;
			end loop;
		return tmpstr(1 TO std'LENGTH);
	end f_stdvtostrswap;	   
	
	-----------------------------------------------------------------------------
	-- Function name: stdvtostrhex                                              --
	-- Function:      Converts a std_logic_vector to a hex string               --	
	--				!!! only works on vectors that are a multiple of 4 bits !!! --
	--				!!! hex values other than 0 to F are simply assigned to U   --
	-----------------------------------------------------------------------------
	function f_stdvtostrhex (
		std	: in	std_logic_vector
	  ) return string IS
	    variable tmpstr : string(1 to std'LENGTH/4);	
	    variable j,k : integer;	
		variable tmpvec : std_logic_vector(3 downto 0);
		begin
			tmpstr := (others => '0');	   
			k := 0;
			for i in std'REVERSE_range  loop
				tmpvec  := "UUUU";
				for j in 0 to 3 loop
					case std(i) IS	
						when '0'    => tmpvec(j) := '0';
						when '1'    => tmpvec(j) := '1';
						when 'X'    => tmpvec(j) := 'X';
						when 'Z'    => tmpvec(j) := 'Z';
						when 'W'    => tmpvec(j) := 'W';
						when 'L'    => tmpvec(j) := 'L';
						when 'H'    => tmpvec(j) := 'H';
						when others => tmpvec(j) := 'U';
					end case; 
					if j = 3 then
						case tmpvec is
							when x"0"    => tmpstr(k) := '0';
							when x"1"    => tmpstr(k) := '1';
							when x"2"    => tmpstr(k) := '2';
							when x"3"    => tmpstr(k) := '3';
							when x"4"    => tmpstr(k) := '4';
							when x"5"    => tmpstr(k) := '5';
							when x"6"    => tmpstr(k) := '6';
							when x"7"    => tmpstr(k) := '7';
							when x"8"    => tmpstr(k) := '8';
							when x"9"    => tmpstr(k) := '9';
							when x"A"    => tmpstr(k) := 'A';
							when x"B"    => tmpstr(k) := 'B';
							when x"C"    => tmpstr(k) := 'C';
							when x"D"    => tmpstr(k) := 'D';
							when x"E"    => tmpstr(k) := 'E';
							when x"F"    => tmpstr(k) := 'F';
							when others => tmpstr(k) := 'U';	
						end case;	
						k := k+1;
					end if;
				end loop;
			end loop;
		return tmpstr(1 TO std'LENGTH/4);
	end f_stdvtostrhex;

	-----------------------------------------------------------------------------
	-- Function name: stdtostr                                                 --
	-- Function:      Converts a std_logic to a string                  --
	-----------------------------------------------------------------------------
	function f_stdtostr (
		std	: in	std_logic
	  ) return string IS
	begin
			case std IS
				when '0'    => return "0";
				when '1'    => return "1";
				when 'X'    => return "X";
				when 'Z'    => return "Z";
				when 'W'    => return "W";
				when 'L'    => return "L";
				when 'H'    => return "H";
				when others => return "U";
			end case;
	end f_stdtostr;

   -----------------------------------------------------------------------------
   -- Function name: inttostr                                                 --
   -- Function:      Converts an integer to a string                          --  
   -- jed 7/25/09:  added leading 0's to line up in columns better and        --
   --               the case of int = 0 was not handled!                      --
   -----------------------------------------------------------------------------
   function f_inttostr (
      int	: in	integer
      ) return string IS

      variable tmpstr     : string(1 TO 10);
      variable i          : integer;
      variable tmpint     : integer;
   begin
      tmpstr := (others => '0');
      i := 11;
      tmpint := int; 
	 while i > 1 loop
         i := i - 1;
         case tmpint REM 10 IS
            when 1      => tmpstr(i) := '1';
            when 2      => tmpstr(i) := '2';
            when 3      => tmpstr(i) := '3';
            when 4      => tmpstr(i) := '4';
            when 5      => tmpstr(i) := '5';
            when 6      => tmpstr(i) := '6';
            when 7      => tmpstr(i) := '7';
            when 8      => tmpstr(i) := '8';
            when 9      => tmpstr(i) := '9';
            when others => tmpstr(i) := '0';
         end case;
         tmpint := tmpint / 10;
      end loop;
      return tmpstr(i TO 10);
   end f_inttostr;

   -----------------------------------------------------------------------------
   -- Function name: sinttostr                                                 --
   -- Function:      Converts a signed integer to a string                          --  
   -----------------------------------------------------------------------------
   FUNCTION f_sinttostr (
      int	: IN	integer
      ) RETURN string IS

      VARIABLE tmpstr     : string(1 TO 10);
      VARIABLE i          : integer;
      VARIABLE tmpint     : integer;
   BEGIN
      tmpstr := (OTHERS => '0');
      i := 10;
      tmpint := int;   
	  IF (tmpint < 0) THEN
		  tmpstr(10) := '-';
	  ELSE
		  tmpstr(10) := ' ';
	  END IF;
	  WHILE i > 1 LOOP
         i := i - 1;
         CASE tmpint REM 10 IS	 			 
            WHEN 1      => tmpstr(i) := '1';
            WHEN 2      => tmpstr(i) := '2';
            WHEN 3      => tmpstr(i) := '3';
            WHEN 4      => tmpstr(i) := '4';
            WHEN 5      => tmpstr(i) := '5';
            WHEN 6      => tmpstr(i) := '6';
            WHEN 7      => tmpstr(i) := '7';
            WHEN 8      => tmpstr(i) := '8';
            WHEN 9      => tmpstr(i) := '9';
            WHEN OTHERS => tmpstr(i) := '0';
         END CASE;
         tmpint := tmpint / 10;
      END LOOP;
      RETURN tmpstr(i TO 10);
   END f_sinttostr;



 
END PCK_tb;	