----  PCK_FIO: a VHDL package for C-style formatted file output
----  Copyright (C) 1995, 2001 Easics NV
----
----  This library is free software; you can redistribute it and/or
----  modify it under the terms of the GNU Lesser General Public
----  License as published by the Free Software Foundation; either
----  version 2.1 of the License, or (at your option) any later version.
----
----  This library is distributed in the hope that it will be useful,
----  but WITHOUT ANY WARRANTY; without even the implied warranty of
----  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
----  Lesser General Public License for more details.
----
----  You should have received a copy of the GNU Lesser General Public
----  License along with this library; if not, write to the Free Software
----  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
----
----  For suggestions, bug reports, enhancement requests, and info about
----  our design services, you can contact us at the following address:
----
----     Easics NV, http://www.easics.com
----     tel.: +32 16 395 611
----     e-mail: info@easics.be
----

library IEEE;
use IEEE.std_logic_1164.all;
-- signed/unsigned definition: use either std_logic_arith or numeric_std
-- use IEEE.std_logic_arith.all; -- the Synopsys one
use IEEE.numeric_std.all;

use STD.TEXTIO.all;

library EASICS_PACKAGES;
use EASICS_PACKAGES.PCK_FIO.all;

entity TBE_PCK_FIO is

end TBE_PCK_FIO;

architecture TB of TBE_PCK_FIO is

begin

USAGE: process

  file RESULT:        text open write_mode is "PCK_FIO.out";
  variable L:         line;
  variable Pointer:   integer;
  variable BoolTrue:  boolean := TRUE;
  variable BoolFalse: boolean := FALSE;
  variable IntPos:    integer := 5678;
  variable BigIntPos: integer := 12345678;
  variable IntNeg:    integer := -8765;
  variable BigIntNeg: integer := -87654321;
  variable Uns:       unsigned(6 downto 0) := unsigned'("1010010");
  variable Unk:       unsigned(6 downto 0) := unsigned'("1X10Z10");
  variable Std:       std_logic_vector(6 downto 0) := std_logic_vector'("0010010");
  variable SigPos:    signed(6 downto 0) := signed'("0010010");
  variable SigNeg:    signed(6 downto 0) := signed'("1010010");
  variable BitX:      std_logic := 'X';
  variable Bit1:      std_logic := '1';
  variable Bit0:      std_logic := '0';

  variable parameter_1: integer := 1234;
  variable parameter_2: integer := 2345;
  variable parameter_3: integer := 4567;

  constant TempString: string :="ABC" & NUL & "DEF";
begin

fprint(RESULT, L,
       "PCK_FIO.out: TESTBENCH OUTPUT FOR PCK_FIO\n" &
       "-----------------------------------------\n\n" &
       "This is the result of running the PCK_FIO testbench.\n" &
       "This file should be compared with PCK_FIO.out.gold, which is\n" &
       "the reference output. Any difference indicates a portability\n" &
       "issue or a simulator non-compliance or bug.\n\n"
      );


fprint(RESULT, L,
       "\n--------------------\n" &
         "-- Basic features --\n" &
         "--------------------\n\n"
      );

fprint(RESULT, L,
         "-- First some auxiliary declarations\n"
      );

fprint(RESULT, L,
  "> file     RESULT:     text is out ""PCK_FIO.out"";\n" &
  "> variable L:          line;\n" &
  "> variable BoolTrue:   boolean := TRUE;\n" &
  "> variable BoolFalse:  boolean := FALSE;\n" &
  "> variable IntPos:     integer := 5678;\n" &
  "> variable BigIntPos:  integer := 12345678;\n" &
  "> variable IntNeg:     integer := -8765;\n" &
  "> variable BigIntNeg:  integer := -87654321;\n" &
  "> variable Uns:        unsigned(6 downto 0) := unsigned'(""1010010"");\n" &
  "> variable Unk:        unsigned(6 downto 0) := unsigned'(""1X10Z10"");\n" &
  "> variable Std:        std_logic_vector(6 downto 0) := std_logic_vector'(""0010010"");\n" &
  "> variable SigPos:     signed(6 downto 0) := signed'(""0010010"");\n" &
  "> variable SigNeg:     signed(6 downto 0) := signed'(""1010010"");\n" &
  "> variable BitX:       std_logic := 'X';\n" &
  "> variable Bit1:       std_logic := '1';\n" &
  "> variable Bit0:       std_logic := '0';\n" &
  "> constant TempString: string := ""ABC"" & NUL & ""DEF"";\n"
);


