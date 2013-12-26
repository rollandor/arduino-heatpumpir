#include <Arduino.h>
#include <CarrierHeatpumpIR.h>

CarrierHeatpumpIR::CarrierHeatpumpIR()
{
}

void CarrierHeatpumpIR::send(IRSender& IR, byte powerModeCmd, byte operatingModeCmd, byte fanSpeedCmd, byte temperatureCmd, byte swingVCmd, byte swingHCmd)
{
  // Sensible defaults for the heat pump mode

  byte operatingMode = CARRIER_AIRCON1_MODE_HEAT;
  byte fanSpeed = CARRIER_AIRCON1_FAN_AUTO;
  byte temperature = 23;

  if (powerModeCmd == POWER_OFF)
  {
    operatingMode = CARRIER_AIRCON1_MODE_OFF;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = CARRIER_AIRCON1_MODE_AUTO;
        break;
      case MODE_HEAT:
        operatingMode = CARRIER_AIRCON1_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = CARRIER_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = CARRIER_AIRCON1_MODE_DRY;
        break;
      case MODE_FAN:
        operatingMode = CARRIER_AIRCON1_MODE_FAN;
        temperatureCmd = 22; // Temperature is always 22 in FAN mode
        break;
    }
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = CARRIER_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = CARRIER_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = CARRIER_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = CARRIER_AIRCON1_FAN3;
      break;
    case FAN_4:
      fanSpeed = CARRIER_AIRCON1_FAN4;
      break;
    case FAN_5:
      fanSpeed = CARRIER_AIRCON1_FAN5;
      break;
  }

  if ( temperatureCmd > 16 && temperatureCmd < 31)
  {
    temperature = temperatureCmd;
  }

  sendCarrier(IR, operatingMode, fanSpeed, temperature);
}

// Send the Carrier code
// Carrier has the LSB and MSB in different format than Panasonic

void CarrierHeatpumpIR::sendCarrier(IRSender& IR, byte operatingMode, byte fanSpeed, byte temperature)
{
  byte sendBuffer[9] = { 0x4f, 0xb0, 0xc0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00 }; // The data is on the last four bytes

  static const prog_uint8_t temperatures[] PROGMEM = { 0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e, 0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b };
  byte checksum = 0;

  // PROGMEM arrays cannot be addressed directly, see http://forum.arduino.cc/index.php?topic=106603.0
  sendBuffer[5] = pgm_read_byte(&(temperatures[(temperature-17)]));

  sendBuffer[6] = operatingMode | fanSpeed;

  // Checksum

  for (int i=0; i<8; i++) {
    checksum += IR.bitReverse(sendBuffer[i]);
  }

  sendBuffer[8] = IR.bitReverse(checksum);

  // 40 kHz PWM frequency
  IR.setFrequency(40);

  // Header
  IR.mark(CARRIER_AIRCON1_HDR_MARK);
  IR.space(CARRIER_AIRCON1_HDR_SPACE);

  // Payload
  for (int i=0; i<sizeof(sendBuffer); i++) {
    IR.sendIRByte(sendBuffer[i], CARRIER_AIRCON1_BIT_MARK, CARRIER_AIRCON1_ZERO_SPACE, CARRIER_AIRCON1_ONE_SPACE);
  }

  // Pause + new header
  IR.mark(CARRIER_AIRCON1_BIT_MARK);
  IR.space(CARRIER_AIRCON1_MSG_SPACE);

  IR.mark(CARRIER_AIRCON1_HDR_MARK);
  IR.space(CARRIER_AIRCON1_HDR_SPACE);

  // Payload again
  for (int i=0; i<sizeof(sendBuffer); i++) {
    IR.sendIRByte(sendBuffer[i], CARRIER_AIRCON1_BIT_MARK, CARRIER_AIRCON1_ZERO_SPACE, CARRIER_AIRCON1_ONE_SPACE);
  }

  // End mark
  IR.mark(CARRIER_AIRCON1_BIT_MARK);
  IR.space(0);
}