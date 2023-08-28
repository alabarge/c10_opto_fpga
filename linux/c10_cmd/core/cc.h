#pragma once

   static cc_entry_t cc_table[] = {
      //
      // Variable Text Name      Default Value           Data Type      Address to Variable        Dim
      // ==================      =============           =========      ===================        =====
      //
      { "gc.debug",              "0x00000000",           CC_HEX,        &gc.debug,                 1 },
      { "gc.trace",              "0x00000000",           CC_HEX,        &gc.trace,                 1 },
      { "gc.feature",            "0x00000000",           CC_HEX,        &gc.feature,               1 },
      { "opc.opcode",            "0",                    CC_INT,        &cc.opc_opcode,            1 },
      { "opc.timeout",           "5000",                 CC_UINT,       &cc.opc_timeout,           1 },
      { "opc.comport",           "0",                    CC_UINT,       &cc.opc_comport,           1 },
      { "opc.mac_addr_hi",       "0x0002C94E",           CC_HEX,        &cc.opc_mac_addr_hi,       1 },
      { "opc.mac_addr_lo",       "0x7FC80000",           CC_HEX,        &cc.opc_mac_addr_lo,       1 },
      { "opc.ip_addr",           "0xC0A8013C",           CC_HEX,        &cc.opc_ip_addr,           1 },
      { "opc.cm_udp_port",       "0x00000ADD",           CC_HEX,        &cc.opc_cm_udp_port,       1 },
      { "daq.opcmd",             "0x00000000",           CC_HEX,        &cc.daq_opcmd,             1 },
      { "daq.file",              "daq_data.csv",         CC_STR,        &cc.daq_file,              1 },
      { "daq.packets",           "32",                   CC_UINT,       &cc.daq_packets,           1 },
      { "daq.to_file",           "0",                    CC_UINT,       &cc.daq_to_file,           1 },
      { "daq.file_type",         "0",                    CC_UINT,       &cc.daq_file_type,         1 },
      { "daq.file_stamp",        "0",                    CC_UINT,       &cc.daq_file_stamp,        1 },
      { "daq.real",              "0",                    CC_UINT,       &cc.daq_real,              1 },
      { "daq.ramp",              "0",                    CC_UINT,       &cc.daq_ramp,              1 },
   };
