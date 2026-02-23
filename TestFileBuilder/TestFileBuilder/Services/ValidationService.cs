using TestFileBuilder.Models;

namespace TestFileBuilder.Services
{
    public class ValidationResult
    {
        public bool IsValid => Errors.Count == 0;
        public List<string> Errors { get; } = new();
        public List<string> Warnings { get; } = new();
    }

    public static class ValidationService
    {
        /// <summary>Validate the full list of test entries.</summary>
        public static ValidationResult ValidateAll(IEnumerable<TestEntry> entries)
        {
            var result = new ValidationResult();
            int index = 0;

            foreach (var entry in entries)
            {
                string prefix = $"Test #{index + 1} ({entry.Command})";
                ValidateEntry(entry, prefix, result);
                index++;
            }

            if (index == 0)
                result.Errors.Add("The test list is empty. Add at least one test before saving.");

            return result;
        }

        /// <summary>Validate a single test entry.</summary>
        public static ValidationResult ValidateSingle(TestEntry entry, string prefix = "This test")
        {
            var result = new ValidationResult();
            ValidateEntry(entry, prefix, result);
            return result;
        }

        private static void ValidateEntry(TestEntry entry, string prefix, ValidationResult result)
        {
            if (string.IsNullOrWhiteSpace(entry.Command))
            {
                result.Errors.Add($"{prefix}: Command must be selected.");
                return;
            }

            var def = entry.Definition;
            if (def == null)
            {
                result.Warnings.Add($"{prefix}: No matching definition found for command '{entry.Command}'. It will be written as-is.");
                return;
            }

            // Validate required args
            foreach (var arg in def.ParsedArgs)
            {
                if (arg.IsStruct)
                {
                    foreach (var field in arg.StructFields)
                        ValidateArgValue(entry, field, $"{prefix}, field '{field.Name}'", result);
                }
                else
                {
                    ValidateArgValue(entry, arg, $"{prefix}, arg '{arg.Name}'", result);
                }
            }

            // If command has a reply, replyLength should be set
            if (def.ParsedReply != null && !entry.ReplyLength.HasValue)
                result.Warnings.Add($"{prefix}: Command '{entry.Command}' has a reply but replyLength is not set.");

            // If replyLength is set but command has no reply definition, warn
            if (entry.ReplyLength.HasValue && def.ParsedReply == null)
                result.Warnings.Add($"{prefix}: replyLength is set but this command has no defined reply.");

            // Validate expected pattern characters
            if (!string.IsNullOrEmpty(entry.Expected))
                ValidateExpectedPattern(entry.Expected, prefix, result);
        }

        private static void ValidateArgValue(TestEntry entry, ArgDescriptor arg, string prefix, ValidationResult result)
        {
            if (!entry.ArgValues.TryGetValue(arg.Name, out var val) || string.IsNullOrWhiteSpace(val))
            {
                result.Errors.Add($"{prefix}: Value is required.");
                return;
            }

            switch (arg.TypeChar)
            {
                case ArgDescriptor.TYPE_BOOL:
                    if (!bool.TryParse(val, out _) && val != "0" && val != "1")
                        result.Errors.Add($"{prefix}: Expected a boolean value (true/false/0/1), got '{val}'.");
                    break;

                case ArgDescriptor.TYPE_UNSIGNED:
                    if (!ulong.TryParse(val, out _))
                        result.Errors.Add($"{prefix}: Expected an unsigned integer, got '{val}'.");
                    break;

                case ArgDescriptor.TYPE_SIGNED:
                    if (!long.TryParse(val, out _))
                        result.Errors.Add($"{prefix}: Expected a signed integer, got '{val}'.");
                    break;

                case ArgDescriptor.TYPE_FIXED_LEN:
                    if (arg.Size > 0 && val.Length > arg.Size)
                        result.Warnings.Add($"{prefix}: Value length {val.Length} exceeds defined max of {arg.Size}.");
                    break;

                case ArgDescriptor.TYPE_VAR_LEN:
                    // Variable length strings – no hard constraint, but warn on very long values
                    if (arg.Size > 0 && val.Length > arg.Size * 4)
                        result.Warnings.Add($"{prefix}: Value seems unusually long for a variable-length field.");
                    break;
            }
        }

        private static void ValidateExpectedPattern(string pattern, string prefix, ValidationResult result)
        {
            // Valid pattern chars: ? # @ % plus any literal character
            // This is informational only – no actual errors
            var validPatternChars = new HashSet<char> { '?', '#', '@', '%' };
            // No restriction needed, just confirm it's a non-empty string (already checked)
        }

        /// <summary>
        /// Validates that the proposed filename (without extension) is 8 chars or fewer
        /// and contains only valid file system characters.
        /// </summary>
        public static ValidationResult ValidateFilename(string name)
        {
            var result = new ValidationResult();

            if (string.IsNullOrWhiteSpace(name))
            {
                result.Errors.Add("Filename cannot be empty.");
                return result;
            }

            if (name.Length > 8)
                result.Errors.Add($"Filename '{name}' is {name.Length} characters long. Maximum is 8.");

            var invalid = Path.GetInvalidFileNameChars();
            foreach (char c in name)
            {
                if (Array.IndexOf(invalid, c) >= 0)
                {
                    result.Errors.Add($"Filename contains invalid character: '{c}'");
                    break;
                }
            }

            return result;
        }
    }
}
