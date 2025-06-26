@echo off
echo Chess Humanizer SABLE Release - Windows Installer
echo ================================================

echo.
echo Installing dependencies...
pip install -r requirements.txt

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Failed to install dependencies
    echo Please check your Python installation and try again
    pause
    exit /b 1
)

echo.
echo Running system test...
python test_sable.py

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo WARNING: Some tests failed, but installation may still work
    echo Check the test output above for details
) else (
    echo.
    echo SUCCESS: All tests passed!
)

echo.
echo Running quick start guide...
python quick_start.py

echo.
echo Installation completed!
echo.
echo Next steps:
echo 1. Add trained model files to the models/ directory
echo 2. Run: python examples/basic_usage.py
echo 3. Try: python predict_moves.py --help
echo 4. Read README.md for detailed documentation
echo.
pause 