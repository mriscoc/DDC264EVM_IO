/**
 * Acquisition software demo for the DDC264EVM_IO DLL
 * https://www.ti.com/tool/DDC264EVM
 *
 * Author: Miguel Risco-Castillo
 * Version: 3.2
 * Date: 2024/02/27
 *
 * LICENSE: MIT License.
 */

using System.Diagnostics;
using System.Text;

partial class Program
{

    string FPGARegStr(int RegN)
    {
        StringBuilder TempStr = new(61);
        EVM_RegNameTable(RegN, TempStr, 61);
        return TempStr.ToString();
    }

    int EVM_RegDataOut(int Reg, int Data)
    {
        int resOut = EVM_RegDataOut(ref USBdev, ref Reg, ref Data);
        if (resOut != 0)
        {
            string RegS = Reg.ToString("X");
            Console.WriteLine($"Error writting FPGA register: 0x{RegS} {FPGARegStr(Reg)}");
        }
        return resOut;
    }

    bool EVM_Prepare()
    {
        bool resOut = EVM_ResetDDC(ref USBdev);
        if (!resOut)
        {
            Console.WriteLine($"Error when resetting DDC");
            return false;
        }
        resOut = EVM_ClearTriggers(ref USBdev);
        if (!resOut)
        {
            Console.WriteLine("Error when clearing registers");
            return false;
        }
        resOut = EVM_DataSequence(ref USBdev, ref CFGHIGH, ref CFGLOW);
        if (!resOut) Console.WriteLine("Error when writting dataSequence");

        return resOut;
    }

    void Reset_Regs()
    {
        for (int i = 0; i<regsSize; i++) RegsEnable[i] = 0;
    }

    void Set_RegIn(int reg, int val)
    {
        RegsIn[reg] = val & 0xFF;
        RegsEnable[reg] = 1;
    }

    void Set_Regs()
    {
        int channelValue = CHANNEL_COUNT switch
        {
            <= 1 => 0,
            <= 2 => 1,
            <= 4 => 2,
            <= 8 => 3,
            <= 16 => 4,
            <= 32 => 5,
            <= 64 => 6,
            <= 128 => 7,
            <= 256 => 8,
            _ => 100,
        };

        if (channelValue == 100)
        {
            Console.WriteLine("Invalid channel count");
            Environment.Exit(-1);
        }

        else CHANNEL_COUNT = (int)Math.Pow(2, channelValue);
        // CONV
        Set_RegIn(0x01, CONV_LOW_INT - 1 >> 16);
        Set_RegIn(0x02, CONV_LOW_INT - 1 >> 8);
        Set_RegIn(0x03, CONV_LOW_INT - 1);
        Set_RegIn(0x04, CONV_HIGH_INT - 1 >> 16);
        Set_RegIn(0x05, CONV_HIGH_INT - 1 >> 8);
        Set_RegIn(0x06, CONV_HIGH_INT - 1);

        // DDC Sys Clock
        Set_RegIn(0x07, (CLC_HIGH << 4) | (CLC_LOW & 0x0F));
        Set_RegIn(0x08, DDC_CLK_CONFIG);

        // Format and Channel Count
        int FORMAT = CFGHIGH & 1;
        Set_RegIn(0x09, (FORMAT << 4) + channelValue);

        // DDC Data Clock
        Set_RegIn(0x0A, (DCLK_HIGH << 4) | (DCLK_LOW & 0x0F));
        Set_RegIn(0x0B, DCLK_CONFIG);

        // nDVALIDS
        Set_RegIn(0x0C, NDVALID_IGNORE);
        Set_RegIn(0x0D, NDVALID_READ);
        Set_RegIn(0x0E, NDVALID_READ >> 8);
        Set_RegIn(0x0F, NDVALID_READ >> 16);

        // DCLK Wait
        Set_RegIn(0x13, DCLK_WAIT_MCLK >> 8);
        Set_RegIn(0x14, DCLK_WAIT_MCLK);

        //// Format
        Set_RegIn(0x1F, FORMAT);

        // CLK CFG
        Set_RegIn(0x20, (CLK_CFG_HI << 4) | (CLK_CFG_LO & 0x0F));

        // CONV Config 
        Set_RegIn(0x57, CONV_CONFIG); // 0-Freerun, 1-Freerun, 2-Low, 3-High

        // Aditional undocummented registers
        Set_RegIn(0x51, CONV_WAIT_LOW >> 8);
        Set_RegIn(0x52, CONV_WAIT_LOW);
        Set_RegIn(0x53, CONV_WAIT_HIGH >> 8);
        Set_RegIn(0x54, CONV_WAIT_HIGH);
        Set_RegIn(0xEB, CLKDELAY_AROUND_CONV);
    }

