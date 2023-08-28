library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.lib_pkg.all;

entity c10_top is
   port(
      -- Global Signals
      iCLK_12M             : in    std_logic;
      iRSTn                : in    std_logic;

      -- DRAM Interface
      oDRAM_ADDR           : out   std_logic_vector(11 downto 0);
      oDRAM_BA             : out   std_logic_vector(1 downto 0);
      oDRAM_CASn           : out   std_logic;
      oDRAM_CKE            : out   std_logic;
      oDRAM_CLK            : out   std_logic;
      oDRAM_CSn            : out   std_logic;
      ioDRAM_DQ            : inout std_logic_vector(15 downto 0);
      oDRAM_DQM            : out   std_logic_vector(1 downto 0);
      oDRAM_RASn           : out   std_logic;
      oDRAM_WEn            : out   std_logic;

      -- Fast Serial FTDI
      oFSCLK               : out   std_logic;
      iFSCTS               : in    std_logic;
      iFSDO                : in    std_logic;
      oFSDI                : out   std_logic;

      -- STDOUT UART
      oSTDOUT_UART_TX      : out   std_logic;
      iSTDOUT_UART_RX      : in    std_logic;

      -- ADC
      oADC_SCLK            : out   std_logic;
      oADC_CSn             : out   std_logic;
      oADC_MOSI            : out   std_logic;
      iADC_MISO            : in    std_logic;
      oADC_CNVTBn          : out   std_logic;
      iADC_INTBn           : in    std_logic;

      -- GPIO
      oLED                 : out   std_logic_vector(3 downto 0);
      ioGPX                : inout std_logic_vector(6 downto 0);
      iGPX                 : in    std_logic_vector(8 downto 0);

      -- Test Points
      oTP1                 : out   std_logic;
      oTP2                 : out   std_logic;
      oTP3                 : out   std_logic;
      oTP4                 : out   std_logic
   );
end c10_top;

architecture rtl of c10_top is

--
-- COMPONENTS
--
component c10_fpga is
   port (
      clk_clk        : in    std_logic;
      reset_reset_n  : in    std_logic;
      locked_export  : out   std_logic;
      watchdog_reset : out   std_logic;
      stdout_rxd     : in    std_logic;
      stdout_txd     : out   std_logic;
      opto_head_addr : in    std_logic_vector(15 downto 0);
      opto_tail_addr : out   std_logic_vector(15 downto 0);
      opto_fsclk     : out   std_logic;
      opto_fscts     : in    std_logic;
      opto_fsdo      : in    std_logic;
      opto_fsdi      : out   std_logic;
      opto_test_bit  : out   std_logic;
      gpi_export     : in    std_logic_vector(8 downto 0);
      gpx_export     : inout std_logic_vector(6 downto 0);
      dram_clk       : out   std_logic;
      sdram_addr     : out   std_logic_vector(11 downto 0);
      sdram_ba       : out   std_logic_vector(1 downto 0);
      sdram_cas_n    : out   std_logic;
      sdram_cke      : out   std_logic;
      sdram_cs_n     : out   std_logic;
      sdram_dq       : inout std_logic_vector(15 downto 0);
      sdram_dqm      : out   std_logic_vector(1 downto 0);
      sdram_ras_n    : out   std_logic;
      sdram_we_n     : out   std_logic;
      adc_cs_n       : out   std_logic;
      adc_mosi       : out   std_logic;
      adc_miso       : in    std_logic;
      adc_intb_n     : in    std_logic;
      adc_cnvtb_n    : out   std_logic;
      adc_sclk       : out   std_logic;
      adc_head_addr  : out   std_logic_vector(15 downto 0);
      adc_tail_addr  : in    std_logic_vector(15 downto 0)
   );
end component c10_fpga;

--
-- SIGNAL DECLARATIONS
--

signal async_pll_rst_n     : std_logic;
signal pll_locked          : std_logic;
signal watchdog            : std_logic;
signal watchdog_fired      : std_logic;
signal sys_rst_n           : std_logic;
signal sys_rst_vect        : unsigned(27 downto 0);
signal heartbeat_count     : unsigned(27 downto 0);
signal heartbeat           : std_logic;
signal sw_test_bit         : std_logic;
signal head_addr           : std_logic_vector(15 downto 0);
signal tail_addr           : std_logic_vector(15 downto 0);
signal debug               : std_logic_vector(3 downto 0);

