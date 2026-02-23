using TestFileBuilder.Models;
using TestFileBuilder.Services;

namespace TestFileBuilder
{
    public partial class MainForm : Form
    {
        // ── State ─────────────────────────────────────────────────────────────
        private List<CommandDefinition> _commands = new();
        private List<TestEntry> _entries = new();
        private TestEntry? _currentEntry;
        private string? _currentFilePath;
        private bool _isDirty;
        private bool _suppressEvents;

        // ── Constructor ───────────────────────────────────────────────────────
        public MainForm()
        {
            InitializeComponent();
            LoadCommands();
            UpdateTitle();
            UpdateButtonStates();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            // Set splitter constraints after form is sized
            splitMain.Panel1MinSize = 200;
            splitMain.Panel2MinSize = 300;
            
            // Load embedded icon
            var iconStream = System.Reflection.Assembly.GetExecutingAssembly()
                .GetManifestResourceStream("TestFileBuilder.fujinet.ico");
            if (iconStream != null)
                this.Icon = new System.Drawing.Icon(iconStream);
        }

        private void MainForm_Shown(object sender, EventArgs e)
        {
            // Form is now visible
        }

        // ── Command loading ───────────────────────────────────────────────────
        private void LoadCommands()
        {
            try
            {
                _commands = CommandLoader.Load();
                PopulateCommandCombo();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Failed to load commands.jsn",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void PopulateCommandCombo()
        {
            cmbCommand.Items.Clear();
            foreach (var cmd in _commands)
                cmbCommand.Items.Add(cmd);
            cmbCommand.DisplayMember = "DisplayName";
        }

        // ── Title management ─────────────────────────────────────────────────
        private void UpdateTitle()
        {
            string file = _currentFilePath != null
                ? Path.GetFileName(_currentFilePath)
                : "(untitled)";
            string dirty = _isDirty ? " *" : "";
            this.Text = $"FujiNet Test JSON Builder – {file}{dirty}";
        }

        private void MarkDirty()
        {
            _isDirty = true;
            UpdateTitle();
        }

        // ── Test list management ──────────────────────────────────────────────
        private void RefreshTestList()
        {
            lstTests.BeginUpdate();
            lstTests.Items.Clear();
            foreach (var e in _entries)
                lstTests.Items.Add(e);
            lstTests.DisplayMember = "";
            lstTests.EndUpdate();

            // Restore selection
            if (_currentEntry != null)
            {
                int idx = _entries.IndexOf(_currentEntry);
                if (idx >= 0)
                    lstTests.SelectedIndex = idx;
            }

            UpdateButtonStates();
            UpdateEntryCount();
        }

        private void UpdateButtonStates()
        {
            bool hasSelection = lstTests.SelectedIndex >= 0;
            bool hasEntries = _entries.Count > 0;
            int idx = lstTests.SelectedIndex;

            btnRemove.Enabled = hasSelection;
            btnDuplicate.Enabled = hasSelection;
            btnMoveUp.Enabled = hasSelection && idx > 0;
            btnMoveDown.Enabled = hasSelection && idx < _entries.Count - 1;
        }

        // ── "Add Test" button ─────────────────────────────────────────────────
        private void btnAdd_Click(object sender, EventArgs e)
        {
            // Commit current editing first
            CommitCurrentEntry();

            var entry = new TestEntry { Command = string.Empty };
            _entries.Add(entry);
            _currentEntry = entry;
            RefreshTestList();
            lstTests.SelectedIndex = _entries.Count - 1;
            MarkDirty();
            cmbCommand.Focus();
        }

        // ── "Duplicate" button ────────────────────────────────────────────────
        private void btnDuplicate_Click(object sender, EventArgs e)
        {
            if (_currentEntry == null) return;
            CommitCurrentEntry();

            // Deep-copy via JSON round-trip
            var json = _currentEntry.ToJObject().ToString();
            var copy = TestEntry.FromJObject(
                Newtonsoft.Json.Linq.JObject.Parse(json), _commands);
            copy.Definition = _currentEntry.Definition;

            int idx = _entries.IndexOf(_currentEntry);
            _entries.Insert(idx + 1, copy);
            _currentEntry = copy;
            RefreshTestList();
            lstTests.SelectedIndex = idx + 1;
            MarkDirty();
        }

        // ── "Remove" button ───────────────────────────────────────────────────
        private void btnRemove_Click(object sender, EventArgs e)
        {
            if (_currentEntry == null) return;
            int idx = _entries.IndexOf(_currentEntry);
            _entries.Remove(_currentEntry);
            _currentEntry = null;

            RefreshTestList();
            if (_entries.Count > 0)
                lstTests.SelectedIndex = Math.Min(idx, _entries.Count - 1);

            MarkDirty();
        }

        // ── Move Up / Down ────────────────────────────────────────────────────
        private void btnMoveUp_Click(object sender, EventArgs e)
        {
            int idx = lstTests.SelectedIndex;
            if (idx <= 0) return;
            CommitCurrentEntry();
            (_entries[idx], _entries[idx - 1]) = (_entries[idx - 1], _entries[idx]);
            RefreshTestList();
            lstTests.SelectedIndex = idx - 1;
            MarkDirty();
        }

        private void btnMoveDown_Click(object sender, EventArgs e)
        {
            int idx = lstTests.SelectedIndex;
            if (idx < 0 || idx >= _entries.Count - 1) return;
            CommitCurrentEntry();
            (_entries[idx], _entries[idx + 1]) = (_entries[idx + 1], _entries[idx]);
            RefreshTestList();
            lstTests.SelectedIndex = idx + 1;
            MarkDirty();
        }

        // ── List selection changed ────────────────────────────────────────────
        private void lstTests_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (_suppressEvents) return;
            if (lstTests.SelectedIndex < 0)
            {
                _currentEntry = null;
                ClearEditor();
                UpdateButtonStates();
                return;
            }

            _currentEntry = _entries[lstTests.SelectedIndex];
            PopulateEditor(_currentEntry);
            UpdateButtonStates();
        }

        // ── Editor population ─────────────────────────────────────────────────
        private void ClearEditor()
        {
            _suppressEvents = true;
            cmbCommand.SelectedIndex = -1;
            txtDevice.Text = string.Empty;
            chkWarnOnly.Checked = false;
            chkErrorExpected.Checked = false;
            chkHasReply.Checked = false;
            numReplyLength.Value = 0;
            txtExpected.Text = string.Empty;
            grpArgs.Controls.Clear();
            SetReplyControlsEnabled(false);
            _suppressEvents = false;
        }

        private void PopulateEditor(TestEntry entry)
        {
            _suppressEvents = true;

            // Command
            var def = entry.Definition ?? _commands.FirstOrDefault(c =>
                string.Equals(c.CommandKey, entry.Command, StringComparison.OrdinalIgnoreCase));

            int cmdIdx = def != null ? _commands.IndexOf(def) : -1;
            cmbCommand.SelectedIndex = cmdIdx;

            // Device
            txtDevice.Text = entry.Device ?? string.Empty;

            // Flags
            chkWarnOnly.Checked = entry.WarnOnly == true;
            chkErrorExpected.Checked = entry.ErrorExpected == true;

            // Reply
            bool hasReply = def?.ParsedReply != null || entry.ReplyLength.HasValue;
            chkHasReply.Checked = hasReply;
            numReplyLength.Value = entry.ReplyLength.HasValue
                ? Math.Max(0, Math.Min(entry.ReplyLength.Value, (int)numReplyLength.Maximum))
                : 0;
            txtExpected.Text = entry.Expected ?? string.Empty;
            SetReplyControlsEnabled(hasReply);

            _suppressEvents = false;

            // Args grid (calls BuildArgsGrid which uses _suppressEvents internally)
            BuildArgsGrid(def, entry);
        }

        // ── Command combo changed ─────────────────────────────────────────────
        private void cmbCommand_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (_suppressEvents) return;
            
            // If no entry selected, user must click Add first
            if (_currentEntry == null)
            {
                _suppressEvents = true;
                cmbCommand.SelectedIndex = -1;
                _suppressEvents = false;
                MessageBox.Show("Please click 'Add' to create a new test entry first.", "No Test Selected",
                    MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }

            var def = cmbCommand.SelectedItem as CommandDefinition;
            _currentEntry.Definition = def;
            _currentEntry.Command = def?.CommandKey ?? string.Empty;

            // Reset args when command changes
            _currentEntry.ArgValues.Clear();

            // Auto-set reply from definition
            if (def?.ParsedReply != null)
            {
                chkHasReply.Checked = true;
                SetReplyControlsEnabled(true);
                int suggestedLength = def.ParsedReply.Size > 0 ? def.ParsedReply.Size : 0;
                numReplyLength.Value = Math.Min(suggestedLength, (int)numReplyLength.Maximum);
                _currentEntry.ReplyLength = (int)numReplyLength.Value;
            }
            else
            {
                chkHasReply.Checked = false;
                SetReplyControlsEnabled(false);
                numReplyLength.Value = 0;
                txtExpected.Clear();
                _currentEntry.ReplyLength = null;
                _currentEntry.Expected = null;
            }

            BuildArgsGrid(def, _currentEntry);
            UpdateTestListItem();
            MarkDirty();
        }

        // ── Args grid builder ─────────────────────────────────────────────────
        private void BuildArgsGrid(CommandDefinition? def, TestEntry entry)
        {
            grpArgs.SuspendLayout();
            grpArgs.Controls.Clear();

            if (def == null || def.ParsedArgs.Count == 0)
            {
                var lbl = new Label
                {
                    Text = "This command has no arguments.",
                    ForeColor = Color.Gray,
                    AutoSize = true,
                    Location = new Point(8, 22)
                };
                grpArgs.Controls.Add(lbl);
                grpArgs.ResumeLayout();
                return;
            }

            // Build a flat list of (fieldName, typeChar, descriptor) for every arg/field
            var fields = new List<(string name, ArgDescriptor desc, string groupLabel)>();

            foreach (var arg in def.ParsedArgs)
            {
                if (arg.IsStruct)
                {
                    foreach (var field in arg.StructFields)
                        fields.Add((field.Name, field, "struct"));
                }
                else
                {
                    fields.Add((arg.Name, arg, ""));
                }
            }

            // Create a DataGridView for easy tabbing
            var grid = new DataGridView
            {
                Dock = DockStyle.Fill,
                AllowUserToAddRows = false,
                AllowUserToDeleteRows = false,
                AllowUserToResizeRows = false,
                RowHeadersVisible = false,
                AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode.Fill,
                SelectionMode = DataGridViewSelectionMode.CellSelect,
                EditMode = DataGridViewEditMode.EditOnKeystrokeOrF2,
                BackgroundColor = SystemColors.Window,
                BorderStyle = BorderStyle.None,
                ColumnHeadersHeightSizeMode = DataGridViewColumnHeadersHeightSizeMode.AutoSize
            };

            // Columns: Argument | Type | Value
            grid.Columns.Add(new DataGridViewTextBoxColumn
            {
                HeaderText = "Argument",
                Name = "colName",
                ReadOnly = true,
                FillWeight = 35
            });
            grid.Columns.Add(new DataGridViewTextBoxColumn
            {
                HeaderText = "Type",
                Name = "colType",
                ReadOnly = true,
                FillWeight = 20
            });
            grid.Columns.Add(new DataGridViewTextBoxColumn
            {
                HeaderText = "Value",
                Name = "colValue",
                ReadOnly = false,
                FillWeight = 45
            });

            // Boolean args get a CheckBox column override
            foreach (var (name, desc, _) in fields)
            {
                DataGridViewRow row;
                if (desc.TypeChar == ArgDescriptor.TYPE_BOOL)
                {
                    // Use a ComboBox cell for booleans
                    var nameCell = new DataGridViewTextBoxCell { Value = name };
                    var typeCell = new DataGridViewTextBoxCell { Value = "bool" };
                    var combo = new DataGridViewComboBoxCell();
                    combo.Items.AddRange(new object[] { "true", "false" });
                    string boolVal = entry.ArgValues.TryGetValue(name, out var bv) ? bv.ToLower() : "false";
                    combo.Value = boolVal == "true" || boolVal == "1" ? "true" : "false";

                    row = new DataGridViewRow();
                    row.Cells.Add(nameCell);
                    row.Cells.Add(typeCell);
                    row.Cells.Add(combo);
                }
                else
                {
                    string existingVal = entry.ArgValues.TryGetValue(name, out var ev) ? ev : "";
                    row = new DataGridViewRow();
                    row.Cells.Add(new DataGridViewTextBoxCell { Value = name });
                    row.Cells.Add(new DataGridViewTextBoxCell { Value = desc.TypeLabel });
                    row.Cells.Add(new DataGridViewTextBoxCell { Value = existingVal });
                }

                row.Tag = name; // store arg name for lookup
                row.Cells[0].Style.BackColor = SystemColors.Control;
                row.Cells[1].Style.BackColor = SystemColors.Control;
                row.Cells[1].Style.ForeColor = Color.FromArgb(0, 100, 160);
                grid.Rows.Add(row);
            }

            grid.CellValueChanged += (s, ev) =>
            {
                if (_currentEntry == null || ev.RowIndex < 0) return;
                var r = grid.Rows[ev.RowIndex];
                string argName = r.Tag?.ToString() ?? "";
                string val = grid.Rows[ev.RowIndex].Cells[2].Value?.ToString() ?? "";
                _currentEntry.ArgValues[argName] = val;
                UpdateTestListItem();
                MarkDirty();
            };

            // Commit cell edits when moving away
            grid.CurrentCellDirtyStateChanged += (s, ev) =>
            {
                if (grid.IsCurrentCellDirty)
                    grid.CommitEdit(DataGridViewDataErrorContexts.Commit);
            };

            grpArgs.Controls.Add(grid);
            grpArgs.ResumeLayout();
        }

        // ── Reply group controls ──────────────────────────────────────────────
        private void SetReplyControlsEnabled(bool enabled)
        {
            numReplyLength.Enabled = enabled;
            txtExpected.Enabled = enabled;
            lblReplyLength.Enabled = enabled;
            lblExpected.Enabled = enabled;
        }

        private void chkHasReply_CheckedChanged(object sender, EventArgs e)
        {
            if (_suppressEvents || _currentEntry == null) return;
            bool enabled = chkHasReply.Checked;
            SetReplyControlsEnabled(enabled);
            if (!enabled)
            {
                _currentEntry.ReplyLength = null;
                _currentEntry.Expected = null;
                numReplyLength.Value = 0;
                txtExpected.Clear();
            }
            MarkDirty();
        }

        private void numReplyLength_ValueChanged(object sender, EventArgs e)
        {
            if (_suppressEvents || _currentEntry == null) return;
            _currentEntry.ReplyLength = chkHasReply.Checked ? (int?)numReplyLength.Value : null;
            UpdateTestListItem();
            MarkDirty();
        }

        private void txtExpected_TextChanged(object sender, EventArgs e)
        {
            if (_suppressEvents || _currentEntry == null) return;
            _currentEntry.Expected = string.IsNullOrEmpty(txtExpected.Text) ? null : txtExpected.Text;
            MarkDirty();
        }

        // ── Flag checkboxes ───────────────────────────────────────────────────
        private void chkWarnOnly_CheckedChanged(object sender, EventArgs e)
        {
            if (_suppressEvents || _currentEntry == null) return;
            _currentEntry.WarnOnly = chkWarnOnly.Checked ? true : (bool?)null;
            MarkDirty();
        }

        private void chkErrorExpected_CheckedChanged(object sender, EventArgs e)
        {
            if (_suppressEvents || _currentEntry == null) return;
            _currentEntry.ErrorExpected = chkErrorExpected.Checked ? true : (bool?)null;
            MarkDirty();
        }

        private void txtDevice_TextChanged(object sender, EventArgs e)
        {
            if (_suppressEvents || _currentEntry == null) return;
            _currentEntry.Device = string.IsNullOrWhiteSpace(txtDevice.Text) ? null : txtDevice.Text.Trim();
            UpdateTestListItem();
            MarkDirty();
        }

        // ── Helpers ───────────────────────────────────────────────────────────
        private void CommitCurrentEntry()
        {
            // Force any in-progress grid edits to commit
            var grid = grpArgs.Controls.OfType<DataGridView>().FirstOrDefault();
            grid?.EndEdit();
        }

        private void UpdateTestListItem()
        {
            if (_currentEntry == null) return;
            int idx = _entries.IndexOf(_currentEntry);
            if (idx < 0 || idx >= lstTests.Items.Count) return;

            // Force ListBox to refresh the display text for this item
            _suppressEvents = true;
            lstTests.BeginUpdate();
            // Replace with itself triggers a redraw of the item text
            lstTests.Items[idx] = _currentEntry;
            lstTests.SelectedIndex = idx;
            lstTests.EndUpdate();
            _suppressEvents = false;

            UpdateEntryCount();
        }

        private void UpdateEntryCount()
        {
            lblEntryCount.Text = $"{_entries.Count} test{(_entries.Count == 1 ? "" : "s")}";
        }

        // ── File menu: New ────────────────────────────────────────────────────
        private void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (!ConfirmDiscardChanges()) return;
            _entries.Clear();
            _currentEntry = null;
            _currentFilePath = null;
            _isDirty = false;
            ClearEditor();
            RefreshTestList();
            UpdateTitle();
        }

