@echo off
title %1

:: Flags
:: -ldcomp -ldxguid -ldxgi -ld3d11 -lwindowscodecs -mwindows -lgdi32 -luser32 -lole32
:: To generate the assembly code, use g++ -c "%1" 
g++ "%1" -o "%2" -municode -ldwrite -ld2d1
if exist "%2" ( move "%2" Output\ > nul & cd Output & .\"%2" )

echo.
pause
exit