/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
#include <chrono>
#include <string>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <dataflow_lite/dataflow/sink.h>
#include <dataflow_lite/dataflow/source.h>
#include <dataflow_lite/dataflow/observed_queue.h>

using namespace Aws::DataFlow;


void TestEnqueueDequeue(IObservedQueue<std::string> &observed_queue) {
  auto status_monitor = std::make_shared<StatusMonitor>();
  observed_queue.SetStatusMonitor(status_monitor);

  EXPECT_EQ(Status::UNAVAILABLE, status_monitor->GetStatus());
  observed_queue.Enqueue("hello");
  EXPECT_EQ(Status::AVAILABLE, status_monitor->GetStatus());
  std::string data;
  ASSERT_TRUE(observed_queue.Dequeue(data, std::chrono::microseconds(0)));
  EXPECT_EQ("hello", data);
  EXPECT_TRUE(observed_queue.Empty());
  EXPECT_EQ(Status::UNAVAILABLE, status_monitor->GetStatus());
}

TEST(observed_queue_test, Sanity) {
  ASSERT_TRUE(true);
}

TEST(observed_queue_test, enqueue_dequeue_test) {
  ObservedQueue<std::string> observed_queue;
  TestEnqueueDequeue(observed_queue);
}

TEST(observed_queue_test, blocking_enqueue_dequeue_test) {
  ObservedBlockingQueue<std::string> observed_queue(1);
  TestEnqueueDequeue(observed_queue);
}

TEST(observed_queue_test, synchronized_enqueue_dequeue_test) {
  ObservedSynchronizedQueue<std::string> observed_queue;
  TestEnqueueDequeue(observed_queue);
}

TEST(observed_queue_test, enqueue_blocked_dequeue_test) {
  ObservedBlockingQueue<std::string> observed_queue(1);
  auto status_monitor = std::make_shared<StatusMonitor>();
  observed_queue.SetStatusMonitor(status_monitor);

  EXPECT_EQ(Status::UNAVAILABLE, status_monitor->GetStatus());
  EXPECT_TRUE(observed_queue.TryEnqueue("hello", std::chrono::seconds(0)));
  EXPECT_FALSE(observed_queue.TryEnqueue("fail", std::chrono::seconds(0)));
  std::string data;
  ASSERT_TRUE(observed_queue.Dequeue(data, std::chrono::microseconds(0)));
  EXPECT_EQ("hello", data);
  EXPECT_TRUE(observed_queue.TryEnqueue("hello", std::chrono::seconds(0)));
}
