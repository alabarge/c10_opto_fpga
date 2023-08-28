library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity adc_irq is
   generic (
      C_NUM_INT            : integer      := 2
   );
   port (
      clk                  : in    std_logic;
      reset_n              : in    std_logic;
      int_req              : out   std_logic_vector(C_NUM_INT-1 downto 0);
      int_ack              : in    std_logic_vector(C_NUM_INT-1 downto 0);
      int                  : in    std_logic_vector(C_NUM_INT-1 downto 0);
      irq                  : out   std_logic
   );
end entity adc_irq;

architecture rtl of adc_irq is

--
-- SIGNAL DECLARATIONS
--

   signal intReq           : std_logic_vector(C_NUM_INT-1 downto 0);

--
-- MAIN CODE
--
begin

   --
   -- INTERRUPTS
   --
   process (all) begin
      if (reset_n = '0') then
         intReq            <= (others => '0');
      elsif (rising_edge(clk)) then
         for i in 0 to int'length-1 loop
            if (int(i) = '1') then
               intReq(i)   <= '1';
            elsif (int_ack(i) = '1') then
               intReq(i)   <= '0';
            else
               intReq(i)   <= intReq(i);
            end if;
         end loop;
      end if;
   end process;

   --
   -- Combinatorial Signal Outputs
   --

   irq      <= intReq(0) or intReq(1);
   int_req  <= intReq;

end rtl;
