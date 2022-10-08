/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "iblock.h"
#include <cstdio>
#include <memory>
#include <vector>

namespace mdf::detail {
class DataListBlock : public IBlock {
public:
  using BlockList = std::vector<std::unique_ptr<IBlock>>;
  BlockList &DataBlockList() { return block_list_; }
  [[nodiscard]] const BlockList &DataBlockList() const { return block_list_; }
  [[nodiscard]] virtual size_t DataSize() const;
  [[nodiscard]] const IBlock *Find(int64_t index) const override;
  void ReadBlockList(std::FILE *file, size_t data_index);
  void ReadLinkList(std::FILE *file, size_t data_index, uint32_t nof_link);

protected:
  BlockList block_list_;
};
} // namespace mdf::detail
