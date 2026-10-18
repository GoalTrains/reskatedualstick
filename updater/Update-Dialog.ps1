Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
[Windows.Forms.Application]::EnableVisualStyles()

function New-DualStickUpdateDialog($Installed, $Release) {
    $form = New-Object Windows.Forms.Form
    $form.Text = 'Dual Stick Controls - update available'
    $form.Size = New-Object Drawing.Size(820, 650)
    $form.MinimumSize = New-Object Drawing.Size(700, 500)
    $form.StartPosition = 'CenterScreen'
    $form.AutoScaleMode = 'Dpi'
    $form.ShowIcon = $false
    $form.Font = New-Object Drawing.Font('Segoe UI', 10)
    $form.Tag = 'later'
    $form.KeyPreview = $true
    $form.Add_KeyDown({ param($sender, $eventArgs) if ($eventArgs.KeyCode -eq 'Escape') { $sender.Close() } })
    $layout = New-Object Windows.Forms.TableLayoutPanel
    $layout.Dock = 'Fill'; $layout.Padding = New-Object Windows.Forms.Padding(20)
    $layout.ColumnCount = 1; $layout.RowCount = 4
    $layout.RowStyles.Add((New-Object Windows.Forms.RowStyle('Absolute', 76))) | Out-Null
    $layout.RowStyles.Add((New-Object Windows.Forms.RowStyle('Percent', 100))) | Out-Null
    $layout.RowStyles.Add((New-Object Windows.Forms.RowStyle('Absolute', 58))) | Out-Null
    $layout.RowStyles.Add((New-Object Windows.Forms.RowStyle('Absolute', 48))) | Out-Null
    $header = New-Object Windows.Forms.Label
    $header.Dock = 'Fill'
    $previous = if ($Installed) { 'v' + $Installed } else { 'Not installed' }
    $header.Text = "Installed: $previous     Available: v$($Release.Version)`r`n`r`nRead the changes below. Updating keeps your settings."
    $layout.Controls.Add($header, 0, 0)
    $notes = New-Object Windows.Forms.TextBox
    $notes.Name = 'PatchNotes'; $notes.Dock = 'Fill'; $notes.ReadOnly = $true
    $notes.Multiline = $true; $notes.WordWrap = $true; $notes.ScrollBars = 'Vertical'
    $notes.BackColor = [Drawing.SystemColors]::Window
    $notes.Text = $Release.Notes
    $layout.Controls.Add($notes, 0, 1)
    $explanation = New-Object Windows.Forms.Label
    $explanation.Dock = 'Fill'; $explanation.Padding = New-Object Windows.Forms.Padding(0, 8, 0, 0)
    $explanation.Text = 'Skip this release keeps your version and asks again for a newer release. Disable update checks stops all checks. Closing this window skips this launch only.'
    $layout.Controls.Add($explanation, 0, 2)
    $buttons = New-Object Windows.Forms.FlowLayoutPanel
    $buttons.Dock = 'Fill'; $buttons.FlowDirection = 'RightToLeft'; $buttons.WrapContents = $false
    foreach ($item in @(@('update', 'Update now'), @('skip', 'Skip this release'), @('disable', 'Disable update checks'))) {
        $button = New-Object Windows.Forms.Button
        $button.Name = $item[0]; $button.Tag = $item[0]; $button.Text = $item[1]
        $button.AutoSize = $true; $button.MinimumSize = New-Object Drawing.Size(145, 36)
        $button.Add_Click({ param($sender, $eventArgs) $window = $sender.FindForm(); $window.Tag = $sender.Tag; $window.Close() })
        $buttons.Controls.Add($button)
    }
    $layout.Controls.Add($buttons, 0, 3); $form.Controls.Add($layout)
    $form.Add_Shown({ param($sender, $eventArgs) $sender.Controls.Find('PatchNotes', $true)[0].Select(0, 0) })
    return $form
}

function Show-DualStickUpdateChoice($Installed, $Release) {
    $form = New-DualStickUpdateDialog $Installed $Release
    try { $form.ShowDialog() | Out-Null; return [string]$form.Tag }
    finally { $form.Dispose() }
}

function Show-DualStickUpdateNotice([string]$Text, [switch]$Failed) {
    $title = if ($Failed) { 'Dual Stick update skipped' } else { 'Dual Stick updated' }
    $icon = if ($Failed) { [Windows.Forms.MessageBoxIcon]::Warning } else { [Windows.Forms.MessageBoxIcon]::Information }
    [Windows.Forms.MessageBox]::Show($Text, $title, [Windows.Forms.MessageBoxButtons]::OK, $icon) | Out-Null
}
