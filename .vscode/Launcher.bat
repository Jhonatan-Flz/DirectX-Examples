@echo off
title %1

:: Flags
:: -ldcomp -ldxguid -ldxgi -ld3d11 -lwindowscodecs -luser32 -mwindows -lole32 -static-libgcc -static-libstdc++
g++ "%1" -o "%2" -municode -ldwrite -ld2d1 -lgdi32 
if exist "%2" ( move "%2" output\ > nul & cd output & .\"%2" )

echo.
pause
exit