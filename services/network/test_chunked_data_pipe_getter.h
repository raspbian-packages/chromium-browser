// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_NETWORK_TEST_CHUNKED_DATA_PIPE_GETTER_H_
#define SERVICES_NETWORK_TEST_CHUNKED_DATA_PIPE_GETTER_H_

#include <memory>

#include "base/macros.h"
#include "base/run_loop.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/public/cpp/system/data_pipe.h"
#include "services/network/public/mojom/chunked_data_pipe_getter.mojom.h"

namespace network {

// Test implementation of mojom::DataPipeGetter that lets tests wait for
// the mojo::ScopedDataPipeProducerHandle and ReadCallback to be received
// and then manage them both directly.
class TestChunkedDataPipeGetter : public mojom::ChunkedDataPipeGetter {
 public:
  TestChunkedDataPipeGetter();
  ~TestChunkedDataPipeGetter() override;

  // Returns the mojo::ChunkedDataPipeGetterPtr corresponding to |this|. May
  // only be called once.
  mojom::ChunkedDataPipeGetterPtr GetDataPipeGetterPtr();

  // Close the mojom::DataPipeGetter pipe.
  void ClosePipe();

  GetSizeCallback WaitForGetSize();
  mojo::ScopedDataPipeProducerHandle WaitForStartReading();

 private:
  // mojom::ChunkedDataPipeGetter implementation:
  void GetSize(GetSizeCallback get_size_callback) override;
  void StartReading(mojo::ScopedDataPipeProducerHandle pipe) override;

  std::unique_ptr<base::RunLoop> get_size_run_loop_;
  std::unique_ptr<base::RunLoop> start_reading_run_loop_;

  mojo::Binding<mojom::ChunkedDataPipeGetter> binding_;
  mojo::ScopedDataPipeProducerHandle write_pipe_;
  GetSizeCallback get_size_callback_;
  bool received_size_callback_ = false;

  DISALLOW_COPY_AND_ASSIGN(TestChunkedDataPipeGetter);
};

}  // namespace network

#endif  // SERVICES_NETWORK_TEST_CHUNKED_DATA_PIPE_GETTER_H_
