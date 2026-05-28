@echo off
PowerShell -ExecutionPolicy Bypass -File "%~dp0switch-kernel.ps1" site
if errorlevel 1 pause
