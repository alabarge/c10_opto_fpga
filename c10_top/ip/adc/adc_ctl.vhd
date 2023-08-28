library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.lib_pkg.all;

entity adc_ctl is
   port (
      -- Avalon Clock, Reset & Interrupt
      clk                  : in    std_logic;
      reset_n              : in    std_logic;
      int                  : out   std_logic_vector(1 downto 0);
      -- Avalon Memory-Mapped Write Master
      m1_write             : out   std_logic;
      m1_wr_address        : out   std_logic_vector(31 downto 0);
      m1_writedata         : out   std_logic_vector(31 downto 0);
      m1_wr_waitreq        : in    std_logic;
      m1_wr_burstcount     : out   std_logic_vector(8 downto 0);
      -- Control Registers
      adc_CONTROL          : in    std_logic_vector(31 downto 0);
      adc_ADR_BEG          : in    std_logic_vector(31 downto 0);
      adc_ADR_END          : in    std_logic_vector(31 downto 0);
      adc_PKT_CNT          : in    std_logic_vector(31 downto 0);
      adc_POOL_CNT         : in    std_logic_vector(7 downto 0);
      adc_ADC_RATE         : in    std_logic_vector(15 downto 0);
      adc_DEV_CFG          : in    std_logic_vector(15 downto 0);
      adc_PORT_CFG         : in    std_logic_vector(15 downto 0);
      adc_STATUS           : out   std_logic_vector(31 downto 0);
      -- Block RAM I/F
      cpu_DIN              : out   std_logic_vector(31 downto 0);
      cpu_DOUT             : in    std_logic_vector(31 downto 0);
      cpu_ADDR             : in    std_logic_vector(10 downto 0);
      cpu_WE               : in    std_logic;
      cpu_RE               : in    std_logic;
      -- Memory Head-Tail Pointers
      head_addr            : out   std_logic_vector(15 downto 0);
      tail_addr            : in    std_logic_vector(15 downto 0);
      -- Exported Signals
      sclk                 : out   std_logic;
      cs_n                 : out   std_logic;
      mosi                 : out   std_logic;
      miso                 : in    std_logic;
      cnvtb_n              : out   std_logic;
      intb_n               : in    std_logic
   );
end adc_ctl;

architecture rtl of adc_ctl is

--
-- COMPONENTS
--

--
-- TYPES
--
type   adc_state_t is (IDLE,ADC_CFG_LO,ADC_CFG_HI,ADC_DELAY,HEADER,ADC_INT,
                       CS_DELAY,CONVST,WAIT_ADC,SCLK_HI,SCLK_LO,STORE,CHECK);
type   wr_state_t  is (IDLE, WAIT_SLOT, DELAY, WR_SLOT);
type   adc_cfg_t   is array (0 to 23) of std_logic_vector(23 downto 0);

type  ADC_SV_t is record
   state       : adc_state_t;
   cfg         : std_logic_vector(23 downto 0);
   pkt_cnt     : unsigned(31 downto 0);
   delay       : integer range 0 to 32768;
   reg         : integer range 0 to 31;
   run         : std_logic;
   run_r0      : std_logic;
   busy        : std_logic;
   done        : std_logic;
   in_ptr      : unsigned(7 downto 0);
   ch_cnt      : unsigned(2 downto 0);
   out_dat     : std_logic_vector(31 downto 0);
   adc_dat     : std_logic_vector(47 downto 0);
   ramp        : unsigned(15 downto 0);
   head        : unsigned(1 downto 0);
   in_we       : std_logic;
   seq_id      : unsigned(31 downto 0);
   bit_cnt     : integer range 0 to 64;
   hdr_cnt     : integer range 0 to 10;
   mosi        : std_logic;
   sclk        : std_logic;
   cs          : std_logic;
   cnvtb       : std_logic;
   intb        : std_logic;
   intb_r0     : std_logic;
end record ADC_SV_t;

type  WR_SV_t is record
   state       : wr_state_t;
   run         : std_logic;
   run_r0      : std_logic;
   addr        : unsigned(31 downto 0);
   wrd_cnt     : unsigned(8 downto 0);
   pool_cnt    : unsigned(7 downto 0);
   head_addr   : unsigned(15 downto 0);
   tail        : unsigned(1 downto 0);
   master      : std_logic;
   burstcnt    : std_logic_vector(8 downto 0);
   busy        : std_logic;
   pkt_rdy     : std_logic;
end record WR_SV_t;

--
-- CONSTANTS
--

