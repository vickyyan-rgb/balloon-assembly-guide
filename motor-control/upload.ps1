param([string]$Port, [switch]$ListPorts, [switch]$VerifyOnly)
$ErrorActionPreference = 'Stop'
$cli = (Get-Command arduino-cli -ErrorAction SilentlyContinue).Source
if (!$cli) {
    $cli = Join-Path $env:LOCALAPPDATA 'Programs/Arduino IDE/resources/app/lib/backend/resources/arduino-cli.exe'
}
if (!(Test-Path -LiteralPath $cli)) { throw 'Install Arduino CLI on PATH or the Arduino IDE 2 user installation.' }
$config = Join-Path $env:USERPROFILE '.arduinoIDE/arduino-cli.yaml'
$options = @()
if (Test-Path -LiteralPath $config) { $options = @('--config-file', $config) }
if ($ListPorts) {
    & $cli board list @options
    exit $LASTEXITCODE
}
$sketch = Join-Path $PSScriptRoot 'BalloonMotorRemote'
$actionArgs = @('compile', '--fqbn', 'esp32:esp32:adafruit_feather_esp32_v2')
if (!$VerifyOnly) {
    if (!$Port) { throw 'Specify -Port COMx after listing ports.' }
    $actionArgs += @('--upload', '--port', $Port)
}
& $cli @actionArgs @options $sketch
exit $LASTEXITCODE