        // ── File menu: Open ───────────────────────────────────────────────────
        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (!ConfirmDiscardChanges()) return;

            using var dlg = new OpenFileDialog
            {
                Title = "Open Test File",
                Filter = "Test Files (*.tst)|*.tst|All Files (*.*)|*.*",
                DefaultExt = "tst"
            };

            if (dlg.ShowDialog() != DialogResult.OK) return;

            try
            {
                var loaded = TestFileService.Load(dlg.FileName, _commands);
                _entries = loaded;
                _currentEntry = null;
                _currentFilePath = dlg.FileName;
                _isDirty = false;
                ClearEditor();
                RefreshTestList();
                UpdateTitle();

                if (_entries.Count > 0)
                {
                    lstTests.SelectedIndex = 0;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Failed to open file:\n{ex.Message}", "Open Error",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        // ── File menu: Save ───────────────────────────────────────────────────
        private void saveToolStripMenuItem_Click(object sender, EventArgs e) => SaveFile(false);

        // ── File menu: Save As ────────────────────────────────────────────────
        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e) => SaveFile(true);

        private void SaveFile(bool forceDialog)
        {
            CommitCurrentEntry();

            // Validate
            var validation = ValidationService.ValidateAll(_entries);
            if (!validation.IsValid)
            {
                string msg = "The following errors must be fixed before saving:\n\n"
                    + string.Join("\n", validation.Errors.Select(e => "• " + e));
                MessageBox.Show(msg, "Validation Errors", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (validation.Warnings.Count > 0)
            {
                string msg = "The following warnings were found:\n\n"
                    + string.Join("\n", validation.Warnings.Select(w => "• " + w))
                    + "\n\nDo you want to save anyway?";
                if (MessageBox.Show(msg, "Validation Warnings", MessageBoxButtons.YesNo,
                        MessageBoxIcon.Warning) != DialogResult.Yes)
                    return;
            }

            if (forceDialog || _currentFilePath == null)
            {
                string initialName = _currentFilePath != null
                    ? Path.GetFileNameWithoutExtension(_currentFilePath)
                    : "tests";

                using var dlg = new SaveFileDialog
                {
                    Title = "Save Test File",
                    Filter = "Test Files (*.tst)|*.tst|All Files (*.*)|*.*",
                    DefaultExt = "tst",
                    FileName = initialName
                };

                if (dlg.ShowDialog() != DialogResult.OK) return;

                // Validate the filename (≤8 chars)
                string baseName = Path.GetFileNameWithoutExtension(dlg.FileName);
                var fnValidation = ValidationService.ValidateFilename(baseName);
                if (!fnValidation.IsValid)
                {
                    string errMsg = string.Join("\n", fnValidation.Errors.Select(err => "• " + err));
                    MessageBox.Show($"Invalid filename:\n\n{errMsg}", "Filename Error",
                        MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                _currentFilePath = TestFileService.NormalisePath(dlg.FileName);
            }

            try
            {
                TestFileService.Save(_currentFilePath, _entries);
                _isDirty = false;
                UpdateTitle();
                MessageBox.Show($"Saved to:\n{_currentFilePath}", "Saved",
                    MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Failed to save:\n{ex.Message}", "Save Error",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        // ── File menu: Exit ───────────────────────────────────────────────────
        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            if (!ConfirmDiscardChanges())
                e.Cancel = true;
            base.OnFormClosing(e);
        }

        private bool ConfirmDiscardChanges()
        {
            if (!_isDirty) return true;
            var result = MessageBox.Show(
                "You have unsaved changes. Discard them?",
                "Unsaved Changes",
                MessageBoxButtons.YesNoCancel,
                MessageBoxIcon.Question);
            return result == DialogResult.Yes;
        }

        // ── JSON Preview ──────────────────────────────────────────────────────
        private void previewToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CommitCurrentEntry();
            string json = TestFileService.Serialise(_entries);
            using var preview = new JsonPreviewForm(json);
            preview.ShowDialog(this);
        }

        // ── Help: Pattern reference ───────────────────────────────────────────
        private void patternHelpToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MessageBox.Show(
                "Pattern characters for the 'Expected' field:\n\n" +
                "  ?  = any single character\n" +
                "  #  = a single digit (0-9)\n" +
                "  @  = a single alphabetic character (A-Z, a-z)\n" +
                "  %  = a single alphanumeric character\n\n" +
                "All other characters are treated as literals.\n\n" +
                "Example:  %%%%%%%%-%%%%-%%%%-%%%%-%%%%%%%%%%%%\n" +
                "matches a UUID-style string.",
                "Expected Pattern Reference",
                MessageBoxButtons.OK,
                MessageBoxIcon.Information);
        }
    }
}
