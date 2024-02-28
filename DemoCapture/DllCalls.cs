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

using System.Runtime.InteropServices;
using System.Text;

partial class Program
{

    [DllImport(dllFile, CallingConvention = CallingConvention.StdCall)]
    public static extern int ReadDeviceDescriptors(ref int USBdevCount, ref int bLengthPass, ref int bDescriptorTypePass, ref long bcdUSBPass, ref int bDeviceClass, ref int bDeviceSubClass, ref int bDeviceProtocol, ref int bMaxPacketSize0, ref long idVendor, ref long idProduct, ref long bcdDevice, ref int iManufacturer, ref int iProduct, ref int iSerialNumber, ref int bNumConfigurations);

    [DllImport(dllFile, CallingConvention = CallingConvention.StdCall)]
    public static extern int ReadInterfaceDescriptors(ref int USBdev, ref int bLengthPass, ref int bDescriptorTypePass, ref int bInterfaceNumberPass, ref int bAlternateSettingPass, ref short bNumEndpointsPass, ref int bInterfaceClassPass, ref int bInterfaceSubClassPass, ref int bInterfaceProtocolPass, ref int iInterfacePass);

    [DllImport(dllFile, CallingConvention = CallingConvention.StdCall)]
    public static extern int XferDataOut(ref int USBdev, [MarshalAs(UnmanagedType.LPArray)] byte[] Data, ref long DataLength);

    [DllImport(dllFile, CallingConvention = CallingConvention.StdCall)]
    public static extern int XferDataIn(ref int USBdev, [MarshalAs(UnmanagedType.LPArray)] byte[] Data, ref long DataLength);

    // =============================================================================================================

    [DllImport(dllFile, CallingConvention = CallingConvention.StdCall)]
    public static extern void dllID([MarshalAs(UnmanagedType.LPStr)] StringBuilder text, int bufsize);

    [DllImport(dllFile, CallingConvention = CallingConvention.StdCall)]
    public static extern void dllCprght([MarshalAs(UnmanagedType.LPStr)] StringBuilder text, int bufsize);

    [DllImport(dllFile, CallingConvention = CallingConvention.StdCall)]
    public static extern int EVM_RegDataOut(ref int USBdev, ref int Reg, ref int Data);

    [DllImport(dllFile, CallingConvention = CallingConvention.StdCall)]
    public static extern bool EVM_ResetDDC(ref int USBdev);

    [DllImport(dllFile, CallingConvention = CallingConvention.StdCall)]
    public static extern bool EVM_ClearTriggers(ref int USBdev);

    [DllImport(dllFile, CallingConvention = CallingConvention.StdCall)]
    public static extern bool EVM_DataSequence(ref int USBdev, ref byte CFGHIGH, ref byte CFGLOW);

    [DllImport(dllFile, CallingConvention = CallingConvention.StdCall)]
    public static extern bool EVM_RegNameTable(int RegN, [MarshalAs(UnmanagedType.LPStr)] StringBuilder buf, int bufsize);

    [DllImport(dllFile, CallingConvention = CallingConvention.StdCall)]
    public static extern int EVM_RegsTransfer(ref int USBdev, ref int Array_RegsIn, ref int Array_RegEnable, ref int Array_RegsOut);

    [DllImport(dllFile, CallingConvention = CallingConvention.StdCall)]
    public static extern int EVM_DataCap(ref int USBdev, int Channels, int Samples, ref int AllData, ref int AllDataAorBfirst);

}
