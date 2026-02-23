using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using TestFileBuilder.Models;

namespace TestFileBuilder.Services
{
    public static class TestFileService
    {
        /// <summary>
        /// Serialises a list of TestEntry objects to a formatted JSON string.
        /// </summary>
        public static string Serialise(IEnumerable<TestEntry> entries)
        {
            var array = new JArray();
            foreach (var entry in entries)
                array.Add(entry.ToJObject());

            return array.ToString(Formatting.Indented);
        }

        /// <summary>
        /// Saves the test list to a .tst file.
        /// </summary>
        public static void Save(string filePath, IEnumerable<TestEntry> entries)
        {
            string json = Serialise(entries);
            File.WriteAllText(filePath, json);
        }

        /// <summary>
        /// Loads a .tst file and returns a list of TestEntry objects.
        /// </summary>
        public static List<TestEntry> Load(string filePath, IEnumerable<CommandDefinition> definitions)
        {
            if (!File.Exists(filePath))
                throw new FileNotFoundException($"File not found: {filePath}", filePath);

            string json = File.ReadAllText(filePath);
            var token = JToken.Parse(json);

            if (token is not JArray array)
                throw new InvalidDataException("The .tst file must contain a JSON array at the top level.");

            var entries = new List<TestEntry>();
            foreach (var item in array)
            {
                if (item is JObject obj)
                    entries.Add(TestEntry.FromJObject(obj, definitions));
            }

            return entries;
        }

        /// <summary>
        /// Ensures the file path has the .tst extension and the base name is ≤8 chars.
        /// Returns the corrected full path.
        /// </summary>
        public static string NormalisePath(string filePath)
        {
            string dir = Path.GetDirectoryName(filePath) ?? string.Empty;
            string name = Path.GetFileNameWithoutExtension(filePath);
            if (name.Length > 8) name = name[..8];
            return Path.Combine(dir, name + ".tst");
        }
    }
}
