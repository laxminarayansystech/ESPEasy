#include "_Plugin_Helper.h"

#ifdef USES_P116

// #######################################################################################################
// #################### Plugin 116: ST7735/ST7789 TFT displays ###########################################
// #######################################################################################################


// History:
// 2021-08 tonhuisman: Refactor into AdafruitGFX_helper
// 2021-08 tonhuisman: Continue development, added new features, font scaling, display limits, extra text lines
//                     update to current ESPEasy state/style of development, make multi-instance possible
// 2020-08 tonhuisman: Adaptations for multiple ST77xx chips, ST7735s, ST7789vw (shelved temporarily)
//                     Added several features like display button, rotation
// 2020-04 WDS (Wolfdieter): initial plugin for ST7735, based on P012

# define PLUGIN_116
# define PLUGIN_ID_116         116
# define PLUGIN_NAME_116       "Display - ST7735/ST7789 TFT [DEVELOPMENT]"
# define PLUGIN_VALUENAME1_116 "TFT"

# include "src/PluginStructs/P116_data_struct.h"

boolean Plugin_116(uint8_t function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_116;
      Device[deviceCount].Type               = DEVICE_TYPE_SPI3;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_116);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_116));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output_optional(F("TFT CS"));
      event->String2 = formatGpioName_output(F("TFT DC"));
      event->String3 = formatGpioName_output_optional(F("TFT RST"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      # ifdef ESP32

      if (Settings.InitSPI == 2) { // When using ESP32 H(ardware-)SPI
        PIN(0) = P116_TFT_CS_HSPI;
      } else {
        PIN(0) = P116_TFT_CS;
      }
      # else // ifdef ESP32
      PIN(0) = P116_TFT_CS;
      # endif // ifdef ESP32
      PIN(1)                        = P116_TFT_DC;
      PIN(2)                        = P116_TFT_RST;
      P116_CONFIG_BUTTON_PIN        = -1;  // No button connected
      P116_CONFIG_BACKLIGHT_PIN     = P116_BACKLIGHT_PIN;
      P116_CONFIG_BACKLIGHT_PERCENT = 100; // Percentage backlight

      uint32_t lSettings = 0;
      # ifdef P116_USE_ADA_GRAPHICS

      // Truncate exceeding message
      set4BitToUL(lSettings, P116_CONFIG_FLAG_MODE,        static_cast<int>(AdaGFXTextPrintMode::TruncateExceedingMessage));
      # else // ifdef P116_USE_ADA_GRAPHICS
      // Truncate exceeding message
      set4BitToUL(lSettings, P116_CONFIG_FLAG_MODE,        1);
      # endif // ifdef P116_USE_ADA_GRAPHICS
      set4BitToUL(lSettings, P116_CONFIG_FLAG_FONTSCALE,   2); // Font scaling 2 as 1 is very small
      set4BitToUL(lSettings, P116_CONFIG_FLAG_CMD_TRIGGER, 1); // Default trigger on st77xx
      P116_CONFIG_FLAGS = lSettings;

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormPinSelect(formatGpioName_output_optional(F("Backlight")), F("p116_backlight"), P116_CONFIG_BACKLIGHT_PIN);
      addFormNumericBox(F("Backlight percentage"), F("p116_backpercentage"), P116_CONFIG_BACKLIGHT_PERCENT, 1, 100);
      addUnit(F("1-100%"));

      addFormPinSelect(F("Display button"), F("p116_button"), P116_CONFIG_BUTTON_PIN);

      addFormCheckBox(F("Inversed Logic"), F("p116_buttonInverse"), bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_INVERT_BUTTON)); // Bit 1

      addFormNumericBox(F("Display Timeout"), F("p116_timer"), P116_CONFIG_DISPLAY_TIMEOUT);

      # ifdef P116_USE_ADA_GRAPHICS
      AdaGFXFormTextPrintMode(F("p116_mode"), P116_CONFIG_FLAG_GET_MODE);
      # else // ifdef P116_USE_ADA_GRAPHICS
      {
        const __FlashStringHelper *options3[] = {
          F("Continue to next line"),
          F("Truncate exceeding message"),
          F("Clear then truncate exceeding message") };
        const int optionValues3[] = { 0, 1, 2 };
        addFormSelector(F("Text print Mode"), F("p116_mode"), 3, options3, optionValues3, P116_CONFIG_FLAG_GET_MODE);
      }
      # endif // ifdef P116_USE_ADA_GRAPHICS

      {
        const __FlashStringHelper *options4[] = {
          ST77xx_type_toString(ST77xx_type_e::ST7735s_128x128),
          ST77xx_type_toString(ST77xx_type_e::ST7735s_128x160),
          ST77xx_type_toString(ST77xx_type_e::ST7735s_80x160),
          ST77xx_type_toString(ST77xx_type_e::ST7789vw_240x320),
          ST77xx_type_toString(ST77xx_type_e::ST7789vw_240x240),
          ST77xx_type_toString(ST77xx_type_e::ST7789vw_240x280) };
        const int optionValues4[] = {
          static_cast<int>(ST77xx_type_e::ST7735s_128x128),
          static_cast<int>(ST77xx_type_e::ST7735s_128x160),
          static_cast<int>(ST77xx_type_e::ST7735s_80x160),
          static_cast<int>(ST77xx_type_e::ST7789vw_240x320),
          static_cast<int>(ST77xx_type_e::ST7789vw_240x240),
          static_cast<int>(ST77xx_type_e::ST7789vw_240x280) };
        addFormSelector(F("TFT display model"), F("p116_type"), 6, options4, optionValues4, P116_CONFIG_FLAG_GET_TYPE);
      }

      const __FlashStringHelper *options5[] = { F("Normal"), F("+90&deg;"), F("+180&deg;"), F("+270&deg;") };
      const int optionValues5[]             = { 0, 1, 2, 3 };
      addFormSelector(F("Rotation"), F("p116_rotate"), 4, options5, optionValues5, P116_CONFIG_FLAG_GET_ROTATION);

      addFormNumericBox(F("Font scaling"), F("p116_fontscale"), P116_CONFIG_FLAG_GET_FONTSCALE, 1, 10);
      addUnit(F("1x..10x"));

      addFormCheckBox(F("Clear display on exit"), F("p116_clearOnExit"), bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_CLEAR_ON_EXIT));

      {
        const __FlashStringHelper *commandTriggers[] = {
          P116_CommandTrigger_toString(P116_CommandTrigger::tft),
          P116_CommandTrigger_toString(P116_CommandTrigger::st77xx),
          P116_CommandTrigger_toString(P116_CommandTrigger::st7735),
          P116_CommandTrigger_toString(P116_CommandTrigger::st7789)
        };
        const int commandTriggerOptions[] = {
          static_cast<int>(P116_CommandTrigger::tft),
          static_cast<int>(P116_CommandTrigger::st77xx),
          static_cast<int>(P116_CommandTrigger::st7735),
          static_cast<int>(P116_CommandTrigger::st7789)
        };
        addFormSelector(F("Write Command trigger"),
                        F("p116_commandtrigger"),
                        4,
                        commandTriggers,
                        commandTriggerOptions,
                        P116_CONFIG_FLAG_GET_CMD_TRIGGER);
        addFormNote(F("Select the command that is used to handle commands for this display."));
      }

      # ifdef P116_USE_ADA_GRAPHICS
      addFormCheckBox(F("Text Coordinates in col/row"), F("p116_colrow"), bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_USE_COL_ROW));
      addUnit(F("Default in pixels."));
      # endif // ifdef P116_USE_ADA_GRAPHICS

      addFormSubHeader(F("Content"));

      // Inverted state!
      addFormCheckBox(F("Wake display on receiving text"), F("p116_NoDisplay"), !bitRead(P116_CONFIG_FLAGS, P116_CONFIG_FLAG_NO_WAKE));
      addFormNote(F("When checked, the display wakes up at receiving remote updates."));

      String strings[P116_Nlines];
      LoadCustomTaskSettings(event->TaskIndex, strings, P116_Nlines, 0);

      String line; // Default reserved length is plenty

      for (uint8_t varNr = 0; varNr < P116_Nlines; varNr++) {
        line  = F("Line ");
        line += (varNr + 1);
        addFormTextBox(line, getPluginCustomArgName(varNr), strings[varNr], P116_Nchars);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P116_CONFIG_BUTTON_PIN        = getFormItemInt(F("p116_button"));
      P116_CONFIG_DISPLAY_TIMEOUT   = getFormItemInt(F("p116_timer"));
      P116_CONFIG_BACKLIGHT_PIN     = getFormItemInt(F("p116_backlight"));
      P116_CONFIG_BACKLIGHT_PERCENT = getFormItemInt(F("p116_backpercentage"));

      uint32_t lSettings = 0;
      bitWrite(lSettings, P116_CONFIG_FLAG_NO_WAKE,       !isFormItemChecked(F("p116_NoDisplay")));    // Bit 0 NoDisplayOnReceivingText,
                                                                                                       // reverse logic, default=checked!
      bitWrite(lSettings, P116_CONFIG_FLAG_INVERT_BUTTON, isFormItemChecked(F("p116_buttonInverse"))); // Bit 1 buttonInverse
      bitWrite(lSettings, P116_CONFIG_FLAG_CLEAR_ON_EXIT, isFormItemChecked(F("p116_clearOnExit")));   // Bit 2 ClearOnExit
      # ifdef P116_USE_ADA_GRAPHICS
      bitWrite(lSettings, P116_CONFIG_FLAG_USE_COL_ROW,   isFormItemChecked(F("p116_colrow")));        // Bit 3 Col/Row addressing
      # endif // ifdef P116_USE_ADA_GRAPHICS
      set4BitToUL(lSettings, P116_CONFIG_FLAG_MODE,        getFormItemInt(F("p116_mode")));            // Bit 4..7 Text print mode
      set4BitToUL(lSettings, P116_CONFIG_FLAG_ROTATION,    getFormItemInt(F("p116_rotate")));          // Bit 8..11 Rotation
      set4BitToUL(lSettings, P116_CONFIG_FLAG_FONTSCALE,   getFormItemInt(F("p116_fontscale")));       // Bit 12..15 Font scale
      set4BitToUL(lSettings, P116_CONFIG_FLAG_TYPE,        getFormItemInt(F("p116_type")));            // Bit 16..19 Hardwaretype
      set4BitToUL(lSettings, P116_CONFIG_FLAG_CMD_TRIGGER, getFormItemInt(F("p116_commandtrigger")));  // Bit 20..23 Command trigger
      P116_CONFIG_FLAGS = lSettings;

      String strings[P116_Nlines];
      String error;

      for (uint8_t varNr = 0; varNr < P116_Nlines; varNr++) {
        strings[varNr] = web_server.arg(getPluginCustomArgName(varNr));
      }

      error = SaveCustomTaskSettings(event->TaskIndex, strings, P116_Nlines, 0);

      if (error.length() > 0) {
        addHtmlError(error);
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (Settings.InitSPI != 0) {
        initPluginTaskData(event->TaskIndex,
                           new (std::nothrow) P116_data_struct(static_cast<ST77xx_type_e>(P116_CONFIG_FLAG_GET_TYPE),
                                                               P116_CONFIG_FLAG_GET_ROTATION,
                                                               P116_CONFIG_FLAG_GET_FONTSCALE,
                                                               static_cast<AdaGFXTextPrintMode>(P116_CONFIG_FLAG_GET_MODE),
                                                               P116_CONFIG_DISPLAY_TIMEOUT,
                                                               P116_CommandTrigger_toString(static_cast<P116_CommandTrigger>(
                                                                                              P116_CONFIG_FLAG_GET_CMD_TRIGGER))));
        P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P116_data) {
          success = P116_data->plugin_init(event); // Start the display
        }
      } else {
        addLog(LOG_LEVEL_ERROR, F("ST77xx: SPI not enabled, init cancelled."));
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P116_data) {
        success = P116_data->plugin_exit(event); // Stop the display
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P116_data) {
        success = P116_data->plugin_ten_per_second(event); // 10 per second actions
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P116_data) {
        success = P116_data->plugin_once_a_second(event); // Once a second actions
      }
      break;
    }

    case PLUGIN_READ:
    {
      P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P116_data) {
        success = P116_data->plugin_read(event); // Read operation, redisplay the configured content
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P116_data_struct *P116_data = static_cast<P116_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P116_data) {
        success = P116_data->plugin_write(event, string); // Write operation, handle commands, mostly delegated to AdafruitGFX_helper
      }
      break;
    }
  }
  return success;
}

#endif // USES_P116
