# DDC264EVM_IO
This repository contains a dynamic link library (DLL) for Raw data acquisition for the [Texas Instruments DDC264 evaluation module](https://www.ti.com/tool/DDC264EVM) (**EVM**).
Some demo programs in Visual Studio are provided as example of use.

[DDC264EVM User guide](https://www.ti.com/lit/ug/sbau186/sbau186.pdf?ts=1710186906433&ref_url=https%253A%252F%252Fwww.ti.com%252Ftool%252FDDC264EVM)

![image](https://github.com/mriscoc/DDC264EVM_IO/assets/2745567/fcf62f8c-9b78-4df2-8d6f-95210497c325)

From the DDC264EVM User guide: "The DDC264EVM provides an easy-to-use platform for evaluating the DDC264 charge-digitizing A/D
converters. A PC interface board (DDC264EVM) and an analog input daughterboard (AIB) for the DDC264
devices are included along with software that makes analysis and testing of this device simple.
The DDC264 is a 20-bit, 64-channel, current-input analog-to-digital (A/D) converter. It
combines both current-to-voltage and A/D conversion so that 64 separate low-level current output devices,
such as photodiodes, can be directly connected to its inputs and digitized. For each of the 64 inputs, the
DDC264 uses the proven dual switched-integrator front-end. This design allows for continuous current
integration. The EVM consists of four DDC264 devices, a USB device for interfacing
to a PC, an FPGA for device communication, 16 MB of memory for temporary data storage, and a
high-density socket to allow connection to the analog inputs."

For be able to adquire data from the EVM, it is first necessary to install the correct drivers to recognize and enumerate the board trought
the USB interface. Two drivers are installed for the board usage, look at the release section to get the EVM drivers.

## Programming model
The DDC264EVM programming model consist in an USB interface provided by the Cypress MCU USB Peripheral HI SPD 56SSOP (CY7C68013A-56PVXC),
the control and glue logic implemented in an Xilinx Spartan-3E FPGA 250k 144TQFP (XC3S250E-4TQG144C) and the
[DDC264](https://www.ti.com/lit/ds/symlink/ddc264.pdf?ts=1710187491977) 64-Channel, Current-Input Analog-to-Digital Converters.

The USB Peripheral allows the communication between the PC and the EVM, the drivers and dll implement the routines to setup
the board registers which control the behavior and data acquisition of the EVM, those registers are keeping inside of the FPGA.

After the correct installation and enumeration, the program can read the value of the current board registers.

## EVM registers:

| Address | Register |
|---------|----------|
|0x00 | No Op |
|0x01 | CONV_LOW_REG_MSB |
|0x02 | CONV_LOW_REG_MidB |
|0x03 | CONV_LOW_REG_LSB |
|0x04 | CONV_HIGH_REG_MSB |
|0x05 | CONV_HIGH_REG_MidB |
|0x06 | CONV_HIGH_REG_LSB |
|0x07 | DIVXCLK_REGS |
|0x08 | DDC_CLK_SEL |
|0x09 | FORMAT[4],CHANNELS |
|0x0A | DIVXCLK_DATA_REGS |
|0x0B | DDC_DATA_CLK_SEL |
|0x0C | nDVALIDS_IGNORE |
|0x0D | nDVALIDS_READ_LSB |
|0x0E | nDVALIDS_READ_MidB |
|0x0F | nDVALIDS_READ_MSB |
|0x10 | DONE[1],START_CONVERSIONS[0] |
|0x11 | CLK_CFG |
|0x12 | DIN_CFG |
|0x13 | DCLK_WAIT_COUNT_MSB |
|0x14 | DCLK_WAIT_COUNT_LSB |
|0x15 | DDC_RESETN |
|0x16 | HARDWARE_TRIGGER_EN |
|0x1A | DCLK_SELECT_MANUAL_OR_AUTO |
|0x1B | DOUT_IN[1],DCLK_MANUAL_SET_VALUE[0] |
|0x1C | DDC CFGHIGH |
|0x1D | DDC CFGLOW |
|0x1E | TRIGGER |
|0x1F | FORMAT_DIN_CFG |
|0x20 | REG_FREQ_DIV_DIN_CLK_HIGH[3:0],REG_FREQ_DIV_DIN_CLK_LOW[3:0] |
|0x22 | DDC Daughter Card Select |
|0x23 | RESERVED |
|0x24 | RESERVED |
|0x25 | RESERVED |
|0x26 | RESERVED |
|-|-|
|0x51 | CONV_WAIT_LOW_REG_MSB |
|0x52 | CONV_WAIT_LOW_REG_LSB |
|0x53 | CONV_WAIT_HIGH_REG_MSB |
|0x54 | CONV_WAIT_HIGH_REG_LSB |
|0x55 | NON_CONT |
|0x56 | RESET_CONV |
|0x57 | CONV_CONFIG |
|-|-|
|0x5E | FIRMWARE_VERSION_MSB |
|0x5F | FIRMWARE_VERSION_LSB |
|0xD0 | read_out_trigger |
|0xD1 | RESERVED |
|-|-|
|0xDA | TRIGGER_READ_AVG_RAM |
|0xDB | STOP_ADDR_MSB |
|0xDC | STOP_ADDR_LSB |
|0xDD | AB_AVG_SEL |
|0xDE | USE_RAM_CHIPS |
|-|-|
|0xE0 | RESERVED |
|0xE1 | RESERVED |
|0xE2 | RESERVED |
|0xE3 | RESERVED |
|0xE4 | RESERVED |
|0xE5 | RESERVED |
|-|-|
|0xEB | CLKDELAY_AROUND_CONV |
|-|-|
|0xFF | SOFT_FPGA_RESET |

## Main methods exported in the DLL
```cpp
// Returns simple dll version string
void __stdcall dllID(char* text, int bufsize);

// Allows writing a value to a register
int __stdcall EVM_RegDataOut(int* USBdev, int* Reg, int* Data);

// Resets the DDC at the EVM
bool  __stdcall EVM_ResetDDC(int* USBdev);

// Clear triggers register
bool  __stdcall EVM_ClearTriggers(int* USBdev);

// Prepare the DDC for acquire data
bool  __stdcall EVM_DataSequence(int* USBdev, byte* CFGHIGH, byte* CFGLOW);

// Get the register name
int __stdcall EVM_RegNameTable(int RegN, char* buf, int bufsize);

// Get and Set data to board registers
long __stdcall EVM_RegsTransfer(int* USBdev, long* RegsIn, long* RegEnable, long* RegsOut = nullptr);

// Capture a block of data
long __stdcall EVM_DataCap(int* USBdev, long Channels, long nDVALIDReads, double* DataArray, long* AllDataAorBfirst);
```
