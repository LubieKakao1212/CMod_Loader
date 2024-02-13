tasklist | find "Cosmoteer.exe" >nul: && goto cosmoteer_running

echo "not running"

exit



:cosmoteer_running
echo "running"