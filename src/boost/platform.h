#ifndef PLATFORM_H
#define PLATFORM_H

namespace endian {
	uint64_t ConvertUnsignedLe(const std::vector<uint8_t>& data_buffer);
}

namespace locale {
	... to_utf(...);
}

#endif // PLATFORM_H