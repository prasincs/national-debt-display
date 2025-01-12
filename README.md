# National Debt E-ink Display

This project creates a real-time U.S. National Debt display using an affordable Inkplate 2 e-ink screen. Inspired by the display shown at the U.S. Capitol, this implementation provides a low-cost, energy-efficient way to monitor the national debt using official Treasury Direct data.

## Features

The display updates every 12 hours to show the current U.S. National Debt using the official Treasury Direct API. The three-color e-ink display (black, white, and red) provides excellent readability in any lighting condition while consuming minimal power.

Key features include:
- Real-time debt updates from Treasury Direct
- Automatic WiFi reconnection
- Error handling and retry logic
- Power-efficient deep sleep mode
- Formatted numbers with commas for readability
- Three-color display with red highlighting for emphasis

## Hardware Requirements

To build this project, you will need:
- Inkplate 2 ($21.38) - Available from [Soldered Electronics](https://soldered.com/product/inkplate-2/)
- USB-C cable for programming
- WiFi connection
- Optional: Battery pack for portable operation

## Software Requirements

Before starting, ensure you have:
1. Arduino IDE (2.0 or newer recommended)
2. Inkplate board support package
3. Required libraries:
   - Inkplate
   - ArduinoJson
   - HTTPClient
   - WiFi

## Installation

1. Add Inkplate board support to Arduino IDE:
   - Open Arduino IDE
   - Go to File -> Preferences
   - Add this URL to "Additional Board Manager URLs":
     ```
     https://github.com/SolderedElectronics/Dasduino-Board-Definitions-for-Arduino-IDE/raw/master/package_Dasduino_Boards_index.json
     ```
   - Go to Tools -> Board -> Boards Manager
   - Search for "Inkplate" and install

2. Install required libraries through Library Manager:
   - Go to Tools -> Manage Libraries
   - Search for and install:
     - Inkplate
     - ArduinoJson

3. Configure your WiFi credentials:
   - Copy `secrets.h.example` to `secrets.h`
   - Edit `secrets.h` with your WiFi credentials:
     ```cpp
     #define WIFI_SSID "your_wifi_name"
     #define WIFI_PASSWORD "your_wifi_password"
     ```

4. Select the correct board:
   - Go to Tools -> Board
   - Select "Soldered Inkplate 2"

5. Upload the code:
   - Open `National_Debt_Display.ino`
   - Click Upload button

## Usage

After uploading the code, the display will:
1. Connect to WiFi
2. Fetch the current debt data
3. Display formatted debt figure
4. Update every 12 hours
5. Show error messages if connection fails

The display will continue to show the last successful reading even when powered off, thanks to the e-ink technology.

## Customization

You can modify several parameters in the code:
- Update interval (default: 12 hours)
- Text size and positioning
- Display colors
- Error retry attempts and delays

Look for the constant definitions at the top of the code to adjust these settings.

## Troubleshooting

Common issues and solutions:

1. Display shows "Failed to fetch data":
   - Check WiFi credentials
   - Verify internet connection
   - Ensure Treasury Direct API is accessible

2. Numbers appear malformed:
   - Check serial monitor for parsing errors
   - Verify API response format
   - Clear device memory and restart

3. Display not updating:
   - Check USB connection
   - Verify correct board selection
   - Monitor serial output for errors

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Representative Thomas Massie for the inspiration
- Treasury Direct for providing the public API
- Soldered Electronics for the Inkplate 2 hardware
- The Arduino community for libraries and support

## Disclaimer

This project is for educational purposes only. The displayed debt figures are obtained from public API data provided by the U.S. Treasury.

---

For questions or support, please open an issue in this repository.
