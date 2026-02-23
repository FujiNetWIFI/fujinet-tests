using Newtonsoft.Json;
using TestFileBuilder.Models;

namespace TestFileBuilder.Services
{
    public static class CommandLoader
    {
        /// <summary>
        /// Loads and parses the commands.jsn file from the application's base directory.
        /// Throws FileNotFoundException or JsonException if anything goes wrong.
        /// </summary>
        public static List<CommandDefinition> Load(string? filePath = null)
        {
            filePath ??= Path.Combine(AppContext.BaseDirectory, "commands.jsn");

            if (!File.Exists(filePath))
                throw new FileNotFoundException(
                    $"commands.jsn not found at: {filePath}\n\n" +
                    "Please ensure commands.jsn is in the same directory as TestFileBuilder.exe.",
                    filePath);

            string json = File.ReadAllText(filePath);
            var list = JsonConvert.DeserializeObject<List<CommandDefinition>>(json)
                       ?? throw new InvalidDataException("commands.jsn parsed as null.");

            // Parse the descriptor strings for each command
            foreach (var cmd in list)
                cmd.ParseDescriptors();

            // Sort by name for easy browsing
            list.Sort((a, b) => string.Compare(a.Name, b.Name, StringComparison.OrdinalIgnoreCase));

            return list;
        }

        /// <summary>
        /// Tries to find a definition whose CommandKey matches the given name (case-insensitive).
        /// </summary>
        public static CommandDefinition? FindByKey(IEnumerable<CommandDefinition> defs, string commandKey) =>
            defs.FirstOrDefault(d => string.Equals(d.CommandKey, commandKey, StringComparison.OrdinalIgnoreCase));
    }
}
