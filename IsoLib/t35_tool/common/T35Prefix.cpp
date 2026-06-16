#include "T35Prefix.hpp"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace t35
{

T35Prefix::T35Prefix() {}

T35Prefix::T35Prefix(const std::string &input) { parse(input); }

T35Prefix::T35Prefix(const std::string &hexStr, const std::string &description)
  : hexString(normalizeHex(hexStr))
  , desc(description)
{
}

bool T35Prefix::parse(const std::string &input)
{
  if(input.empty())
  {
    return false;
  }

  // Find colon separator
  size_t colonPos = input.find(':');

  if(colonPos == std::string::npos)
  {
    // No description, entire string is hex
    hexString = normalizeHex(input);
    desc.clear();
  }
  else
  {
    // Split into hex and description
    hexString = normalizeHex(input.substr(0, colonPos));
    desc      = input.substr(colonPos + 1);

    // Validate description
    if(!isValidDescription(desc))
    {
      hexString.clear();
      desc.clear();
      return false;
    }
  }

  // Validate hex string
  if(!isValidHex(hexString))
  {
    hexString.clear();
    desc.clear();
    return false;
  }

  return true;
}

std::string T35Prefix::toString() const
{
  if(desc.empty())
  {
    return hexString;
  }
  return hexString + ":" + desc;
}

std::vector<uint8_t> T35Prefix::toBytes() const { return hexToBytes(hexString); }

bool T35Prefix::isValid() const { return !hexString.empty() && isValidHex(hexString); }

// Static helper: Check if string is valid hex (uppercase, even length)
bool T35Prefix::isValidHex(const std::string &str)
{
  if(str.empty())
  {
    return false;
  }

  // Must be even length (pairs of hex digits)
  if(str.size() % 2 != 0)
  {
    return false;
  }

  // All characters must be uppercase hex digits (0-9, A-F)
  return std::all_of(str.begin(), str.end(), [](unsigned char c)
                     { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'); });
}

// Static helper: Validate description (no colon, no C0 control chars)
bool T35Prefix::isValidDescription(const std::string &desc)
{
  if(desc.empty())
  {
    return true; // Empty description is valid
  }

  // Check for forbidden characters:
  // - Colon (U+003A)
  // - C0 control characters (U+0000-U+001F)
  return std::none_of(desc.begin(), desc.end(),
                      [](unsigned char c) { return c == ':' || c <= 0x1F; });
}

// Static helper: Normalize hex string to uppercase
std::string T35Prefix::normalizeHex(const std::string &hex)
{
  std::string result = hex;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return result;
}

// Static helper: Convert hex string to bytes
std::vector<uint8_t> T35Prefix::hexToBytes(const std::string &hex)
{
  std::vector<uint8_t> bytes;

  if(hex.size() % 2 != 0)
  {
    return bytes; // Invalid, return empty
  }

  bytes.reserve(hex.size() / 2);

  for(size_t i = 0; i < hex.size(); i += 2)
  {
    std::string byteStr  = hex.substr(i, 2);
    unsigned int byteVal = 0;

    std::stringstream ss;
    ss << std::hex << byteStr;
    ss >> byteVal;

    bytes.push_back(static_cast<uint8_t>(byteVal));
  }

  return bytes;
}

// Static helper: Convert bytes to hex string
std::string T35Prefix::bytesToHex(const std::vector<uint8_t> &bytes)
{
  std::ostringstream oss;
  oss << std::hex << std::uppercase;

  for(uint8_t byte : bytes)
  {
    oss << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
  }

  return oss.str();
}

} // namespace t35
