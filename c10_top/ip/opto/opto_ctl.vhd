library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity opto_ctl is
   port (
      clk                  : in    std_logic;
      reset_n              : in    std_logic;
      int                  : out   std_logic_vector(1 downto 0);
      m1_read              : out   std_logic;
      m1_rd_address        : out   std_logic_vector(31 downto 0);
      m1_readdata          : in    std_logic_vector(31 downto 0);
      m1_rd_waitreq        : in    std_logic;
      m1_rd_burstcount     : out   std_logic_vector(15 downto 0);
      m1_rd_datavalid      : in    std_logic;
      cpu_DIN              : out   std_logic_vector(31 downto 0);
      cpu_DOUT             : in    std_logic_vector(31 downto 0);
      cpu_ADDR             : in    std_logic_vector(13 downto 0);
      cpu_WE               : in    std_logic;
      cpu_RE               : in    std_logic_vector(1 downto 0);
      head_addr            : in    std_logic_vector(15 downto 0);
      tail_addr            : out   std_logic_vector(15 downto 0);
      opto_CONTROL         : in    std_logic_vector(31 downto 0);
      opto_ADR_BEG         : in    std_logic_vector(31 downto 0);
      opto_ADR_END         : in    std_logic_vector(31 downto 0);
      opto_STATUS          : out   std_logic_vector(31 downto 0);
      fsclk                : out   std_logic;
      fscts                : in    std_logic;
      fsdo                 : in    std_logic;
      fsdi                 : out   std_logic
   );
end opto_ctl;

architecture rtl of opto_ctl is

--
-- TYPES
--
type ft_state_t    is (IDLE,WAIT_REQ,TX_START,TX_HDR,TX_SEND_0,TX_SEND_1,TX_PICK,TX_ESC,TX_FLAG,
                       TX_DATA,TX_END,RX_GET,RX_GET_0,RX_FRAME,RX_DATA,RX_STORE,RX_NEXT,
                       PIPE_START,PIPE_WR,PIPE_HDR,PIPE_PICK,PIPE_ESC,PIPE_FLAG, PIPE_DAT, PIPE_END);
type rd_state_t    is (IDLE, WAIT_REQ, RD_REQ, RD_SLOT);

type  FT_SV_t is record
   state       : ft_state_t;
   tx_wr       : std_logic;
   tx_din      : std_logic_vector(7 downto 0);
   tx_len      : unsigned(10 downto 0);
   io_ptr      : unsigned(10 downto 0);
   in_ptr      : unsigned(10 downto 0);
   out_ptr     : unsigned(10 downto 0);
   pipe_ptr    : unsigned(10 downto 0);
   in_data     : std_logic_vector(7 downto 0);
   out_data    : std_logic_vector(7 downto 0);
   tx_bram     : std_logic;
   opto_din    : std_logic_vector(7 downto 0);
   opto_we     : std_logic;
   tx_ack      : std_logic;
   tx_int      : std_logic;
   tx_msg      : std_logic;
   pipe_msg    : std_logic;
   pipe_ack    : std_logic;
   rx_msg      : std_logic;
   rx_busy     : std_logic;
   rx_rd       : std_logic;
   rx_hdr      : std_logic;
   rx_int      : std_logic;
   rx_esc      : std_logic;
end record FT_SV_t;

type  RD_SV_t is record
   state       : rd_state_t;
   addr        : unsigned(31 downto 0);
   blk_cnt     : unsigned(31 downto 0);
   wrd_cnt     : unsigned(7 downto 0);
   master      : std_logic;
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

constant C_BCAST           : std_logic_vector(3 downto 0) := X"F";
constant C_TX_HDR          : std_logic_vector(7 downto 0) := X"0F";

