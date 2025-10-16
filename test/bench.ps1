<#
.SYNOPSIS
  Benchmark all .exe tools from \bin on all *.b files from \b.

.PARAMETER Root
  Root folder containing \bin, \b, \test. Default: parent of this script's folder.

.PARAMETER TimeoutSec
  Optional per-run timeout in seconds. 0 = no timeout.
#>

[CmdletBinding()]
param(
  [string]$Root,
  [int]$TimeoutSec = 0
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# --- Resolve paths ---
if (-not $Root -or $Root.Trim() -eq '') {
  $Root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
} else {
  $Root = (Resolve-Path $Root).Path
}

$BinDir = Join-Path $Root 'bin'
$BDir   = Join-Path $Root 'b'
$OutCsv = Join-Path $PSScriptRoot 'results.csv'

if (-not (Test-Path $BinDir)) { throw "Tools folder not found: $BinDir" }
if (-not (Test-Path $BDir))   { throw "Input folder not found: $BDir" }

# --- Discover tools and inputs ---
$tools  = Get-ChildItem -Path $BinDir -File -Filter '*bf.exe' | Sort-Object Name
$inputs = Get-ChildItem -Path $BDir  -File -Filter '*.b'   | Sort-Object Name

if (-not $tools)  { throw "No .exe tools found in $BinDir." }
if (-not $inputs) { throw "No *.b files found in $BDir." }

Write-Host "Tools: $($tools.Count). Inputs: $($inputs.Count)." -ForegroundColor Cyan
Write-Host "CSV output: $OutCsv" -ForegroundColor Cyan

# --- Helper: run a process with timing (and optional timeout) using System.Diagnostics.Process ---
function Invoke-TimedProcess {
  param(
    [Parameter(Mandatory=$true)][string]$ExePath,
    [Parameter(Mandatory=$true)][string]$Argument,
    [int]$TimeoutSec = 0
  )

  $startTime = Get-Date
  $sw = [System.Diagnostics.Stopwatch]::StartNew()

  $psi = New-Object System.Diagnostics.ProcessStartInfo
  $psi.FileName = $ExePath
  # Quote the argument in case of spaces
  $psi.Arguments = '"' + $Argument + '"'
  $psi.WorkingDirectory = (Split-Path $ExePath -Parent)
  $psi.UseShellExecute = $false
  $psi.RedirectStandardOutput = $true
  $psi.RedirectStandardError  = $true
  $psi.CreateNoWindow = $true

  $proc = New-Object System.Diagnostics.Process
  $proc.StartInfo = $psi
  [void]$proc.Start()

  $timedOut = $false
  if ($TimeoutSec -gt 0) {
    if (-not $proc.WaitForExit($TimeoutSec * 1000)) {
      try { $proc.Kill() } catch {}
      $timedOut = $true
    }
  } else {
    $proc.WaitForExit()
  }

  $stdout = $proc.StandardOutput.ReadToEnd()
  $stderr = $proc.StandardError.ReadToEnd()
  $exit   = if ($timedOut) { $null } else { $proc.ExitCode }

  $sw.Stop()

  [PSCustomObject]@{
    StartTime  = $startTime
    EndTime    = Get-Date
    DurationMs = [int64]$sw.Elapsed.TotalMilliseconds
    ExitCode   = $exit
    TimedOut   = $timedOut
    StdOut     = $stdout
    StdErr     = $stderr
  }
}

# --- Main benchmark loop ---
$results = New-Object System.Collections.Generic.List[object]

foreach ($tool in $tools) {
  foreach ($inp in $inputs) {
    Write-Host ("[{0}] {1} -> {2}" -f (Get-Date).ToString('HH:mm:ss'),
      $tool.Name, $inp.Name)

    $run = Invoke-TimedProcess -ExePath $tool.FullName -Argument $inp.FullName -TimeoutSec $TimeoutSec

    $results.Add([PSCustomObject]@{
      Tool        = $tool.Name
      InputFile   = $inp.Name
      DurationMs  = $run.DurationMs
      ExitCode    = $run.ExitCode
      TimedOut    = $run.TimedOut
    })
  }
}

# --- Save CSV ---
$results | Export-Csv -Path $OutCsv -Encoding UTF8 -NoTypeInformation

# --- Print CSV ---
Write-Host "`n=== CSV results ===" -ForegroundColor Green
Import-Csv -Path $OutCsv | Format-Table -AutoSize

# --- Per-tool totals and overall total ---
Write-Host "`n=== Per-tool totals ===" -ForegroundColor Green
$byTool = Import-Csv -Path $OutCsv |
  Group-Object Tool |
  ForEach-Object {
    $sumMs = ($_.Group | Measure-Object -Property DurationMs -Sum).Sum
    $cnt   = $_.Count
    $avgMs = [double]$sumMs / [math]::Max(1,$cnt)
    [PSCustomObject]@{
      Tool      = $_.Name
      Runs      = $cnt
      TotalMs   = [int64]$sumMs
      TotalTime = [System.TimeSpan]::FromMilliseconds($sumMs).ToString()
      AvgMs     = [int][math]::Round($avgMs,0)
    }
  } | Sort-Object TotalMs

$byTool | Format-Table -AutoSize

Write-Host "`n=== Overall total ===" -ForegroundColor Green
$overallMs = ($results | Measure-Object -Property DurationMs -Sum).Sum
$overallTs = [System.TimeSpan]::FromMilliseconds($overallMs)
"{0} run(s), total: {1} ({2} ms)" -f $results.Count, $overallTs, $overallMs