fprint(RESULT, L,
       "\n\n-- fprint call with no arguments\n" &
       "> fprint(RESULT, L, ""No arguments.\\n"");\n"
      );
fprint(RESULT, L, "No arguments.\n");


fprint(RESULT, L,
       "\n-- Special characters need to be escaped\n" &
       "> fprint(RESULT, L, ""Special characters: \\\% \\\\ "" \\n"");\n"
      );

fprint(RESULT, L, "Special characters: \% \\ "" \n");


fprint(RESULT, L,
       "\n-- Various format specifiers interprete the same object differently\n" &
       "> fprint(RESULT, L, ""Format specifiers: \%r \%d \%b \%q\\n"",\n" &
       ">                   fo(Uns), fo(Uns), fo(Uns), fo(Uns)\n" &
       ">       );\n"
      );

fprint(RESULT, L, "Format specifiers: %r %d %b %q\n",
                  fo(Uns), fo(Uns), fo(Uns), fo(Uns)
      );

fprint(RESULT, L,
       "\n-- Format string can be shortened by using the iteration specifier\n" &
       "> fprint(RESULT, L, ""Iteration specifier: \%4{\%r }\\n"",\n" &
       ">                   fo(Uns), fo(SigPos), fo(SigNeg), fo(Std)\n" &
       ">       );\n"
      );

fprint(RESULT, L, "Iteration specifier: %4{%r }\n",
                  fo(Uns), fo(SigPos), fo(SigNeg), fo(Std)
      );
fprint(RESULT, L,
       "\n-------------------------------\n"&
       "--Testing string manipulation--\n"&
       "-------------------------------\n");
fprint(RESULT, L, "--testing done using the string TempString : " & """ABC"""
                    & " & NUL & ""DEF""\n");
-- TempString: string:="ABC" & NUL & "DEF"
-- test for detecting NUL in string
fprint(RESULT, L ,"\n--test for NUL ending in strings\n");
fprint(RESULT, L, "> fprint(RESULT, L, fo(TempString));\n");
fprint(RESULT, L, fo(TempString));

-- direct inclusion of string in fprint should give whole string
fprint(RESULT, L, "\n--same without fo,so without NUL testing\n");
fprint(RESULT, L, "> fprint(RESULT, L, TempString);\n");
fprint(RESULT, L, TempString & "\n");

-- influence of given width

fprint(RESULT, L, "\n--if there are less characters in the given string then in the width fo will\n"&
               "-- pad the remaining with blanco's according to the given justification\n");
fprint(RESULT, L, "\n--with specified width, justified right,using fo\n");
fprint(RESULT, L, "> fprint(RESULT, L, ""\%5s"", fo(TempString));\n");
fprint(RESULT, L, "%5s", fo(TempString));
fprint(RESULT, L, "\n--with specified width, justified left, using fo\n");
fprint(RESULT, L, "> fprint(RESULT, L, ""\%-5s"", fo(TempString));\n");
fprint(RESULT, L, "%-5s", fo(TempString));
fprint(RESULT, L, "\n--with specified width without fo\n");
fprint(RESUlT, L, "> fprint(RESULT, L, ""\%9s"", TempString);\n");
fprint(RESULT, L, "%9s", TempString);
fprint(RESULT, L, "\n\n-- if the width is smaller then the number of characters in the string fprint\n"&
                "-- will output the string with the minimum of characters needed\n");
fprint(RESULT, L, "\n--with specified width,using fo\n");
fprint(RESULT, L, "> fprint(RESULT, L, ""\%2s"", fo(TempString));\n");
fprint(RESULT, L, "%2s", fo(TempString));
fprint(RESULT, L, "\n--with specified width without fo\n");
fprint(RESUlT, L, "> fprint(RESULT, L, ""\%4s"", TempString);\n");
fprint(RESULT, L, "%4s", TempString);

