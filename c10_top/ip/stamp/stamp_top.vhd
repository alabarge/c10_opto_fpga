library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity stamp_top is
   generic (
      C_DWIDTH             : integer              := 32;
      C_NUM_REG            : integer              := 16
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
end entity stamp_top;

architecture rtl of stamp_top is

--
-- SIGNAL DECLARATIONS
--

--
-- MAIN CODE
--
begin

   --
   -- COMBINATORIAL OUTPUTS
   --

   --
   -- REGISTER FILE
   --
   STAMP_REGS_I: entity work.stamp_regs
   generic map (
      C_DWIDTH             => C_DWIDTH,
      C_NUM_REG            => C_NUM_REG
   )
   port map (
      clk                  => clk,
      reset_n              => reset_n,
      read_n               => read_n,
      write_n              => write_n,
      address              => address,
      readdata             => readdata,
      writedata            => writedata,
      tp                   => tp
   );

end rtl;
