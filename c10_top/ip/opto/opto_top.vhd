library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity opto_top is
   generic (
      C_DWIDTH             : integer              := 32;
      C_NUM_REG            : integer              := 32
   );
   port (
      clk                  : in    std_logic;
      reset_n              : in    std_logic;
      read_n               : in    std_logic;
      write_n              : in    std_logic;
      address              : in    std_logic_vector(11 downto 0);
      readdata             : out   std_logic_vector(31 downto 0);
      writedata            : in    std_logic_vector(31 downto 0);
      irq                  : out   std_logic;
      m1_read              : out   std_logic;
      m1_rd_address        : out   std_logic_vector(31 downto 0);
      m1_readdata          : in    std_logic_vector(31 downto 0);
      m1_rd_waitreq        : in    std_logic;
      m1_rd_burstcount     : out   std_logic_vector(15 downto 0);
      m1_rd_datavalid      : in    std_logic;
      head_addr            : in    std_logic_vector(15 downto 0);
      tail_addr            : out   std_logic_vector(15 downto 0);
      fsclk                : out   std_logic;
      fscts                : in    std_logic;
      fsdo                 : in    std_logic;
      fsdi                 : out   std_logic;
      test_bit             : out   std_logic;
      debug                : out   std_logic_vector(3 downto 0)
   );
end entity opto_top;

architecture rtl of opto_top is

--
-- SIGNAL DECLARATIONS
--
   signal opto_CONTROL     : std_logic_vector(31 downto 0);
   signal opto_ADDR        : std_logic_vector(31 downto 0);
   signal opto_INT_REQ     : std_logic_vector(2 downto 0);
   signal opto_INT_ACK     : std_logic_vector(2 downto 0);
   signal opto_STATUS      : std_logic_vector(31 downto 0);
   signal opto_ADR_BEG     : std_logic_vector(31 downto 0);
   signal opto_ADR_END     : std_logic_vector(31 downto 0);
   signal opto_PKT_CNT     : std_logic_vector(31 downto 0);
   signal opto_int         : std_logic_vector(2 downto 0);

   signal cpu_DIN          : std_logic_vector(31 downto 0);
   signal cpu_DOUT         : std_logic_vector(31 downto 0);
   signal cpu_ADDR         : std_logic_vector(11 downto 0);
   signal cpu_WE           : std_logic_vector(1 downto 0);
   signal cpu_RE           : std_logic_vector(2 downto 0);

--
-- MAIN CODE
--
begin

   --
   -- REGISTER FILE
   --
   OPTO_REGS_I: entity work.opto_regs
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
      opto_CONTROL         => opto_CONTROL,
      opto_INT_REQ         => opto_INT_REQ,
      opto_INT_ACK         => opto_INT_ACK,
      opto_STATUS          => opto_STATUS,
      opto_ADR_BEG         => opto_ADR_BEG,
      opto_ADR_END         => opto_ADR_END,
      opto_PKT_CNT         => opto_PKT_CNT,
      opto_TEST_BIT        => test_bit
   );

   --
   -- OPTO MESSAGE STATE MACHINE
   --
   OPTO_CTL_I: entity work.opto_ctl
   port map (
      clk                  => clk,
      reset_n              => reset_n,
      int                  => opto_int,
      m1_read              => m1_read,
      m1_rd_address        => m1_rd_address,
      m1_readdata          => m1_readdata,
      m1_rd_waitreq        => m1_rd_waitreq,
      m1_rd_burstcount     => m1_rd_burstcount,
      m1_rd_datavalid      => m1_rd_datavalid,
      cpu_DIN              => cpu_DIN,
      cpu_DOUT             => cpu_DOUT,
      cpu_ADDR             => cpu_ADDR,
      cpu_WE               => cpu_WE,
      cpu_RE               => cpu_RE,
      opto_CONTROL         => opto_CONTROL,
      opto_STATUS          => opto_STATUS,
      opto_ADR_BEG         => opto_ADR_BEG,
      opto_ADR_END         => opto_ADR_END,
      opto_PKT_CNT         => opto_PKT_CNT,
      head_addr            => head_addr,
      tail_addr            => tail_addr,
      fsclk                => fsclk,
      fscts                => fscts,
      fsdo                 => fsdo,
      fsdi                 => fsdi,
      debug                => debug
   );

   --
   -- INTERRUPTS
   --
   OPTO_IRQ_I: entity work.opto_irq
   generic map (
      C_NUM_INT            => 3
   )
   port map (
      clk                  => clk,
      reset_n              => reset_n,
      int_req              => opto_INT_REQ,
      int_ack              => opto_INT_ACK,
      int                  => opto_int,
      irq                  => irq
   );

end rtl;