fprint(RESULT, L,
       "\n\n------------------------------------\n" &
         "-- Support for bit-oriented types --\n" &
         "------------------------------------\n"
      );

fprint (RESULT, L, "\n-- Reasonable format\n" &
                   "> fprint(RESULT, L, ""Reasonable format: \%9{\%r }\\n"",\n" &
                   ">                   fo(Std), fo(Uns), fo(SigPos), fo(SigNeg), fo(Unk),\n" &
                   ">                   fo(Bit1), fo(BitX), fo(BoolTrue), fo(BoolFalse) \n" &
                   ">       );\n"
       );


fprint(RESULT, L, "Reasonable format: %9{%r }\n",
                  fo(Std), fo(Uns), fo(SigPos), fo(SigNeg), fo(Unk),
                  fo(Bit1), fo(BitX), fo(BoolTrue), fo(BoolFalse)
      );

fprint (RESULT, L, "\n-- Decimal format\n" &
                   "> fprint(RESULT, L, ""Decimal format: \%7{\%d }\\n"",\n" &
                   ">                   fo(Std), fo(Uns), fo(SigPos), fo(SigNeg), fo(Unk),\n" &
                   ">                   fo(Bit1), fo(BitX), fo(BoolTrue), fo(BoolFalse) \n" &
                   ">       );\n"
       );


fprint(RESULT, L, "Decimal format: %9{%d }\n",
                  fo(Std), fo(Uns), fo(SigPos), fo(SigNeg), fo(Unk),
                  fo(Bit1), fo(BitX), fo(BoolTrue), fo(BoolFalse)
      );

fprint (RESULT, L, "\n-- Bit format\n" &
                   "> fprint(RESULT, L, ""Bit format: \%7{\%b }\\n"",\n" &
                   ">                   fo(Std), fo(Uns), fo(SigPos), fo(SigNeg), fo(Unk),\n" &
                   ">                   fo(Bit1), fo(BitX), fo(BoolTrue), fo(BoolFalse) \n" &
                   ">       );\n"
       );


fprint(RESULT, L, "Bit format: %9{%b }\n",
                  fo(Std), fo(Uns), fo(SigPos), fo(SigNeg), fo(Unk),
                  fo(Bit1), fo(BitX), fo(BoolTrue), fo(BoolFalse)
      );

fprint(RESULT, L,
       "\n\n" &
         "-- Alignment\n-- Template of the call: (<fs> stands for the format specifier)\n"
      );


fprint(RESULT, L, "\n" &
                 "> fprint(RESULT, L, ""Bitvectors with <fs>:    \%4{<fs>|}\\n"",\n" &
                 ">                  fo(Std), fo(Uns), fo(SigPos), fo(SigNeg)\n" &
                 ">       );\n"
      );

fprint(RESULT, L,
       "\n" &
         "-- Results of the call with various format specifiers\n\n"
      );

