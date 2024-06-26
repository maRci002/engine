// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "gtest/gtest.h"
#include "impeller/entity/entity.h"
#include "impeller/entity/entity_pass_clip_stack.h"

namespace impeller {
namespace testing {

TEST(EntityPassClipStackTest, CanPushAndPopEntities) {
  EntityPassClipStack recorder =
      EntityPassClipStack(Rect::MakeLTRB(0, 0, 100, 100));

  EXPECT_TRUE(recorder.GetReplayEntities().empty());

  Entity entity;
  recorder.RecordEntity(entity, Contents::ClipCoverage::Type::kAppend);
  EXPECT_EQ(recorder.GetReplayEntities().size(), 1u);

  recorder.RecordEntity(entity, Contents::ClipCoverage::Type::kAppend);
  EXPECT_EQ(recorder.GetReplayEntities().size(), 2u);

  recorder.RecordEntity(entity, Contents::ClipCoverage::Type::kRestore);
  EXPECT_EQ(recorder.GetReplayEntities().size(), 1u);

  recorder.RecordEntity(entity, Contents::ClipCoverage::Type::kRestore);
  EXPECT_TRUE(recorder.GetReplayEntities().empty());
}

TEST(EntityPassClipStackTest, CanPopEntitiesSafely) {
  EntityPassClipStack recorder =
      EntityPassClipStack(Rect::MakeLTRB(0, 0, 100, 100));

  EXPECT_TRUE(recorder.GetReplayEntities().empty());

  Entity entity;
  recorder.RecordEntity(entity, Contents::ClipCoverage::Type::kRestore);
  EXPECT_TRUE(recorder.GetReplayEntities().empty());
}

TEST(EntityPassClipStackTest, CanAppendNoChange) {
  EntityPassClipStack recorder =
      EntityPassClipStack(Rect::MakeLTRB(0, 0, 100, 100));

  EXPECT_TRUE(recorder.GetReplayEntities().empty());

  Entity entity;
  recorder.RecordEntity(entity, Contents::ClipCoverage::Type::kNoChange);
  EXPECT_TRUE(recorder.GetReplayEntities().empty());
}

TEST(EntityPassClipStackTest, AppendCoverageNoChange) {
  EntityPassClipStack recorder =
      EntityPassClipStack(Rect::MakeLTRB(0, 0, 100, 100));

  EXPECT_EQ(recorder.GetClipCoverageLayers()[0].coverage,
            Rect::MakeSize(Size::MakeWH(100, 100)));
  EXPECT_EQ(recorder.GetClipCoverageLayers()[0].clip_depth, 0u);

  Entity entity;
  recorder.AppendClipCoverage(
      Contents::ClipCoverage{
          .type = Contents::ClipCoverage::Type::kNoChange,
          .coverage = std::nullopt,
      },
      entity, 0, Point(0, 0));

  EXPECT_EQ(recorder.GetClipCoverageLayers()[0].coverage,
            Rect::MakeSize(Size::MakeWH(100, 100)));
  EXPECT_EQ(recorder.GetClipCoverageLayers()[0].clip_depth, 0u);
}

TEST(EntityPassClipStackTest, AppendAndRestoreClipCoverage) {
  EntityPassClipStack recorder =
      EntityPassClipStack(Rect::MakeLTRB(0, 0, 100, 100));

  ASSERT_EQ(recorder.GetClipCoverageLayers().size(), 1u);

  // Push a clip.
  Entity entity;
  entity.SetClipDepth(0);
  recorder.AppendClipCoverage(
      Contents::ClipCoverage{
          .type = Contents::ClipCoverage::Type::kAppend,
          .coverage = Rect::MakeLTRB(50, 50, 55, 55),
      },
      entity, 0, Point(0, 0));

  ASSERT_EQ(recorder.GetClipCoverageLayers().size(), 2u);
  EXPECT_EQ(recorder.GetClipCoverageLayers()[1].coverage,
            Rect::MakeLTRB(50, 50, 55, 55));
  EXPECT_EQ(recorder.GetClipCoverageLayers()[1].clip_depth, 1u);
  EXPECT_EQ(recorder.GetReplayEntities().size(), 1u);

  // Restore the clip.
  entity.SetClipDepth(0);
  recorder.AppendClipCoverage(
      Contents::ClipCoverage{
          .type = Contents::ClipCoverage::Type::kRestore,
          .coverage = Rect::MakeLTRB(50, 50, 55, 55),
      },
      entity, 0, Point(0, 0));

  ASSERT_EQ(recorder.GetClipCoverageLayers().size(), 1u);
  EXPECT_EQ(recorder.GetClipCoverageLayers()[0].coverage,
            Rect::MakeSize(Size::MakeWH(100, 100)));
  EXPECT_EQ(recorder.GetClipCoverageLayers()[0].clip_depth, 0u);
  EXPECT_EQ(recorder.GetReplayEntities().size(), 0u);
}

TEST(EntityPassClipStackTest, UnbalancedRestore) {
  EntityPassClipStack recorder =
      EntityPassClipStack(Rect::MakeLTRB(0, 0, 100, 100));

  ASSERT_EQ(recorder.GetClipCoverageLayers().size(), 1u);

  // Restore the clip.
  Entity entity;
  entity.SetClipDepth(0);
  recorder.AppendClipCoverage(
      Contents::ClipCoverage{
          .type = Contents::ClipCoverage::Type::kRestore,
          .coverage = Rect::MakeLTRB(50, 50, 55, 55),
      },
      entity, 0, Point(0, 0));

  ASSERT_EQ(recorder.GetClipCoverageLayers().size(), 1u);
  EXPECT_EQ(recorder.GetClipCoverageLayers()[0].coverage,
            Rect::MakeSize(Size::MakeWH(100, 100)));
  EXPECT_EQ(recorder.GetClipCoverageLayers()[0].clip_depth, 0u);
  EXPECT_EQ(recorder.GetReplayEntities().size(), 0u);
}

TEST(EntityPassClipStackTest, ClipAndRestoreWithSubpasses) {
  EntityPassClipStack recorder =
      EntityPassClipStack(Rect::MakeLTRB(0, 0, 100, 100));

  ASSERT_EQ(recorder.GetClipCoverageLayers().size(), 1u);

  // Push a clip.
  Entity entity;
  entity.SetClipDepth(0u);
  recorder.AppendClipCoverage(
      Contents::ClipCoverage{
          .type = Contents::ClipCoverage::Type::kAppend,
          .coverage = Rect::MakeLTRB(50, 50, 55, 55),
      },
      entity, 0, Point(0, 0));

  ASSERT_EQ(recorder.GetClipCoverageLayers().size(), 2u);
  EXPECT_EQ(recorder.GetClipCoverageLayers()[1].coverage,
            Rect::MakeLTRB(50, 50, 55, 55));
  EXPECT_EQ(recorder.GetClipCoverageLayers()[1].clip_depth, 1u);
  EXPECT_EQ(recorder.GetReplayEntities().size(), 1u);

  // Begin a subpass.
  recorder.PushSubpass(Rect::MakeLTRB(50, 50, 55, 55), 1);
  ASSERT_EQ(recorder.GetClipCoverageLayers().size(), 1u);
  EXPECT_EQ(recorder.GetClipCoverageLayers()[0].coverage,
            Rect::MakeLTRB(50, 50, 55, 55));

  entity.SetClipDepth(1);
  recorder.AppendClipCoverage(
      Contents::ClipCoverage{
          .type = Contents::ClipCoverage::Type::kAppend,
          .coverage = Rect::MakeLTRB(54, 54, 55, 55),
      },
      entity, 0, Point(0, 0));

  EXPECT_EQ(recorder.GetClipCoverageLayers()[1].coverage,
            Rect::MakeLTRB(54, 54, 55, 55));

  // End subpass.
  recorder.PopSubpass();

  EXPECT_EQ(recorder.GetClipCoverageLayers()[1].coverage,
            Rect::MakeLTRB(50, 50, 55, 55));
}

}  // namespace testing
}  // namespace impeller