-- ADC State Vector Initialization
constant C_ADC_SV_INIT : ADC_SV_t := (
   state       => IDLE,
   cfg         => (others => '0'),
   pkt_cnt     => (others => '0'),
   delay       => 0,
   reg         => 0,
   run         => '0',
   run_r0      => '0',
   busy        => '0',
   done        => '0',
   in_ptr      => (others => '0'),
   ch_cnt      => (others => '0'),
   out_dat     => (others => '0'),
   adc_dat     => (others => '0'),
   ramp        => (others => '0'),
   head        => (others => '0'),
   in_we       => '0',
   seq_id      => (others => '0'),
   bit_cnt     => 0,
   hdr_cnt     => 0,
   mosi        => '0',
   sclk        => '0',
   cs          => '0',
   cnvtb       => '0',
   intb        => '0',
   intb_r0     => '0'
);

-- WR State Vector Initialization
constant C_WR_SV_INIT : WR_SV_t := (
   state       => IDLE,
   run         => '0',
   run_r0      => '0',
   addr        => (others => '0'),
   wrd_cnt     => (others => '0'),
   pool_cnt    => (others => '0'),
   head_addr   => (others => '0'),
   tail        => (others => '0'),
   master      => '0',
   burstcnt    => (others => '0'),
   busy        => '0',
   pkt_rdy     => '0'
);

--
-- Timer Constants are based on a 100MHz FPGA Clock
--

-- SPI Clock, 16.6 MHZ Nominal, 60 ns period
constant C_ADC_CLK_LO         : integer                  := 2;
constant C_ADC_CLK_HI         : integer                  := 2;

-- First and Last ADC Read Registers
constant C_ADC_FIRST_REG      : integer                  := 14;
constant C_ADC_LAST_REG       : integer                  := 21;

-- MAX11300 Registers
constant C_MAX_DEV_CTL        : integer                  := 10;
constant C_MAX_PORT_CFG_0     : integer                  := 2;
constant C_MAX_PORT_CFG_1     : integer                  := 3;
constant C_MAX_PORT_CFG_2     : integer                  := 4;
constant C_MAX_PORT_CFG_3     : integer                  := 5;
constant C_MAX_PORT_CFG_4     : integer                  := 6;
constant C_MAX_PORT_CFG_5     : integer                  := 7;
constant C_MAX_PORT_CFG_6     : integer                  := 8;
constant C_MAX_PORT_CFG_7     : integer                  := 9;

-- ADC Conversion Time, 1.0 uS max
constant C_ADC_CONV           : integer                  := 100;

-- ADC Conversion Start Time, 500 ns
constant C_ADC_CONVST         : integer                  := 50;

-- Minimum ADC Rate
constant C_ADC_RATE_MIN       : unsigned(15 downto 0)    := X"01F4";

-- Delay between configuration writes
constant C_CFG_DELAY          : integer                  := 10000;

--
-- MAX11300 Configuration Constants
--
-- X"AADDDD" where AA = AD6..0 & RB/W, DDDD = D15..0
--
constant C_ADC_CFG : adc_cfg_t := (
   0     => X"200070",    -- Device Control, ADC IDLE, RB/W = 0
   1     => X"200070",    -- Device Control, ADC IDLE
   2     => X"400000",    -- Port CFG 0, or'd with adc_PORT_CFG register
   3     => X"420000",    -- Port CFG 1
   4     => X"440000",    -- Port CFG 2
   5     => X"460000",    -- Port CFG 3
   6     => X"480000",    -- Port CFG 4
   7     => X"4A0000",    -- Port CFG 5
   8     => X"4C0000",    -- Port CFG 6
   9     => X"4E0000",    -- Port CFG 7
   10    => X"200000",    -- Device Control, or'd with adc_DEV_CFG register
   11    => X"22FFFE",    -- Interrupt Mask, ONLY ADCFLAGMSK ENABLED
   12    => X"030000",    -- Interrupt Status
   13    => X"050000",    -- ADC Data Status
   14    => X"810000",    -- Port 0 ADC Data, Straight Binary Format, RB/W = 1
   15    => X"830000",    -- Port 1 ADC Data
   16    => X"850000",    -- Port 2 ADC Data
   17    => X"870000",    -- Port 3 ADC Data
   18    => X"890000",    -- Port 4 ADC Data
   19    => X"8B0000",    -- Port 5 ADC Data
   20    => X"8D0000",    -- Port 6 ADC Data
   21    => X"8F0000",    -- Port 7 ADC Data
   22    => X"000000",    --
   23    => X"000000"     --
);

--
-- SIGNAL DECLARATIONS
--

--ADC State Vector
signal ad               : ADC_SV_t;
signal wr               : WR_SV_t;

