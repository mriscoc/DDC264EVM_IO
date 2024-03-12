# DDC264EVM_IO
This repository implements a dynamic link library (DLL) for Raw data adquisition from the [Texas Instruments DDC264 evaluation module](https://www.ti.com/tool/DDC264EVM) (**EVM**).
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

## Programming model
The DDC264EVM programming model consist in an USB interface provided by the Cypress MCU USB Peripheral HI SPD 56SSOP (CY7C68013A-56PVXC),
the control and glue logic implemented in an Xilinx Spartan-3E FPGA 250k 144TQFP (XC3S250E-4TQG144C) and the
[DDC264](https://www.ti.com/lit/ds/symlink/ddc264.pdf?ts=1710187491977) 64-Channel, Current-Input Analog-to-Digital Converters.

For be able to adquire data from the EVM, it is first necessary to install the correct drivers to recognize and enumerate the board trought
the USB interface.
