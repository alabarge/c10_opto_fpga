library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity opto_ctl is
   port (
      clk                  : in    std_logic;
      reset_n              : in    std_logic;
      int                  : out   std_logic_vector(2 downto 0);
      m1_read              : out   std_logic;
      m1_rd_address        : out   std_logic_vector(31 downto 0);
      m1_readdata          : in    std_logic_vector(31 downto 0);
      m1_rd_waitreq        : in    std_logic;
      m1_rd_burstcount     : out   std_logic_vector(15 downto 0);
      m1_rd_datavalid      : in    std_logic;
      cpu_DIN              : out   std_logic_vector(31 downto 0);
      cpu_DOUT             : in    std_logic_vector(31 downto 0);
      cpu_ADDR             : in    std_logic_vector(11 downto 0);
      cpu_WE               : in    std_logic_vector(1 downto 0);
      cpu_RE               : in    std_logic_vector(2 downto 0);
      head_addr            : in    std_logic_vector(15 downto 0);
      tail_addr            : out   std_logic_vector(15 downto 0);
      opto_CONTROL         : in    std_logic_vector(31 downto 0);
      opto_ADR_BEG         : in    std_logic_vector(31 downto 0);
      opto_ADR_END         : in    std_logic_vector(31 downto 0);
      opto_PKT_CNT         : in    std_logic_vector(31 downto 0);
      opto_STATUS          : out   std_logic_vector(31 downto 0);
      fsclk                : out   std_logic;
      fscts                : in    std_logic;
      fsdo                 : in    std_logic;
      fsdi                 : out   std_logic;
      debug                : out   std_logic_vector(3 downto 0)
   );
end opto_ctl;

architecture rtl of opto_ctl is

--
-- TYPES
--
type ft_state_t    is (IDLE,FLUSH,WAIT_REQ,TX_START,TX_PICK,TX_LEN,TX_ESC,TX_FLAG,TX_DATA,TX_END,
                       RX_GET,RX_FRAME,RX_DATA,RX_STORE,RX_NEXT,
                       PIPE_START,PIPE_WR,PIPE_PICK,PIPE_ESC,PIPE_FLAG, PIPE_DAT, PIPE_END, PIPE_INT);
type rd_state_t    is (IDLE, WAIT_REQ, RD_REQ, RD_SLOT);

type  FT_SV_t is record
   state       : ft_state_t;
   rx_head     : unsigned(1 downto 0);
   rx_tail     : unsigned(1 downto 0);
   rx_ptr      : unsigned(8 downto 0);
   rx_din      : std_logic_vector(7 downto 0);
   rx_rd       : std_logic;
   rx_we       : std_logic;
   rx_int      : std_logic;
   rx_esc      : std_logic;
   rx_busy     : std_logic;
   tx_tail     : unsigned(1 downto 0);
   tx_head     : unsigned(1 downto 0);
   tx_ptr      : unsigned(8 downto 0);
   tx_len      : unsigned(11 downto 0);
   tx_cnt      : unsigned(11 downto 0);
   tx_din      : std_logic_vector(7 downto 0);
   tx_wr       : std_logic;
   tx_int      : std_logic;
   tx_busy     : std_logic;
   dout        : std_logic_vector(7 downto 0);
   pipe_ptr    : unsigned(10 downto 0);
   pipe_busy   : std_logic;
   pipe_ack    : std_logic;
   pipe_int    : std_logic;
   pipe_cnt    : unsigned(31 downto 0);
   pipe_run    : std_logic;
   flush_cnt   : unsigned(15 downto 0);
end record FT_SV_t;

type  RD_SV_t is record
   state       : rd_state_t;
   addr        : unsigned(31 downto 0);
   pkt_cnt     : unsigned(31 downto 0);
   wrd_cnt     : unsigned(7 downto 0);
   master      : std_logic;
   run         : std_logic;
   pipe_int    : std_logic;
   tail_addr   : unsigned(15 downto 0);
   burstcnt    : std_logic_vector(15 downto 0);
   rdy         : std_logic;
   dma_ack     : std_logic;
