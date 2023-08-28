library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity adc_top is
   generic (
      C_DWIDTH             : integer              := 32;
      C_NUM_REG            : integer              := 32
   );
   port (
      clk                  : in    std_logic;
      reset_n              : in    std_logic;
      irq                  : out   std_logic;
      m1_write             : out   std_logic;
      m1_wr_address        : out   std_logic_vector(31 downto 0);
      m1_writedata         : out   std_logic_vector(31 downto 0);
      m1_wr_waitreq        : in    std_logic;
      m1_wr_burstcount     : out   std_logic_vector(8 downto 0);
      read_n               : in    std_logic;
      write_n              : in    std_logic;
      address              : in    std_logic_vector(10 downto 0);
      readdata             : out   std_logic_vector(31 downto 0);
      writedata            : in    std_logic_vector(31 downto 0);
      head_addr            : out   std_logic_vector(15 downto 0);
      tail_addr            : in    std_logic_vector(15 downto 0);
      sclk                 : out   std_logic;
      cs_n                 : out   std_logic;
      mosi                 : out   std_logic;
      miso                 : in    std_logic;
      cnvtb_n              : out   std_logic;
      intb_n               : in    std_logic
   );
end entity adc_top;

architecture rtl of adc_top is

--
-- SIGNAL DECLARATIONS
--
   signal adc_CONTROL      : std_logic_vector(31 downto 0);
   signal adc_INT_REQ      : std_logic_vector(1 downto 0);
   signal adc_INT_ACK      : std_logic_vector(1 downto 0);
   signal adc_STATUS       : std_logic_vector(31 downto 0);
   signal adc_ADR_BEG      : std_logic_vector(31 downto 0);
   signal adc_ADR_END      : std_logic_vector(31 downto 0);
   signal adc_PKT_CNT      : std_logic_vector(31 downto 0);
   signal adc_POOL_CNT     : std_logic_vector(7 downto 0);
   signal adc_ADC_RATE     : std_logic_vector(15 downto 0);
   signal adc_DEV_CFG      : std_logic_vector(15 downto 0);
   signal adc_PORT_CFG     : std_logic_vector(15 downto 0);

   signal adcInt           : std_logic_vector(1 downto 0);

   signal cpu_DIN          : std_logic_vector(31 downto 0);
   signal cpu_DOUT         : std_logic_vector(31 downto 0);
   signal cpu_ADDR         : std_logic_vector(10 downto 0);
   signal cpu_WE           : std_logic;
   signal cpu_RE           : std_logic;

--
-- MAIN CODE
--
begin

   --
   -- REGISTER FILE
   --
   ADC_REGS_I: entity work.adc_regs
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
      cpu_DIN              => cpu_DIN,
      cpu_DOUT             => cpu_DOUT,
      cpu_ADDR             => cpu_ADDR,
      cpu_WE               => cpu_WE,
      cpu_RE               => cpu_RE,
      adc_CONTROL          => adc_CONTROL,
      adc_INT_REQ          => adc_INT_REQ,
      adc_INT_ACK          => adc_INT_ACK,
      adc_STATUS           => adc_STATUS,
      adc_ADR_BEG          => adc_ADR_BEG,
      adc_ADR_END          => adc_ADR_END,
      adc_PKT_CNT          => adc_PKT_CNT,
      adc_POOL_CNT         => adc_POOL_CNT,
      adc_ADC_RATE         => adc_ADC_RATE,
      adc_DEV_CFG          => adc_DEV_CFG,
      adc_PORT_CFG         => adc_PORT_CFG
   );

   --
   -- ADC STATE MACHINE
   --
   ADC_CTL_I: entity work.adc_ctl
   port map (
      clk                  => clk,
      reset_n              => reset_n,
      int                  => adcInt,
      m1_write             => m1_write,
      m1_wr_address        => m1_wr_address,
      m1_writedata         => m1_writedata,
      m1_wr_waitreq        => m1_wr_waitreq,
      m1_wr_burstcount     => m1_wr_burstcount,
      adc_CONTROL          => adc_CONTROL,
      adc_STATUS           => adc_STATUS,
      adc_ADR_BEG          => adc_ADR_BEG,
      adc_ADR_END          => adc_ADR_END,
      adc_PKT_CNT          => adc_PKT_CNT,
      adc_POOL_CNT         => adc_POOL_CNT,
      adc_ADC_RATE         => adc_ADC_RATE,
      adc_DEV_CFG          => adc_DEV_CFG,
      adc_PORT_CFG         => adc_PORT_CFG,
      cpu_DIN              => cpu_DIN,
      cpu_DOUT             => cpu_DOUT,
      cpu_ADDR             => cpu_ADDR,
      cpu_WE               => cpu_WE,
      cpu_RE               => cpu_RE,
      head_addr            => head_addr,
      tail_addr            => tail_addr,
      sclk                 => sclk,    
      cs_n                 => cs_n,
      mosi                 => mosi,    
      miso                 => miso,    
      cnvtb_n              => cnvtb_n, 
      intb_n               => intb_n  
   );

   --
   -- INTERRUPTS
   --
   ADC_IRQ_I: entity work.adc_irq
   generic map (
      C_NUM_INT            => 2
   )
   port map (
      clk                  => clk,
      reset_n              => reset_n,
      int_req              => adc_INT_REQ,
      int_ack              => adc_INT_ACK,
      int                  => adcInt,
      irq                  => irq
   );

end rtl;
