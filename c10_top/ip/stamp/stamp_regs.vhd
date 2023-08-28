library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.fpga_ver.all;

entity stamp_regs is
   generic (
      C_DWIDTH             : integer   := 32;
      C_NUM_REG            : integer   := 16
   );
   port (
      clk                  : in    std_logic;
      reset_n              : in    std_logic;
      read_n               : in    std_logic;
      write_n              : in    std_logic;
      address              : in    std_logic_vector(9 downto 0);
      readdata             : out   std_logic_vector(31 downto 0);
      writedata            : in    std_logic_vector(31 downto 0);
      tp                   : out   std_logic_vector(3 downto 0)
   );
end stamp_regs;

architecture rtl of stamp_regs is

--
-- CONSTANTS
--

--
-- SIGNAL DECLARATIONS
--

signal rdCE                : std_logic_vector(C_NUM_REG-1 downto 0);
signal wrCE                : std_logic_vector(C_NUM_REG-1 downto 0);

signal stamp_TEST          : std_logic_vector(31 downto 0);
signal stamp_TP            : std_logic_vector(3 downto 0);
signal stamp_cnt           : unsigned(31 downto 0);

--
-- MAIN CODE
--
begin

   --
   -- COMBINATORIAL OUTPUTS
   --

   tp       <= stamp_TP;

   --
   -- READ REGISTER STROBES
   --
   process (all) begin
      for i in 0 to wrCE'length-1 loop
         if (to_integer(unsigned(address)) = i and write_n = '0') then
            wrCE(i) <= '1';
         else
            wrCE(i) <= '0';
         end if;
      end loop;
      for i in 0 to rdCE'length-1 loop
         if (to_integer(unsigned(address)) = i and read_n = '0') then
            rdCE(i) <= '1';
         else
            rdCE(i) <= '0';
         end if;
      end loop;
    end process;

   --
   -- WRITE REGISTERS
   --
   process (all) begin
      if (reset_n = '0') then
         stamp_TEST           <= (others => '0');
         stamp_TP             <= (others => '0');
      elsif (rising_edge(clk)) then
         if (wrCE(5) = '1') then
            stamp_TEST        <= writedata;
         elsif (wrCE(6) = '1') then
            stamp_TP          <= writedata(3 downto 0);
         else
            stamp_TEST        <= stamp_TEST;
            stamp_TP          <= stamp_TP;
         end if;
      end if;
   end process;

   --
   -- READ REGISTERS
   --
   process (all) begin
      if (rdCE(0) = '1') then
         readdata    <= X"000000" & C_BUILD_PID;
      elsif (rdCE(1) = '1') then
         readdata    <= C_BUILD_EPOCH_HEX;
      elsif (rdCE(2) = '1') then
         readdata    <= C_BUILD_DATE_HEX;
      elsif (rdCE(3) = '1') then
         readdata    <= C_BUILD_TIME_HEX;
      elsif (rdCE(4) = '1') then
         readdata    <= X"000000" & C_BUILD_INC;
      elsif (rdCE(5) = '1') then
         readdata    <= stamp_TEST;
      elsif (rdCE(6) = '1') then
         readdata    <= X"0000000" & stamp_TP;
      elsif (rdCE(7) = '1') then
         readdata    <= X"012355AA";
      elsif (rdCE(8) = '1') then
         readdata    <= std_logic_vector(stamp_cnt);
      elsif (rdCE(9) = '1') then
         -- 8-Bit H/W ID, 12-Bit Map Rev, 12-Bit Logic Rev
         readdata    <= C_BUILD_PID & C_BUILD_MAP & C_BUILD_LOGIC;
      elsif (rdCE(10) = '1') then
         -- Register Map Date
         readdata    <= C_BUILD_MAP_DATE;
      else
         readdata    <= (others => '0');
      end if;
   end process;

   --
   -- FREE RUNNING COUNTER, USED FOR SOFTWARE TIMERS
   --
   process (all) begin
      if (reset_n = '0') then
         stamp_cnt      <= (others => '0');
      elsif (rising_edge(clk)) then
         stamp_cnt      <= stamp_cnt + 1;
      end if;
   end process;

end rtl;