    bool EVM_ShowRegisters()
    {
        int ErrorFlag = -1;
        Console.WriteLine("Reading current register values");
        try
        {
            ErrorFlag = EVM_RegsTransfer(ref USBdev, ref RegsIn[0], ref RegsEnable[0], ref RegsOut[0]);
        }
        catch (Exception ex)
        {
            Console.WriteLine("Code exception: " + ex.Message);
        }

        if (ErrorFlag == 0)
        {
            // Print EVM Firmware version
            Console.WriteLine($"Firmware version: {RegsOut[0x5E] * 256 + RegsOut[0x5F]}");
            for (int i = 1; i < regsSize; i++) Console.WriteLine($"Register {FPGARegStr(i)} [0x{i:X2}] : 0x{RegsOut[i]:X2}");
            return true;
        }
        else Console.WriteLine("Board is not connected or error in EVM_RegsTransfer, result: " + ErrorFlag);
        return false;
    }

    /*
    From the DDC264EVM Datasheet:

    1. (CHANNEL_COUNT) * (2) * (NDVALID_READ) < 1048576; and
    2. (CHANNEL_COUNT) * (2) * (NDVALID_READ) / 131072 must result as an integer. 

    Example: 32 channels

    1. 32 x 2 x (NDVALID_READ) < 1048576 -> (NDVALID_READ) < 16384, so 16383 meets this condition
    2. 32 x 2 x (NDVALID_READ) / 131072 must result as an integer -> 64x16383/131072 = 7.9995...

    So, the more close result that meet the condition 2 is "7" then 32 x 2 x (NDVALID_READ) / 131072 = 7
    Finally, the maximum NDVALID_READ is 14336 for 32 channels

    Also NDVALID_READ must be even

    */

    void DDC_Adquisition(ref double AllData)
    {
        if (!EVM_ShowRegisters()) Environment.Exit(0);

        // Prepare EVM board
        if (!EVM_Prepare()) Environment.Exit(-1);

        // Clear RegsEnable
        Reset_Regs();

        // Load configuration into registers
        Set_Regs();

        try
        {
            int ErrorFlag = EVM_RegsTransfer(ref USBdev, ref RegsIn[0], ref RegsEnable[0], ref RegsOut[0]);
            if (ErrorFlag == 0)
            {
                Console.WriteLine("FPGA registers written");
            }
            else
            {
                Console.WriteLine("Error in EVM_RegsTransfer, result: " + ErrorFlag);
                Environment.Exit(-1);
            }

            int AllDataAorBfirst = 0;
            ErrorFlag = EVM_DataCap(ref USBdev, CHANNEL_COUNT, NDVALID_READ, ref AllData, ref AllDataAorBfirst);
            if (ErrorFlag != 0)
            {
                // If there was an error in EVM_DataCap, display the error code
                Console.WriteLine("Error in data capture: " + ErrorFlag);
                Environment.Exit(-1);
            }

            Console.WriteLine($"Data captured\n");

        }
        catch (Exception ex)
        {
            Console.WriteLine("Code exception: " + ex.Message);
        }
    }
}