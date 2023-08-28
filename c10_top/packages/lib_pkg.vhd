library ieee;
use ieee.std_logic_1164.ALL;

package lib_pkg is

   --
   -- AD5678 Data Type
   --
   type   dac_data_t is array (0 to 7) of std_logic_vector(15 downto 0);

end lib_pkg;



