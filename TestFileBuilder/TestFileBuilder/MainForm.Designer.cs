namespace TestFileBuilder
{
    partial class MainForm
    {
        private System.ComponentModel.IContainer components = null;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
                components.Dispose();
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            components = new System.ComponentModel.Container();

            SuspendLayout();

            // ── Menu Strip ─────────────────────────────────────────────────────
            menuStrip = new MenuStrip();
            
            // File Menu
            fileMenu = new ToolStripMenuItem();
            fileMenu.Text = "&File";
            
            newToolStripMenuItem = new ToolStripMenuItem();
            newToolStripMenuItem.Text = "&New";
            newToolStripMenuItem.ShortcutKeys = Keys.Control | Keys.N;
            newToolStripMenuItem.Click += newToolStripMenuItem_Click;
            
            openToolStripMenuItem = new ToolStripMenuItem();
            openToolStripMenuItem.Text = "&Open...";
            openToolStripMenuItem.ShortcutKeys = Keys.Control | Keys.O;
            openToolStripMenuItem.Click += openToolStripMenuItem_Click;
            
            saveToolStripMenuItem = new ToolStripMenuItem();
            saveToolStripMenuItem.Text = "&Save";
            saveToolStripMenuItem.ShortcutKeys = Keys.Control | Keys.S;
            saveToolStripMenuItem.Click += saveToolStripMenuItem_Click;
            
            saveAsToolStripMenuItem = new ToolStripMenuItem();
            saveAsToolStripMenuItem.Text = "Save &As...";
            saveAsToolStripMenuItem.ShortcutKeys = Keys.Control | Keys.Shift | Keys.S;
            saveAsToolStripMenuItem.Click += saveAsToolStripMenuItem_Click;
            
            exitToolStripMenuItem = new ToolStripMenuItem();
            exitToolStripMenuItem.Text = "E&xit";
            exitToolStripMenuItem.Click += exitToolStripMenuItem_Click;
            
            fileMenu.DropDownItems.Add(newToolStripMenuItem);
            fileMenu.DropDownItems.Add(openToolStripMenuItem);
            fileMenu.DropDownItems.Add(new ToolStripSeparator());
            fileMenu.DropDownItems.Add(saveToolStripMenuItem);
            fileMenu.DropDownItems.Add(saveAsToolStripMenuItem);
            fileMenu.DropDownItems.Add(new ToolStripSeparator());
            fileMenu.DropDownItems.Add(exitToolStripMenuItem);
            
            // View Menu
            viewMenu = new ToolStripMenuItem();
            viewMenu.Text = "&View";
            
            previewToolStripMenuItem = new ToolStripMenuItem();
            previewToolStripMenuItem.Text = "&JSON Preview...";
            previewToolStripMenuItem.ShortcutKeys = Keys.Control | Keys.P;
            previewToolStripMenuItem.Click += previewToolStripMenuItem_Click;
            
            viewMenu.DropDownItems.Add(previewToolStripMenuItem);
            
            // Help Menu
            helpMenu = new ToolStripMenuItem();
            helpMenu.Text = "&Help";
            
            patternHelpToolStripMenuItem = new ToolStripMenuItem();
            patternHelpToolStripMenuItem.Text = "&Pattern Reference...";
            patternHelpToolStripMenuItem.Click += patternHelpToolStripMenuItem_Click;
            
            helpMenu.DropDownItems.Add(patternHelpToolStripMenuItem);
            
            // Add menus to menu strip
            menuStrip.Items.Add(fileMenu);
            menuStrip.Items.Add(viewMenu);
            menuStrip.Items.Add(helpMenu);
            menuStrip.Dock = DockStyle.Top;

            // ── Status Strip ───────────────────────────────────────────────────
            statusStrip = new StatusStrip();
            lblStatus = new ToolStripStatusLabel("Ready");
            lblStatus.Spring = true;
            lblStatus.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            lblEntryCount = new ToolStripStatusLabel("0 tests");
            statusStrip.Items.Add(lblStatus);
            statusStrip.Items.Add(lblEntryCount);
            statusStrip.Dock = DockStyle.Bottom;

            // ── Main SplitContainer ────────────────────────────────────────────
            splitMain = new SplitContainer();
            splitMain.Dock = DockStyle.Fill;
            splitMain.Orientation = Orientation.Vertical;

            // ══ LEFT PANEL ════════════════════════════════════════════════════

            pnlListButtons = new Panel();
            pnlListButtons.Dock = DockStyle.Bottom;
            pnlListButtons.Height = 34;

            btnAdd = new Button();
            btnAdd.Text = "Add";
            btnAdd.Width = 50;
            btnAdd.Height = 26;
            btnAdd.Location = new Point(2, 4);
            btnAdd.Click += btnAdd_Click;

            btnRemove = new Button();
            btnRemove.Text = "Remove";
            btnRemove.Width = 58;
            btnRemove.Height = 26;
            btnRemove.Location = new Point(56, 4);
            btnRemove.Click += btnRemove_Click;

            btnDuplicate = new Button();
            btnDuplicate.Text = "Copy";
            btnDuplicate.Width = 48;
            btnDuplicate.Height = 26;
            btnDuplicate.Location = new Point(118, 4);
            btnDuplicate.Click += btnDuplicate_Click;

            btnMoveUp = new Button();
            btnMoveUp.Text = "\u25B2";
            btnMoveUp.Width = 30;
            btnMoveUp.Height = 26;
            btnMoveUp.Location = new Point(170, 4);
            btnMoveUp.Click += btnMoveUp_Click;

            btnMoveDown = new Button();
            btnMoveDown.Text = "\u25BC";
            btnMoveDown.Width = 30;
            btnMoveDown.Height = 26;
            btnMoveDown.Location = new Point(204, 4);
            btnMoveDown.Click += btnMoveDown_Click;

            pnlListButtons.Controls.Add(btnAdd);
            pnlListButtons.Controls.Add(btnRemove);
            pnlListButtons.Controls.Add(btnDuplicate);
            pnlListButtons.Controls.Add(btnMoveUp);
            pnlListButtons.Controls.Add(btnMoveDown);

            lblTestList = new Label();
            lblTestList.Text = "Tests";
            lblTestList.Font = new System.Drawing.Font("Segoe UI", 9f, System.Drawing.FontStyle.Bold);
            lblTestList.Dock = DockStyle.Top;
            lblTestList.Height = 22;
            lblTestList.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            lblTestList.Padding = new Padding(4, 0, 0, 0);

            lstTests = new ListBox();
            lstTests.Dock = DockStyle.Fill;
            lstTests.Font = new System.Drawing.Font("Consolas", 8.5f);
            lstTests.IntegralHeight = false;
            lstTests.HorizontalScrollbar = true;
            lstTests.SelectedIndexChanged += lstTests_SelectedIndexChanged;

            // Dock order: Bottom first, then Top, then Fill
            splitMain.Panel1.Controls.Add(lstTests);
            splitMain.Panel1.Controls.Add(pnlListButtons);
            splitMain.Panel1.Controls.Add(lblTestList);

            // ══ RIGHT PANEL ═══════════════════════════════════════════════════
            // TableLayoutPanel gives each row a fixed or percentage height
            pnlEditor = new TableLayoutPanel();
            pnlEditor.Dock = DockStyle.Fill;
            pnlEditor.ColumnCount = 1;
            pnlEditor.RowCount = 4;
            pnlEditor.RowStyles.Add(new RowStyle(SizeType.Absolute, 90F));
            pnlEditor.RowStyles.Add(new RowStyle(SizeType.Absolute, 56F));
            pnlEditor.RowStyles.Add(new RowStyle(SizeType.Absolute, 90F));
            pnlEditor.RowStyles.Add(new RowStyle(SizeType.Percent, 100F));
            pnlEditor.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100F));
            pnlEditor.Padding = new Padding(4);
            pnlEditor.Margin = new Padding(0);

            // ── Command Group ──────────────────────────────────────────────────
            grpCommand = new GroupBox();
            grpCommand.Text = "Command";
            grpCommand.Dock = DockStyle.Fill;
            grpCommand.Margin = new Padding(0, 0, 0, 2);
            grpCommand.Padding = new Padding(8, 2, 8, 2);

            lblCommand = new Label();
            lblCommand.Text = "Command:";
            lblCommand.AutoSize = true;
            lblCommand.Location = new Point(8, 24);

            cmbCommand = new ComboBox();
            cmbCommand.Location = new Point(90, 20);
            cmbCommand.Width = 280;
            cmbCommand.DropDownStyle = ComboBoxStyle.DropDownList;
            cmbCommand.AutoCompleteSource = AutoCompleteSource.ListItems;
            cmbCommand.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
            cmbCommand.Anchor = AnchorStyles.Left | AnchorStyles.Top;
            cmbCommand.SelectedIndexChanged += cmbCommand_SelectedIndexChanged;

            lblDevice = new Label();
            lblDevice.Text = "Device prefix:";
            lblDevice.AutoSize = true;
            lblDevice.Location = new Point(8, 56);

            txtDevice = new TextBox();
            txtDevice.Location = new Point(90, 53);
            txtDevice.Width = 160;
            txtDevice.PlaceholderText = "(optional, e.g. apetime)";
            txtDevice.TextChanged += txtDevice_TextChanged;

            lblDeviceHint = new Label();
            lblDeviceHint.Text = "Optional - adds a \"device\" property to the output.";
            lblDeviceHint.AutoSize = true;
            lblDeviceHint.Location = new Point(258, 56);
            lblDeviceHint.ForeColor = System.Drawing.Color.Gray;
            lblDeviceHint.Font = new System.Drawing.Font("Segoe UI", 7.5f);

            grpCommand.Controls.Add(lblCommand);
            grpCommand.Controls.Add(cmbCommand);
            grpCommand.Controls.Add(lblDevice);
            grpCommand.Controls.Add(txtDevice);
            grpCommand.Controls.Add(lblDeviceHint);

            // ── Flags Group ────────────────────────────────────────────────────
            grpFlags = new GroupBox();
            grpFlags.Text = "Flags";
            grpFlags.Dock = DockStyle.Fill;
            grpFlags.Margin = new Padding(0, 0, 0, 2);
            grpFlags.Padding = new Padding(8, 2, 8, 2);

            chkWarnOnly = new CheckBox();
            chkWarnOnly.Text = "warnOnly";
            chkWarnOnly.AutoSize = true;
            chkWarnOnly.Location = new Point(8, 22);
            chkWarnOnly.CheckedChanged += chkWarnOnly_CheckedChanged;

            chkErrorExpected = new CheckBox();
            chkErrorExpected.Text = "errorExpected";
            chkErrorExpected.AutoSize = true;
            chkErrorExpected.Location = new Point(110, 22);
            chkErrorExpected.CheckedChanged += chkErrorExpected_CheckedChanged;

            grpFlags.Controls.Add(chkWarnOnly);
            grpFlags.Controls.Add(chkErrorExpected);

            // ── Reply Group ────────────────────────────────────────────────────
            grpReply = new GroupBox();
            grpReply.Text = "Reply";
            grpReply.Dock = DockStyle.Fill;
            grpReply.Margin = new Padding(0, 0, 0, 2);
            grpReply.Padding = new Padding(8, 2, 8, 2);

            chkHasReply = new CheckBox();
            chkHasReply.Text = "Has Reply";
            chkHasReply.AutoSize = true;
            chkHasReply.Location = new Point(8, 22);
            chkHasReply.CheckedChanged += chkHasReply_CheckedChanged;

            lblReplyLength = new Label();
            lblReplyLength.Text = "replyLength:";
            lblReplyLength.AutoSize = true;
            lblReplyLength.Location = new Point(8, 54);
            lblReplyLength.Enabled = false;

            numReplyLength = new NumericUpDown();
            numReplyLength.Location = new Point(90, 51);
            numReplyLength.Width = 80;
            numReplyLength.Minimum = 0;
            numReplyLength.Maximum = 65535;
            numReplyLength.Value = 0;
            numReplyLength.Enabled = false;
            numReplyLength.ValueChanged += numReplyLength_ValueChanged;

            lblExpected = new Label();
            lblExpected.Text = "expected:";
            lblExpected.AutoSize = true;
            lblExpected.Location = new Point(185, 54);
            lblExpected.Enabled = false;

            txtExpected = new TextBox();
            txtExpected.Location = new Point(252, 51);
            txtExpected.Width = 200;
            txtExpected.Anchor = AnchorStyles.Left | AnchorStyles.Top;
            txtExpected.PlaceholderText = "literal or pattern (? # @ %)";
            txtExpected.Enabled = false;
            txtExpected.TextChanged += txtExpected_TextChanged;

            grpReply.Controls.Add(chkHasReply);
            grpReply.Controls.Add(lblReplyLength);
            grpReply.Controls.Add(numReplyLength);
            grpReply.Controls.Add(lblExpected);
            grpReply.Controls.Add(txtExpected);

            // ── Args Group ─────────────────────────────────────────────────────
            grpArgs = new GroupBox();
            grpArgs.Text = "Arguments";
            grpArgs.Dock = DockStyle.Fill;
            grpArgs.Margin = new Padding(0);
            grpArgs.Padding = new Padding(8, 4, 8, 4);

            // Add groups to TableLayoutPanel rows
            pnlEditor.Controls.Add(grpCommand, 0, 0);
            pnlEditor.Controls.Add(grpFlags, 0, 1);
            pnlEditor.Controls.Add(grpReply, 0, 2);
            pnlEditor.Controls.Add(grpArgs, 0, 3);

            splitMain.Panel2.Controls.Add(pnlEditor);

            // ── Add to Form ────────────────────────────────────────────────────
            // WinForms processes docked controls in reverse Controls order.
            // Add Fill last so menu (Top) and status (Bottom) are claimed first.
            this.Controls.Add(splitMain);   // Fill - added first, processed last = fills remaining space
            this.Controls.Add(statusStrip); // Bottom
            this.Controls.Add(menuStrip);   // Top

            this.MainMenuStrip = menuStrip;
            this.Text = "FujiNet Test JSON Builder";
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.MinimumSize = new Size(820, 580);
            this.Size = new Size(1050, 700);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.Load += new EventHandler(MainForm_Load);
            this.Shown += new EventHandler(MainForm_Shown);

            ResumeLayout(false);
            PerformLayout();
        }

        // ── Controls ──────────────────────────────────────────────────────────
        private MenuStrip menuStrip = null!;
        private ToolStripMenuItem fileMenu = null!, viewMenu = null!, helpMenu = null!;
        private ToolStripMenuItem newToolStripMenuItem = null!, openToolStripMenuItem = null!;
        private ToolStripMenuItem saveToolStripMenuItem = null!, saveAsToolStripMenuItem = null!;
        private ToolStripMenuItem exitToolStripMenuItem = null!;
        private ToolStripMenuItem previewToolStripMenuItem = null!;
        private ToolStripMenuItem patternHelpToolStripMenuItem = null!;

        private SplitContainer splitMain = null!;
        private Label lblTestList = null!;
        private ListBox lstTests = null!;
        private Panel pnlListButtons = null!;
        private Button btnAdd = null!, btnRemove = null!, btnDuplicate = null!, btnMoveUp = null!, btnMoveDown = null!;

        private TableLayoutPanel pnlEditor = null!;
        private GroupBox grpCommand = null!, grpFlags = null!, grpReply = null!, grpArgs = null!;

        private Label lblCommand = null!, lblDevice = null!, lblDeviceHint = null!;
        private ComboBox cmbCommand = null!;
        private TextBox txtDevice = null!;

        private CheckBox chkWarnOnly = null!, chkErrorExpected = null!, chkHasReply = null!;

        private Label lblReplyLength = null!, lblExpected = null!;
        private NumericUpDown numReplyLength = null!;
        private TextBox txtExpected = null!;

        private StatusStrip statusStrip = null!;
        private ToolStripStatusLabel lblStatus = null!, lblEntryCount = null!;
    }
}
