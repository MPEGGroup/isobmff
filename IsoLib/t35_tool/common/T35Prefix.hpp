#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace t35 {

/**
 * T.35 Prefix representation
 * Handles parsing and conversion of ITU-T T.35 message prefixes
 *
 * Format: T35Prefix[":" T35Description]
 *
 * T35Prefix consists of an even number of uppercase hexadecimal digits (0-9, A-F),
 * representing the initial bytes of an ITU-T T.35 message. It includes:
 * - Country code (including any extension bytes)
 * - Terminal provider code
 * - Terminal provider-oriented code
 *
 * T35Description (if present) follows the colon (U+003A) and provides a
 * human-readable description. It shall not contain:
 * - Colon (U+003A)
 * - C0 control characters (U+0000–U+001F)
 *
 * IMPORTANT: T35Description is informative only and shall not be used for
 * identification, matching, or processing. Message identification is based
 * solely on T35Prefix hex bytes.
 *
 * Examples:
 *   "B500900001:SMPTE-ST2094-50"  - With description
 *   "B500900001"                  - Without description
 *
 * Regex: ^[0-9A-F]{2}(?:[0-9A-F]{2})*(?::[^\x00-\x1F:]+)?$
 */
class T35Prefix {
public:
    /**
     * Default constructor - empty prefix
     */
    T35Prefix();

    /**
     * Construct from hex string with optional description
     * @param input Format: "HEXSTRING" or "HEXSTRING:DESCRIPTION"
     */
    explicit T35Prefix(const std::string& input);

    /**
     * Construct from hex string and description separately
     */
    T35Prefix(const std::string& hexStr, const std::string& desc);

    /**
     * Parse from string format
     * @param input Format: T35Prefix[":" T35Description]
     * @return true if parsed successfully
     *
     * Note: Hex string will be normalized to uppercase
     */
    bool parse(const std::string& input);

    /**
     * Get hex string only (without description)
     * Always uppercase as per spec
     */
    const std::string& hex() const { return hexString; }

    /**
     * Get description (may be empty)
     * Description is informative only
     */
    const std::string& description() const { return desc; }

    /**
     * Get full string representation "HEX:DESCRIPTION" or "HEX"
     */
    std::string toString() const;

    /**
     * Convert hex string to binary bytes
     */
    std::vector<uint8_t> toBytes() const;

    /**
     * Check if prefix is empty/invalid
     */
    bool empty() const { return hexString.empty(); }

    /**
     * Check if prefix is valid (hex string is valid hex)
     */
    bool isValid() const;

    /**
     * Get byte length of prefix
     */
    size_t byteLength() const { return hexString.size() / 2; }

    /**
     * Compare prefixes (by hex string only, ignoring description)
     */
    bool operator==(const T35Prefix& other) const {
        return hexString == other.hexString;
    }

    bool operator!=(const T35Prefix& other) const {
        return !(*this == other);
    }

    /**
     * Static helper: Check if string is valid hex
     * Must be uppercase hex digits (0-9, A-F) and even length
     */
    static bool isValidHex(const std::string& str);

    /**
     * Static helper: Check if description is valid
     * Must not contain colon (U+003A) or C0 control characters (U+0000-U+001F)
     */
    static bool isValidDescription(const std::string& desc);

    /**
     * Static helper: Normalize hex string to uppercase
     */
    static std::string normalizeHex(const std::string& hex);

    /**
     * Static helper: Convert hex string to bytes
     */
    static std::vector<uint8_t> hexToBytes(const std::string& hex);

    /**
     * Static helper: Convert bytes to hex string
     */
    static std::string bytesToHex(const std::vector<uint8_t>& bytes);

private:
    std::string hexString;    // Hex digits only (e.g., "B500900001")
    std::string desc;         // Optional description (e.g., "SMPTE-ST2094-50")
};

} // namespace t35