end record RD_SV_t;

--
-- CONSTANTS
--

-- OCTET FRAMING FLAGS
constant C_SOF             : std_logic_vector(7 downto 0) := X"7E";
constant C_EOF             : std_logic_vector(7 downto 0) := X"7D";
constant C_ESC             : std_logic_vector(7 downto 0) := X"7C";

constant C_CM_MSG_LEN      : unsigned(11 downto 0) := X"00C";

-- FT State Vector Initialization
constant C_FT_SV_INIT : FT_SV_t := (
   state       => IDLE,
   rx_head     => (others => '0'),
   rx_tail     => (others => '0'),
   rx_ptr      => (others => '0'),
   rx_din      => (others => '0'),
   rx_rd       => '0',
   rx_we       => '0',
   rx_int      => '0',
   rx_esc      => '0',
   rx_busy     => '0',
   tx_tail     => (others => '0'),
   tx_head     => (others => '0'),
   tx_ptr      => (others => '0'),
   tx_len      => (others => '0'),
   tx_cnt      => (others => '0'),
   tx_din      => (others => '0'),
   tx_wr       => '0',
   tx_int      => '0',
   tx_busy     => '0',
   dout        => (others => '0'),
   pipe_ptr    => (others => '0'),
   pipe_busy   => '0',
   pipe_ack    => '0',
   pipe_int    => '0',
   pipe_cnt    => (others => '0'),
   pipe_run    => '0',
   flush_cnt   => (others => '0')
);

-- RD State Vector Initialization
constant C_RD_SV_INIT : RD_SV_t := (
   state       => IDLE,
   addr        => (others => '0'),
   pkt_cnt     => (others => '0'),
   wrd_cnt     => (others => '0'),
   master      => '0',
   run         => '0',
   pipe_int    => '0',
   tail_addr   => (others => '0'),
   burstcnt    => (others => '0'),
   rdy         => '0',
   dma_ack     => '0'
);

--
-- SIGNAL DECLARATIONS
--

-- State Machine Data Types
signal ft               : FT_SV_t;
signal rd               : RD_SV_t;

-- 32-Bit State Machine Status
signal opto_stat        : std_logic_vector(31  downto 0);
alias  xl_TAIL_ADDR     : std_logic_vector(15 downto 0) is opto_stat(15 downto 0);
alias  xl_RX_HEAD       : std_logic_vector(1 downto 0) is opto_stat(17 downto 16);
alias  xl_TX_TAIL       : std_logic_vector(1 downto 0) is opto_stat(19 downto 18);
alias  xl_UNUSED        : std_logic_vector(8 downto 0) is opto_stat(28 downto 20);
alias  xl_RX_RDY        : std_logic is opto_stat(29);
alias  xl_TX_RDY        : std_logic is opto_stat(30);
alias  xl_TX_BUSY       : std_logic is opto_stat(31);

-- 32-Bit Control Register
alias  xl_TX_HEAD       : std_logic_vector(1 downto 0) is opto_CONTROL(1 downto 0);
alias  xl_RX_TAIL       : std_logic_vector(1 downto 0) is opto_CONTROL(3 downto 2);
alias  xl_PIPE_INT      : std_logic is opto_CONTROL(25);
alias  xl_DMA_REQ       : std_logic is opto_CONTROL(26);
alias  xl_RX_INT        : std_logic is opto_CONTROL(27);
alias  xl_TX_INT        : std_logic is opto_CONTROL(28);
alias  xl_PIPE_RUN      : std_logic is opto_CONTROL(29);
alias  xl_FTDI_RUN      : std_logic is opto_CONTROL(30);
alias  xl_ENABLE        : std_logic is opto_CONTROL(31);

signal tx_rdy           : std_logic;
signal rx_rdy           : std_logic;
signal rx_dout          : std_logic_vector(7 downto 0);

-- Master Read Signals
signal readdata         : std_logic_vector(31 downto 0);
signal rd_waitreq       : std_logic;
signal rd_datavalid     : std_logic;

