using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using TestFileBuilder.Models;

namespace TestFileBuilder.Models
{
    /// <summary>
    /// Represents one test entry that will be serialized into the output .tst file.
    /// </summary>
    public class TestEntry
    {
        // ── Core ──────────────────────────────────────────────────────────────
        /// <summary>The command key (lower-snake-case name from CommandDefinition.CommandKey)</summary>
        public string Command { get; set; } = string.Empty;

        /// <summary>Optional device prefix (e.g. "apetime")</summary>
        public string? Device { get; set; }

        // ── Flags ─────────────────────────────────────────────────────────────
        public bool? WarnOnly { get; set; }
        public bool? ErrorExpected { get; set; }

        // ── Reply ─────────────────────────────────────────────────────────────
        /// <summary>Set when the associated command has a reply descriptor</summary>
        public int? ReplyLength { get; set; }
        public string? Expected { get; set; }

        // ── Args ──────────────────────────────────────────────────────────────
        /// <summary>
        /// Keyed by arg.Name, value is the string representation of whatever the user typed.
        /// For structs the key is the field name.
        /// </summary>
        public Dictionary<string, string> ArgValues { get; set; } = new();

        // ── Back-reference ────────────────────────────────────────────────────
        [JsonIgnore]
        public CommandDefinition? Definition { get; set; }

        // ── Display helpers ───────────────────────────────────────────────────
        public override string ToString()
        {
            var parts = new List<string> { Command.ToUpperInvariant() };
            if (!string.IsNullOrWhiteSpace(Device))
                parts.Insert(0, $"[{Device}]");
            if (ArgValues.Count > 0)
                parts.Add($"({string.Join(", ", ArgValues.Select(kv => $"{kv.Key}={kv.Value}"))})");
            return string.Join(" ", parts);
        }

        // ── Serialisation ─────────────────────────────────────────────────────

        /// <summary>
        /// Builds a JObject for this entry in the correct property order.
        /// </summary>
        public JObject ToJObject()
        {
            var obj = new JObject();

            // Optional device comes first in the sample output
            if (!string.IsNullOrWhiteSpace(Device))
                obj["device"] = Device;

            obj["command"] = Command;

            // Args (required fields matching the command definition)
            if (Definition != null)
            {
                foreach (var arg in Definition.ParsedArgs)
                {
                    if (arg.IsStruct)
                    {
                        foreach (var field in arg.StructFields)
                        {
                            if (ArgValues.TryGetValue(field.Name, out var val))
                                AppendTyped(obj, field.Name, val, field.TypeChar);
                        }
                    }
                    else
                    {
                        if (ArgValues.TryGetValue(arg.Name, out var val))
                            AppendTyped(obj, arg.Name, val, arg.TypeChar);
                    }
                }
            }

            // Reply length
            if (ReplyLength.HasValue)
                obj["replyLength"] = ReplyLength.Value;

            // Expected
            if (!string.IsNullOrEmpty(Expected))
                obj["expected"] = Expected;

            // Flags
            if (ErrorExpected.HasValue)
                obj["errorExpected"] = ErrorExpected.Value;

            if (WarnOnly.HasValue)
                obj["warnOnly"] = WarnOnly.Value;

            return obj;
        }

        private static void AppendTyped(JObject obj, string name, string value, char typeChar)
        {
            switch (typeChar)
            {
                case ArgDescriptor.TYPE_BOOL:
                    if (bool.TryParse(value, out bool b))
                        obj[name] = b;
                    else if (int.TryParse(value, out int bi))
                        obj[name] = bi != 0;
                    else
                        obj[name] = value;
                    break;

                case ArgDescriptor.TYPE_UNSIGNED:
                case ArgDescriptor.TYPE_SIGNED:
                    if (long.TryParse(value, out long n))
                        obj[name] = n;
                    else
                        obj[name] = value;
                    break;

                default:
                    obj[name] = value;
                    break;
            }
        }

        /// <summary>
        /// Creates a TestEntry from a JObject (for loading .tst files).
        /// </summary>
        public static TestEntry FromJObject(JObject obj, IEnumerable<CommandDefinition> definitions)
        {
            var entry = new TestEntry();

            if (obj.TryGetValue("device", out var dev))
                entry.Device = dev.ToString();

            if (obj.TryGetValue("command", out var cmd))
                entry.Command = cmd.ToString();

            // Resolve definition
            entry.Definition = definitions.FirstOrDefault(d =>
                string.Equals(d.CommandKey, entry.Command, StringComparison.OrdinalIgnoreCase));

            if (obj.TryGetValue("replyLength", out var rl) && rl.Type == JTokenType.Integer)
                entry.ReplyLength = (int)rl;

            if (obj.TryGetValue("expected", out var exp))
                entry.Expected = exp.ToString();

            if (obj.TryGetValue("warnOnly", out var wo))
                entry.WarnOnly = (bool)wo;

            if (obj.TryGetValue("errorExpected", out var ee))
                entry.ErrorExpected = (bool)ee;

            // Pull all remaining properties as arg values
            var knownKeys = new HashSet<string>(StringComparer.OrdinalIgnoreCase)
                { "device", "command", "replyLength", "expected", "warnOnly", "errorExpected" };

            foreach (var prop in obj.Properties())
            {
                if (!knownKeys.Contains(prop.Name))
                    entry.ArgValues[prop.Name] = prop.Value.ToString();
            }

            return entry;
        }
    }
}
