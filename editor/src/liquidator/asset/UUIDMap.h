#pragma once

namespace liquid::editor {

using UUIDMap = std::unordered_map<String, Uuid>;

/**
 * @brief Get uuid from map
 *
 * @param uuids Uuid map
 * @param key Uuid map key
 * @return Uuid or empty uuid
 */
Uuid getUUIDFromMap(const UUIDMap &uuids, const String &key);

} // namespace liquid::editor
