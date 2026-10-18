$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Update-Dialog.ps1')
$release = [pscustomobject]@{ Version = [version]'5.158'; Notes = "Changes in this version`r`n" + ('Grind and XL changes. ' * 500) }
$form = New-DualStickUpdateDialog ([version]'5.157') $release
try {
    if ($form.Tag -ne 'later') { throw 'Default action approves an update' }
    $notes = $form.Controls.Find('PatchNotes', $true)[0]
    if (-not $notes.ReadOnly -or -not $notes.Multiline -or $notes.ScrollBars -ne 'Vertical' -or $notes.Text -notmatch 'Changes in this version') { throw 'Patch notes are not safely displayed and scrollable' }
    foreach ($name in @('update','skip','disable')) {
        $buttons = @($form.Controls.Find($name, $true))
        if ($buttons.Count -ne 1 -or $buttons[0].Tag -ne $name) { throw "Missing decision: $name" }
    }
    $form.ShowInTaskbar = $false; $form.Opacity = 0
    $form.Show(); $form.PerformLayout(); [Windows.Forms.Application]::DoEvents()
    $image = New-Object Drawing.Bitmap($form.Width, $form.Height)
    try {
        $form.DrawToBitmap($image, (New-Object Drawing.Rectangle(0, 0, $form.Width, $form.Height)))
        $output = Join-Path (Split-Path $PSScriptRoot -Parent) 'build/updater-dialog-preview.png'
        $image.Save($output, [Drawing.Imaging.ImageFormat]::Png)
    } finally { $image.Dispose() }
    $form.Controls.Find('skip', $true)[0].PerformClick()
    if ($form.Tag -ne 'skip') { throw 'Skip button did not return skip' }
} finally { $form.Dispose() }
Write-Output 'PASS: versioned update dialog, scrollable read-only patch notes, explicit choices and safe close default; preview rendered'
