@echo off
PowerShell -ExecutionPolicy Bypass -File "%~dp0switch-kernel.ps1" simulate
if errorlevel 1 pause
