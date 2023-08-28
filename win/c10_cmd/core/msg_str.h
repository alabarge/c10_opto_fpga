#pragma once

   static msg_entry_t msg_table[] = {
      //
      // CMID                 MSG ID                  CMID STR          MSGID STR
      // ============         ============            ============      ============
      //

      { CM_ID_CP_SRV,         CP_NULL_MSG,            "CP_SRV",         "NULL_MSG",             },
      { CM_ID_CP_SRV,         CP_VER_REQ,             "CP_SRV",         "VER_REQ",              },
      { CM_ID_CP_SRV,         CP_VER_RESP,            "CP_SRV",         "VER_RESP",             },
      { CM_ID_CP_SRV,         CP_MEM_REQ,             "CP_SRV",         "MEM_REQ",              },
      { CM_ID_CP_SRV,         CP_MEM_RESP,            "CP_SRV",         "MEM_RESP",             },
      { CM_ID_CP_SRV,         CP_TRACE_REQ,           "CP_SRV",         "TRACE_REQ",            },
      { CM_ID_CP_SRV,         CP_TRACE_RESP,          "CP_SRV",         "TRACE_RESP",           },
      { CM_ID_CP_SRV,         CP_RESET_REQ,           "CP_SRV",         "RESET_REQ",            },
      { CM_ID_CP_SRV,         CP_RESET_RESP,          "CP_SRV",         "RESET_RESP",           },
      { CM_ID_CP_SRV,         CP_BLOCK_REQ,           "CP_SRV",         "BLOCK_REQ",            },
      { CM_ID_CP_SRV,         CP_BLOCK_RESP,          "CP_SRV",         "BLOCK_RESP",           },
      { CM_ID_CP_SRV,         CP_XL345_REQ,           "CP_SRV",         "XL345_REQ",            },
      { CM_ID_CP_SRV,         CP_XL345_RESP,          "CP_SRV",         "XL345_RESP",           },
      { CM_ID_CP_SRV,         CP_EEPROM_REQ,          "CP_SRV",         "EEPROM_REQ",           },
      { CM_ID_CP_SRV,         CP_EEPROM_RESP,         "CP_SRV",         "EEPROM_RESP",          },
      { CM_ID_CP_SRV,         CP_RPC_REQ,             "CP_SRV",         "RPC_REQ",              },
      { CM_ID_CP_SRV,         CP_RPC_RESP,            "CP_SRV",         "RPC_RESP",             },
      { CM_ID_CP_SRV,         CP_STREAM_REQ,          "CP_SRV",         "STREAM_REQ",           },
      { CM_ID_CP_SRV,         CP_STREAM_RESP,         "CP_SRV",         "STREAM_RESP",          },
      { CM_ID_CP_SRV,         CP_PING_REQ,            "CP_SRV",         "PING_REQ",             },
      { CM_ID_CP_SRV,         CP_PING_RESP,           "CP_SRV",         "PING_RESP",            },
      { CM_ID_CP_SRV,         CP_ERROR_REQ,           "CP_SRV",         "ERROR_REQ",            },
      { CM_ID_CP_SRV,         CP_ERROR_RESP,          "CP_SRV",         "ERROR_RESP",           },
      { CM_ID_CP_SRV,         CP_INT_IND,             "CP_SRV",         "INT_IND",              },
      { CM_ID_CP_SRV,         CP_STATUS_IND,          "CP_SRV",         "STATUS_IND",           },
      { CM_ID_CP_SRV,         CP_XL345_IND,           "CP_SRV",         "XL345_IND",            },
      { CM_ID_CP_SRV,         CP_PING_IND,            "CP_SRV",         "PING_IND",             },

      { CM_ID_DAQ_SRV,        DAQ_NULL_MSG,           "DAQ_SRV",        "NULL_MSG",             },
      { CM_ID_DAQ_SRV,        DAQ_RUN_REQ,            "DAQ_SRV",        "RUN_REQ",              },
      { CM_ID_DAQ_SRV,        DAQ_RUN_RESP,           "DAQ_SRV",        "RUN_RESP",             },
      { CM_ID_DAQ_SRV,        DAQ_DATA_REQ,           "DAQ_SRV",        "DATA_REQ",             },
      { CM_ID_DAQ_SRV,        DAQ_DATA_RESP,          "DAQ_SRV",        "DATA_RESP",            },
      { CM_ID_DAQ_SRV,        DAQ_ERROR_REQ,          "DAQ_SRV",        "ERROR_REQ",            },
      { CM_ID_DAQ_SRV,        DAQ_ERROR_RESP,         "DAQ_SRV",        "ERROR_RESP",           },
      { CM_ID_DAQ_SRV,        DAQ_INT_IND,            "DAQ_SRV",        "INT_IND",              },
      { CM_ID_DAQ_SRV,        DAQ_PKT_IND,            "DAQ_SRV",        "PKT_IND",              },
      { CM_ID_DAQ_SRV,        DAQ_DONE_IND,           "DAQ_SRV",        "DONE_IND",             },

      { CM_ID_OPC_SRV,        OPC_NULL_MSG,           "OPC_SRV",        "NULL_MSG",             },
      { CM_ID_OPC_SRV,        OPC_RUN_REQ,            "OPC_SRV",        "RUN_REQ",              },
      { CM_ID_OPC_SRV,        OPC_RUN_RESP,           "OPC_SRV",        "RUN_RESP",             },
      { CM_ID_OPC_SRV,        OPC_ERROR_REQ,          "OPC_SRV",        "ERROR_REQ",            },
      { CM_ID_OPC_SRV,        OPC_ERROR_RESP,         "OPC_SRV",        "ERROR_RESP",           },
      { CM_ID_OPC_SRV,        OPC_INT_IND,            "OPC_SRV",        "INT_IND",              },
      { CM_ID_OPC_SRV,        OPC_RUN_IND,            "OPC_SRV",        "RUN_IND",              },
      { CM_ID_OPC_SRV,        OPC_STEP_IND,           "OPC_SRV",        "STEP_IND",             },
      { CM_ID_OPC_SRV,        OPC_TMR_APP_TIMEOUT,    "OPC_SRV",        "TMR_APP_TIMEOUT",      },

      { CM_ID_INSTANCE,       CM_NULL_MSG,            "CM_INST",        "NULL_MSG",             },
      { CM_ID_INSTANCE,       CM_REG_REQ,             "CM_INST",        "REG_REQ",              },
      { CM_ID_INSTANCE,       CM_REG_RESP,            "CM_INST",        "REG_RESP",             },
      { CM_ID_INSTANCE,       CM_RPC_REQ,             "CM_INST",        "RPC_REQ",              },
      { CM_ID_INSTANCE,       CM_RPC_RESP,            "CM_INST",        "RPC_RESP",             },
      { CM_ID_INSTANCE,       CM_ECHO_REQ,            "CM_INST",        "ECHO_REQ",             },
      { CM_ID_INSTANCE,       CM_ECHO_RESP,           "CM_INST",        "ECHO_RESP",            },
      { CM_ID_INSTANCE,       CM_STATUS_REQ,          "CM_INST",        "STATUS_REQ",           },
      { CM_ID_INSTANCE,       CM_STATUS_RESP,         "CM_INST",        "STATUS_RESP",          },
      { CM_ID_INSTANCE,       CM_QUERY_REQ,           "CM_INST",        "QUERY_REQ",            },
      { CM_ID_INSTANCE,       CM_QUERY_RESP,          "CM_INST",        "QUERY_RESP",           },
      { CM_ID_INSTANCE,       CM_ERROR_REQ,           "CM_INST",        "ERROR_REQ",            },
      { CM_ID_INSTANCE,       CM_ERROR_RESP,          "CM_INST",        "ERROR_RESP",           },
      { CM_ID_INSTANCE,       CM_START_IND,           "CM_INST",        "START_IND",            },
      { CM_ID_INSTANCE,       CM_REG_IND,             "CM_INST",        "REG_IND",              },
      { CM_ID_INSTANCE,       CM_PING_IND,            "CM_INST",        "PING_IND",             },

      { CM_ID_CP_CLI,         CP_TMR_PING,            "CP_CLI",         "TMR_PING",             },
      { CM_ID_CP_CLI,         CP_TMR_PING_TIMEOUT,    "CP_CLI",         "TMR_PING_TIMEOUT",     },

      { CM_ID_PIPE,           CM_PIPE_NULL,           "CM_PIPE",        "PIPE_NULL",            },
      { CM_ID_PIPE,           CM_PIPE_DAQ_DATA,       "CM_PIPE",        "PIPE_DAQ_DATA",        },

   };