signal dma_req          : std_logic;
signal dma_req_r0       : std_logic;
signal pipe_req         : std_logic;
signal pipe_req_r0      : std_logic;
signal rd_we            : std_logic;
signal rd_addr          : std_logic_vector(7 downto 0);
signal head_addr_i      : unsigned(15 downto 0);

-- BlockRAM Signals
signal rx_cpu_dat       : std_logic_vector(31 downto 0);
signal tx_cpu_dat       : std_logic_vector(31 downto 0);
signal tx_bram_dat      : std_logic_vector(7 downto 0);

signal ft_data          : std_logic_vector(7 downto 0);
signal pipe_data        : std_logic_vector(31 downto 0);


--
-- MAIN CODE
--
begin

   --
   -- COMBINATORIAL OUTPUTS
   --
   int(0)               <= ft.tx_int   and xl_TX_INT;
   int(1)               <= ft.rx_int   and xl_RX_INT;
   int(2)               <= ft.pipe_int and xl_PIPE_INT;

   opto_STATUS          <= opto_stat;

   debug(0)             <= '0';
   debug(1)             <= '0';
   debug(2)             <= '0';
   debug(3)             <= '0';

   cpu_DIN              <= rx_cpu_dat  when cpu_RE(0) = '1' else
                           tx_cpu_dat  when cpu_RE(1) = '1' else
                           pipe_data   when cpu_RE(2) = '1' else
                           (others => '0');

   -- Master Read
   m1_rd_address        <= std_logic_vector(rd.addr);
   readdata             <= m1_readdata;
   rd_waitreq           <= m1_rd_waitreq;
   m1_read              <= rd.master;
   m1_rd_burstcount     <= rd.burstcnt;
   rd_datavalid         <= m1_rd_datavalid;

   -- Shared Packet Address
   tail_addr            <= std_logic_vector(rd.tail_addr);

   --
   --   THIS BRAM IS ONLY USED FOR 512-BYTE CM MESSAGES
   --   THERE ARE FOUR 512-BYTE SLOTS, RX
   --
   --   2048x8 <==> 512x32 Dual-Port BLOCK RAM
   --   BRAM_a[7:0] <==> FT232[7:0]
   --   CPU <==> BRAM_b[31:0]
   --   CPU <==> BRAM <==> FT232 <==> USB OUT TRANSFER
   --
   OPTO_2K_II : entity work.opto_2k
      port map (
         address_a      => std_logic_vector(ft.rx_head) & std_logic_vector(ft.rx_ptr),
         address_b      => cpu_ADDR(8 downto 0),
         clock          => clk,
         data_a         => ft.rx_din,
         data_b         => cpu_DOUT,
         wren_a         => ft.rx_we,
         wren_b         => cpu_WE(0),
         q_a            => open,
         q_b            => rx_cpu_dat
      );

   --
   --   THIS BRAM IS ONLY USED FOR 512-BYTE CM MESSAGES
   --   THERE ARE FOUR 512-BYTE SLOTS, TX
   --
   --   2048x8 <==> 512x32 Dual-Port BLOCK RAM
   --   BRAM_a[7:0] <==> FT232[7:0]
   --   CPU <==> BRAM_b[31:0]
   --   CPU <==> BRAM <==> FT232 <==> USB IN TRANSFER
   --
   OPTO_2K_I : entity work.opto_2k
      port map (
         address_a      => std_logic_vector(ft.tx_tail) & std_logic_vector(ft.tx_ptr),
         address_b		=> cpu_ADDR(8 downto 0),
         clock		      => clk,
         data_a		   => (others => '0'),
         data_b		   => cpu_DOUT,
         wren_a		   => '0',
         wren_b		   => cpu_WE(1),
         q_a		      => tx_bram_dat,
         q_b		      => tx_cpu_dat
      );

   --
   --   FTDI  RX/TX
   --
   FTDI_RTX_I: entity work.opto_rtx
      port map (
         clk               => clk,
         reset_n           => xl_ENABLE,
         fsclk             => fsclk,
         fscts             => fscts,
         fsdo              => fsdo,
         dat_rd            => ft.rx_rd,
         rx_rdy            => rx_rdy,
         dout              => rx_dout,
         fsdi              => fsdi,
         dat_wr            => ft.tx_wr,
         tx_rdy            => tx_rdy,
         din               => ft.tx_din
      );

   --
   --  FTDI STATE MACHINE
   --
   --  ONLY USED FOR 512-BYTE CM MESSAGES
   --
   --  The FTDI state machine emptys slots from the CPU circular
   --  buffer and sends the packets to the USB IN endpoint.
   --  The state machine also fills slots from the USB OUT
   --  transfers. These slots only contain CM control messages.
   --  Each slot is a 512-Byte packet.
   --
   --  Additionally, the state machine will send
   --  1K pipe messages from the buffer that is
   --  filled by the read burst state machine.
   --
   process(all) begin
      if (reset_n = '0' or xl_ENABLE = '0') then

         -- Init the State Vector
         ft             <= C_FT_SV_INIT;

         -- Status is shared by RD FSM
         xl_RX_HEAD     <= (others => '0');
         xl_TX_TAIL     <= (others => '0');
         xl_UNUSED      <= (others => '0');
         xl_RX_RDY      <= '0';
         xl_TX_RDY      <= '0';
         xl_TX_BUSY     <= '0';

      elsif (rising_edge(clk)) then

         ft.rx_tail     <= unsigned(xl_RX_TAIL);
         ft.tx_head     <= unsigned(xl_TX_HEAD);
         ft.pipe_run    <= xl_PIPE_RUN;

         -- Update Status
         xl_RX_HEAD     <= std_logic_vector(ft.rx_head);
         xl_TX_TAIL     <= std_logic_vector(ft.tx_tail);
         xl_UNUSED      <= (others => '0');
         xl_TX_BUSY     <= ft.tx_busy or ft.pipe_busy;
         xl_TX_RDY      <= tx_rdy;
         xl_RX_RDY      <= rx_rdy;

         case ft.state is

            when IDLE =>
               -- Wait for RUN Assertion
               if (xl_FTDI_RUN = '1') then
                  ft.state    <= FLUSH;
                  ft.tx_ptr   <= (others => '0');
                  ft.rx_ptr   <= (others => '0');
                  ft.rx_head  <= (others => '0');
                  ft.tx_tail  <= (others => '0');
                  ft.pipe_cnt <= (others => '0');
                  ft.flush_cnt <= X"1000";
               else
                  ft.state    <= IDLE;
                  ft.rx_rd    <= '0';
               end if;

            --
            -- ALWAYS FLUSH THE FTDI OUT OF IDLE
            -- ASSERT RD FOR 4K CLOCKS, ~275uS
            --
            when FLUSH =>
               if (ft.flush_cnt = 0) then
                  ft.state    <= WAIT_REQ;
                  ft.rx_rd    <= '0';
               else
                  ft.state    <= FLUSH;
                  ft.flush_cnt <= ft.flush_cnt - 1;
                  ft.rx_rd    <= '1';
               end if;

            --
            -- Wait for RX/TX/PIPE Request, Priority Based
            --
            when WAIT_REQ =>
               -- Always cleared in WAIT_REQ
               ft.tx_wr       <= '0';
               ft.tx_din      <= (others => '0');
               ft.tx_int      <= '0';
               ft.rx_int      <= '0';
               ft.pipe_int    <= '0';
               -- Abort
               if (xl_FTDI_RUN = '0') then
                  ft.state    <= IDLE;
               -- Clear Pipe Count
               elsif (xl_PIPE_RUN = '1' and ft.pipe_run = '0') then
                  ft.state    <= WAIT_REQ;
                  ft.pipe_cnt <= (others => '0');
               -- Get Incoming Character, highest priority
               elsif (rx_rdy = '1') then
                  ft.state    <= RX_GET;
                  ft.rx_rd    <= '1';
                  ft.rx_busy  <= '1';
               -- Start Control Msg Transmission
               elsif (ft.tx_head /= ft.tx_tail and tx_rdy = '1' and
                      ft.pipe_busy = '0' and ft.tx_busy = '0' and
                      ft.rx_busy = '0') then
                  ft.state    <= TX_START;
                  ft.tx_busy  <= '1';
                  ft.tx_ptr   <= (others => '0');
                  ft.tx_cnt   <= (others => '0');
                  -- force cm message length to start, length is
                  -- read from the message header
                  ft.tx_len   <= C_CM_MSG_LEN;
               -- Continue Control Msg until all Bytes Sent
               elsif (ft.tx_busy = '1' and tx_rdy = '1' and
                      ft.rx_busy = '0') then
                  ft.state    <= TX_PICK;
               -- Start Pipe Msg Transmission if not Busy
               elsif (pipe_req = '1' and tx_rdy = '1' and
                      ft.tx_busy = '0' and ft.rx_busy = '0') then
                  ft.state    <= PIPE_START;
                  ft.pipe_ack <= '1';
                  ft.pipe_ptr <= (others => '0');
                  ft.pipe_busy <= '1';
               -- Continue Pipe Msg Transmission
               elsif (tx_rdy = '1' and ft.pipe_busy = '1') then
                  ft.state    <= PIPE_PICK;
               else
                  ft.state    <= WAIT_REQ;
               end if;

            --
            -- SEND START-OF-FRAME FLAG
            --
            when TX_START =>
               ft.state       <= TX_PICK;
               ft.tx_din      <= C_SOF;
               ft.tx_wr       <= '1';

            --
            -- GET OCTET FROM BRAM
            --
            when TX_PICK =>
               ft.tx_wr       <= '0';
               if (ft.tx_cnt = ft.tx_len) then
                  ft.state    <= TX_END;
               elsif (tx_rdy = '1') then
                  ft.state    <= TX_LEN;
                  ft.dout     <= tx_bram_dat;
                  ft.tx_ptr   <= ft.tx_ptr + 1;
                  ft.tx_cnt   <= ft.tx_cnt + 1;
               else
                  ft.state    <= TX_PICK;
               end if;

            --
            -- READ MSG LENGTH FROM HEADER
            --
            when TX_LEN =>
               if (ft.tx_cnt = 7) then
                  ft.state    <= TX_ESC;
                  ft.tx_len(7 downto 0)  <= unsigned(ft.dout);
               elsif (ft.tx_cnt = 8) then
                  ft.state    <= TX_ESC;
                  ft.tx_len(11 downto 8) <= unsigned(ft.dout(3 downto 0));
               else
                  ft.state    <= TX_ESC;
               end if;

            --
            -- CHECK FOR OCTET STUFFING
            --
            when TX_ESC =>
               if (ft.dout = C_SOF or
                   ft.dout = C_EOF or
                   ft.dout = C_ESC) then
                  ft.state    <= TX_FLAG;
                  ft.dout     <= ft.dout xor X"20";
               else
                  ft.state    <= TX_DATA;
               end if;

            --
            -- SEND ESCAPE FLAG
            --
            when TX_FLAG =>
               if (tx_rdy = '1') then
                  ft.state    <= TX_DATA;
                  ft.tx_din   <= C_ESC;
                  ft.tx_wr    <= '1';
               else
                  ft.state    <= TX_FLAG;
                  ft.tx_wr    <= '0';
               end if;

            --
            -- SEND MESSAGE BYTE
            --
            when TX_DATA =>
               if (tx_rdy = '1') then
                  ft.state    <= WAIT_REQ;
                  ft.tx_din   <= ft.dout;
                  ft.tx_wr    <= '1';
               else
                  ft.state    <= TX_DATA;
                  ft.tx_wr    <= '0';
               end if;

            --
            -- SEND END-OF-FRAME FLAG
            --
            when TX_END =>
               ft.state    <= WAIT_REQ;
               ft.tx_din   <= C_EOF;
               ft.tx_wr    <= '1';
               ft.tx_int   <= '1';
               ft.tx_busy  <= '0';
               ft.tx_tail  <= ft.tx_tail + 1;

            --
            -- ACCOUNT FOR FIFO READ DELAY
            --
            when RX_GET =>
               ft.state       <= RX_FRAME;
               ft.rx_rd       <= '0';

            --
            -- CHECK FOR FRAMING
            --
            when RX_FRAME =>
               -- start of frame
               if (rx_dout = C_SOF) then
                  ft.state    <= WAIT_REQ;
                  ft.rx_esc   <= '0';
                  ft.rx_ptr   <= (others => '0');
               -- end of frame
               elsif (rx_dout = C_EOF) then
                  ft.state    <= WAIT_REQ;
                  ft.rx_esc   <= '0';
                  ft.rx_int   <= '1';
                  ft.rx_busy  <= '0';
                  ft.rx_head  <= ft.rx_head + 1;
               -- escape next character
               elsif (rx_dout = C_ESC) then
                  ft.state    <= WAIT_REQ;
                  ft.rx_esc   <= '1';
               else
                  ft.state    <= RX_DATA;
               end if;

            --
            -- ESCAPE OCTET
            --
            when RX_DATA =>
               -- Escape
               if (ft.rx_esc = '1') then
                  ft.state    <= RX_STORE;
                  ft.rx_din   <= rx_dout xor X"20";
                  ft.rx_esc   <= '0';
               -- w/o Escape
               else
                  ft.state    <= RX_STORE;
                  ft.rx_din   <= rx_dout;
               end if;

            --
            -- STORE BYTE TO BRAM
            --
            when RX_STORE =>
               ft.state       <= RX_NEXT;
               ft.rx_we       <= '1';

            --
            -- INCREMENT BRAM ADDRESS
            --
            when RX_NEXT =>
               ft.state       <= WAIT_REQ;
               ft.rx_we       <= '0';
               ft.rx_din      <= (others => '0');
               ft.rx_ptr      <= ft.rx_ptr + 1;

            --
            -- START PIPE MESSAGE BURST, ALWAYS 1024 BYTES
            --
            when PIPE_START =>
               ft.state       <= PIPE_PICK;
               ft.pipe_ack    <= '0';
               ft.tx_din      <= C_SOF;
               ft.tx_wr       <= '1';

            --
            -- GET NEXT OCTET
            --
            when PIPE_PICK =>
               ft.tx_wr       <= '0';
               if (ft.pipe_ptr(10) = '1') then
                  ft.state    <= PIPE_END;
               elsif (tx_rdy = '1') then
                  ft.state    <= PIPE_ESC;
                  ft.dout     <= ft_data;
               else
                  ft.state    <= PIPE_PICK;
               end if;

            --
            -- CHECK FOR OCTET STUFFING
            --
            when PIPE_ESC =>
               ft.tx_wr       <= '0';
               ft.pipe_ptr    <= ft.pipe_ptr + 1;
               if (ft.dout = C_SOF or
                   ft.dout = C_EOF or
                   ft.dout = C_ESC) then
                  ft.state    <= PIPE_FLAG;
                  ft.dout     <= ft.dout xor X"20";
               else
                  ft.state    <= PIPE_DAT;
               end if;

            --
            -- SEND ESCAPE FLAG
            --
            when PIPE_FLAG =>
               if (tx_rdy = '1') then
                  ft.state    <= PIPE_DAT;
                  ft.tx_din   <= C_ESC;
                  ft.tx_wr    <= '1';
               else
                  ft.state    <= PIPE_FLAG;
                  ft.tx_wr    <= '0';
               end if;

            --
            -- SEND MESSAGE BYTE
            --
            when PIPE_DAT =>
               if (tx_rdy = '1') then
                  ft.state    <= WAIT_REQ;
                  ft.tx_din   <= ft.dout;
                  ft.tx_wr    <= '1';
               else
                  ft.state    <= PIPE_DAT;
                  ft.tx_wr    <= '0';
               end if;

            --
            -- SEND END-OF-FRAME FLAG
            --
            when PIPE_END =>
               if (tx_rdy = '1') then
                  ft.state    <= PIPE_INT;
                  ft.tx_din   <= C_EOF;
                  ft.tx_wr    <= '1';
                  ft.tx_int   <= '1';
                  ft.pipe_ptr <= (others => '0');
                  ft.pipe_cnt <= ft.pipe_cnt + 1;
               else
                  ft.state    <= PIPE_END;
                  ft.tx_wr    <= '0';
               end if;

            --
            -- TRANSFER COMPLETE INTERRUPT
            --
            when PIPE_INT =>
               ft.tx_wr       <= '0';
               ft.tx_din      <= (others => '0');
               ft.tx_int      <= '0';
               ft.pipe_busy   <= '0';
               -- Transfer Complete
               if (opto_PKT_CNT /= X"00000000" and
                   ft.pipe_cnt = unsigned(opto_PKT_CNT)) then
                  ft.state    <= WAIT_REQ;
                  ft.pipe_int <= '1';
                  ft.pipe_cnt <= (others => '0');
               else
                  ft.state    <= WAIT_REQ;
               end if;

            when others =>
               ft.state       <= IDLE;

         end case;

      end if;
   end process;

   --
   --   In order to modify the pipe message size the
   --   following items must be changed:
   --
   --   1. ft.pipe_ptr
   --   2. rd.addr, rd.wrd_cnt
   --   3. opto_burst.vhd size
   --   4. rd.burstcnt
   --   5. rd.addr boundary
   --
   --   Current pipe message size is 1024 Bytes
   --

   --
   --   THIS BRAM IS ONLY USED FOR A SINGLE PIPE MESSAGE
   --
   --   1024x8 <==> 256x32 Dual-Port BLOCK RAM
   --   BLOCKRAM_a[7:0] -> FT232[7:0]
   --   master_readdata -> BLOCKRAM_b[31:0]
   --
   --   ON-CHIP -> BLOCKRAM -> FT232 -> USB, "IN" TRANSFER
   --
   OPTO_BURST_I : entity work.opto_burst
      port map (
         address_a		=> std_logic_vector(ft.pipe_ptr(9 downto 0)),
         address_b		=> rd_addr,
         clock  	      => clk,
         data_a		   => X"00",
         data_b		   => readdata,
         wren_a		   => '0',
         wren_b		   => rd_we,
         q_a		      => ft_data,
         q_b		      => pipe_data
      );
   rd_addr              <= std_logic_vector(rd.wrd_cnt) when cpu_RE(2) = '0'
                           else cpu_ADDR(7 downto 0);
   rd_we                <= '1' when rd.state = RD_SLOT and rd_datavalid = '1' else '0';

   --
   --  MASTER READ BURST TRANSFER, ON-CHIP TO USB
   --
   --  The FTDI state machine emptys a single slot from the DMA read
   --  transfer. The slot is a 1024-Byte packet. Takes about 100 uS
   --
   --  NOTES:
   --    * Master read/write addresses are byte pointers.
   --    * Avalon transfers are always 32-Bits.
   --
   process(all) begin
      if (reset_n = '0' or xl_ENABLE = '0') then

         -- Init the State Vector
         rd             <= C_RD_SV_INIT;

         -- Status is shared by FTDI FSM
         xl_TAIL_ADDR   <= (others => '0');

      elsif (rising_edge(clk)) then

         -- update status
         xl_TAIL_ADDR   <= std_logic_vector(rd.tail_addr);
         rd.run         <= xl_PIPE_RUN;

         case rd.state is
            -- Wait for RUN Assertion
            when IDLE =>
               if (xl_PIPE_RUN = '1' and rd.run = '0') then
                  rd.state    <= WAIT_REQ;
                  -- Address must be on a 32-Bit boundary
                  rd.addr     <= unsigned(opto_ADR_BEG);
                  rd.tail_addr <= (others => '0');
                  rd.pkt_cnt  <= (others => '0');
                  rd.pipe_int <= '0';
               else
                  rd.state    <= IDLE;
                  rd.pipe_int <= '0';
               end if;

            -- Wait for Pipe Messages to Send
            when WAIT_REQ =>
               -- Abort
               if (xl_PIPE_RUN = '0') then
                  rd.state    <= IDLE;
               -- Transfer Complete
               elsif (opto_PKT_CNT /= X"00000000" and rd.pkt_cnt = unsigned(opto_PKT_CNT)) then
                  rd.state    <= IDLE;
                  rd.pipe_int <= '1';
               -- Account for Circular Memory, Restart
               elsif (rd.addr >= unsigned(opto_ADR_END)) then
                  rd.state    <= WAIT_REQ;
                  rd.addr     <= unsigned(opto_ADR_BEG);
               -- software controlled pipe message
               elsif (dma_req = '1' and ft.pipe_busy = '0' and pipe_req = '0') then
                  rd.state    <= RD_REQ;
                  -- Address must be on a 32-Bit boundary
                  -- software must update this address for each message sent
                  rd.addr     <= unsigned(opto_ADR_BEG);
                  rd.burstcnt <= X"0100";
                  rd.wrd_cnt  <= (others => '0');
                  rd.tail_addr <= (others => '0');
                  rd.dma_ack  <= '1';
                  rd.master   <= '1';
               -- hardware controlled pipe message
               elsif (head_addr_i /= 0 and (rd.tail_addr /= head_addr_i) and
                     ft.pipe_busy = '0' and pipe_req = '0') then
                  rd.state    <= RD_REQ;
                  rd.burstcnt <= X"0100";
                  rd.wrd_cnt  <= (others => '0');
                  rd.master   <= '1';
               else
                  rd.state    <= WAIT_REQ;
                  rd.rdy      <= '0';
               end if;

            --
            -- Issue a single burst request of
            -- 256 32-Bit words, the master_read signal
            -- is only asserted during this state.
            --
            when RD_REQ =>
               rd.dma_ack     <= '0';
               if (rd_waitreq = '0') then
                  rd.state    <= RD_SLOT;
                  rd.master   <= '0';
               else
                  rd.state    <= RD_REQ;
               end if;

            --
            -- Wait for Burst Transfer to Complete
            --
            when RD_SLOT =>
               if (rd.wrd_cnt = 255 and rd_datavalid = '1') then
                  rd.state    <= WAIT_REQ;
                  -- indicate that message is ready to send
                  rd.rdy      <= '1';
                  rd.tail_addr <= rd.tail_addr + 1;
                  -- increment address by pipe message, 1024 bytes
                  rd.addr     <= rd.addr + X"400";
                  rd.pkt_cnt  <= rd.pkt_cnt + 1;
               elsif (rd_datavalid = '1') then
                  rd.state    <= RD_SLOT;
                  rd.wrd_cnt  <= rd.wrd_cnt + 1;
               else
                  rd.state    <= RD_SLOT;
               end if;

            when others =>
               rd.state       <= IDLE;

         end case;

      end if;
   end process;

   --
   --  MASTER READ BURST RUN REQ/ACK, DMA REQUEST
   --
   process(all) begin
      if (reset_n = '0' or xl_ENABLE = '0') then
         dma_req     <= '0';
         dma_req_r0  <= '0';
      elsif (rising_edge(clk)) then
         dma_req_r0  <= xl_DMA_REQ;
         if (xl_DMA_REQ = '1' and dma_req_r0 = '0') then
            dma_req  <= '1';
         elsif (rd.dma_ack = '1') then
            dma_req  <= '0';
         end if;
      end if;
   end process;

   --
   --  PIPE MESSAGE REQ/ACK
   --
   process(all) begin
      if (reset_n = '0' or xl_ENABLE = '0') then
         pipe_req    <= '0';
         pipe_req_r0 <= '0';
      elsif (rising_edge(clk)) then
         pipe_req_r0 <= rd.rdy;
         if (rd.rdy = '1' and pipe_req_r0 = '0') then
            pipe_req <= '1';
         elsif (ft.pipe_ack = '1') then
            pipe_req <= '0';
         end if;
      end if;
   end process;

   --
   --  CAPTURE head_addr
   --
   process(all) begin
      if (reset_n = '0' or xl_ENABLE = '0') then
         head_addr_i    <= (others => '0');
      elsif (rising_edge(clk)) then
         head_addr_i    <= unsigned(head_addr);
      end if;
   end process;

end rtl;
