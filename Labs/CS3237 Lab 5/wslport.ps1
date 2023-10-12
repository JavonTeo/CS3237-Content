$ports = @(1883);
 
$wslAddress = bash.exe -c "ip addr show eth0 | grep -oP '(?<=inet\s)\d+(\.\d+){3}'";
 
if ($wslAddress -match '^(\d{1,3}\.){3}\d{1,3}$') {
  Write-Host "WSL IP address: $wslAddress" -ForegroundColor Green;
  Write-Host "Ports: $ports" -ForegroundColor Green;
}
else {
  Write-Host "Error: Could not find WSL IP address." -ForegroundColor Red;
  exit;
}
 
$listenAddress = '0.0.0.0';
 
foreach ($port in $ports) {
  netsh interface portproxy delete v4tov4 listenport=$port listenaddress=$listenAddress;
  netsh interface portproxy add v4tov4 listenport=$port listenaddress=$listenAddress connectport=$port connectaddress=$wslAddress;
}
 
$fireWallDisplayName = 'WSL Port Forwarding';
 
Remove-NetFireWallRule -DisplayName $fireWallDisplayName;
New-NetFireWallRule -DisplayName $fireWallDisplayName -Direction Outbound -LocalPort $ports -Action Allow -Protocol TCP;
New-NetFireWallRule -DisplayName $fireWallDisplayName -Direction Inbound -LocalPort $ports -Action Allow -Protocol TCP;