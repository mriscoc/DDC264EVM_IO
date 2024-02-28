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

using System.Text;

partial class Program
{
    private const string dllFile = "DDC264EVM_IO.dll";
    private const string dataFile = "EVMdata.csv";

    // Configuration Registers
    const int CONV_LOW_INT = 1600;
    const int CONV_HIGH_INT = 1600;
    const int CONV_CONFIG = 0; // 0-Freerun, 1-Freerun, 2-Low, 3-High
    const int CLC_HIGH = 7;
    const int CLC_LOW = 7;
    const int DDC_CLK_CONFIG = 1; // 1=Running, 0=disabled;
    int CHANNEL_COUNT = 16;
    const int NDVALID_IGNORE = 255;
    int NDVALID_READ = 1024; // This number must meet some requirements 
    const int DCLK_HIGH = 0;
    const int DCLK_LOW = 0;
    const int DCLK_CONFIG = 1; // 1=Running, 0=disabled
    const int DCLK_WAIT_MCLK = 13000;
    const int CLK_CFG_HI = 3;
    const int CLK_CFG_LO = 3;
    const int CONV_WAIT_LOW = 1550;
    const int CONV_WAIT_HIGH = 1550;
    const int CLKDELAY_AROUND_CONV = 0;

    // DDC Configuration Register bits
    const int DDCbit13 = 0; // CLKDIV
    const int DDCbit10 = 1; // Range[1]
    const int DDCbit9 = 1; // Range[0]
    const int DDCbit8 = 1; // Format: 0=16 bits, 1=20 bits
    const int DDCbit7 = 0; // SPEED
    const int DDCbit4 = 0; // SLEW
    const int DDCbit0 = 0; // TEST
    byte CFGLOW = (DDCbit7 << 7) + (DDCbit4 << 4) + DDCbit0;
    byte CFGHIGH = (DDCbit13 << 5) + (DDCbit10 << 2) + (DDCbit9 << 1) + DDCbit8;

    int USBdev = 0; // DDC EVM board to send/receive data to/from

    // Register arrays
    const int regsSize = 255;
    int[] RegsIn = new int[regsSize];
    int[] RegsOut = new int[regsSize];
    int[] RegsEnable = new int[regsSize];

    static void Main()
    {

        Program program = new();
        Console.WriteLine("\nAcquisition software demo for the DDC264EVM_IO DLL Version: 3.2\n");

        try // Check if the DLL can be loaded
        {
            StringBuilder tempStr = new(100);
            dllID(tempStr, 100);
            Console.WriteLine("DLL id: " + tempStr);
            dllCprght(tempStr, 100);
            Console.WriteLine(tempStr);
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex.Message);
            System.Environment.Exit(-1);
        }

        // Initialize the array to store values
        int[] AllData = new int[program.CHANNEL_COUNT * program.NDVALID_READ];

        program.DDC_Adquisition(ref AllData);

        // Show captured data
        for (int i = 0; i < program.CHANNEL_COUNT * program.NDVALID_READ; i++)
        {
            Console.WriteLine($"AllData[{i}] = {AllData[i]}");
        }

    }

}