-- 32-Bit Machine Status
signal adc_stat         : std_logic_vector(31  downto 0);
alias  xl_HEAD_ADDR     : std_logic_vector(15 downto 0) is adc_stat(15 downto 0);
alias  xl_TAIL          : std_logic_vector(3 downto 0) is adc_stat(19 downto 16);
alias  xl_HEAD          : std_logic_vector(3 downto 0) is adc_stat(23 downto 20);
alias  xl_UNUSED        : std_logic_vector(5 downto 0) is adc_stat(29 downto 24);
alias  xl_DMA_BUSY      : std_logic is adc_stat(30);
alias  xl_ADC_BUSY      : std_logic is adc_stat(31);

-- 32-Bit Control Register
alias  xl_HEAD_EN       : std_logic is adc_CONTROL(25);
alias  xl_SCAN          : std_logic is adc_CONTROL(26);
alias  xl_RAMP          : std_logic is adc_CONTROL(27);
alias  xl_RUN           : std_logic is adc_CONTROL(28);
alias  xl_PKT_INT_EN    : std_logic is adc_CONTROL(29);
alias  xl_DONE_INT_EN   : std_logic is adc_CONTROL(30);
alias  xl_ENABLE        : std_logic is adc_CONTROL(31);

signal writedata        : std_logic_vector(31 downto 0);
signal wr_clk_en        : std_logic;
signal out_addr         : std_logic_vector(9 downto 0);
signal stamp            : unsigned(31 downto 0);
signal cnvst_cnt        : unsigned(15 downto 0);
signal convert          : std_logic;