-- FT State Vector Initialization
constant C_FT_SV_INIT : FT_SV_t := (
   state       => IDLE,
   tx_wr       => '0',
   tx_din      => (others => '0'),
   tx_len      => (others => '0'),
   io_ptr      => (others => '0'),
   in_ptr      => (others => '0'),
   out_ptr     => (others => '0'),
   pipe_ptr    => (others => '0'),
   in_data     => (others => '0'),
   out_data    => (others => '0'),
   tx_bram     => '0',
   opto_din    => (others => '0'),
   opto_we     => '0',
   tx_ack      => '0',
   tx_int      => '0',
   tx_msg      => '0',
   pipe_msg    => '0',
   pipe_ack    => '0',
   rx_msg      => '0',
   rx_busy     => '0',
   rx_rd       => '0',
   rx_hdr      => '0',
   rx_int      => '0',
   rx_esc      => '0'
);

-- RD State Vector Initialization
constant C_RD_SV_INIT : RD_SV_t := (
   state       => IDLE,
   addr        => (others => '0'),
   blk_cnt     => (others => '0'),
   wrd_cnt     => (others => '0'),
   master      => '0',
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
alias  xl_RX_LEN        : std_logic_vector(11  downto 0) is opto_stat(11  downto 0);
alias  xl_TAIL_ADDR     : std_logic_vector(14 downto 0) is opto_stat(26 downto 12);
alias  xl_RX_PEND       : std_logic is opto_stat(27);
alias  xl_RX_RDY        : std_logic is opto_stat(28);
alias  xl_TX_RDY        : std_logic is opto_stat(29);
alias  xl_TX_MSG        : std_logic is opto_stat(30);
alias  xl_BCAST         : std_logic is opto_stat(31);

-- 32-Bit Control Register
alias  xl_TX_LEN        : std_logic_vector(11  downto 0) is opto_CONTROL(11  downto 0);
alias  xl_DEV_ID        : std_logic_vector(3  downto 0) is opto_CONTROL(15 downto 12);
alias  xl_FLUSH         : std_logic is opto_CONTROL(22);
alias  xl_DMA_REQ       : std_logic is opto_CONTROL(23);
alias  xl_MSG_REQ       : std_logic is opto_CONTROL(24);
alias  xl_RX_INT        : std_logic is opto_CONTROL(25);
alias  xl_TX_INT        : std_logic is opto_CONTROL(26);
alias  xl_RX_CLR        : std_logic is opto_CONTROL(27);
alias  xl_ID_CHECK      : std_logic is opto_CONTROL(28);
alias  xl_PIPE_RUN      : std_logic is opto_CONTROL(29);
alias  xl_OPTO_RUN      : std_logic is opto_CONTROL(30);
alias  xl_ENABLE        : std_logic is opto_CONTROL(31);

signal tx_rdy           : std_logic;
signal rx_rdy           : std_logic;
signal rx_dout          : std_logic_vector(7 downto 0);

-- BlockRAM Signals
signal opto_dout        : std_logic_vector(7 downto 0);

-- Master Read Signals
signal readdata         : std_logic_vector(31 downto 0);
signal rd_waitreq       : std_logic;
signal rd_datavalid     : std_logic;

signal rx_pend          : std_logic;
signal rx_pend_r0       : std_logic;
signal tx_req           : std_logic;
signal tx_req_r0        : std_logic;
signal dma_req          : std_logic;
signal dma_req_r0       : std_logic;
signal pipe_req         : std_logic;
signal pipe_req_r0      : std_logic;
signal rd_we            : std_logic;
signal rd_addr          : std_logic_vector(7 downto 0);
signal head_addr_i      : unsigned(15 downto 0);

signal ctl_data         : std_logic_vector(31 downto 0);
signal ft_data          : std_logic_vector(7 downto 0);
signal pipe_data        : std_logic_vector(31 downto 0);

--
-- MAIN CODE
--
begin

   --
   -- COMBINATORIAL OUTPUTS
   --
   int(0)               <= ft.tx_int and xl_TX_INT;
   int(1)               <= ft.rx_int and xl_RX_INT;

   opto_STATUS          <= opto_stat;

   cpu_DIN              <= ctl_data  when cpu_RE(0) = '1' else
                           pipe_data when cpu_RE(1) = '1' else
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
   --   THIS BRAM IS ONLY USED FOR CM MESSAGES
   --
   --   4096x8 <==> 1024x32 Dual-Port BLOCK RAM
   --   BRAM_a[7:0] <==> FT232[7:0]
   --   CPU <==> BRAM_b[31:0]
   --
   --   CPU <==> BRAM <==> FT232 <==> USB, IN/OUT TRANSFER
   --
   OPTO_4K_I : entity work.opto_4k
      port map (
         address_a      => ft.tx_bram & std_logic_vector(ft.io_ptr),
         address_b		=> cpu_ADDR(9 downto 0),
         clock		      => clk,
         data_a		   => ft.opto_din,
         data_b		   => cpu_DOUT,
         wren_a		   => ft.opto_we,
         wren_b		   => cpu_WE,
         q_a		      => opto_dout,
         q_b		      => ctl_data
      );

   --
   --   OPTO  RX/TX
   --
   TDI_RTX_I: entity work.opto_rtx
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
      rx_pend           => rx_pend,
      din               => ft.tx_din
   );

   --
   --   OPTO STATE MACHINE
   --
   process(all) begin
      if (reset_n = '0' or xl_ENABLE = '0') then

         -- Init the State Vector
         ft             <= C_FT_SV_INIT;

         -- Status is shared by RD FSM
         xl_RX_LEN      <= (others => '0');
         xl_RX_PEND     <= '0';
         xl_RX_RDY      <= '0';
         xl_TX_RDY      <= '0';
         xl_TX_MSG      <= '0';
         xl_BCAST       <= '0';

      elsif (rising_edge(clk)) then

         -- Update Status
         xl_TX_MSG      <= ft.tx_msg or ft.pipe_msg;
         xl_TX_RDY      <= tx_rdy;
         xl_RX_PEND     <= rx_pend;
         xl_RX_RDY      <= rx_rdy;

         case ft.state is

            when IDLE =>
               -- Wait for RUN Assertion
               if (xl_OPTO_RUN = '1') then
                  ft.state    <= WAIT_REQ;
               -- Flush the internal FT232H RX Fifo
               -- using software, hold rd_n = 0 for 
               -- at least 100 microseconds
               elsif (xl_FLUSH = '1') then
                  ft.state    <= IDLE;
                  ft.rx_rd    <= '1';
               else
                  ft.state    <= IDLE;
               end if;


            when WAIT_REQ =>

               -- Always set in WAIT_REQ
               ft.tx_wr       <= '0';
               ft.tx_din      <= (others => '0');
               ft.tx_int      <= '0';
               ft.rx_int      <= '0';

               -- Abort
               if (xl_OPTO_RUN = '0') then
                  ft.state    <= IDLE;
               -- Get Received Character
               elsif (rx_rdy = '1' and rx_pend = '0') then
                  ft.state    <= RX_GET;
                  ft.io_ptr   <= ft.in_ptr;
                  ft.tx_bram  <= '0';
                  ft.rx_rd    <= '1';
                  ft.rx_busy  <= '1';
               -- Start Pipe Msg Transmission if not Busy
               elsif (pipe_req = '1' and tx_rdy = '1' and ft.tx_msg = '0' and ft.rx_busy = '0') then
                  ft.state    <= PIPE_START;
                  ft.pipe_ack <= '1';
                  ft.pipe_ptr <= (others => '0');
                  ft.pipe_msg <= '1';
               -- Continue Pipe Msg Transmission if not Busy
               elsif (tx_rdy = '1' and ft.pipe_msg = '1' and ft.tx_msg = '0' and ft.rx_busy = '0') then
                  ft.state    <= PIPE_PICK;
               -- Start Control Msg Transmission
               elsif (tx_req = '1' and tx_rdy = '1' and ft.pipe_msg = '0') then
                  ft.state    <= TX_START;
                  ft.tx_ack   <= '1';
               -- Continue Control Msg until all Bytes Sent
               elsif (ft.tx_len /= 0 and tx_rdy = '1' and ft.pipe_msg = '0') then
                  ft.state    <= TX_SEND_0;
                  ft.tx_len   <= ft.tx_len - 1;
                  ft.io_ptr   <= ft.out_ptr;
                  ft.tx_bram  <= '1';
               -- Send Last Framing Flag for Control Msg
               elsif (ft.tx_len = 0 and ft.tx_msg = '1' and ft.pipe_msg = '0') then
                  ft.state    <= TX_END;
               else
                  ft.state    <= IDLE;
               end if;

            --
            -- SEND START-OF-FRAME FLAG
            --
            when TX_START =>
               ft.state       <= TX_HDR;
               ft.tx_ack      <= '0';
               ft.tx_msg      <= '1';
               ft.tx_len      <= unsigned(xl_TX_LEN(10 downto 0));
               ft.out_ptr     <= (others => '0');
               ft.tx_din      <= C_SOF;
               ft.tx_wr       <= '1';

            --
            -- SEND THE FRAME HEADER
            --
            when TX_HDR =>
               if (tx_rdy = '1') then
                  ft.state    <= IDLE;
                  ft.tx_din   <= C_TX_HDR;
                  ft.tx_wr    <= '1';
               else
                  ft.state    <= TX_HDR;
                  ft.tx_wr    <= '0';
               end if;

            --
            -- BRAM ADDRESS DELAY
            --
            when TX_SEND_0 =>
               ft.state       <= TX_SEND_1;

            when TX_SEND_1 =>
               ft.state       <= TX_PICK;

            --
            -- GET OCTET FROM 8-BIT M10K
            --
            when TX_PICK =>
               ft.state       <= TX_ESC;
               ft.out_data    <= opto_dout;

            --
            -- CHECK FOR OCTET STUFFING
            --
            when TX_ESC =>
               ft.out_ptr     <= ft.out_ptr + 1;
               if (ft.out_data = C_SOF or
                   ft.out_data = C_EOF or
                   ft.out_data = C_ESC) then
                  ft.state    <= TX_FLAG;
                  ft.out_data <= ft.out_data xor X"20";
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
                  ft.state    <= IDLE;
                  ft.tx_din   <= ft.out_data;
                  ft.tx_wr    <= '1';
               else
                  ft.state    <= TX_DATA;
                  ft.tx_wr    <= '0';
               end if;

            --
            -- SEND END-OF-FRAME FLAG
            --
            when TX_END =>
               if (tx_rdy = '1') then
                  ft.state    <= IDLE;
                  ft.tx_din   <= C_EOF;
                  ft.tx_wr    <= '1';
                  ft.tx_int   <= '1';
                  ft.tx_msg   <= '0';
               else
                  ft.state    <= TX_END;
                  ft.tx_wr    <= '0';
               end if;

            --
            -- GET BYTE FROM RECEIVER
            --
            when RX_GET =>
               ft.state       <= RX_GET_0;
               ft.rx_rd       <= '0';

            --
            -- DELAY FOR FIFO READ
            --
            when RX_GET_0 =>
               ft.state       <= RX_FRAME;
               ft.in_data     <= rx_dout;

            --
            -- CHECK FOR FRAMING
            --
            when RX_FRAME =>
               if (ft.in_data = C_SOF) then
                  ft.state    <= IDLE;
                  ft.in_ptr   <= (others => '0');
                  ft.rx_msg   <= '0';
                  ft.rx_hdr   <= '1';
                  ft.rx_esc   <= '0';
                  xl_BCAST    <= '0';
               elsif (ft.in_data = C_EOF and ft.in_ptr /= 0) then
                  ft.state    <= IDLE;
                  ft.in_ptr   <= (others => '0');
                  xl_RX_LEN   <= '0' & std_logic_vector(ft.in_ptr);
                  ft.rx_int   <= '1';
                  ft.rx_msg   <= '0';
                  ft.rx_hdr   <= '0';
                  ft.rx_esc   <= '0';
                  ft.rx_busy  <= '0';
               elsif (ft.in_data = C_EOF and ft.in_ptr = 0) then
                  ft.state    <= IDLE;
                  ft.rx_msg   <= '0';
                  ft.rx_hdr   <= '0';
                  ft.rx_esc   <= '0';
                  ft.rx_busy  <= '0';
               elsif (ft.in_data = C_ESC) then
                  ft.state    <= IDLE;
                  ft.rx_esc   <= '1';
               else
                  ft.state    <= RX_DATA;
               end if;

            --
            -- UNSTUFF OCTET
            --
            when RX_DATA =>
               -- Bypass Device ID Match
               if (ft.rx_hdr = '1' and xl_ID_CHECK = '0') then
                  ft.state    <= IDLE;
                  ft.rx_hdr   <= '0';
                  ft.rx_msg   <= '1';
                  xl_BCAST    <= '0';
               -- Check for Device ID Match
               elsif (ft.rx_hdr = '1' and ft.in_data(3 downto 0) = xl_DEV_ID) then
                  ft.state    <= IDLE;
                  ft.rx_hdr   <= '0';
                  ft.rx_msg   <= '1';
                  xl_BCAST    <= '0';
               -- Check for Broadcast Match
               elsif (ft.rx_hdr = '1' and ft.in_data(3 downto 0) = C_BCAST) then
                  ft.state    <= IDLE;
                  ft.rx_hdr   <= '0';
                  ft.rx_msg   <= '1';
                  xl_BCAST    <= '1';
               -- Device ID Mismatch
               elsif (ft.rx_hdr = '1') then
                  ft.state    <= IDLE;
                  ft.rx_hdr   <= '0';
                  ft.rx_msg   <= '0';
                  ft.rx_esc   <= '0';
                  ft.rx_busy  <= '0';
               -- Valid Message with Escape
               elsif (ft.rx_msg = '1' and ft.rx_esc = '1') then
                  ft.state    <= RX_STORE;
                  ft.opto_din <= ft.in_data xor X"20";
                  ft.rx_esc   <= '0';
               -- Valid Message w/o Escape
               elsif (ft.rx_msg = '1' and ft.rx_esc = '0') then
                  ft.state    <= RX_STORE;
                  ft.opto_din <= ft.in_data;
               -- Ignore Message
               else
                  ft.state    <= IDLE;
                  ft.rx_busy  <= '0';
               end if;

            --
            -- STORE BYTE TO M9K
            --
            when RX_STORE =>
               ft.state       <= RX_NEXT;
               ft.opto_we     <= '1';

            --
            -- INCREMENT M9K ADDRESS
            --
            when RX_NEXT =>
               ft.state       <= IDLE;
               ft.opto_we     <= '0';
               ft.in_ptr      <= ft.in_ptr + 1;

            --
            -- START PIPE MESSAGE BURST, 1024 BYTES
            --
            when PIPE_START =>
               ft.state       <= PIPE_HDR;
               ft.pipe_ack    <= '0';
               ft.tx_din      <= C_SOF;
               ft.tx_wr       <= '1';

            --
            -- SEND THE FRAME HEADER
            --
            when PIPE_HDR =>
               if (tx_rdy = '1') then
                  ft.state    <= PIPE_PICK;
                  ft.tx_din   <= C_TX_HDR;
                  ft.tx_wr    <= '1';
               else
                  ft.state    <= PIPE_HDR;
                  ft.tx_wr    <= '0';
               end if;

            --
            -- GET NEXT OCTET
            --
            when PIPE_PICK =>
               ft.tx_wr       <= '0';
               if (ft.pipe_ptr(10) = '1') then
                  ft.state    <= PIPE_END;
               else
                  ft.state    <= PIPE_ESC;
                  ft.out_data <= ft_data;
               end if;

            --
            -- CHECK FOR OCTET STUFFING
            --
            when PIPE_ESC =>
               ft.tx_wr       <= '0';
               ft.pipe_ptr    <= ft.pipe_ptr + 1;
               if (ft.out_data = C_SOF or
                   ft.out_data = C_EOF or
                   ft.out_data = C_ESC) then
                  ft.state    <= PIPE_FLAG;
                  ft.out_data <= ft.out_data xor X"20";
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
                  ft.state    <= IDLE;
                  ft.tx_din   <= ft.out_data;
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
                  ft.state    <= IDLE;
                  ft.tx_din   <= C_EOF;
                  ft.tx_wr    <= '1';
                  ft.pipe_ptr <= (others => '0');
                  ft.pipe_msg <= '0';
               else
                  ft.state    <= PIPE_END;
                  ft.tx_wr    <= '0';
               end if;

            when others =>
               ft.state       <= IDLE;

         end case;

      end if;
   end process;

   --
   --   In order to change the pipe message size the
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
   --   THIS BRAM IS ONLY USED FOR SINGLE PIPE MESSAGE
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
   rd_addr              <= std_logic_vector(rd.wrd_cnt) when cpu_RE(1) = '0' 
                           else cpu_ADDR(7 downto 0);
   rd_we                <= '1' when rd.state = RD_SLOT and rd_datavalid = '1' else '0';
   
   --
   --  MASTER READ BURST TRANSFER, ON-CHIP TO USB
   --
   --  The OPTO state machine emptys a single slot from the DMA read
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

         -- Status is shared by OPTO FSM
         xl_TAIL_ADDR   <= (others => '0');

      elsif (rising_edge(clk)) then

         -- update status
         xl_TAIL_ADDR   <= std_logic_vector(rd.tail_addr(14 downto 0));

         case rd.state is
            -- Wait for RUN Assertion
            when IDLE =>
               if (xl_PIPE_RUN = '1') then
                  rd.state    <= WAIT_REQ;
                  -- Address must be on a 32-Bit boundary
                  rd.addr     <= unsigned(opto_ADR_BEG);
                  rd.tail_addr <= (others => '0');
               else
                  rd.state    <= IDLE;
               end if;

            -- Wait for Pipe Messages to Send
            when WAIT_REQ =>
               -- Abort
               if (xl_PIPE_RUN = '0') then
                  rd.state    <= IDLE;
               -- Account for Circular Memory, Restart
               elsif (rd.addr >= unsigned(opto_ADR_END)) then
                  rd.state    <= WAIT_REQ;
                  rd.addr     <= unsigned(opto_ADR_BEG);
               -- software controlled pipe message
               elsif (dma_req = '1' and ft.pipe_msg = '0' and pipe_req = '0') then
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
                     ft.pipe_msg = '0' and pipe_req = '0') then
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
   -- CAPTURE xl_MSG_REQ RISING EDGE
   --
   process(all) begin
      if (reset_n = '0') then
         tx_req      <= '0';
         tx_req_r0   <= '0';
      elsif (rising_edge(clk)) then
         -- Double-Buffer
         tx_req_r0   <= xl_MSG_REQ;
         -- Edge Detect
         if (tx_req_r0 = '0' and xl_MSG_REQ = '1') then
            tx_req   <= '1';
         elsif (ft.tx_ack = '1') then
            tx_req   <= '0';
         else
            tx_req   <= tx_req;
         end if;
      end if;
   end process;

   --
   -- Capture ft.rx_int rising-edge
   --
   process(all) begin
      if (reset_n = '0') then
         rx_pend     <= '0';
         rx_pend_r0  <= '0';
      elsif (rising_edge(clk)) then
         -- Register for edge detect
         rx_pend_r0  <= ft.rx_int;
         -- Edge Detect
         if (rx_pend_r0 = '0' and ft.rx_int = '1') then
            rx_pend  <= '1';
         elsif (xl_RX_CLR = '1') then
            rx_pend  <= '0';
         else
            rx_pend  <= rx_pend;
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
