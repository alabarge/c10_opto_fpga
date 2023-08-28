library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library std;
use std.textio.all;

use work.pck_fio.all;
use work.pck_tb.all;

-- Test Bench Entity
entity adc_top_tb is
end adc_top_tb;

architecture tb_arch of adc_top_tb is

-- 32-Bit Control Register
signal adc_CONTROL         : std_logic_vector(31  downto 0) := X"00000000";
alias  xl_SCAN             : std_logic is adc_CONTROL(26);
alias  xl_RAMP             : std_logic is adc_CONTROL(27);
alias  xl_RUN              : std_logic is adc_CONTROL(28);
alias  xl_PKT_INT_EN       : std_logic is adc_CONTROL(29);
alias  xl_DONE_INT_EN      : std_logic is adc_CONTROL(30);
alias  xl_ENABLE           : std_logic is adc_CONTROL(31);

-- Stimulus signals - signals mapped to the input and inout ports of tested entity
signal clk                 : std_logic := '0';
signal reset_n             : std_logic := '0';
signal m1_wr_waitreq       : std_logic := '0';
signal read_n              : std_logic := '1';
signal write_n             : std_logic := '1';
signal address             : std_logic_vector(10 downto 0) := "000" & X"00";
signal writedata           : std_logic_vector(31 downto 0) := X"00000000";
signal miso                : std_logic := '0';
signal intb_n              : std_logic := '1';

-- Observed signals - signals mapped to the output ports of tested entity
signal m1_write            : std_logic;
signal m1_wr_address       : std_logic_vector(31 downto 0);
signal m1_writedata        : std_logic_vector(31 downto 0);
signal m1_wr_burstcount    : std_logic_vector(8 downto 0);
signal readdata            : std_logic_vector(31 downto 0);
signal irq                 : std_logic;
signal sclk                : std_logic;
signal cs_n                : std_logic;
signal mosi                : std_logic;
signal cnvtb_n             : std_logic;

signal ramp                : unsigned(11 downto 0) := X"000";
signal sout                : unsigned(23 downto 0) := X"000000";
signal sclk_r0             : std_logic := '0';
signal intb_r0             : std_logic := '0';
signal adc_read            : std_logic := '0';
signal ad_bit_cnt          : integer range 0 to 32 := 0;

-- constants
constant C_CLK_PERIOD:     TIME :=  10.000 ns;    -- 100 MHz
constant C_ADC_INT_PERIOD: TIME :=  20.000 us;    --  50 KHz

begin

   --
   -- Unit Under Test
   --
   ADC_TOP_I : entity work.adc_pixi
   port map (
      -- Avalon Clock & Reset
      clk                  => clk,
      reset_n              => reset_n,
      irq                  => irq,
      -- Avalon Memory-Mapped Write Master
      m1_write             => m1_write,
      m1_wr_address        => m1_wr_address,
      m1_writedata         => m1_writedata,
      m1_wr_waitreq        => m1_wr_waitreq,
      m1_wr_burstcount     => m1_wr_burstcount,
      -- Memory Head-Tail Pointers
      head_addr            => open,
      tail_addr            => X"0000",
      -- Avalon Memory-Mapped Slave
      read_n               => read_n,
      write_n              => write_n,
      address              => address,
      readdata             => readdata,
      writedata            => writedata,
      -- Exported Signals
      sclk                 => sclk,
      cs_n                 => cs_n,
      mosi                 => mosi,
      miso                 => miso,
      cnvtb_n              => cnvtb_n,
      intb_n               => intb_n
   );

   --
   -- Clocks
   --

   --
   -- 100 MHZ
   --
   process begin
      clk <= '1';
      wait for C_CLK_PERIOD/2;
      clk <= '0';
      wait for C_CLK_PERIOD/2;
   end process;

   --
   -- 50 KHZ
   --
   process begin
      intb_n <= '1';
      wait for C_ADC_INT_PERIOD/2;
      intb_n <= '0';
      wait for C_ADC_INT_PERIOD/2;
   end process;

   --
   -- Reset
   --
   process begin
      reset_n <= '0';
      wait for 10*C_CLK_PERIOD;
      reset_n <= '1';
      wait;
   end process;

   --
   -- Capture master write burst
   --
   process
      file outadc : text;
      variable l  : line;
   begin
      wait until reset_n = '1';
      file_open(outadc, "adc_out.txt", write_mode);
      loop
         wait until rising_edge(clk);
         if (m1_write = '1') then
            fprint(outadc, l, "Tc=%4d ns, adc_d=%r\n", fo(NOW), fo(m1_writedata));
         end if;
      end loop;
   end process;

   --
   -- Generate ADC Serial Ramp Input
   --
   process begin
      wait until reset_n = '1';
      ramp  <= (others => '0');
      adc_read <= '0';
      loop
         wait until rising_edge(clk);
         sclk_r0        <= sclk;
         intb_r0        <= intb_n;
         if ((xl_SCAN = '0' and cnvtb_n = '0') or
             (xl_SCAN = '1' and intb_r0 = '1' and intb_n = '0')) then
            ad_bit_cnt  <= 0;
            sout        <= "00000000000" & ramp(11 downto 0) & '0';
            miso        <= '0';
            adc_read    <= '1';
         elsif (sclk_r0 = '1' and sclk = '0' and cs_n = '0' and adc_read = '1') then
            miso        <= std_logic(sout(23));
            sout        <= sout(22 downto 0) & '0';
            ad_bit_cnt  <= ad_bit_cnt + 1;
         elsif (ad_bit_cnt = 24) then
            ad_bit_cnt  <= 0;
            ramp        <= ramp + 1;
            adc_read    <= '0';
         end if;
      end loop;
   end process;

   --
   -- Main Process
   --
   process

   procedure BUS_WR(addr: in std_logic_vector(11 downto 0);
                    data: in std_logic_vector(31 downto 0)) is
   begin
      wait until rising_edge(clk);
      wait for (1 ns);
      write_n     <= '0';
      address     <= addr(10 downto 0);
      writedata   <= data;
      wait until rising_edge(clk);
      wait for (1 ns);
      write_n     <= '1';
      address     <= (others => '0');
      writedata   <= (others => '0');
   end;

   procedure BUS_RD(addr: in std_logic_vector(11 downto 0)) is
   begin
      wait until rising_edge(clk);
      wait for (1 ns);
      read_n      <= '0';
      address     <= addr(10 downto 0);
      wait until rising_edge(clk);
      wait for (1 ns);
      read_n      <= '1';
      address     <= (others => '0');
   end;

   begin

      wait until reset_n = '1';

      -- Enable module
      xl_ENABLE       <= '1';
      BUS_WR(X"000", adc_CONTROL);  -- CONTROL

      wait for 100 ns;

      -- Register Setup
      xl_SCAN        <= '1';
      xl_PKT_INT_EN  <= '1';
      xl_DONE_INT_EN <= '1';
      xl_RUN         <= '1';
      xl_RAMP        <= '0';
      BUS_WR(X"004", X"03000000");  -- ADDRESS BEGIN
      BUS_WR(X"005", X"03007FFF");  -- ADDRESS END
      BUS_WR(X"006", X"00000001");  -- PACKET COUNT
      BUS_WR(X"007", X"00000001");  -- POOL COUNT
      BUS_WR(X"008", X"000001FA");  -- ADC RATE
      BUS_WR(X"009", X"00000072");  -- DEV CFG
      BUS_WR(X"00A", X"00007200");  -- PORT CFG
      BUS_WR(X"000", adc_CONTROL);  -- CONTROL

      BUS_RD(X"001");               -- VERSION

      wait;

   end process;

end tb_arch;

