/** 8 channel relay board.
 * DIY version of the widely used "Conrad relay board",
 * where the communication protocol emulated with an Arduino.
 * Order # 197720 - www.conrad.de
 */

/// Low-active relay outputs.
/// Bit 0 = relay 1 ... bit 7 = relay 8
/// Bit set means inverted output.
constexpr uint8_t invertedRelays = 0xFF;

/// Output pins connected to relays.
/// First entry = relay 1 ... last entry = relay 8
/// 0 = no relay connected
constexpr uint8_t relayPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};

/// Current address setting.
uint8_t settingAddress = 1;

/// Current option setting.
uint8_t settingOption = 1;

/// Current relay states.
uint8_t settingPort = 0x00;

/// Buffer for receiving frames.
uint8_t frameData[4] = {0};
uint8_t frameIndex = 0;

void setup()
{
  for (uint8_t i = 0; i < 8; ++i)
  {
    const uint8_t pin = relayPins[i];
    if (pin)
    {
      pinMode(pin, OUTPUT);
    }
  }
  setRelays(0x00);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  Serial.begin(19200);
}

void loop()
{
  if (Serial.available() > 0)
  {
    int inByte = Serial.read();
    frameData[frameIndex] = uint8_t(inByte);
    if (++frameIndex >= 4)
    {
      digitalWrite(LED_BUILTIN, 1);
      processFrame();
      frameIndex = 0;
      digitalWrite(LED_BUILTIN, 0);
    }
  }
}

void processFrame()
{
  const uint8_t cmd = frameData[0];
  const uint8_t addr = frameData[1];
  const uint8_t data = frameData[2];

  uint8_t crc = cmd;
  crc ^= addr;
  crc ^= data;

  // CRC error?
  if (crc != frameData[3])
  {
    sendFrame(255, 1, 0);
  }

  // broadcast?
  else if (addr == 0)
  {
    // execute broadcast?
    if (settingOption & 0x01)
    {
      executeCommand(cmd, data);
    }

    // block broadcast?
    if (settingOption & 0x02)
    {
      // send NOP
      sendFrame(0, addr + 1, data);
    }
    // no broadcast blocking
    else
    {
      // forward broadcast
      sendFrame(cmd, addr + 1, data);
    }
  }

  // SETUP?
  else if (cmd == 1)
  {
    executeCommand(cmd, addr);
    // SETUP for next device
    sendFrame(cmd, addr + 1, data);
  }

  // command for me?
  else if (addr == settingAddress)
  {
    executeCommand(cmd, data);
  }

  // command for someone else!
  else
  {
    sendFrame(cmd, addr, data);
  }
}

void executeCommand(uint8_t cmd, uint8_t data)
{
  switch (cmd)
  {
  case 0: // NO OPERATION
    sendFrame(~cmd, settingAddress, data);
    break;

  case 1: // SETUP
    settingAddress = data;
    sendFrame(~cmd, settingAddress, 0xA0);
    break;

  case 2: // GET PORT
    sendFrame(~cmd, settingAddress, settingPort);
    break;

  case 3: // SET PORT
    setRelays(data);
    sendFrame(~cmd, settingAddress, data);
    break;

  case 4: // GET OPTION
    sendFrame(~cmd, settingAddress, settingOption);
    break;

  case 5: // SET OPTION
    settingOption = data;
    sendFrame(~cmd, settingAddress, data);
    break;

  case 6: // SET SINGLE
    setRelays(settingPort | data);
    sendFrame(~cmd, settingAddress, data);
    break;

  case 7: // DEL SINGLE
    setRelays(settingPort & ~data);
    sendFrame(~cmd, settingAddress, data);
    break;

  case 8: // TOGGLE
    setRelays(settingPort ^ data);
    sendFrame(~cmd, settingAddress, data);
    break;

  default: // unknown
    sendFrame(0xFF, settingAddress, cmd);
    break;
  }
}

void setRelays(uint8_t data)
{
  settingPort = data;
  data ^= invertedRelays;
  for (uint8_t i = 0; i < 8; ++i)
  {
    const uint8_t pin = relayPins[i];
    if (pin)
    {
      digitalWrite(pin, data & (0x01 << i) ? 1 : 0);
    }
  }
}

void sendFrame(uint8_t cmd, uint8_t addr, uint8_t data)
{
  uint8_t txFrame[4] = {cmd, addr, data, cmd};
  txFrame[3] ^= addr;
  txFrame[3] ^= data;
  Serial.write(txFrame, sizeof(txFrame));
}
