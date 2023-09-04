library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity opto_rtx is
   port (
      clk                  : in  std_logic;
      reset_n              : in  std_logic;
      fsclk                : out std_logic;
      fscts                : in  std_logic;
      fsdo                 : in  std_logic;
      dat_rd               : in  std_logic;
      rx_rdy               : out std_logic;
      dout                 : out std_logic_vector(7 downto 0);
      fsdi                 : out std_logic;
      dat_wr               : in  std_logic;
      tx_rdy               : out std_logic;
      din                  : in  std_logic_vector(7 downto 0)
   );
end opto_rtx;

architecture rtl of opto_rtx is

--
-- TYPES
--
type   tx_state_t is (IDLE,TX_CTS,TX_DAT,TX_NEXT,TX_WAIT);
type   rx_state_t is (IDLE,RX_SAMPLE,RX_NEXT);

type  TX_SV_t is record
   state       : tx_state_t;
   dat         : std_logic_vector(9 downto 0);
   delay       : integer range 0 to 255;
   bit_cnt     : integer range 0 to 10;
   tick        : std_logic;
   busy        : std_logic;
   sout        : std_logic;
end record TX_SV_t;

type  RX_SV_t is record
   state       : rx_state_t;
   dat         : std_logic_vector(8 downto 0);
   store       : std_logic_vector(7 downto 0);
   bit_cnt     : integer range 0 to 10;
   tick        : std_logic;
   fifo_wr     : std_logic;
   busy        : std_logic;
end record RX_SV_t;

--
-- CONSTANTS
--

-- 16.666 MHZ @ 100MHZ SYSCLK, (60 nS)
constant C_FSCLK           : integer := 2;

-- TX State Vector Initialization
constant C_TX_SV_INIT : TX_SV_t := (
   state       => IDLE,
   dat         => (others => '0'),
   delay       => 0,
   bit_cnt     => 0,
   tick        => '0',
   busy        => '0',
   sout        => '1'
);

-- RX State Vector Initialization
constant C_RX_SV_INIT : RX_SV_t := (
   state       => IDLE,
   dat         => (others => '0'),
   store       => (others => '0'),
   bit_cnt     => 0,
   tick        => '0',
   fifo_wr     => '0',
   busy        => '0'
);

--
-- SIGNAL DECLARATIONS
--
signal rx                  : RX_SV_t;
signal tx                  : TX_SV_t;

signal rx_dout             : std_logic_vector(7 downto 0);

signal fifo_ef             : std_logic;

signal fs_clk_cnt          : integer range 0 to 2;
signal fs_tick             : std_logic;