fprint(RESULT, L, "Bitvectors with \%1r:  %4{%1r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%2r:  %4{%2r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%3r:  %4{%3r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%4r:  %4{%4r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%5r:  %4{%5r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%6r:  %4{%6r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%7r:  %4{%7r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%8r:  %4{%8r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%9r:  %4{%9r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "\n");
fprint(RESULT, L, "Bitvectors with \%-1r:  %4{%-1r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-2r:  %4{%-2r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-3r:  %4{%-3r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-4r:  %4{%-4r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-5r:  %4{%-5r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-6r:  %4{%-6r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-7r:  %4{%-7r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-8r:  %4{%-8r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-9r:  %4{%-9r|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "\n");
fprint(RESULT, L, "Bitvectors with \%1d:  %4{%1d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%2d:  %4{%2d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%3d:  %4{%3d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%4d:  %4{%4d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%5d:  %4{%5d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%6d:  %4{%6d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%7d:  %4{%7d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%8d:  %4{%8d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%9d:  %4{%9d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "\n");
fprint(RESULT, L, "Bitvectors with \%-1d:  %4{%-1d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-2d:  %4{%-2d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-3d:  %4{%-3d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-4d:  %4{%-4d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-5d:  %4{%-5d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-6d:  %4{%-6d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-7d:  %4{%-7d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-8d:  %4{%-8d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "Bitvectors with \%-9d:  %4{%-9d|} \n",
                   fo(Uns), fo(Std), fo(SigPos), fo(SigNeg)
       );
fprint(RESULT, L, "\n");



fprint(RESULT, L,
       "\n-------------------------------\n" &
         "-- Support for integer types --\n" &
         "-------------------------------\n"
      );

fprint(RESULT, L,
       "\n\n" &
         "-- Template of the call: (<fs> stands for the format specifier)\n"
      );


fprint(RESULT, L, "\n" &
                 "> fprint(RESULT, L, ""Integers with <fs>:    \%4{<fs>|}\\n"",\n" &
                 ">                  fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)\n" &

                 ">       );\n"
      );

fprint(RESULT, L,
       "\n" &
         "-- Results of the call with various format specifiers\n\n"
      );


fprint(RESULT, L, "Integers with \%5d:    %4{%5d|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%6d:    %4{%6d|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%7d:    %4{%7d|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%8d:    %4{%8d|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%9d:    %4{%9d|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%10d:   %4{%10d|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "\n");
fprint(RESULT, L, "Integers with \%-5d:    %4{%-5d|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%-6d:    %4{%-6d|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%-7d:    %4{%-7d|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%-8d:    %4{%-8d|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%-9d:    %4{%-9d|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%-10d:   %4{%-10d|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "\n");
fprint(RESULT, L, "Integers with \%5r:    %4{%5r|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%6r:    %4{%6r|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%7r:    %4{%7r|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%8r:    %4{%8r|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%9r:    %4{%9r|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%10r:   %4{%10r|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "\n");
fprint(RESULT, L, "Integers with \%-5r:    %4{%-5r|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%-6r:    %4{%-6r|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%-7r:    %4{%-7r|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%-8r:    %4{%-8r|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%-9r:    %4{%-9r|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "Integers with \%-10r:   %4{%-10r|} \n",
                   fo(IntPos), fo(IntNeg), fo(BigIntPos), fo(BigIntNeg)
      );
fprint(RESULT, L, "\n");


fprint(RESULT, L,
       "\n----------------------\n" &
         "-- Support for time --\n" &
         "----------------------\n"
      );

fprint (RESULT, L, "-- For portability reasons, time is converted \n" &
                   "-- into an integer value (of nanoseconds)\n"
       );
fprint (RESULT, L, "> wait for 648 ns\n");
fprint (RESULT, L, "> fprint (RESULT, L, ""The time is now \%d ns\\n"", fo(now));\n");

wait for 648 ns;

fprint (RESULT, L, "The time is now %d ns\n", fo(now));


fprint(RESULT, L,
       "\n----------------------------------------\n" &
         "-- Erroneous calls and error messages --\n" &
         "----------------------------------------\n"
      );

fprint(RESULT, L, "\n-- Less arguments than formats specifiers\n" &
                  "> fprint(RESULT, L, ""\%d \%d \%d \%d \\n"", fo(Uns), fo(Std));\n"
      );

fprint(RESULT, L, "%d %d %d %d \n", fo(Uns), fo(Std));

fprint(RESULT, L, "\n");

fprint(RESULT, L, "-- Unsupported format specifier\n" &
                  "> fprint(RESULT, L, ""\%r \%v \%r \%r\\n"", " &
                  "fo(Uns), fo(Std), fo(Uns), fo(Std));\n"
      );

fprint(RESULT, L, "%r %v %r %r\n", fo(Uns), fo(Std), fo(Uns), fo(Std));

fprint(RESULT, L, "\n");

fprint(RESULT, L,
       "\n----------------------------------------\n" &
         "-- Things you wouldn't expect to work --\n" &
         "----------------------------------------\n"
      );

fprint(RESULT, L, "\n-- Nested iteration\n");
fprint(RESULT, L, "> fprint(RESULT, L, ""\%3{<<\%3{_END_}>>}\\n"");\n");
fprint(RESULT, L, "%3{<<%3{_END_}>>}\n");

wait;

end process USAGE;

end TB;

