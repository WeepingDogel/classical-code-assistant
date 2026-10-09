$py = 'C:\Users\WeepingDogel\AppData\Local\Microsoft\WindowsApps\python.exe'
$arg = 'd:\Project\classical-code-assistant\src\server\main.py'
$p = Start-Process -FilePath $py -ArgumentList $arg -PassThru -WindowStyle Hidden
Start-Sleep -Seconds 2
Write-Output ('proc alive: ' + (-not $p.HasExited))
try {
    $r = Invoke-WebRequest -Uri 'http://127.0.0.1:8000/' -UseBasicParsing -TimeoutSec 5
    Write-Output ('GET status=' + $r.StatusCode + ' body=' + $r.Content)
} catch {
    Write-Output ('GET failed: ' + $_.Exception.Message)
}
try {
    $body = '{"message":"hello world"}'
    $r2 = Invoke-WebRequest -Uri 'http://127.0.0.1:8000/v1/chat' -Method POST -Body $body -ContentType 'application/json' -UseBasicParsing -TimeoutSec 5
    Write-Output ('POST status=' + $r2.StatusCode + ' body=' + $r2.Content)
} catch {
    Write-Output ('POST failed: ' + $_.Exception.Message)
}
Stop-Process -Id $p.Id -Force
Write-Output 'done'