--
-- MAIN CODE
--
begin

   --
   -- COMBINATORIAL OUTPUTS
   --
   dout        <= rx_dout;
   rx_rdy      <= not fifo_ef;

   -- FTDI output signals
   fsclk       <= fs_tick;
   fsdi        <= tx.sout;

   -- withold ready indication until character
   -- is fully transmitted
   tx_rdy      <= '1' when (tx.busy = '0' and fscts = '1' and
                            tx.tick = '1' and fs_tick = '0') else '0';

   --
   --   1024x8 BLOCKRAM RX FIFO
   --
   FTDI_FIFO_I : entity work.opto_fifo
      port map (
         clock             => clk,
         sclr              => not reset_n,
         data              => rx.store,
         rdreq             => dat_rd,
         wrreq             => rx.fifo_wr,
         empty             => fifo_ef,
         full              => open,
         q                 => rx_dout
      );

   --
   --  SERIAL TRANSMIT FSM
   --
   process(all) begin
      if (reset_n = '0') then
         -- Init the State Vector
         tx             <= C_TX_SV_INIT;

      elsif (rising_edge(clk)) then

         tx.tick        <= fs_tick;

         case tx.state is
            when IDLE =>
               --
               -- Look for Serial Write, if not Busy Receiving
               --
               if (dat_wr = '1') then
                  tx.state    <= TX_CTS;
                  tx.busy     <= '1';
                  -- start + symbol + stop, LSB first
                  tx.dat      <= '0' & din & '0';
                  tx.bit_cnt  <= 0;
                  tx.delay    <= 2;
                  tx.sout     <= '1';
               else
                  tx.state    <= IDLE;
                  tx.sout     <= '1';
               end if;

            --
            -- Check for Clear-to-Send at Rising-Edge
            --
            when TX_CTS =>
               if (tx.tick = '1' and fs_tick = '0' and fscts = '1') then
                  tx.state    <= TX_DAT;
                  tx.sout     <= tx.dat(0);
               else
                  tx.state    <= TX_CTS;
               end if;

            --
            -- Assert Serial Bit at Falling-Edge
            --
            when TX_DAT =>
               if (tx.tick = '0' and fs_tick = '1') then
                  tx.state    <= TX_NEXT;
                  -- next serial bit
                  tx.dat      <= '1' & tx.dat(9 downto 1);
                  tx.bit_cnt  <= tx.bit_cnt + 1;
               else
                  tx.state    <= TX_DAT;
               end if;

            --
            -- 10-Bits per Symbol
            --
            when TX_NEXT =>
               if (tx.bit_cnt = 10) then
                  tx.state    <= TX_WAIT;
                  tx.bit_cnt  <= 0;
                  tx.sout     <= '1';
               elsif (tx.tick = '1' and fs_tick = '0') then
                  tx.state    <= TX_DAT;
                  tx.sout     <= tx.dat(0);
               else
                  tx.state    <= TX_NEXT;
               end if;

            --
            -- Wait for 2 Serial Clocks
            --
            when TX_WAIT =>
               if (tx.tick = '1' and fs_tick = '0' and tx.delay = 0) then
                  tx.state    <= IDLE;
                  tx.busy     <= '0';
               elsif (tx.tick = '1' and fs_tick = '0') then
                  tx.state    <= TX_WAIT;
                  tx.delay    <= tx.delay - 1;
               else
                  tx.state    <= TX_WAIT;
                  tx.delay    <= tx.delay;
               end if;

            when others =>
               tx.state       <= IDLE;

         end case;

      end if;
   end process;

   --
   --  SERIAL RECIEVE FSM
   --
   process(all) begin
      if (reset_n = '0') then

         -- Init the State Vector
         rx             <= C_RX_SV_INIT;

      elsif (rising_edge(clk)) then

         rx.tick        <= fs_tick;

         case rx.state is
            when IDLE =>
               rx.fifo_wr     <= '0';
               --
               -- Look for the Start Bit, at the Rising-Edge
               -- of the Serial Clock
               --
               if (fsdo = '0' and rx.tick = '0' and fs_tick = '1') then
                  rx.state    <= RX_SAMPLE;
                  rx.dat      <= (others => '0');
                  rx.bit_cnt  <= 0;
                  rx.busy     <= '1';
               else
                  rx.state    <= IDLE;
                  rx.busy     <= '0';
               end if;

            --
            -- Sample the Incoming Serial Bit at
            -- the Rising-Edge of the Serial Clock
            --
            when RX_SAMPLE =>
               -- Check for time to sample
               if (rx.tick = '0' and fs_tick = '1') then
                  rx.state    <= RX_NEXT;
                  -- LSB first
                  rx.dat      <= fsdo & rx.dat(8 downto 1);
                  rx.bit_cnt  <= rx.bit_cnt + 1;
               else
                  rx.state    <= RX_SAMPLE;
               end if;

            --
            -- 9-Bits per Symbol, Start/SRCE Bits are dropped
            --
            when RX_NEXT =>
               if (rx.bit_cnt = 9) then
                  rx.state    <= IDLE;
                  rx.bit_cnt  <= 0;
                  -- drop start/srce bits
                  rx.store    <= rx.dat(7 downto 0);
                  rx.fifo_wr  <= '1';
               else
                  rx.state    <= RX_SAMPLE;
               end if;

            when others =>
               rx.state       <= IDLE;

         end case;

      end if;
   end process;

   --
   -- FTDI FT232H FSCLK
   --
   process(all) begin
      if (reset_n = '0') then
         fs_clk_cnt     <= 0;
         fs_tick        <= '0';
      elsif (rising_edge(clk)) then
         if (fs_clk_cnt = 0) then
            fs_tick     <= not fs_tick;
            fs_clk_cnt  <= C_FSCLK;
         else
            fs_tick     <= fs_tick;
            fs_clk_cnt  <= fs_clk_cnt - 1;
         end if;
      end if;
   end process;

end rtl;
