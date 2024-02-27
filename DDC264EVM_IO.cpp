/**
 * Acquisition library for the DDC264 Evaluation Module
 * https://www.ti.com/tool/DDC264EVM
 *
 * Author: Miguel Risco-Castillo
 * Version: 3.2
 * Date: 2024/02/27
 *
 * LICENSE: MIT License.
 * 
 * Based on code examples published by Texas Instruments at USB_IO_FOR_VB6_API.
 */

#include "StdAfx.h"
#include "CyApi.h"
#include "DDC264EVM_IO.h"
#include <cstring>
#include <malloc.h>
#include <math.h>
#include <map>

BOOL APIENTRY DllMain(HANDLE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


// DDC264EVM_IO ID
constexpr char DLL_ID[] = "DDC264EVM_IO ver 3.2";
constexpr char DLL_C[] = "Miguel Risco-Castillo (c) 2024";

#define STRINGLEN 65536 //the larger this number is, the faster the data is shifted in.
#define MAX_CHANNELS_FAST 4096 // 2048 = 1024A + 1024B

// This function reads the device descriptors from the Cypress USB Chip(s).
// It returns arrays of values, one set of values per device detected.

int __stdcall ReadDeviceDescriptors(int* USBdevCount, int* bLengthPass, int* bDescriptorTypePass,
    long* bcdUSBPass, int* bDeviceClassPass, int* bDeviceSubClassPass,
    int* bDeviceProtocolPass, int* bMaxPacketSize0Pass, long* idVendorPass,
    long* idProductPass, long* bcdDevicePass, int* iManufacturerPass,
    int* iProductPass, int* iSerialNumberPass, int* bNumConfigurationsPass)
{
    CCyUSBDevice* USBDevice;
    USB_DEVICE_DESCRIPTOR descr;

    USBDevice = new CCyUSBDevice(NULL);   // Create an instance of CCyUSBDevice

    USBdevCount[0] = USBDevice->DeviceCount();

    for (int i = 0; i < USBDevice->DeviceCount(); i++)
    {

        if (USBDevice->Open(i))
        {
            USBDevice->GetDeviceDescriptor(&descr);
            bLengthPass[i] = descr.bLength;
            bDescriptorTypePass[i] = descr.bDescriptorType;
            bcdUSBPass[i] = descr.bcdUSB;
            bDeviceClassPass[i] = descr.bDeviceClass;
            bDeviceSubClassPass[i] = descr.bDeviceSubClass;
            bDeviceProtocolPass[i] = descr.bDeviceProtocol;
            bMaxPacketSize0Pass[i] = descr.bMaxPacketSize0;
            idVendorPass[i] = descr.idVendor;
            idProductPass[i] = descr.idProduct;
            bcdDevicePass[i] = descr.bcdDevice;
            iManufacturerPass[i] = descr.iManufacturer;
            iProductPass[i] = descr.iProduct;
            iSerialNumberPass[i] = descr.iSerialNumber;
            bNumConfigurationsPass[i] = descr.bNumConfigurations;
            USBDevice->Close();
        }
    }

    return(USBdevCount[0]);
}

// This function reads the interface descriptors from the Cypress USB Chip defined by USBdev

int __stdcall ReadInterfaceDescriptors(int* USBdev, int* bLengthPass, int* bDescriptorTypePass,
    int* bInterfaceNumberPass, int* bAlternateSettingPass, short* bNumEndpointsPass,
    int* bInterfaceClassPass, int* bInterfaceSubClassPass, int* bInterfaceProtocolPass,
    int* iInterfacePass)
{
    CCyUSBDevice* USBDevice;
    USB_INTERFACE_DESCRIPTOR descr;

    USBDevice = new CCyUSBDevice(NULL);   // Create an instance of CCyUSBDevice

    if (USBDevice->Open(USBdev[0]))
    {
        USBDevice->GetIntfcDescriptor(&descr);
        bLengthPass[0] = descr.bLength;
        bDescriptorTypePass[0] = descr.bDescriptorType;
        bInterfaceNumberPass[0] = descr.bInterfaceNumber;
        bAlternateSettingPass[0] = descr.bAlternateSetting;
        bNumEndpointsPass[0] = descr.bNumEndpoints;
        bInterfaceClassPass[0] = descr.bInterfaceClass;
        bInterfaceSubClassPass[0] = descr.bInterfaceSubClass;
        bInterfaceProtocolPass[0] = descr.bInterfaceProtocol;
        iInterfacePass[0] = descr.iInterface;
        USBDevice->Close();
    }
    else
    {
        return(-1);
    }

    return(0);
}

// This function writes a string of bytes to the USB
int __stdcall XferDataOut(int* USBdev, unsigned char* Data, long* DataLength)
{
    CCyUSBDevice* USBDevice = new CCyUSBDevice(NULL); // Create an instance of CCyUSBDevice - NULL means we don't register for pnp events

    if (USBDevice->Open(USBdev[0]))
    {
        if (USBDevice->BulkOutEndPt)
        {
            USBDevice->BulkOutEndPt->TimeOut = 100;
            USBDevice->BulkOutEndPt->XferData(Data, DataLength[0]);
        }
        USBDevice->Close();
    }
    else
    {
        return(-1);
    }

    return(0);
}


// This is the primary conduit for onesie/twosie data from the USB to the computer.
int __stdcall XferDataIn(int* USBdev, unsigned char* Data, long* DataLength)
{

    bool XferSuccess;
    CCyUSBDevice* USBDevice = new CCyUSBDevice(NULL); // Create an instance of CCyUSBDevice - NULL means we don't register for pnp events

    if (USBDevice->Open(USBdev[0]))
    {
        if (USBDevice->BulkInEndPt)
        {
            USBDevice->BulkInEndPt->TimeOut = 250; //500ms = 0.5s
            XferSuccess = USBDevice->BulkInEndPt->XferData(Data, DataLength[0]);
        }
        else
        {
            USBDevice->Close();
            return(-10);  //-10 means couldn't open USB endpoint
        }
        USBDevice->Close();
    }
    else
    {
        return(-2); //-2 means couldn't open USB device at all
    }

    if (XferSuccess) { return(1); } // 1 means probably more data coming, since no timeout
    else { return(0); } // 0 means no more data, and we timed out!
}


//===================================================================================================================


 // Return simple dll ID string
void __stdcall dllID(char* text, int bufsize)
{
    int textSize = min((size_t)bufsize, strlen(DLL_ID));
    memcpy(text, DLL_ID, textSize);
    text[textSize] = 0;
}

// Return simple dll Copyright string
void __stdcall dllCprght(char* text, int bufsize)
{
    int textSize;
    textSize = min((size_t)bufsize, strlen(DLL_C));
    memcpy(text, DLL_C, textSize);
    text[textSize] = 0;
}


// This function writes a byte to a Register via USB.
int __stdcall EVM_RegDataOut(int* USBdev, int* Reg, int* Data)
{
    constexpr auto ArraySize = 2;
    unsigned char DataArr[ArraySize] = { 0 };

    CCyUSBDevice* USBDevice = new CCyUSBDevice(NULL); // Create an instance of CCyUSBDevice - NULL means we don't register for pnp events

    DataArr[0] = Reg && 0xFF;
    DataArr[1] = Data && 0xFF;

    if (USBDevice->Open(USBdev[0]))
    {
        if (USBDevice->BulkOutEndPt)
        {
            long DataLength = ArraySize;
            USBDevice->BulkOutEndPt->TimeOut = 100;
            USBDevice->BulkOutEndPt->XferData(DataArr, DataLength);
        }
        USBDevice->Close();
    }
    else
    {
        return(-1);
    }

    return(0);
}

bool __stdcall EVM_ResetDDC(int* USBdev) // Soft Reset DDC
{
    byte Array[] = { 0x00, 0x00, 0x00, 0x00, 0x15, 0xFF, 0x15, 0xFF, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0xFF, 0x15, 0xFF, 0x15, 0xFF, 0x15, 0xFF };
    long arraySize = sizeof(Array) / sizeof(Array[0]);
    return (XferDataOut(USBdev, Array, &arraySize) == 0);
}

bool __stdcall EVM_ClearTriggers(int* USBdev) // Makes sure the CFG state machine is reset
{
    byte Array[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x1E, 0x00, 0xD1, 0x00 };
    long arraySize = sizeof(Array) / sizeof(Array[0]);
    return (XferDataOut(USBdev, Array, &arraySize) == 0);
}

bool __stdcall EVM_DataSequence(int* USBdev, byte* CFGHIGH, byte* CFGLOW) // Send DataSequence Array
{
    byte Array[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x12, 0x00, 0x1C, 0x00, 0x1D, 0x00, 0x1F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00 };
    Array[11] = *CFGHIGH; Array[13] = *CFGLOW;
    long arraySize = sizeof(Array) / sizeof(Array[0]);
    return (XferDataOut(USBdev, Array, &arraySize) == 0);
}

std::map<int, const char*> RegNameTable = {
    {0x00, "No Op"},
    {0x01, "CONV_LOW_REG_MSB"},
    {0x02, "CONV_LOW_REG_MidB"},
    {0x03, "CONV_LOW_REG_LSB"},
    {0x04, "CONV_HIGH_REG_MSB"},
    {0x05, "CONV_HIGH_REG_MidB"},
    {0x06, "CONV_HIGH_REG_LSB"},
    {0x07, "DIVXCLK_REGS"},
    {0x08, "DDC_CLK_SEL"},
    {0x09, "FORMAT[4],CHANNELS"},
    {0x0A, "DIVXCLK_DATA_REGS"},
    {0x0B, "DDC_DATA_CLK_SEL"},
    {0x0C, "nDVALIDS_IGNORE"},
    {0x0D, "nDVALIDS_READ_LSB"},
    {0x0E, "nDVALIDS_READ_MidB"},
    {0x0F, "nDVALIDS_READ_MSB"},
    {0x10, "DONE[1],START_CONVERSIONS[0]"},
    {0x11, "CLK_CFG"},
    {0x12, "DIN_CFG"},
    {0x13, "DCLK_WAIT_COUNT_MSB"},
    {0x14, "DCLK_WAIT_COUNT_LSB"},
    {0x15, "DDC_RESETN"},
    {0x16, "HARDWARE_TRIGGER_EN"},
    {0x1A, "DCLK_SELECT_MANUAL_OR_AUTO"},
    {0x1B, "DOUT_IN[1],DCLK_MANUAL_SET_VALUE[0]"},
    {0x1C, "DDC CFGHIGH"},
    {0x1D, "DDC CFGLOW"},
    {0x1E, "TRIGGER"},
    {0x1F, "FORMAT_DIN_CFG"},
    {0x20, "REG_FREQ_DIV_DIN_CLK_HIGH[3:0],REG_FREQ_DIV_DIN_CLK_LOW[3:0]"},
    {0x22, "DDC Daughter Card Select"},
    {0x23, "RESERVED"},
    {0x24, "RESERVED"},
    {0x25, "RESERVED"},
    {0x26, "RESERVED"},
    //
    {0x51, "CONV_WAIT_LOW_REG_MSB"},
    {0x52, "CONV_WAIT_LOW_REG_LSB"},
    {0x53, "CONV_WAIT_HIGH_REG_MSB"},
    {0x54, "CONV_WAIT_HIGH_REG_LSB"},
    {0x55, "NON_CONT"},
    {0x56, "RESET_CONV"},
    {0x57, "CONV_CONFIG"},
    //
    {0x5E, "FIRMWARE_VERSION_MSB"},
    {0x5F, "FIRMWARE_VERSION_LSB"},
    {0xD0, "read_out_trigger"},
    {0xD1, "RESERVED"},
    //
    {0xDA, "TRIGGER_READ_AVG_RAM"},
    {0xDB, "STOP_ADDR_MSB"},
    {0xDC, "STOP_ADDR_LSB"},
    {0xDD, "AB_AVG_SEL"},
    {0xDE, "USE_RAM_CHIPS"},
    //
    {0xE0, "RESERVED"},
    {0xE1, "RESERVED"},
    {0xE2, "RESERVED"},
    {0xE3, "RESERVED"},
    {0xE4, "RESERVED"},
    {0xE5, "RESERVED"},
    //
    {0xEB, "CLKDELAY_AROUND_CONV"},
    //
    {0xFF, "SOFT_FPGA_RESET"},
    //
    //Default "UNKNOW"
};

int __stdcall EVM_RegNameTable(int RegN, char* buf, int bufsize)
{
    int textSize;
    const char* RegName = RegNameTable[RegN];
    if (RegName != nullptr) {
        textSize = min((size_t)bufsize, strlen(RegName));
        memcpy(buf, RegName, textSize);
    }
    else {
        textSize = min((size_t)bufsize, strlen("UNKNOW"));
        memcpy(buf, "UNKNOW", textSize);
    }
    buf[textSize] = 0;
    return textSize;
}


long __stdcall EVM_RegsTransfers(int* USBdev, long* RegsIn, long* RegEnable, long* RegsOut) {

    CCyUSBDevice* USBDevice = new CCyUSBDevice(NULL);

    bool XferSuccess;
    int AllowedWaitCount;

    long DataLen;
    unsigned char DataStr[512];
    unsigned char* Data = (unsigned char*)calloc(2048, sizeof(unsigned char));
    if (Data == NULL) return -3;

    DataLen = 2;
    DataStr[0] = (char)0x00;
    DataStr[1] = (char)0x00;

    for (long i = 0; i < 256; i++)
    {
        if (RegEnable[i] == 1)
        {
            DataStr[DataLen] = (char)i;
            DataStr[DataLen + 1] = (char)RegsIn[i];
            DataLen += 2;
        }
    }

    DataStr[DataLen] = (char)0xD0;  //D0 is the opcode to start/stop reading the FPGA registers
    DataStr[DataLen + 1] = (char)0x00;
    DataLen += 2;

    if (USBDevice->Open(USBdev[0]))
    {
        //Write the Data Str
        if (USBDevice->BulkOutEndPt)
        {
            USBDevice->BulkOutEndPt->TimeOut = 100;
            USBDevice->BulkOutEndPt->XferData(DataStr, DataLen);
        }
        else
        {
            USBDevice->Close();
            return(-9);  //-9 means couldn't open USB endpoint
        }

        //Clear out the buffer
        DataLen = 2048;
        XferSuccess = true;
        AllowedWaitCount = 16383;
        while (XferSuccess == true && AllowedWaitCount > 0);
        {
            if (USBDevice->BulkInEndPt)
            {
                USBDevice->BulkInEndPt->TimeOut = 50; //500ms = 0.5s
                USBDevice->BulkInEndPt->SetXferSize(DataLen);
                XferSuccess = USBDevice->BulkInEndPt->XferData(Data, DataLen);
            }
            else
            {
                USBDevice->Close();
                return(-10);  //-10 means couldn't open USB endpoint
            }
            AllowedWaitCount--;
        }

        if (XferSuccess) return(-5); //Never timed out, probably more data in the pipe.

        //Write the "Read FPGA Register" opcode: D001
        DataStr[0] = char(0xD0);  //D0 is the opcode to start/stop reading the FPGA registers
        DataStr[1] = char(0x01);
        DataLen = 2;
        if (USBDevice->BulkOutEndPt)
        {
            USBDevice->BulkOutEndPt->TimeOut = 100;
            USBDevice->BulkOutEndPt->XferData(DataStr, DataLen);
        }
        else
        {
            USBDevice->Close();
            return(-9);  //-9 means couldn't open USB endpoint
        }

        //Read the Data back
        if (RegsOut != nullptr)
        {
            if (USBDevice->BulkInEndPt)
            {
                USBDevice->BulkInEndPt->TimeOut = 100; //500ms = 0.5s
                DataLen = 512;
                XferSuccess = USBDevice->BulkInEndPt->XferData(Data, DataLen);
            }
            else
            {
                USBDevice->Close();
                return(-10);  //-10 means couldn't open USB endpoint
            }

            for (long i = 0; i < (2 * (DataLen - 1)); i += 2)
            {
                if (((long)Data[i]) < 256)
                {
                    RegsOut[((long)Data[i])] = ((long)Data[i + 1]);
                }
            }
        }

        //Stop the "Read FPGA Register" opcode: D000
        //then reset CONV with 5600 and 5601
        DataStr[0] = char(0xD0);  //D0 is the opcode to start/stop reading the FPGA registers
        DataStr[1] = char(0x00);

        DataStr[2] = char(0x00);
        DataStr[3] = char(0x00);

        DataStr[4] = char(0x56);
        DataStr[5] = char(0x00);

        DataStr[6] = char(0x00);
        DataStr[7] = char(0x00);

        DataStr[8] = char(0x56);
        DataStr[9] = char(0x01);
        DataLen = 10;
        if (USBDevice->BulkOutEndPt)
        {
            USBDevice->BulkOutEndPt->TimeOut = 100;
            USBDevice->BulkOutEndPt->XferData(DataStr, DataLen);
        }
        else
        {
            USBDevice->Close();
            return(-9);  //-9 means couldn't open USB endpoint
        }

        USBDevice->Close();
    }
    else
    {
        return(-1);
    }

    return(0);

}


long __stdcall EVM_DataCap(int* USBdev, long Channels, long nDVALIDReads, double* DataArray, long* AllDataAorBfirst) {

    long LenVar;
    unsigned char inputCmd[2];
    bool XferSuccess;
    long StringLen, StringLenRet;
    long BytesOfData;
    int AllowedWaitCount;
    long BytesRead;
    long DAi;

    StringLen = STRINGLEN;
    StringLenRet = STRINGLEN;

    //Number of readings = channels * nDVALID Reads
    //Number of readings per channel = nDVALID Reads / 2
    //Bytes of data = Number of readings * 4
    BytesOfData = Channels * nDVALIDReads * 4;

    double WorkingTemp;
    unsigned char DataCap[STRINGLEN];

    CCyUSBDevice* USBDevice = new CCyUSBDevice(NULL);   // Create an instance of CCyUSBDevice

    if (USBDevice->Open(USBdev[0]))
    {

        if (USBDevice->BulkOutEndPt)   //shifts out 0x1000, which stops all conversions
        {
            inputCmd[0] = 0x10;
            inputCmd[1] = 0x00;
            LenVar = 2;
            USBDevice->BulkOutEndPt->TimeOut = 250;
            XferSuccess = USBDevice->BulkOutEndPt->XferData(inputCmd, LenVar);
            if (XferSuccess == false)
            {
                USBDevice->Close();
                return(-5);
            }

            inputCmd[0] = 0x00;
            inputCmd[1] = 0x00;
            LenVar = 2;
            USBDevice->BulkOutEndPt->TimeOut = 250;
            XferSuccess = USBDevice->BulkOutEndPt->XferData(inputCmd, LenVar);
            if (XferSuccess == false)
            {
                USBDevice->Close();
                return(-5);
            }
        }

        if (USBDevice->BulkInEndPt)  //emptys read buffer
        {
            StringLenRet = StringLen;
            XferSuccess = true;
            AllowedWaitCount = 32;
            while (XferSuccess == true && AllowedWaitCount > 0)
            {
                USBDevice->BulkInEndPt->TimeOut = 250;  //1000 = 1s
                USBDevice->BulkInEndPt->SetXferSize(StringLen);
                XferSuccess = USBDevice->BulkInEndPt->XferData(DataCap, StringLenRet);
                AllowedWaitCount--;
            }
        }

        if (USBDevice->BulkOutEndPt)    //shifts out 0x10FF, which starts a conversion
        {
            inputCmd[0] = 0x10;
            inputCmd[1] = 0xFF;
            LenVar = 2;
            USBDevice->BulkOutEndPt->TimeOut = 250;
            XferSuccess = USBDevice->BulkOutEndPt->XferData(inputCmd, LenVar);
            if (XferSuccess == false)
            {
                USBDevice->Close();
                return(-5);
            }
        }

        BytesRead = 0;
        if (USBDevice->BulkInEndPt)
        {
            XferSuccess = false;
            AllowedWaitCount = 3; //10s at 250
            while (XferSuccess == false && AllowedWaitCount > 0)
            {
                StringLenRet = StringLen;
                USBDevice->BulkInEndPt->TimeOut = 10000;  //1000 = 1s
                USBDevice->BulkInEndPt->SetXferSize(StringLen);
                XferSuccess = USBDevice->BulkInEndPt->XferData(DataCap, StringLenRet);
                AllowedWaitCount--;
            }

            if (XferSuccess == false)
            {
                USBDevice->Close();
                return(-4);
            }

            if (StringLenRet % 4 != 0)
            {
                USBDevice->Close();
                return(-8);
            }

            AllDataAorBfirst[0] = (DataCap[0] == 128) ? 0 : 1;

            DAi = 0;
            for (long j = 0; j < StringLenRet; j += 4)
            {
                WorkingTemp = DataCap[j + 1] * 0x10000 + DataCap[j + 2] * 0x100 + DataCap[j + 3];
                DataArray[DAi] = WorkingTemp;
                DAi++;
            }
            BytesRead += StringLenRet;
        }
        else
        {
            USBDevice->Close();
            return(-10);
        }

        while (BytesRead < BytesOfData)
        {
            StringLenRet = StringLen;
            XferSuccess = false;
            AllowedWaitCount = 40; //10s at 250
            while (XferSuccess == false && AllowedWaitCount > 0)
            {
                USBDevice->BulkInEndPt->TimeOut = 250;
                XferSuccess = USBDevice->BulkInEndPt->XferData(DataCap, StringLenRet);
                AllowedWaitCount--;
            }
            if (XferSuccess == false)
            {
                USBDevice->Close();
                return(-4);
            }

            BytesRead += StringLenRet;

            if (StringLenRet % 4 != 0)
            {
                USBDevice->Close();
                return(-8);
            }

            for (long j = 0; j < StringLenRet; j += 4)
            {
                WorkingTemp = DataCap[j + 1] * 0x10000 + DataCap[j + 2] * 0x100 + DataCap[j + 3];
                DataArray[DAi] = WorkingTemp;
                DAi++;
            }
        }

        if (USBDevice->BulkOutEndPt) //shifts out 0x1000, which lets the conversion end
        {
            inputCmd[0] = 0x10;
            inputCmd[1] = 0x00;
            LenVar = 2;
            USBDevice->BulkOutEndPt->TimeOut = 250;
            XferSuccess = USBDevice->BulkOutEndPt->XferData(inputCmd, LenVar);
            if (XferSuccess == false)
            {
                USBDevice->Close();
                return(-6);
            }
        }
        USBDevice->Close();
    }
    else
    {
        return(-2); //-2 means it couldn't open the USB device.
    }

    return(0);
}
