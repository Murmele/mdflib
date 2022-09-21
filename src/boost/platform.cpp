#include "platform.h"

#include <boost/endian/buffers.hpp>
#include <boost/locale.hpp>

namespace endian {
	
	
uint64_t ConvertUnsignedLe(const std::vector<uint8_t>& data_buffer)
{
  switch (data_buffer.size()) {
    case 1: {
      boost::endian::little_uint8_buf_t data;
      memcpy(data.data(), data_buffer.data(), 1);
      return data.value();
    }

    case 2: {
      boost::endian::little_uint16_buf_t data;
      memcpy(data.data(), data_buffer.data(), 2);
      return data.value();
    }

    case 3: {
      boost::endian::little_uint24_buf_t data;
      memcpy(data.data(), data_buffer.data(), 3);
      return data.value();
    }

    case 4: {
      boost::endian::little_uint32_buf_t data;
      memcpy(data.data(), data_buffer.data(), 4);
      return data.value();
    }

    case 5: {
      boost::endian::little_uint40_buf_t data;
      memcpy(data.data(), data_buffer.data(), 5);
      return data.value();
    }

    case 6: {
      boost::endian::little_uint48_buf_t data;
      memcpy(data.data(), data_buffer.data(), 6);
      return data.value();
    }

    case 7: {
      boost::endian::little_uint56_buf_t data;
      memcpy(data.data(), data_buffer.data(), 7);
      return data.value();
    }

    case 8: {
      boost::endian::little_uint64_buf_t data;
      memcpy(data.data(), data_buffer.data(), 8);
      return data.value();
    }

    default:break;
  }
  return 0;
}

uint64_t ConvertUnsignedBe(const std::vector<uint8_t>& data_buffer)
{
  switch (data_buffer.size()) {
    case 1: {
      boost::endian::big_uint8_buf_t data;
      memcpy(data.data(), data_buffer.data(), 1);
      return data.value();
    }

    case 2: {
      boost::endian::big_uint16_buf_t data;
      memcpy(data.data(), data_buffer.data(), 2);
      return data.value();
    }

    case 3: {
      boost::endian::big_uint24_buf_t data;
      memcpy(data.data(), data_buffer.data(), 3);
      return data.value();
    }

    case 4: {
      boost::endian::big_uint32_buf_t data;
      memcpy(data.data(), data_buffer.data(), 4);
      return data.value();
    }

    case 5: {
      boost::endian::big_uint40_buf_t data;
      memcpy(data.data(), data_buffer.data(), 5);
      return data.value();
    }

    case 6: {
      boost::endian::big_uint48_buf_t data;
      memcpy(data.data(), data_buffer.data(), 6);
      return data.value();
    }

    case 7: {
      boost::endian::big_uint56_buf_t data;
      memcpy(data.data(), data_buffer.data(), 7);
      return data.value();
    }

    case 8: {
      boost::endian::big_uint64_buf_t data;
      memcpy(data.data(), data_buffer.data(), 8);
      return data.value();
    }

    default:break;
  }
  return 0;
}

int64_t ConvertSignedLe(const std::vector<uint8_t>& data_buffer)
{
  switch (data_buffer.size()) {
    case 1: {
      boost::endian::little_int8_buf_t data;
      memcpy(data.data(), data_buffer.data(), 1);
      return data.value();
    }

    case 2: {
      boost::endian::little_int16_buf_t data;
      memcpy(data.data(), data_buffer.data(), 2);
      return data.value();
    }

    case 3: {
      boost::endian::little_int24_buf_t data;
      memcpy(data.data(), data_buffer.data(), 3);
      return data.value();
    }

    case 4: {
      boost::endian::little_int32_buf_t data;
      memcpy(data.data(), data_buffer.data(), 4);
      return data.value();
    }

    case 5: {
      boost::endian::little_int40_buf_t data;
      memcpy(data.data(), data_buffer.data(), 5);
      return data.value();
    }

    case 6: {
      boost::endian::little_int48_buf_t data;
      memcpy(data.data(), data_buffer.data(), 6);
      return data.value();
    }

    case 7: {
      boost::endian::little_int56_buf_t data;
      memcpy(data.data(), data_buffer.data(), 7);
      return data.value();
    }

    case 8: {
      boost::endian::little_int64_buf_t data;
      memcpy(data.data(), data_buffer.data(), 8);
      return data.value();
    }

    default:break;
  }
  return 0;
}

int64_t ConvertSignedBe(const std::vector<uint8_t>& data_buffer)
{
  switch (data_buffer.size()) {
    case 1: {
      boost::endian::big_int8_buf_t data;
      memcpy(data.data(), data_buffer.data(), 1);
      return data.value();
    }

    case 2: {
      boost::endian::big_int16_buf_t data;
      memcpy(data.data(), data_buffer.data(), 2);
      return data.value();
    }

    case 3: {
      boost::endian::big_int24_buf_t data;
      memcpy(data.data(), data_buffer.data(), 3);
      return data.value();
    }

    case 4: {
      boost::endian::big_int32_buf_t data;
      memcpy(data.data(), data_buffer.data(), 4);
      return data.value();
    }

    case 5: {
      boost::endian::big_int40_buf_t data;
      memcpy(data.data(), data_buffer.data(), 5);
      return data.value();
    }

    case 6: {
      boost::endian::big_int48_buf_t data;
      memcpy(data.data(), data_buffer.data(), 6);
      return data.value();
    }

    case 7: {
      boost::endian::big_int56_buf_t data;
      memcpy(data.data(), data_buffer.data(), 7);
      return data.value();
    }

    case 8: {
      boost::endian::big_int64_buf_t data;
      memcpy(data.data(), data_buffer.data(), 8);
      return data.value();
    }

    default:break;
  }
  return 0;
}

double ConvertFloatBe(const std::vector<uint8_t>& data_buffer) {
  switch (data_buffer.size()) {
    case 2: {
      boost::endian::endian_buffer<boost::endian::order::big, half_float::half, 16, boost::endian::align::no> data {};
      memcpy(data.data(), data_buffer.data(), 2);
      return data.value();
    }

    case 4: {
      boost::endian::big_float32_buf_t data;
      memcpy(data.data(), data_buffer.data(), 4);
      return data.value();
    }

    case 8: {
      boost::endian::big_float64_buf_t data;
      memcpy(data.data(), data_buffer.data(), 8);
      return data.value();
    }
    default: break;
  }
  return 0.0;
}

double ConvertFloatLe(const std::vector<uint8_t>& data_buffer) {
  switch (data_buffer.size()) {
    case 2: {
      boost::endian::endian_buffer<boost::endian::order::little, half_float::half, 16, boost::endian::align::no> data {};
      memcpy(data.data(), data_buffer.data(), 2);
      return data.value();
    }

    case 4: {
      boost::endian::little_float32_buf_t data;
      memcpy(data.data(), data_buffer.data(), 4);
      return data.value();
    }

    case 8: {
      boost::endian::little_float64_buf_t data;
      memcpy(data.data(), data_buffer.data(), 8);
      return data.value();
    }
    default: break;
  }
  return 0.0;
}

} // namespace endian

namespace locale {
	template<class T>
	... to_utf(....) {
		return boost::locale::conv::to_utf<char>(s.str(), "Latin1");
	}
	
	utf_to_utf() {
		return boost::locale::conv::utf_to_utf<char>(s.str());
	}
} // namespace locale