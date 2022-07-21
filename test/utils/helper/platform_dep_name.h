#include <string>

/**
 * Create a platform (tester)-dependant name for stand-alone use.
 * @param [in] strOrigName  The original Module name
 * @return The modified name.
 */
const std::string makePlatformDepName(const std::string& original_name);