--
-- MAIN CODE
--
begin

   --
   -- COMBINATORIAL OUTPUTS
   --

   int(0)               <= ad.done and xl_DONE_INT_EN;
   int(1)               <= wr.pkt_rdy and xl_PKT_INT_EN;

   adc_STATUS           <= adc_stat;

   -- SPI I/F
   sclk                 <= ad.sclk;
   cs_n                 <= not ad.cs;
   mosi                 <= ad.mosi;
   cnvtb_n              <= not ad.cnvtb;

   -- Master Wite
   m1_wr_address        <= std_logic_vector(wr.addr);
   m1_writedata         <= writedata;
   m1_write             <= wr.master;
   m1_wr_burstcount     <= wr.burstcnt;

   -- CPU Reads FIFO directly
   cpu_DIN              <= writedata;

   -- Shared Packet Address, tail_addr not used
   -- head_addr increments based on adc_POOL_CNT
   -- the same as the packet ready interrupt
   head_addr            <= std_logic_vector(wr.head_addr) when xl_HEAD_EN = '1' else (others => '0');

   --
   --   1024 32-Bit Dual-Port BLOCK RAM
   --   ADC => ONCHIP => FTDI
   --   Circular Buffer with 4 1K Byte slots
   --
   ADC_4K_I : entity work.adc_4k
   port map (
      clock_a     => clk,
      enable_a    => '1',
      address_a   => std_logic_vector(ad.head) & std_logic_vector(ad.in_ptr),
      data_a      => ad.out_dat,
      wren_a      => ad.in_we,
      q_a         => open,
      clock_b     => clk,
      enable_b    => wr_clk_en,
      address_b   => out_addr,
      data_b      => X"00000000",
      wren_b      => '0',
      q_b         => writedata
   );
   out_addr       <= cpu_ADDR(9 downto 0) when cpu_RE = '1' else
                     std_logic_vector(wr.tail) & std_logic_vector(wr.wrd_cnt(7 downto 0));

   wr_clk_en      <= '1' when wr.state = WAIT_SLOT else
                     '1' when wr.state = DELAY else
                     '1' when wr.state = WR_SLOT and m1_wr_waitreq = '0' else 
                     '1' when cpu_RE = '1' else '0';

   --
   --  MAX11300 (PIXI) SPI STATE MACHINE
   --
   process(all) begin
      if (reset_n = '0' or xl_ENABLE = '0') then

         -- Init the State Vector
         ad             <= C_ADC_SV_INIT;

         -- status is shared by master write FSM
         xl_ADC_BUSY    <= '0';
         xl_HEAD        <= (others => '0');
         xl_UNUSED      <= (others => '0');

      elsif (rising_edge(clk)) then

         -- edge-detect
         ad.run         <= xl_RUN;
         ad.run_r0      <= ad.run;

         -- Status
         xl_ADC_BUSY    <= ad.busy;
         xl_HEAD        <= "00" & std_logic_vector(ad.head);
         xl_UNUSED      <= (others => '0');

         -- double-buffer async
         ad.intb        <= intb_n;
         ad.intb_r0     <= ad.intb;

         case ad.state is
            --
            -- Look for Run Bit Rising-Edge
            --
            when IDLE =>
               if (ad.run = '1' and ad.run_r0 = '0') then
                  ad.state    <= ADC_CFG_LO;
                  ad.cfg      <= C_ADC_CFG(0);
                  ad.in_ptr   <= (others => '0');
                  ad.head     <= (others => '0');
                  ad.pkt_cnt  <= (others => '0');
                  ad.seq_id   <= (others => '0');
                  ad.hdr_cnt  <= 0;
                  ad.bit_cnt  <= 0;
                  ad.out_dat  <= (others => '0');
                  ad.adc_dat  <= (others => '0');
                  ad.ramp     <= (others => '0');
                  ad.ch_cnt   <= (others => '0');
                  ad.busy     <= '1';
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.sclk     <= '0';
                  ad.cs       <= '0';
                  ad.reg      <= 0;
               else
                  ad.state    <= IDLE;
                  ad.busy     <= '0';
                  ad.done     <= '0';
               end if;

            -- 
            -- Write ADC Configuration Registers, SPI CLK LO
            --
            -- ONCE PER RUN
            -- 
            when ADC_CFG_LO =>
               if (ad.run = '0') then
                  ad.state    <= IDLE;
               elsif (ad.reg = C_ADC_FIRST_REG) then
                  ad.state    <= HEADER;
               elsif (ad.delay = 0 and ad.bit_cnt = 24) then
                  ad.state    <= ADC_DELAY;
                  ad.bit_cnt  <= 0;
                  ad.cs       <= '0';
                  ad.reg      <= ad.reg + 1;
                  ad.delay    <= C_CFG_DELAY;
               elsif (ad.delay = 0) then
                  ad.state    <= ADC_CFG_HI;
                  ad.delay    <= C_ADC_CLK_HI;
                  ad.sclk     <= '1';
                  ad.bit_cnt  <= ad.bit_cnt + 1;
               else
                  ad.state    <= ADC_CFG_LO;
                  ad.delay    <= ad.delay - 1;
                  ad.mosi     <= ad.cfg(23);
                  ad.cs       <= '1';
               end if;

            -- 
            -- Write ADC Configuration Registers, SPI CLK HI
            -- 
            when ADC_CFG_HI =>
               if (ad.run = '0') then
                  ad.state    <= IDLE;
               elsif (ad.delay = 0) then
                  ad.state    <= ADC_CFG_LO;
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.cfg      <= ad.cfg(22 downto 0) & '0';
                  ad.sclk     <= '0';
               else
                  ad.state    <= ADC_CFG_HI;
                  ad.delay    <= ad.delay - 1;
               end if;

            -- 
            -- 100 uS Delay Between Configuration Writes
            -- 
            when ADC_DELAY =>
               -- append device config
               if (ad.delay = 0 and ad.reg = C_MAX_DEV_CTL) then
                  ad.state    <= ADC_CFG_LO;
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.cfg      <= C_ADC_CFG(ad.reg) or X"00" & adc_DEV_CFG;
               -- append port config 0
               elsif (ad.delay = 0 and ad.reg = C_MAX_PORT_CFG_0) then
                  ad.state    <= ADC_CFG_LO;
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.cfg      <= C_ADC_CFG(ad.reg) or X"00" & adc_PORT_CFG;
               -- append port config 1
               elsif (ad.delay = 0 and ad.reg = C_MAX_PORT_CFG_1) then
                  ad.state    <= ADC_CFG_LO;
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.cfg      <= C_ADC_CFG(ad.reg) or X"00" & adc_PORT_CFG;
               -- append port config 2
               elsif (ad.delay = 0 and ad.reg = C_MAX_PORT_CFG_2) then
                  ad.state    <= ADC_CFG_LO;
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.cfg      <= C_ADC_CFG(ad.reg) or X"00" & adc_PORT_CFG;
               -- append port config 3
               elsif (ad.delay = 0 and ad.reg = C_MAX_PORT_CFG_3) then
                  ad.state    <= ADC_CFG_LO;
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.cfg      <= C_ADC_CFG(ad.reg) or X"00" & adc_PORT_CFG;
               -- append port config 4
               elsif (ad.delay = 0 and ad.reg = C_MAX_PORT_CFG_4) then
                  ad.state    <= ADC_CFG_LO;
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.cfg      <= C_ADC_CFG(ad.reg) or X"00" & adc_PORT_CFG;
               -- append port config 5
               elsif (ad.delay = 0 and ad.reg = C_MAX_PORT_CFG_5) then
                  ad.state    <= ADC_CFG_LO;
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.cfg      <= C_ADC_CFG(ad.reg) or X"00" & adc_PORT_CFG;
               -- append port config 6
               elsif (ad.delay = 0 and ad.reg = C_MAX_PORT_CFG_6) then
                  ad.state    <= ADC_CFG_LO;
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.cfg      <= C_ADC_CFG(ad.reg) or X"00" & adc_PORT_CFG;
               -- append port config 7
               elsif (ad.delay = 0 and ad.reg = C_MAX_PORT_CFG_7) then
                  ad.state    <= ADC_CFG_LO;
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.cfg      <= C_ADC_CFG(ad.reg) or X"00" & adc_PORT_CFG;
               -- default case
               elsif (ad.delay = 0) then
                  ad.state    <= ADC_CFG_LO;
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.cfg      <= C_ADC_CFG(ad.reg);
               else
                  ad.state    <= ADC_DELAY;
                  ad.delay    <= ad.delay - 1;
               end if;

            --
            --  CM_PIPE Message Header
            --
            -- // CM PIPE DAQ MESSAGE DATA STRUCTURE
            -- // USED FOR FIXED 1024-BYTE DAQ PIPE MESSAGES
            -- // SAME SIZE AS SLOTS IN WINDOWS DRIVER
            -- typedef struct _CM_PIPE_DAQ {
            --    uint8_t     dstCMID;        // Destination CM Address
            --    uint8_t     msgID;          // Pipe Message ID, CM_PIPE_DAQ_DATA = 0x10
            --    uint8_t     port;           // Destination Port
            --    uint8_t     flags;          // Message Flags
            --    uint32_t    msgLen;         // Message Length in 32-Bit words
            --    uint32_t    seqID;          // Sequence ID
            --    uint32_t    stamp;          // 32-Bit FPGA Clock Count
            --    uint32_t    stamp_us;       // 32-Bit Time Stamp in microseconds
            --    uint32_t    status;         // Current Machine Status
            --    uint32_t    reserved;       // Reserved
            --    uint32_t    magic;          // Magic Number
            --    uint16_t    samples[496];   // DAQ Samples
            -- } CM_PIPE_DAQ, *PCM_PIPE_DAQ;
            --
            when HEADER =>
               -- 0th 32-bits in CM_PIPE: dstCMID, msgID, port, flags
               if (ad.hdr_cnt = 0) then
                  ad.state    <= HEADER;
                  ad.in_we    <= '1';
                  ad.hdr_cnt  <= ad.hdr_cnt + 1;
                  ad.out_dat  <= X"00001584";
               -- 1st 32-bits in CM_PIPE: msgLen
               elsif (ad.hdr_cnt = 1) then
                  ad.state    <= HEADER;
                  ad.in_ptr   <= ad.in_ptr + 1;
                  ad.hdr_cnt  <= ad.hdr_cnt + 1;
                  ad.out_dat  <= X"00000100";
               -- 2nd 32-bits in CM_PIPE: seqID
               elsif (ad.hdr_cnt = 2) then
                  ad.state    <= HEADER;
                  ad.in_ptr   <= ad.in_ptr + 1;
                  ad.hdr_cnt  <= ad.hdr_cnt + 1;
                  ad.out_dat  <= std_logic_vector(ad.seq_id);
                  ad.seq_id   <= ad.seq_id + 1;
               -- 3rd 32-bits in CM_PIPE: stamp
               elsif (ad.hdr_cnt = 3) then
                  ad.state    <= HEADER;
                  ad.in_ptr   <= ad.in_ptr + 1;
                  ad.hdr_cnt  <= ad.hdr_cnt + 1;
                  ad.out_dat  <= std_logic_vector(stamp);
               -- 4th 32-bits in CM_PIPE: stamp_us
               elsif (ad.hdr_cnt = 4) then
                  ad.state    <= HEADER;
                  ad.in_ptr   <= ad.in_ptr + 1;
                  ad.hdr_cnt  <= ad.hdr_cnt + 1;
                  ad.out_dat  <= X"00000000";
               -- 5th 32-bits in CM_PIPE: status
               elsif (ad.hdr_cnt = 5) then
                  ad.state    <= HEADER;
                  ad.in_ptr   <= ad.in_ptr + 1;
                  ad.hdr_cnt  <= ad.hdr_cnt + 1;
                  ad.out_dat  <= X"00000000";
               -- 6th 32-bits in CM_PIPE: reserved
               elsif (ad.hdr_cnt = 6) then
                  ad.state    <= HEADER;
                  ad.in_ptr   <= ad.in_ptr + 1;
                  ad.hdr_cnt  <= ad.hdr_cnt + 1;
                  ad.out_dat  <= X"00000000";
               -- 7th 32-bits in CM_PIPE: magic
               elsif (ad.hdr_cnt = 7) then
                  ad.state    <= HEADER;
                  ad.in_ptr   <= ad.in_ptr + 1;
                  ad.hdr_cnt  <= ad.hdr_cnt + 1;
                  ad.out_dat  <= X"123455AA";
               else
                  ad.state    <= CS_DELAY;
                  ad.in_ptr   <= ad.in_ptr + 1;
                  ad.in_we    <= '0';
                  ad.hdr_cnt  <= 0;
                  ad.delay    <= C_ADC_CLK_LO;
               end if ;

            --
            -- CS DELAY, NEEDED WHEN SWEEPING FOR Tcsw
            --
            when CS_DELAY =>
               if (ad.delay = 0) then
                  ad.state    <= ADC_INT;
               else
                  ad.state    <= CS_DELAY;
                  ad.delay    <= ad.delay - 1;
               end if;

            --
            -- Continuous ADC Read, Wait for Interrupt
            -- In Continuous Sweep mode the INT pin is asserted
            -- whether the interrupt flag is cleared or not,
            -- so it's not necessary to read the interrupt flags.
            -- Each assertion of INT collects all enabled channels
            --
            when ADC_INT =>
               -- Check Abort
               if (ad.run = '0') then
                  ad.state    <= IDLE;
               -- check ADC read register count
               elsif (ad.reg = C_ADC_LAST_REG + 1) then
                  ad.state    <= ADC_INT;
                  ad.reg      <= C_ADC_FIRST_REG;
               -- check for sweep sampling
               elsif (xl_SCAN = '0') then
                  ad.state    <= CONVST;
               -- falling-edge of INT for sweep ready
               -- only when ad.ch_cnt = 0
               elsif (ad.intb = '0' and ad.intb_r0 = '1' and ad.ch_cnt = 0) then
                  ad.state    <= ADC_INT;
                  ad.delay    <= C_ADC_CLK_HI;
                  ad.cfg      <= C_ADC_CFG(ad.reg);
                  ad.cs       <= '1';
                  ad.mosi     <= C_ADC_CFG(ad.reg)(23);
               -- when scanning only wait for INT every 8th collection
               elsif (ad.ch_cnt /= 0 and ad.cs = '0') then
                  ad.state    <= ADC_INT;
                  ad.delay    <= C_ADC_CLK_HI;
                  ad.cfg      <= C_ADC_CFG(ad.reg);
                  ad.cs       <= '1';
                  ad.mosi     <= C_ADC_CFG(ad.reg)(23);
               elsif (ad.delay = 0 and ad.cs = '1') then                  
                  ad.state    <= SCLK_HI;
                  ad.delay    <= C_ADC_CLK_HI;
               elsif (ad.cs = '1') then
                  ad.state    <= ADC_INT;
                  ad.delay    <= ad.delay - 1;
               else
                  ad.state    <= ADC_INT;
               end if;

            --
            -- START ADC CONVERSION, CONVST ASSERT 500 nS
            --
            when CONVST =>
               -- wait for ADC convert signal
               if (convert = '1') then
                  ad.state    <= CONVST;
                  ad.cnvtb    <= '1';
                  ad.delay    <= C_ADC_CONVST;
               elsif (ad.delay = 0 and ad.cnvtb = '1') then
                  ad.state    <= WAIT_ADC;
                  ad.cnvtb    <= '0';
                  ad.delay    <= C_ADC_CONV;
                  ad.cfg      <= C_ADC_CFG(ad.reg);
               elsif (ad.cnvtb = '1') then
                  ad.state    <= CONVST;
                  ad.delay    <= ad.delay - 1;
               else
                  ad.state    <= CONVST;
               end if;

            --
            -- WAIT 1.0 uS MAX
            --
            when WAIT_ADC =>
               if (ad.delay = 0) then
                  ad.state    <= SCLK_HI;
                  ad.delay    <= C_ADC_CLK_HI;
               -- assert CS and MOSI prior to SCLK
               elsif (ad.delay = 2) then
                  ad.state    <= WAIT_ADC;
                  ad.delay    <= ad.delay - 1;
                  ad.cs       <= '1';                  
                  ad.mosi     <= ad.cfg(23);
               else
                  ad.state    <= WAIT_ADC;
                  ad.delay    <= ad.delay - 1;
               end if;

            --
            -- ASSERT SCK HI
            --
            when SCLK_HI =>
               ad.sclk        <= '1';
               if (ad.delay = C_ADC_CLK_HI) then
                  ad.state    <= SCLK_HI;
                  -- sample the serial ADC data output
                  ad.adc_dat  <= ad.adc_dat(46 downto 0) & miso;
                  ad.delay    <= ad.delay - 1;
               elsif (ad.delay = 0) then
                  ad.state    <= SCLK_LO;
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.bit_cnt  <= ad.bit_cnt + 1;
                  ad.cfg      <= ad.cfg(22 downto 0) & '0';
               else
                  ad.state    <= SCLK_HI;
                  ad.delay    <= ad.delay - 1;
               end if;

            --
            -- ASSERT SCK LO
            --
            when SCLK_LO =>
               ad.sclk        <= '0';
               -- collect second ADC sample
               if (ad.delay = 0 and ad.bit_cnt = 24) then
                  ad.state    <= CS_DELAY;
                  ad.reg      <= ad.reg + 1;
                  ad.ch_cnt   <= ad.ch_cnt + 1;
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.cs       <= '0';
                  ad.mosi     <= '0';
               elsif (ad.delay = 0 and ad.bit_cnt = 48 and xl_RAMP = '1') then
                  ad.state    <= STORE;
                  ad.bit_cnt  <= 0;
                  ad.cs       <= '0';
                  ad.mosi     <= '0';
                  ad.in_we    <= '1';
                  ad.ch_cnt   <= ad.ch_cnt + 1;
                  ad.reg      <= ad.reg + 1;
                  ad.out_dat  <= std_logic_vector(ad.ramp + 1) & 
                                 std_logic_vector(ad.ramp);
                  ad.ramp     <= ad.ramp + 2;
               elsif (ad.delay = 0 and ad.bit_cnt = 48) then
                  ad.state    <= STORE;
                  ad.bit_cnt  <= 0;
                  ad.cs       <= '0';
                  ad.mosi     <= '0';
                  ad.in_we    <= '1';
                  ad.ch_cnt   <= ad.ch_cnt + 1;
                  ad.reg      <= ad.reg + 1;
                  ad.out_dat  <= ad.adc_dat(39 downto 24) & 
                                 ad.adc_dat(15 downto 0);
               elsif (ad.delay = 0) then
                  ad.state    <= SCLK_HI;
                  ad.delay    <= C_ADC_CLK_HI;
               else
                  ad.state    <= SCLK_LO;
                  ad.delay    <= ad.delay - 1;
                  ad.mosi     <= ad.cfg(23);
               end if;

            --
            -- STORE 32-BIT ADC WORD (TWO 12-BIT SAMPLES)
            -- AND NEXT CHANNEL SELECTION
            --
            when STORE =>
               if (ad.in_ptr = X"FF") then
                  ad.state    <= CHECK;
                  ad.in_ptr   <= ad.in_ptr + 1;
                  ad.pkt_cnt  <= ad.pkt_cnt + 1;
                  ad.head     <= ad.head + 1;
                  ad.in_we    <= '0';
                  ad.out_dat  <= (others => '0');
                  ad.adc_dat  <= (others => '0');
               else
                  ad.state    <= CS_DELAY;
                  ad.in_ptr   <= ad.in_ptr + 1;
                  ad.in_we    <= '0';
                  ad.delay    <= C_ADC_CLK_LO;
                  ad.out_dat  <= (others => '0');
                  ad.adc_dat  <= (others => '0');
               end if;

            --
            -- CHECK FOR ALL FRAMES ACQUIRED AND ABORT
            --
            when CHECK =>
               -- Abort
               if (ad.run = '0') then
                  ad.state    <= IDLE;
               -- all requested frames acquired
               elsif (unsigned(adc_PKT_CNT) /= 0 and ad.pkt_cnt = unsigned(adc_PKT_CNT)) then
                  ad.state    <= IDLE;
                  ad.done     <= '1';
               else
                  ad.state    <= HEADER;
                  ad.delay    <= 0;
               end if;

            when others =>
               ad.state       <= IDLE;

         end case;

      end if;
   end process;

   --
   --  MASTER WRITE BURST TRANSFER, WRITE TO ON-CHIP RAM
   --
   --  NOTES:
   --    * Master read/write addresses are byte pointers.
   --    * Transfers are always 32-Bits.
   --    * adc_PKT_CNT is the number of 256 32-Bit transfers, a 1024 Byte packet
   --
   process(all) begin
      if (reset_n = '0' or xl_ENABLE = '0') then

         -- Init the State Vector
         wr             <= C_WR_SV_INIT;

         -- status is shared by the ADC FSM
         xl_DMA_BUSY    <= '0';
         xl_TAIL        <= (others => '0');
         xl_HEAD_ADDR   <= (others => '0');

      elsif (rising_edge(clk)) then

         -- edge-detect
         wr.run         <= xl_RUN;
         wr.run_r0      <= wr.run;

         -- update status
         xl_DMA_BUSY    <= wr.busy;
         xl_TAIL        <= "00" & std_logic_vector(wr.tail);
         xl_HEAD_ADDR   <= std_logic_vector(wr.head_addr);

         case wr.state is
            when IDLE =>
               -- Look for Run Bit Rising-Edge,
               if (wr.run_r0 = '0' and wr.run = '1') then
                  wr.state    <= WAIT_SLOT;
                  -- Address must be on 32-Bit boundary
                  wr.addr     <= unsigned(adc_ADR_BEG);
                  -- 256 32-Bit transfers (1K Bytes)
                  wr.burstcnt <= '1' & X"00";
                  wr.wrd_cnt  <= (others => '0');
                  wr.pool_cnt <= (others => '0');
                  wr.head_addr <= (others => '0');
                  wr.tail     <= (others => '0');
                  wr.busy     <= '1';
              else
                  wr.state    <= IDLE;
                  wr.busy     <= '0';
                  wr.pkt_rdy  <= '0';
                  wr.addr     <= (others => '0');
               end if;

            --
            -- Wait for a Packet in the Circular Buffer
            -- Also check for Abort
            --
            when WAIT_SLOT =>
               -- Abort Transfer
               if (wr.run = '0') then
                  wr.state    <= IDLE;
                  wr.head_addr <= (others => '0');
               -- Account for Circular Memory, Restart
               elsif (wr.addr >= unsigned(adc_ADR_END)) then
                  wr.state    <= WAIT_SLOT;
                  wr.addr     <= unsigned(adc_ADR_BEG);
               -- Interrupt Pool Count Check, if non-zero
               elsif (unsigned(adc_POOL_CNT) /= 0 and wr.pool_cnt >= unsigned(adc_POOL_CNT)) then
                  wr.pool_cnt <= (others => '0');
                  -- packet ready
                  wr.pkt_rdy  <= '1';
                  wr.head_addr <= wr.head_addr + 1;
               elsif (wr.tail /= ad.head) then
                  wr.state    <= DELAY;
                  wr.wrd_cnt  <= wr.wrd_cnt + 1;
               else
                  wr.state    <= WAIT_SLOT;
                  wr.pkt_rdy  <= '0';
               end if;

            --
            -- Account for Block RAM two-cycle latency
            -- and m1_wr_waitreq
            --
            when DELAY =>
               wr.state       <= WR_SLOT;
               wr.master      <= '1';
               wr.wrd_cnt     <= wr.wrd_cnt + 1;

            --
            -- Wait for Burst Transfer to Complete
            --
            when WR_SLOT =>
               if (wr.wrd_cnt = X"101" and m1_wr_waitreq = '0') then
                  wr.state    <= WAIT_SLOT;
                  wr.tail     <= wr.tail + 1;
                  wr.wrd_cnt  <= (others => '0');
                  wr.master   <= '0';
                  wr.addr     <= wr.addr + X"400";
                  wr.pool_cnt <= wr.pool_cnt + 1;
               elsif (m1_wr_waitreq = '0') then
                  wr.state    <= WR_SLOT;
                  wr.wrd_cnt  <= wr.wrd_cnt + 1;
                  wr.master   <= '1';
               else
                  wr.state    <= WR_SLOT;
                  wr.master   <= '1';
               end if;

            when others =>
               wr.state       <= IDLE;

         end case;

      end if;
   end process;

   --
   -- ADC CONVERSION START RATE
   --
   process(all) begin
      if (reset_n = '0' or ad.busy = '0') then
         cnvst_cnt      <= (others => '0');
         convert        <= '0';
      elsif (rising_edge(clk)) then
         if (unsigned(adc_ADC_RATE) > C_ADC_RATE_MIN and
             cnvst_cnt = unsigned(adc_ADC_RATE)) then
            cnvst_cnt   <= (others => '0');
            convert     <= '1';
         -- only allow minimum rate or above
         elsif (unsigned(adc_ADC_RATE) <= C_ADC_RATE_MIN and
                cnvst_cnt = C_ADC_RATE_MIN) then
            cnvst_cnt   <= (others => '0');
            convert     <= '1';
         else
            cnvst_cnt   <= cnvst_cnt + 1;
            convert     <= '0';
         end if;
      end if;
   end process;

   --
   -- ADC 32-BIT STAMP
   --
   process(all) begin
      if (reset_n = '0' or xl_ENABLE = '0') then
         stamp          <= (others => '0');
      elsif (rising_edge(clk)) then
         stamp          <= stamp + 1;
      end if;
   end process;

end rtl;
