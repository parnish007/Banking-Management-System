$exe = 'C:\Users\AB\Desktop\Banking-Management-System\build\banking_system.exe'
$input = 'C:\Users\AB\Desktop\Banking-Management-System\build\in.txt'
$dest = 'C:\Users\AB\Desktop\Banking-Management-System\report\DoCSE_Project_Report_Format (1)\figures\console_output.txt'
Get-Content -Raw $input | & $exe | Out-File -FilePath $dest -Encoding utf8
Write-Output 'TRANSCRIPT_DONE'
