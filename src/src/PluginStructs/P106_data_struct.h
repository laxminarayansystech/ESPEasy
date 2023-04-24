#ifndef PLUGINSTRUCTS_P106_DATA_STRUCT_H
#define PLUGINSTRUCTS_P106_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P106

# define P106_I2C_ADDRESS           PCONFIG(0)
# define P106_ALTITUDE              PCONFIG(1)
# define P106_OPTIONS               PCONFIG_ULONG(0) // Plugin options
// Plugin options:
# define P106_OPT_GAS_OHM              0          // Use Raw value, default off

# define P106_GET_OPT_GAS_OHM    bitRead(P106_OPTIONS, P106_OPT_GAS_OHM)
# define P106_SET_OPT_GAS_OHM(x) bitWrite(P106_OPTIONS, P106_OPT_GAS_OHM, x)

# include <Adafruit_Sensor.h>
# include <Adafruit_BME680.h>

struct P106_data_struct : public PluginTaskData_base {
  P106_data_struct()          = default;
  virtual ~P106_data_struct() = default;


  bool begin(uint8_t addr,
             bool    initSettings = true);

  Adafruit_BME680 bme; // I2C
  bool            initialized = false;
};

#endif // ifdef USES_P106
#endif // ifndef PLUGINSTRUCTS_P106_DATA_STRUCT_H
