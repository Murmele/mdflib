/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "dz4block.h"
#include <cstdio>
#include <mdf/zlibutil.h>
#include <string>

namespace {

std::string MakeZipTypeString(uint8_t type) {
  switch (type) {
  case 0:
    return "ZLIB Deflate";
  case 1:
    return "Transposition + ZLIB Deflate";
  default:
    break;
  }
  return "Unknown";
}
} // namespace

namespace mdf::detail {

void Dz4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);
  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Original Block Type", orig_block_type_);
  dest.emplace_back("Zip Type", MakeZipTypeString(type_));
  dest.emplace_back("Zip Parameter", std::to_string(parameter_));
  dest.emplace_back("Original Size  [byte]", std::to_string(orig_data_length_));
  dest.emplace_back("Data Size  [byte]", std::to_string(data_length_));
}

size_t Dz4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadStr(file, orig_block_type_, 2);
  bytes += ReadNumber(file, type_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 1);
  bytes += ReadNumber(file, parameter_);
  bytes += ReadNumber(file, orig_data_length_);
  bytes += ReadNumber(file, data_length_);
  data_position_ = GetFilePosition(file);
  return bytes;
}

size_t Dz4Block::CopyDataToFile(std::FILE *from_file,
                                std::FILE *to_file) const {
  if (data_position_ == 0 || orig_data_length_ == 0 || data_length_ == 0) {
    return 0;
  }
  SetFilePosition(from_file, data_position_);

  size_t count = 0;
  switch (static_cast<Dz4ZipType>(type_)) {
  case Dz4ZipType::Deflate: {
    const bool inflate = Inflate(from_file, to_file, data_length_);
    count = inflate ? orig_data_length_ : 0;
    break;
  }

  case Dz4ZipType::TransposeAndDeflate: {
    ByteArray temp(data_length_, 0);
    fread(temp.data(), 1, temp.size(), from_file);
    ByteArray out(orig_data_length_, 0);
    const bool inflate = Inflate(temp, out);
    InvTranspose(out, parameter_);
    fwrite(out.data(), 1, out.size(), to_file);
    count = inflate ? orig_data_length_ : 0;
    break;
  }

  default:
    break;
  }
  return count;
}

size_t Dz4Block::CopyDataToBuffer(std::FILE *from_file,
                                  std::vector<uint8_t> &buffer,
                                  size_t &buffer_index) const {
  if (data_position_ == 0 || orig_data_length_ == 0 || data_length_ == 0) {
    return 0;
  }

  SetFilePosition(from_file, data_position_);

  size_t count = 0;
  switch (static_cast<Dz4ZipType>(type_)) {
  case Dz4ZipType::Deflate: {
    ByteArray temp(data_length_, 0);
    fread(temp.data(), 1, temp.size(), from_file);

    ByteArray out(orig_data_length_, 0);
    Inflate(temp, out);
    count = orig_data_length_;
    memcpy(buffer.data() + buffer_index, out.data(), count);
    buffer_index += count;
    break;
  }

  case Dz4ZipType::TransposeAndDeflate: {
    ByteArray temp(data_length_, 0);
    fread(temp.data(), 1, temp.size(), from_file);

    ByteArray out(orig_data_length_, 0);
    Inflate(temp, out);
    InvTranspose(out, parameter_);

    count = orig_data_length_;
    memcpy(buffer.data() + buffer_index, out.data(), count);
    buffer_index += count;
    break;
  }

  default:
    break;
  }
  return count;
}

} // namespace mdf::detail