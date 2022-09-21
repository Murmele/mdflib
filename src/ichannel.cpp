/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <string>

#include "platform.h"
	
#include "mdf/ichannel.h"
#include "util/stringutil.h"
#include "half.hpp"

namespace {




} // namespace
namespace mdf {

void IChannel::CopyToDataBuffer(const std::vector<uint8_t> &record_buffer, std::vector<uint8_t>& data_buffer) const {
  size_t nof_bytes = (BitCount() / 8) + (BitCount() % 8 > 0 ? 1 : 0);
  if (data_buffer.size() != nof_bytes) {
    data_buffer.resize(nof_bytes);
  }
  memset(data_buffer.data(), 0, data_buffer.size());

  if (BitOffset() == 0) {
    // This is the preferred way of doing business
    memcpy(data_buffer.data(),record_buffer.data() + ByteOffset(), data_buffer.size());
  } else if (BitCount() == 1) {
    // This is OK for boolean and bits
    uint8_t in_bit = 0x01 << (BitOffset() - 1);
    data_buffer[0] = (record_buffer[ByteOffset()] & in_bit)? 0x01 : 0x00;
  } else {
    uint8_t in_offset = BitOffset();
    uint32_t in_byte = ByteOffset();
    for (auto ii = BitCount(); ii < BitCount(); ++ii) {
      uint8_t in_bit = 0x01 << (in_offset - 1);
      auto out_byte = ii / 8;
      if (record_buffer[in_byte] & in_bit) {
        data_buffer[out_byte] |= 0x01;
      }
      if (ii < (BitCount() - 1)) {
        data_buffer[out_byte] <<= 1;
      }

      --in_offset;
      if (in_offset == 0) {
        in_offset = 8;
        ++in_byte;
      }
    }

  }
}

bool IChannel::GetUnsignedValue(const std::vector<uint8_t> &record_buffer, uint64_t &dest) const {

  // Copy to a temp data buffer, so we can get rid of this bit offset nonsense
  std::vector<uint8_t> data_buffer;
  CopyToDataBuffer(record_buffer, data_buffer);
  switch (DataType()) {
    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
    case ChannelDataType::UnsignedIntegerLe:
      dest = ConvertUnsignedLe(data_buffer);
      break;

    case ChannelDataType::UnsignedIntegerBe:
      dest = ConvertUnsignedBe(data_buffer);
      break;

    default:
      return false; // Not valid conversion
  }
  return true;
}

bool IChannel::GetSignedValue(const std::vector<uint8_t> &record_buffer, int64_t &dest) const {

  // Copy to a temp data buffer, so we can get rid of the bit offset nonsense
  std::vector<uint8_t> data_buffer;
  CopyToDataBuffer(record_buffer, data_buffer);
  switch (DataType()) {
    case ChannelDataType::SignedIntegerLe:
      dest = ConvertSignedLe(data_buffer);
      break;

    case ChannelDataType::SignedIntegerBe:
      dest = ConvertSignedBe(data_buffer);
      break;

    default:
      return false; // Not valid conversion
  }
  return true;
}

bool IChannel::GetFloatValue(const std::vector<uint8_t> &record_buffer, double &dest) const {

  // Copy to a temp data buffer, so we can get rid of the bit offset nonsense
  std::vector<uint8_t> data_buffer;
  CopyToDataBuffer(record_buffer, data_buffer);
  switch (DataType()) {
    case ChannelDataType::FloatLe:
      dest = ConvertFloatLe(data_buffer);
      break;

    case ChannelDataType::FloatBe:
      dest = ConvertFloatBe(data_buffer);
      break;

    default:
      return false; // Not valid conversion
  }
  return true;
}

bool IChannel::GetTextValue(const std::vector<uint8_t> &record_buffer, std::string &dest) const {
  auto offset = ByteOffset();
  bool valid = true;
  dest.clear();

  switch (DataType()) {
    case ChannelDataType::StringAscii: {
      // Convert ASCII to UTF8
      std::ostringstream s;
      for (size_t ii = offset; ii < record_buffer.size(); ++ii) {
        char in = static_cast<char>(record_buffer[ii]);
        if (in == '\0') {
          break;
        }
        s << in;
      }
      try {
        dest = locale::to_utf(s.str(), "Latin1");
      } catch (const std::exception&) {
        valid = false; // Conversion error
        dest = s.str();
      }
      break;
    }

    case ChannelDataType::StringUTF8: {
      // No conversion needed
      std::ostringstream s;
      for (size_t ii = offset; ii < record_buffer.size(); ++ii) {
        char in = static_cast<char>(record_buffer[ii]);
        if (in == '\0') {
          break;
        }
        s << in;
      }
      dest = s.str();
      break;
    }

    case ChannelDataType::StringUTF16Le: {
      std::wostringstream s;
      for (size_t ii = offset; (ii + 2) <= record_buffer.size(); ii += 2) {
        boost::endian::little_uint16_buf_t data;
        memcpy(data.data(), record_buffer.data() + ii, 2);
        if (data.value() == 0) {
          break;
        }
        s << static_cast<wchar_t>(data.value());
      }
      try {
        dest = locale::utf_to_utf<char>(s.str());
      } catch (const std::exception&) {
        valid = false; // Conversion error
      }
      break;
    }

    case ChannelDataType::StringUTF16Be: {
      std::wostringstream s;
      for (size_t ii = offset; (ii + 2) <= record_buffer.size(); ii += 2) {
        auto* d = record_buffer.data() + ii;
        boost::endian::big_uint16_buf_t data;
        memcpy(data.data(), d, 2);
        if (data.value() == 0) {
          break;
        }
        s << static_cast<wchar_t>(data.value());
      }
      try {
        dest = boost::locale::conv::utf_to_utf<char>(s.str());
      } catch (const std::exception&) {
        valid = false; // Conversion error
      }
      break;
    }
    default:
      break;
  }
  return valid;
}

bool IChannel::GetByteArrayValue(const std::vector<uint8_t> &record_buffer, std::vector<uint8_t> &dest) const {
  if (dest.size() != DataBytes()) {
    dest.resize(DataBytes());
  }
  if (dest.empty()) {
    return true;
  }
  memcpy(dest.data(),record_buffer.data() + ByteOffset(), DataBytes());
  return true;
}

bool IChannel::GetCanOpenDate(const std::vector<uint8_t> &record_buffer, uint64_t &dest) const {
  std::vector<uint8_t> date_array(7,0);
  memcpy(date_array.data(), record_buffer.data() + ByteOffset(), date_array.size());
  dest = MdfHelper::CanOpenDateArrayToNs(date_array);
  return true;
}

bool IChannel::GetCanOpenTime(const std::vector<uint8_t> &record_buffer, uint64_t &dest) const {
  std::vector<uint8_t> time_array(6,0);
  memcpy(time_array.data(), record_buffer.data() + ByteOffset(), time_array.size());
  dest = MdfHelper::CanOpenTimeArrayToNs(time_array);
  return true;
}

void IChannel::SetValid(bool) {
  // Only MDF4 have this functionality
}

void IChannel::SetUnsignedValueLe(uint64_t value, bool valid) {
  SetValid(valid);
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 1: {
      boost::endian::little_uint8_buf_at data(static_cast<uint8_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 2: {
      boost::endian::little_uint16_buf_at data(static_cast<uint16_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 4: {
      boost::endian::little_uint32_buf_at data(static_cast<uint32_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      boost::endian::little_uint64_buf_at data(value);
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    default:
      SetValid(false);
      break;
  }
}

void IChannel::SetUnsignedValueBe(uint64_t value, bool valid) {
  SetValid(valid);
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 1: {
      boost::endian::big_uint8_buf_at data(static_cast<uint8_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 2: {
      boost::endian::big_uint16_buf_at data(static_cast<uint16_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 4: {
      boost::endian::big_uint32_buf_at data(static_cast<uint32_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      boost::endian::big_uint64_buf_at data(value);
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }
    default:
      SetValid(false);
      break;
  }
}

void IChannel::SetSignedValueLe(int64_t value, bool valid) {
  SetValid(valid);
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 1: {
      boost::endian::little_int8_buf_at data(static_cast<int8_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 2: {
      boost::endian::little_int16_buf_at data(static_cast<int16_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 4: {
      boost::endian::little_int32_buf_at data(static_cast<int32_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      boost::endian::little_int64_buf_at data(value);
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    default:
      SetValid(false);
      break;
  }
}

void IChannel::SetSignedValueBe(int64_t value, bool valid) {
  SetValid(valid);
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 1: {
      boost::endian::big_int8_buf_at data(static_cast<int8_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 2: {
      boost::endian::big_int16_buf_at data(static_cast<int16_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 4: {
      boost::endian::big_int32_buf_at data(static_cast<int32_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      boost::endian::big_int64_buf_at data(value);
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }
    default:
      SetValid(false);
      break;
  }
}

void IChannel::SetFloatValueLe(double value, bool valid) {
  SetValid(valid);
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 4: {
      boost::endian::little_float32_buf_at data(static_cast<float>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      boost::endian::little_float64_buf_at data(value);
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    default:
      SetValid(false);
      break;
  }
}

void IChannel::SetFloatValueBe(double value, bool valid) {
  SetValid(valid);
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 4: {
      boost::endian::big_float32_buf_at data(static_cast<float>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      boost::endian::big_float64_buf_at data(value);
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    default:
      SetValid(false);
      break;
  }
}

void IChannel::SetTextValue(const std::string &value, bool valid) {
  SetValid(valid);
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  if (bytes == 0) {
    SetValid(false);
    return;
  }
  // The string shall be null terminated
  memset(buffer.data() + ByteOffset(), '\0', bytes);
  if (value.size() < bytes) {
    memcpy(buffer.data() + ByteOffset(),value.data(), value.size());
  } else {
    memcpy(buffer.data() + ByteOffset(),value.data(), bytes -1);
  }
}

void IChannel::SetByteArray(const std::vector<uint8_t> &value, bool valid) {
  SetValid(valid);
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  if (bytes == 0) {
    SetValid(false);
    return;
  }
  // The string shall be null terminated
  memset(buffer.data() + ByteOffset(), '\0', bytes);
  if (value.size() <= bytes) {
    memcpy(buffer.data() + ByteOffset(),value.data(), value.size());
  } else {
    memcpy(buffer.data() + ByteOffset(),value.data(), bytes);
  }
}

template<>
bool IChannel::GetChannelValue(const std::vector<uint8_t> &record_buffer, std::vector<uint8_t> &dest) const {
  bool valid = false;
  switch (DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      uint64_t value = 0;
      valid = GetUnsignedValue(record_buffer, value);
      dest.resize(1);
      dest[0] = static_cast<uint8_t>(value);
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      int64_t value = 0;
      valid = GetSignedValue(record_buffer, value);
      dest.resize(1);
      dest[0] = static_cast<uint8_t>(value);
      break;
    }

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      double value = 0;
      valid = GetFloatValue(record_buffer, value);
      dest.resize(1);
      dest[0] = static_cast<uint8_t>(value);
      break;
    }

    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii: {
      std::string text;
      valid = GetTextValue(record_buffer, text);
      dest.resize(text.size());
      memcpy(dest.data(), text.data(), text.size());
      break;
    }

    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray: {
      valid = GetByteArrayValue(record_buffer, dest);
      break;
    }

    case ChannelDataType::CanOpenDate: {
      uint64_t ms_since_1970 = 0;
      valid = GetCanOpenDate(record_buffer, ms_since_1970);
      dest.resize(1);
      dest[0] = static_cast<uint8_t>(ms_since_1970);
      break;
    }

    case ChannelDataType::CanOpenTime: {
      uint64_t ms_since_1970 = 0;
      valid = GetCanOpenTime(record_buffer, ms_since_1970);
      dest.resize(1);
      dest[0] = static_cast<uint8_t>(ms_since_1970);
      break;
    }

    default: break;
  }
  return valid;
}

template<>
bool IChannel::GetChannelValue(const std::vector<uint8_t> &record_buffer, std::string &dest) const {
  bool valid = false;
  switch (DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      uint64_t value = 0;
      valid = GetUnsignedValue(record_buffer, value);
      std::ostringstream s;
      s << value;
      dest = s.str();
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      int64_t value = 0;
      valid = GetSignedValue(record_buffer, value);
      dest = std::to_string(value);
      break;
    }
    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      double value = 0;
      valid = GetFloatValue(record_buffer, value);
      dest = IsDecimalUsed() ? util::string::FormatDouble(value, Decimals()) : std::to_string(value);
      break;
    }

    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii: {
      valid = GetTextValue(record_buffer, dest);
      break;
    }

    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray: {
      std::vector<uint8_t> list;
      valid = GetByteArrayValue(record_buffer, list);
      std::ostringstream s;
      for (const auto byte: list) {
        s << std::setfill('0') << std::setw(2)
          << std::hex << std::uppercase
          << static_cast<uint16_t>(byte);
      }
      dest = s.str();
      break;
    }

    case ChannelDataType::CanOpenDate: {
      uint64_t ms_since_1970 = 0;
      valid = GetCanOpenDate(record_buffer, ms_since_1970);

      const auto ms = ms_since_1970 % 1000;
      const auto time = static_cast<time_t>(ms_since_1970 / 1000);
      struct tm bt{};
      localtime_s(&bt, &time);

      std::ostringstream text;
      text << std::put_time(&bt, "%Y-%m-%d %H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms;
      dest = text.str();
      break;
    }

    case ChannelDataType::CanOpenTime: {
      uint64_t ms_since_1970 = 0;
      valid = GetCanOpenTime(record_buffer, ms_since_1970);

      const auto ms = ms_since_1970 % 1000;
      const auto time = static_cast<time_t>(ms_since_1970 / 1000);
      struct tm bt{};
      localtime_s(&bt, &time);

      std::ostringstream text;
      text << std::put_time(&bt, "%Y-%m-%d %H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms;
      dest = text.str();
      break;
    }

    default: break;
  }
  return valid;
}

template<>
void IChannel::SetChannelValue(const std::string& value, bool valid) {
  switch (DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
      SetUnsignedValueLe(std::stoull(value), valid);
      break;

    case ChannelDataType::UnsignedIntegerBe:
      SetUnsignedValueBe(std::stoull(value), valid);
      break;

    case ChannelDataType::SignedIntegerLe:
      SetSignedValueLe(std::stoll(value), valid);
      break;

    case ChannelDataType::SignedIntegerBe:
      SetSignedValueBe(std::stoll(value), valid);
      break;

    case ChannelDataType::FloatLe:
      SetFloatValueLe(std::stod(value), valid);
      break;

    case ChannelDataType::FloatBe:
      SetFloatValueBe(std::stod(value), valid);
      break;

    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
      SetTextValue(value, valid);
      break;

    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::CanOpenDate:
    case ChannelDataType::CanOpenTime:
    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray:
    default:
      SetValid(false);
      break;
  }
}

template<>
void IChannel::SetChannelValue(const std::vector<uint8_t>& value, bool valid) {
  switch (DataType()) {
    case ChannelDataType::ByteArray:
      SetByteArray(value, valid);
      break;

    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe:
    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe:
    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::CanOpenDate:
    case ChannelDataType::CanOpenTime:
    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    default:
      SetValid(false);
      break;
  }
}

void IChannel::Sync(ChannelSyncType type) {
}

ChannelSyncType IChannel::Sync() const {
  return ChannelSyncType::None;
}

void IChannel::Range(double min, double max) {
}

std::optional<std::pair<double, double>> IChannel::Range() const {
  return {};
}

void IChannel::Limit(double min, double max) {
}

std::optional<std::pair<double, double>> IChannel::Limit() const {
  return {};
}

void IChannel::ExtLimit(double min, double max) {
}

std::optional<std::pair<double, double>> IChannel::ExtLimit() const {
  return {};
}

} // end namespace mdf