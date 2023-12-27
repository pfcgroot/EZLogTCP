# EZLogTCP

A tiny C++ utility for Windows that monitors activity on a TCP port (8686) and pin 10 of the printer port. All activity (events) are written to a log file in a specified folder. This utility has been successfuly used in an experiment where events had to be logged during an MRI scanning session where an independend application was executen on another computer.

Note that this application requires a special module (DLL) for accessing the printer port: https://www.highrez.co.uk/downloads/inpout32/
