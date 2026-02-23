namespace TestFileBuilder
{
    /// <summary>
    /// A read-only dialog that shows the current JSON output of the test file.
    /// </summary>
    public class JsonPreviewForm : Form
    {
        private readonly RichTextBox _rtb;

        public JsonPreviewForm(string json)
        {
            Text = "JSON Preview";
            Size = new Size(700, 580);
            StartPosition = FormStartPosition.CenterParent;
            MinimumSize = new Size(400, 300);
            
            // Load embedded icon
            var iconStream = System.Reflection.Assembly.GetExecutingAssembly()
                .GetManifestResourceStream("TestFileBuilder.fujinet.ico");
            if (iconStream != null)
                Icon = new System.Drawing.Icon(iconStream);

            _rtb = new RichTextBox
            {
                Dock = DockStyle.Fill,
                ReadOnly = true,
                Font = new Font("Consolas", 9.5f),
                BackColor = Color.FromArgb(30, 30, 30),
                ForeColor = Color.FromArgb(220, 220, 220),
                WordWrap = false,
                ScrollBars = RichTextBoxScrollBars.Both,
                Text = json
            };

            var pnl = new Panel { Dock = DockStyle.Bottom, Height = 40, Padding = new Padding(8, 6, 8, 6) };
            var btnCopy = new Button { Text = "Copy to Clipboard", AutoSize = true, Location = new Point(8, 6) };
            var btnClose = new Button { Text = "Close", AutoSize = true, Anchor = AnchorStyles.Right | AnchorStyles.Bottom };
            btnClose.Location = new Point(pnl.Width - 90, 6);
            btnClose.Anchor = AnchorStyles.Right | AnchorStyles.Top;

            btnCopy.Click += (_, _) =>
            {
                Clipboard.SetText(_rtb.Text);
                MessageBox.Show("Copied to clipboard.", "Copied", MessageBoxButtons.OK, MessageBoxIcon.Information);
            };
            btnClose.Click += (_, _) => Close();
            pnl.Controls.AddRange(new Control[] { btnCopy, btnClose });

            Controls.Add(_rtb);
            Controls.Add(pnl);
        }
    }
}
