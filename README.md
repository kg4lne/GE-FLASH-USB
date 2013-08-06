GE-Flash-USB X2212/X2444 EPROM Programmer
=========================================

This archive contains all the CAD and firmware files necessary to build the
GE-Flash-USB EPROM programmer for use with the GE-Flash programming software
by KG4LNE. GE-Flash is a complete solution for programming vintage GE Phoenix,
Delta and Ranger VHF/UHF commercial radios. For more information, please visit
the website at http://www.rtzaudio.com/kg4lne for additional information.

This package includes all PCB CAD files and firmware C source code. All PCB CAD
files are in Cadsoft Eagle v6.4.0 format. The source code for the AT90USB firmware
was developed in Atmel Studio 6.1 and was originally adapted from the USBKEY sample
provided by Atmel. I modified this from a mass storage device into to a simple
read/write USB device. The simple USB device can be accessed on Windows 7/8 via
the built-in generic WinUSB interface that ships with Windows.

You will need to install and configure your GE-Flash-USB programmer device to
communicate with WinUSB just like any other device though. The GE-Flash-USB driver
install and INF files are located on my website and are included in the 
GE-Flash application installer also. Be sure and have the INF and other driver
files on your machine (or install the GE-Flash application first). You can let
Windows search the GE-Flash program files application directory for the INF
file as well.

Note the BOM files for the programmer PCB are located in the project BOM 
subdirectory, here you'll also find an order upload file
"GEF-USB-B_OrderUploadDigiKey.csv" that can be uploaded to Digikey.
Upload the parts order file using the Digikey BOM manager in your account,
this will preload an order with all parts needed to build the PCB. 

Enjoy!

KG4LNE