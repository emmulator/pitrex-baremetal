<#
Installs the new PiTrex menu release over an unpacked Malban "PiTrex_sd_Jul29_25" archive
(or over the SD card made from it). Windows PowerShell 5.1 or PowerShell 7.

  .\install.ps1 [-Settings malban-2025|malban-2021|rpiteam-2021|alt] [-ArcadeTweaks] [-DryRun] <folder>
  .\install.ps1 -Restore <folder>

Replaced files are saved in <folder>\backup-menu-<date>\ with a MANIFEST.tsv that -Restore reads.
Nothing outside the files listed in payload\ and payload-if-missing\ (only added when missing) is touched (and, with
-ArcadeTweaks, arcade-tweaks\; with -Settings, the chosen file of settings\; Malban's 2025 file of settings\ also
when the card has no usable vectrexInterface.ini). Running it again with another -Settings value changes only that file.
#>
param(
  [Parameter(Position = 0)][string]$Folder,
  [switch]$ArcadeTweaks,
  [ValidateSet('keep', 'malban-2025', 'malban-2021', 'rpiteam-2021', 'alt', 'tuned')][string]$Settings = 'keep',
  [switch]$DryRun,
  [switch]$Restore
)
$ErrorActionPreference = 'Stop'

function Die($msg) { Write-Host "ERROR: $msg" -ForegroundColor Red; exit 1 }
function P($root, $rel) { Join-Path $root ($rel -replace '/', '\') }
function Same($a, $b) {
  (Test-Path -LiteralPath $a) -and (Test-Path -LiteralPath $b) -and
  ((Get-FileHash -LiteralPath $a -Algorithm SHA256).Hash -eq (Get-FileHash -LiteralPath $b -Algorithm SHA256).Hash)
}

if (-not $Folder) { Write-Host "usage: .\install.ps1 [-Settings malban-2025|malban-2021|rpiteam-2021|alt] [-ArcadeTweaks] [-DryRun] [-Restore] <folder>"; exit 2 }
if (-not (Test-Path -LiteralPath $Folder -PathType Container)) { Die "not a folder: $Folder" }
$target = (Resolve-Path -LiteralPath $Folder).ProviderPath.TrimEnd('\')
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$payload = Join-Path $here 'payload'
$tweaks = Join-Path $here 'arcade-tweaks'
$utf8 = New-Object System.Text.UTF8Encoding($false)
$latin1 = [System.Text.Encoding]::GetEncoding(28591)

# ---------------------------------------------------------------- restore
if ($Restore) {
  $bk = Get-ChildItem -LiteralPath $target -Directory -Filter 'backup-menu-*' |
    Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName 'MANIFEST.tsv') } |
    Sort-Object Name | Select-Object -Last 1
  if (-not $bk) { Die "no backup-menu-* folder with a MANIFEST.tsv in $target" }
  $bkPath = $bk.FullName
  Write-Host "restoring from $bkPath"
  $lines = @(Get-Content -LiteralPath (Join-Path $bkPath 'MANIFEST.tsv'))
  [array]::Reverse($lines)
  foreach ($line in $lines) {
    if (-not $line) { continue }
    $f = $line.Split("`t")
    switch ($f[0]) {
      { $_ -in 'R', 'D' } { Copy-Item -LiteralPath (P $bkPath $f[1]) -Destination (P $target $f[1]) -Force; Write-Host "  restored $($f[1])" }
      'A' {
        $t = P $target $f[1]
        if ($f.Count -gt 2 -and $f[2] -and (Test-Path -LiteralPath $t) -and (Get-FileHash -LiteralPath $t -Algorithm SHA256).Hash -ne $f[2].ToUpper()) {
          Write-Host "  keep     $($f[1]) (changed since it was installed: yours now)"
        } else { Remove-Item -LiteralPath $t -Force -ErrorAction SilentlyContinue; Write-Host "  removed  $($f[1])" }
      }
      'M' {
        if ((Test-Path -LiteralPath (P $target $f[2])) -and -not (Test-Path -LiteralPath (P $target $f[1]))) {
          Move-Item -LiteralPath (P $target $f[2]) -Destination (P $target $f[1]); Write-Host "  renamed back $($f[1])"
        }
      }
    }
  }
  Rename-Item -LiteralPath (Join-Path $bkPath 'MANIFEST.tsv') -NewName 'MANIFEST.tsv.done'
  $menuDir = Join-Path $target 'menu'
  if ((Test-Path -LiteralPath $menuDir) -and -not (Get-ChildItem -LiteralPath $menuDir -Force)) { Remove-Item -LiteralPath $menuDir }
  Write-Host "done. (the backup folder was kept: $bkPath)"
  exit 0
}

# ---------------------------------------------------------------- install
if (-not (Test-Path -LiteralPath $payload -PathType Container)) { Die "payload\ folder missing next to this script" }
foreach ($f in 'pitrex.img', 'loader.ini', 'piZero1/loader.pit') {
  if (-not (Test-Path -LiteralPath (P $target $f) -PathType Leaf)) {
    Die "$target does not look like the unpacked PiTrex_sd_Jul29_25 archive ($f is missing).`nPass the folder that contains pitrex.img, loader.ini and the piZero1 and piZero2 folders."
  }
}

$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$bkPath = Join-Path $target "backup-menu-$stamp"
$n = 1
while (Test-Path -LiteralPath $bkPath) { $n++; $bkPath = Join-Path $target "backup-menu-$stamp-$n" }
$manifest = Join-Path $bkPath 'MANIFEST.tsv'
$script:changed = 0

function StartBackup { if (-not $DryRun -and -not (Test-Path -LiteralPath $bkPath)) { New-Item -ItemType Directory -Path $bkPath | Out-Null } }
function Note($kind, $a, $b) {
  if ($DryRun) { return }
  [System.IO.File]::AppendAllText($manifest, "$kind`t$a`t$b`n", $utf8)
}

function PutFile($src, $rel) {
  $dest = P $target $rel
  if (Same $src $dest) { Write-Host "  same     $rel"; return }
  $script:changed++
  if (Test-Path -LiteralPath $dest) {
    Write-Host "  replace  $rel"
    if (-not $DryRun) {
      StartBackup
      $b = P $bkPath $rel
      New-Item -ItemType Directory -Force -Path (Split-Path -Parent $b) | Out-Null
      Copy-Item -LiteralPath $dest -Destination $b
      Note 'R' $rel ''
    }
  } else {
    Write-Host "  add      $rel"
    if (-not $DryRun) { StartBackup; Note 'A' $rel (Get-FileHash -LiteralPath $src -Algorithm SHA256).Hash.ToLower() }   # restore keeps it if changed since
  }
  if (-not $DryRun) {
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dest) | Out-Null
    Copy-Item -LiteralPath $src -Destination $dest -Force
    if (-not (Same $src $dest)) { Die "copy of $rel does not match the original; the SD card or disk may be failing. Run: .\install.ps1 -Restore `"$target`"" }
  }
}

Write-Host "installing into $target"
if ($DryRun) { Write-Host "(dry run: nothing will be written)" }

$payloadRoot = (Resolve-Path -LiteralPath $payload).ProviderPath.TrimEnd('\')
Get-ChildItem -LiteralPath $payload -Recurse -File | Sort-Object FullName | ForEach-Object {
  $rel = $_.FullName.Substring($payloadRoot.Length + 1) -replace '\\', '/'
  PutFile $_.FullName $rel
}

# files that become the user's own once installed (menu\favorites.ini): put on the card only if it has none yet
$keepDir = Join-Path $here 'payload-if-missing'
if (Test-Path -LiteralPath $keepDir -PathType Container) {
  $keepRoot = (Resolve-Path -LiteralPath $keepDir).ProviderPath.TrimEnd('\')
  Get-ChildItem -LiteralPath $keepDir -Recurse -File | Sort-Object FullName | ForEach-Object {
    $rel = $_.FullName.Substring($keepRoot.Length + 1) -replace '\\', '/'
    if (Test-Path -LiteralPath (P $target $rel)) { Write-Host "  keep     $rel (yours: never replaced)" }
    else { PutFile $_.FullName $rel }
  }
}

# 0.1.0 had one menu\<name>.ini per entry; 0.2.0 has them all in menu\menu.default.ini. Old files that are exactly
# as 0.1.0 shipped them are removed (saved in the backup); files you changed are kept and still read.
$legacy = Join-Path $here 'legacy-menu-files.sha256'
$menuDir = P $target 'menu'
if ((Test-Path -LiteralPath $legacy) -and (Test-Path -LiteralPath $menuDir -PathType Container)) {
  foreach ($line in Get-Content -LiteralPath $legacy) {
    $parts = $line -split '\s+', 2
    if ($parts.Count -lt 2) { continue }
    $f = Join-Path $menuDir $parts[1]
    if (-not (Test-Path -LiteralPath $f -PathType Leaf)) { continue }
    if ((Get-FileHash -LiteralPath $f -Algorithm SHA256).Hash -eq $parts[0].ToUpper()) {
      $script:changed++
      Write-Host "  remove   menu/$($parts[1]) (now in menu.default.ini)"
      if (-not $DryRun) {
        StartBackup
        New-Item -ItemType Directory -Force -Path (Join-Path $bkPath 'menu') | Out-Null
        Copy-Item -LiteralPath $f -Destination (Join-Path (Join-Path $bkPath 'menu') $parts[1])
        Note 'D' "menu/$($parts[1])" ''
        Remove-Item -LiteralPath $f -Force
      }
    } else {
      Write-Host "  keep     menu/$($parts[1]) (you changed it; it is still read after the menu file)"
    }
  }
}
if (Test-Path -LiteralPath (Join-Path $menuDir 'menu.ini')) {
  Write-Host "  keep     menu/menu.ini (your changes; they are read after the new menu.default.ini)"
}

# files that earlier versions put on the card and that moved or are no longer needed (the guide NEW_MENU_README.md is
# now menu\README.md): removed when they are exactly as a release shipped them (saved in the backup), kept when changed
$legacyFiles = Join-Path $here 'legacy-files.sha256'
if (Test-Path -LiteralPath $legacyFiles) {
  $legacyLines = @(Get-Content -LiteralPath $legacyFiles | ForEach-Object { , ($_ -split '\s+', 2) } | Where-Object { $_.Count -ge 2 })
  $seenLegacy = @{}
  foreach ($parts in $legacyLines) {
    $f = P $target $parts[1]
    if (-not (Test-Path -LiteralPath $f -PathType Leaf)) { continue }
    if ((Get-FileHash -LiteralPath $f -Algorithm SHA256).Hash -ne $parts[0].ToUpper()) { continue }
    $seenLegacy[$parts[1]] = 1
    $script:changed++
    Write-Host "  remove   $($parts[1]) (replaced by a newer file)"
    if (-not $DryRun) {
      StartBackup
      $bkDir = Split-Path -Parent (P $bkPath $parts[1])
      New-Item -ItemType Directory -Force -Path $bkDir | Out-Null
      Copy-Item -LiteralPath $f -Destination (P $bkPath $parts[1])
      Note 'D' $parts[1] ''
      Remove-Item -LiteralPath $f -Force
    }
  }
  foreach ($parts in $legacyLines) {
    if ($seenLegacy.ContainsKey($parts[1])) { continue }
    $seenLegacy[$parts[1]] = 1
    if (Test-Path -LiteralPath (P $target $parts[1]) -PathType Leaf) { Write-Host "  keep     $($parts[1]) (you changed it)" }
  }
}

# loader.ini: NEW_MENU = 1, and a NEW_MENU_FONT line with comments if there is none (keeps the file's own line endings)
$ini = P $target 'loader.ini'
$text = $latin1.GetString([System.IO.File]::ReadAllBytes($ini))
$menuOk = $text -match '(?im)^[ \t]*NEW_MENU[ \t]*=[ \t]*1'
$fontOk = $text -match '(?im)^[ \t]*NEW_MENU_FONT[ \t]*='
if ($menuOk -and $fontOk) {
  Write-Host "  same     loader.ini (NEW_MENU already 1)"
} else {
  $crlf = $text.Contains("`r")
  $lf = $text -replace "`r", ''
  if ($lf.Length -gt 0 -and -not $lf.EndsWith("`n")) { $lf += "`n" }
  if (-not $menuOk) {
    if ($lf -match '(?im)^[ \t]*NEW_MENU[ \t]*=') {
      $lf = [regex]::Replace($lf, '(?im)^[ \t]*NEW_MENU[ \t]*=.*$', 'NEW_MENU = 1')
    } else {
      $lf += "NEW_MENU = 1`n"
    }
  }
  if (-not $fontOk) {
    $lf += "; NEW_MENU_FONT is the lettering of the new menu:`n;   DEFAULT     the PiTrex's own font`n;   MINIMALIST  fewer lines per letter, so a screen full of text may flicker less`nNEW_MENU_FONT = DEFAULT`n"
  }
  if ($crlf) { $lf = $lf -replace "`n", "`r`n" }
  $script:changed++
  Write-Host "  edit     loader.ini (NEW_MENU = 1, NEW_MENU_FONT)"
  if (-not $DryRun) {
    StartBackup
    Copy-Item -LiteralPath $ini -Destination (Join-Path $bkPath 'loader.ini')
    Note 'R' 'loader.ini' ''
    [System.IO.File]::WriteAllBytes($ini, $latin1.GetBytes($lf))
  }
}

# vectrexInterface.ini (the screen settings): an existing file is the user's and stays, unless it is missing or empty (the
# original settings screen can leave it empty): then Malban's 2025 file goes in. -Settings picks one of the four files of
# settings\ (see settings\README.txt); the alt one comes with its vectrex.ini.
$cur = P $target 'vectrexInterface.ini'
$size = -1
if (Test-Path -LiteralPath $cur -PathType Leaf) { $size = (Get-Item -LiteralPath $cur).Length }
if ($Settings -eq 'tuned') { $Settings = 'alt' }
$files = @{
  'malban-2025'  = @('vectrexInterface-malban-2025-07.ini',  "Malban's 2025 file")
  'malban-2021'  = @('vectrexInterface-malban-2021-03.ini',  "Malban's 2021 file")
  'rpiteam-2021' = @('vectrexInterface-rpiteam-2021-12.ini', 'the RpiTeaM 2021 file')
  'alt'          = @('vectrexInterface-alt.ini',             'the alternative file')
}
$sdir = Join-Path $here 'settings'
if ($files.ContainsKey($Settings)) {
  $srcIni = $files[$Settings][0]; $label = $files[$Settings][1]
  if (-not (Test-Path -LiteralPath (Join-Path $sdir $srcIni) -PathType Leaf)) { Die "settings\$srcIni missing next to this script" }
  if ($size -ge 100 -and $env:PITREX_FRESH_CARD) { Write-Host "settings: installing $label" }
  elseif ($size -ge 100) { Write-Host "settings: replacing yours with $label (saved in the backup)" }
  elseif ($size -ge 0) { Write-Host "settings: your vectrexInterface.ini is empty; installing $label" }
  else { Write-Host "settings: no vectrexInterface.ini; installing $label" }
  PutFile (Join-Path $sdir $srcIni) 'vectrexInterface.ini'
  # vectrex.ini (which BIOS Exact loads) comes only with the alt file; it uses the original BIOS 7ADB.bin
  if ($Settings -eq 'alt') { PutFile (Join-Path $sdir 'vectrex-alt.ini') 'vectrex.ini' }
} elseif ($size -lt 100) {
  $srcIni = 'vectrexInterface-malban-2025-07.ini'
  if (-not (Test-Path -LiteralPath (Join-Path $sdir $srcIni) -PathType Leaf)) { Die "settings\ folder missing next to this script" }
  if ($size -ge 0) { Write-Host "settings: your vectrexInterface.ini is empty; installing Malban's 2025 file" }
  else { Write-Host "settings: no vectrexInterface.ini; installing Malban's 2025 file" }
  PutFile (Join-Path $sdir $srcIni) 'vectrexInterface.ini'
} else {
  Write-Host "  keep     vectrexInterface.ini (your settings; -Settings malban-2021, rpiteam-2021, malban-2025 or alt replaces them)"
}

# optional arcade tweaks
if ($ArcadeTweaks) {
  if (-not (Test-Path -LiteralPath $tweaks -PathType Container)) { Die "arcade-tweaks\ folder missing next to this script" }
  Write-Host "arcade tweaks:"
  foreach ($f in 'blackwidow.ini', 'battlezone.ini') { PutFile (Join-Path $tweaks $f) $f }
  foreach ($f in 'starwars.ini', 'lunar.ini') {
    $src = P $target $f
    $dst = P $target "$f.author-calibration"
    if ((Test-Path -LiteralPath $src) -and -not (Test-Path -LiteralPath $dst)) {
      Write-Host "  rename   $f -> $f.author-calibration"
      $script:changed++
      if (-not $DryRun) {
        StartBackup
        Move-Item -LiteralPath $src -Destination $dst
        Note 'M' $f "$f.author-calibration"
      }
    }
  }
}

if ($DryRun) { Write-Host "dry run finished: $($script:changed) change(s) would be made." }
elseif ($script:changed -eq 0) { Write-Host "nothing to do: this folder already has the release installed." }
elseif ($env:PITREX_FRESH_CARD) { Write-Host "done: $($script:changed) change(s)." }   # make_card.py: a new card, its backup is removed
else {
  Write-Host "done: $($script:changed) change(s). Originals saved in $bkPath"
  Write-Host "To undo:  .\install.ps1 -Restore `"$target`""
}