--
-- MAIN CODE
--

   begin

   --
   -- FPGA LED
   --
   oLED(0)              <= heartbeat;
   oLED(1)              <= watchdog_fired;
   oLED(2)              <= '0';
   oLED(3)              <= '0';

   --
   -- TEST POINTS
   --
   oTP1                 <= sw_test_bit;
   oTP2                 <= '0';
   oTP3                 <= '0';
   oTP4                 <= '0';

   --
   -- QSYS NIOS
   --
   u0 : c10_fpga
      port map (
         clk_clk           => iCLK_12M,
         reset_reset_n     => sys_rst_n,
         locked_export     => pll_locked,
         watchdog_reset    => watchdog,
         gpx_export        => ioGPX,
         gpi_export        => iGPX,
         stdout_rxd        => iSTDOUT_UART_RX,
         stdout_txd        => oSTDOUT_UART_TX,
         dram_clk          => oDRAM_CLK,
         sdram_addr        => oDRAM_ADDR,
         sdram_ba          => oDRAM_BA,
         sdram_cas_n       => oDRAM_CASn,
         sdram_cke         => oDRAM_CKE,
         sdram_cs_n        => oDRAM_CSn,
         sdram_dq          => ioDRAM_DQ,
         sdram_dqm         => oDRAM_DQM,
         sdram_ras_n       => oDRAM_RASn,
         sdram_we_n        => oDRAM_WEn,
         opto_head_addr    => head_addr,
         opto_tail_addr    => tail_addr,
         opto_fsclk        => oFSCLK,
         opto_fscts        => iFSCTS,
         opto_fsdo         => iFSDO,
         opto_fsdi         => oFSDI,
         opto_test_bit     => sw_test_bit,
         adc_cs_n          => oADC_CSn,
         adc_mosi          => oADC_MOSI,
         adc_miso          => iADC_MISO,
         adc_sclk          => oADC_SCLK,
         adc_intb_n        => iADC_INTBn,
         adc_cnvtb_n       => oADC_CNVTBn,
         adc_head_addr     => head_addr,
         adc_tail_addr     => tail_addr
      );

   --
   -- System Reset, iCLK_12M
   --
   process(all) begin
      if (pll_locked = '0' or watchdog = '1' or iRSTn = '0') then
         sys_rst_vect   <= (others => '0');
         sys_rst_n      <= '0';
      elsif (rising_edge(iCLK_12M)) then
         if (sys_rst_n = '0') then
            sys_rst_vect <= sys_rst_vect + 1;
         end if;
         -- hold reset low for ~85 milliseconds then set high
         if (sys_rst_vect(20) = '1') then
            sys_rst_n <= '1';
         end if;
      end if;
   end process;

   --
   -- Heartbeat, iCLK_12M
   --
   process(all) begin
      if (sys_rst_n = '0') then
          heartbeat_count     <= (others => '0');
          heartbeat           <= '0';
      elsif (rising_edge(iCLK_12M)) then
          if(heartbeat_count(20) = '1') then
              heartbeat_count <= (others => '0');
              heartbeat       <= not heartbeat;
          else
              heartbeat_count <= heartbeat_count + 1;
          end if;
      end if;
   end process;

   --
   -- Capture Watchdog Reset
   --
   process(all) begin
      if (pll_locked = '0' or iRSTn = '0') then
         watchdog_fired       <= '0';
      elsif (rising_edge(iCLK_12M)) then
         if (watchdog_fired = '0' and watchdog = '1') then
            watchdog_fired <= '1';
         else
            watchdog_fired <= watchdog_fired;
         end if;
      end if;
   end process;

   --
   -- Qsys Register Map
   --
   -- pll                        : 0x1000_0000
   -- cpu                        : 0x1001_0000
   -- sdram                      : 0x0000_0000
   -- epcs                       : 0x1002_0000
   -- stamp                      : 0x1003_0000
   -- sysclk                     : 0x1004_0000
   -- systimer                   : 0x1005_0000
   -- watchdog                   : 0x1006_0000
   -- gpx                        : 0x1007_0000
   -- gpi                        : 0x1008_0000
   -- stdout                     : 0x1009_0000
   -- adc                        : 0x100A_0000
   -- ftdi                       : 0x100B_0000
   -- update                     : 0x100C_0000
   --

end rtl;
