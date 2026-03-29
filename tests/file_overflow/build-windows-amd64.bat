@echo off
REM Windows build not applicable for Linux-specific io_uring / inotify tests.
echo file_overflow tests are Linux-only.
exit /b 0
