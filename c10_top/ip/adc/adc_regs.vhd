library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity adc_regs is
   generic (
      C_DWIDTH             : integer   := 32;
      C_NUM_REG            : integer   := 6
   );
   port (
      clk                  : in    std_logic;
      reset_n              : in    std_logic;
      read_n               : in    std_logic;
      write_n              : in    std_logic;
      address              : in    std_logic_vector(10 downto 0);
      readdata             : out   std_logic_vector (31 downto 0);
      writedata            : in    std_logic_vector (31 downto 0);
      cpu_DIN              : in    std_logic_vector(31 downto 0);
      cpu_DOUT             : out   std_logic_vector(31 downto 0);
      cpu_ADDR             : out   std_logic_vector(10 downto 0);
      cpu_WE               : out   std_logic;
      cpu_RE               : out   std_logic;
      adc_CONTROL          : out   std_logic_vector(31 downto 0);
      adc_INT_REQ          : in    std_logic_vector(1 downto 0);
      adc_INT_ACK          : out   std_logic_vector(1 downto 0);
      adc_STATUS           : in    std_logic_vector(31 downto 0);
      adc_ADR_BEG          : out   std_logic_vector(31 downto 0);
      adc_ADR_END          : out   std_logic_vector(31 downto 0);
      adc_PKT_CNT          : out   std_logic_vector(31 downto 0);
      adc_POOL_CNT         : out   std_logic_vector(7 downto 0);
      adc_ADC_RATE         : out   std_logic_vector(15 downto 0);
      adc_DEV_CFG          : out   std_logic_vector(15 downto 0);
      adc_PORT_CFG         : out   std_logic_vector(15 downto 0)
   );
end adc_regs;

architecture rtl of adc_regs is

--
-- CONSTANTS
--
constant C_ADC_VERSION     : std_logic_vector(7 downto 0)  := X"02";
constant C_ADC_CONTROL     : std_logic_vector(31 downto 0) := X"00000F00";
constant C_ADC_DEV_CFG     : std_logic_vector(15 downto 0) := X"0073";
constant C_ADC_PORT_CFG    : std_logic_vector(15 downto 0) := X"7200";

--
-- SIGNAL DECLARATIONS
--
signal wrCE                : std_logic_vector(C_NUM_REG-1 downto 0);
signal rdCE                : std_logic_vector(C_NUM_REG-1 downto 0);

--
-- MAIN CODE
--
begin

   --
   -- COMBINATORIAL OUTPUTS
   --

   -- Write Block RAM
   cpu_DOUT             <= writedata;
   cpu_WE               <= '1' when (address(10) = '1' and write_n = '0') else '0'; 
   cpu_RE               <= '1' when (address(10) = '1' and read_n  = '0') else '0'; 
   cpu_ADDR             <= address;

   --
   -- READ/WRITE REGISTER STROBES
   --
   process (all) begin
      for i in 0 to wrCE'length-1 loop
         if (address(4 downto 0) = std_logic_vector(to_unsigned(i, 5)) and 
                  address(10) = '0' and write_n = '0') then
            wrCE(i) <= '1';
         else
            wrCE(i) <= '0';
         end if;
      end loop;
      for i in 0 to rdCE'length-1 loop
         if (address(4 downto 0) = std_logic_vector(to_unsigned(i, 5)) and 
                  address(10) = '0' and read_n = '0') then
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
         adc_CONTROL          <= C_ADC_CONTROL;
         adc_INT_ACK          <= (others => '0');
         adc_ADR_BEG          <= (others => '0');
         adc_ADR_END          <= (others => '0');
         adc_PKT_CNT          <= (others => '0');
         adc_POOL_CNT         <= (others => '0');
         adc_ADC_RATE         <= (others => '0');
         adc_DEV_CFG          <= C_ADC_DEV_CFG;
         adc_PORT_CFG         <= C_ADC_PORT_CFG;
      elsif (rising_edge(clk)) then
         if (wrCE(0) = '1') then
            adc_CONTROL       <= writedata;
         elsif (wrCE(2) = '1') then
            adc_INT_ACK       <= writedata(1 downto 0);
         elsif (wrCE(4) = '1') then
            adc_ADR_BEG       <= writedata;
         elsif (wrCE(5) = '1') then
            adc_ADR_END       <= writedata;
         elsif (wrCE(6) = '1') then
            adc_PKT_CNT       <= writedata;
         elsif (wrCE(7) = '1') then
            adc_POOL_CNT      <= writedata(7 downto 0);
         elsif (wrCE(8) = '1') then
            adc_ADC_RATE      <= writedata(15 downto 0);
         elsif (wrCE(9) = '1') then
            adc_DEV_CFG       <= writedata(15 downto 0);
         elsif (wrCE(10) = '1') then
            adc_PORT_CFG      <= writedata(15 downto 0);
         else
            adc_CONTROL       <= adc_CONTROL;
            adc_INT_ACK       <= (others => '0');
            adc_ADR_BEG       <= adc_ADR_BEG;
            adc_ADR_END       <= adc_ADR_END;
            adc_PKT_CNT       <= adc_PKT_CNT;
            adc_POOL_CNT      <= adc_POOL_CNT;
            adc_ADC_RATE      <= adc_ADC_RATE;
            adc_DEV_CFG       <= adc_DEV_CFG;
            adc_PORT_CFG      <= adc_PORT_CFG;
         end if;
      end if;
   end process;

   --
   -- READ REGISTERS
   --
   process (all) begin
      if (rdCE(0) = '1') then
         readdata             <= adc_CONTROL;
      elsif (rdCE(1) = '1') then
         readdata             <= X"000000" & C_ADC_VERSION;
      elsif (rdCE(2) = '1') then
         readdata             <= X"0000000" & "00" & adc_INT_REQ;
      elsif (rdCE(3) = '1') then
         readdata             <= adc_STATUS;
      elsif (rdCE(4) = '1') then
         readdata             <= adc_ADR_BEG;
      elsif (rdCE(5) = '1') then
         readdata             <= adc_ADR_END;
      elsif (rdCE(6) = '1') then
         readdata             <= adc_PKT_CNT;
      elsif (rdCE(7) = '1') then
         readdata             <= X"000000" & adc_POOL_CNT;
       elsif (rdCE(8) = '1') then
         readdata             <= X"0000" & adc_ADC_RATE;
       elsif (rdCE(9) = '1') then
         readdata             <= X"0000" & adc_DEV_CFG;
       elsif (rdCE(10) = '1') then
         readdata             <= X"0000" & adc_PORT_CFG;
      --
      -- READ BLOCK RAM
      --
      elsif (address(10) = '1' and read_n = '0') then
         readdata             <= cpu_DIN;
      else
         readdata             <= (others => '0');
      end if;
   end process;

end rtl;
