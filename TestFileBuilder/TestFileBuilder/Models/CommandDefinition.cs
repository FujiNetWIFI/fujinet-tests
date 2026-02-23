using Newtonsoft.Json;

namespace TestFileBuilder.Models
{
    /// <summary>
    /// Represents one entry from commands.jsn
    /// </summary>
    public class CommandDefinition
    {
        [JsonProperty("command")]
        public int CommandNumber { get; set; }

        [JsonProperty("name")]
        public string Name { get; set; } = string.Empty;

        [JsonProperty("args")]
        public List<string>? Args { get; set; }

        [JsonProperty("reply")]
        public List<string>? Reply { get; set; }

        /// <summary>
        /// Parsed argument descriptors (from Args strings like "host_slot:u8")
        /// </summary>
        [JsonIgnore]
        public List<ArgDescriptor> ParsedArgs { get; set; } = new();

        /// <summary>
        /// Parsed reply descriptor (there is at most one reply per command)
        /// </summary>
        [JsonIgnore]
        public ArgDescriptor? ParsedReply { get; set; }

        /// <summary>
        /// Human-readable display name e.g. "SET_HOST_PREFIX (225)"
        /// </summary>
        [JsonIgnore]
        public string DisplayName => $"{Name} ({CommandNumber})";

        /// <summary>
        /// The lowercase_underscore version of the name used in .tst files
        /// </summary>
        [JsonIgnore]
        public string CommandKey => Name.ToLowerInvariant();

        public void ParseDescriptors()
        {
            ParsedArgs.Clear();
            ParsedReply = null;

            if (Args != null)
            {
                foreach (var arg in Args)
                    ParsedArgs.Add(ArgDescriptor.Parse(arg));
            }

            if (Reply != null && Reply.Count > 0)
                ParsedReply = ArgDescriptor.Parse(Reply[0]);
        }
    }

    /// <summary>
    /// Parses a descriptor string like "host_slot:u8", "data:s8", "{creator:u16,app:u8,...}:struct"
    /// </summary>
    public class ArgDescriptor
    {
        public string Name { get; set; } = string.Empty;
        public char TypeChar { get; set; }
        public int Size { get; set; }
        public bool IsStruct { get; set; }
        public List<ArgDescriptor> StructFields { get; set; } = new();

        // TYPE constants
        public const char TYPE_BOOL = 'b';
        public const char TYPE_UNSIGNED = 'u';
        public const char TYPE_FIXED_LEN = 'f';
        public const char TYPE_VAR_LEN = 's';
        public const char TYPE_STRUCT = '{';
        public const char TYPE_SIGNED = 'i';

        public string TypeLabel => TypeChar switch
        {
            TYPE_BOOL => "bool",
            TYPE_UNSIGNED => $"uint{Size * 8}",
            TYPE_FIXED_LEN => $"fixed[{Size}]",
            TYPE_VAR_LEN => $"string[{Size}]",
            TYPE_STRUCT => "struct",
            TYPE_SIGNED => $"int{Size * 8}",
            _ => $"unknown({TypeChar})"
        };

        public static ArgDescriptor Parse(string descriptor)
        {
            var desc = new ArgDescriptor();

            // Handle struct type: "{creator:u16,app:u8,key:u8,mode:i8,reserved:u8}"
            if (descriptor.StartsWith('{'))
            {
                desc.IsStruct = true;
                desc.TypeChar = TYPE_STRUCT;
                desc.Name = "struct";

                // Strip outer braces
                int closeBrace = descriptor.LastIndexOf('}');
                string inner = descriptor.Substring(1, closeBrace - 1);

                // Parse each field
                foreach (var field in inner.Split(','))
                {
                    var fieldTrimmed = field.Trim();
                    if (!string.IsNullOrEmpty(fieldTrimmed))
                        desc.StructFields.Add(ParseSimple(fieldTrimmed));
                }
                return desc;
            }

            return ParseSimple(descriptor);
        }

        private static ArgDescriptor ParseSimple(string descriptor)
        {
            var desc = new ArgDescriptor();
            int colon = descriptor.IndexOf(':');
            if (colon < 0)
            {
                desc.Name = descriptor;
                desc.TypeChar = TYPE_VAR_LEN;
                desc.Size = 0;
                return desc;
            }

            desc.Name = descriptor.Substring(0, colon);
            string typeStr = descriptor.Substring(colon + 1);

            if (typeStr.Length == 0)
            {
                desc.TypeChar = TYPE_VAR_LEN;
                return desc;
            }

            desc.TypeChar = typeStr[0];

            // Parse numeric size suffix
            if (typeStr.Length > 1 && int.TryParse(typeStr.Substring(1), out int sz))
                desc.Size = sz;
            else
                desc.Size = 0;

            return desc;
        }
    }
}
