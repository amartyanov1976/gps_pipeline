#include <gtest/gtest.h>
#include "app/pipeline.h"
#include "mocks/mock_display.h"
#include "mocks/mock_parser.h"
#include "mocks/mock_history.h"
#include "mocks/mock_filter.h"

class PipelineTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto parser = std::make_unique<MockParser>();
        auto history = std::make_unique<MockHistory>(5);
        auto display = std::make_unique<MockDisplay>();

        mockParser_ = parser.get();
        mockHistory_ = history.get();
        mockDisplay_ = display.get();

        pipeline = std::make_unique<GpsPipeline>(
            std::move(parser), std::move(history), std::move(display));
    }

    GpsPoint createValidPoint(double lat = 48.1173, double lon = 11.5167,
                               double speed = 41.5, int sats = 8) {
        GpsPoint p;
        p.latitude = lat;
        p.longitude = lon;
        p.speed = speed;
        p.satellites = sats;
        p.course = 84.4;
        p.altitude = 545.4;
        p.hdop = 0.9f;
        p.isValid = true;
        p.timestamp = 123519000;
        return p;
    }

    GpsPoint createInvalidPoint() {
        GpsPoint p;
        p.isValid = false;
        p.timestamp = 123519000;
        return p;
    }

    MockParser* mockParser_ = nullptr;
    MockHistory* mockHistory_ = nullptr;
    MockDisplay* mockDisplay_ = nullptr;
    std::unique_ptr<GpsPipeline> pipeline;
};

// --- Basic processing ---

TEST_F(PipelineTest, Process_ValidPoint_UpdatesStatistics) {
    mockParser_->enqueueResult(createValidPoint());

    pipeline->process("any line");

    EXPECT_EQ(pipeline->getProcessedCount(), 1);
    EXPECT_EQ(pipeline->getValidCount(), 1);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 0);
    EXPECT_EQ(mockDisplay_->getPointCount(), 1);
}

TEST_F(PipelineTest, Process_ParseError_IncreasesErrorCount) {
    mockParser_->enqueueResult(std::nullopt);

    pipeline->process("bad line");

    EXPECT_EQ(pipeline->getProcessedCount(), 1);
    EXPECT_EQ(pipeline->getValidCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 1);
    EXPECT_EQ(mockDisplay_->getErrorCount(), 1);
}

TEST_F(PipelineTest, Process_InvalidFix_ShowsInvalidFix) {
    mockParser_->enqueueResult(createInvalidPoint());

    pipeline->process("some line");

    EXPECT_EQ(pipeline->getProcessedCount(), 1);
    EXPECT_EQ(pipeline->getValidCount(), 0);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 0);
    EXPECT_EQ(mockDisplay_->getInvalidFixCount(), 1);
}

TEST_F(PipelineTest, Process_EmptyLine_CountsAsProcessed) {
    mockParser_->enqueueResult(std::nullopt);

    pipeline->process("");

    EXPECT_EQ(pipeline->getProcessedCount(), 1);
    EXPECT_EQ(pipeline->getErrorCount(), 1);
}

// --- Filters ---

TEST_F(PipelineTest, Process_WithRejectingFilter_RejectsPoint) {
    pipeline->addFilter(std::make_unique<MockFilter>("Rejector", FilterResult::REJECT));

    mockParser_->enqueueResult(createValidPoint());
    pipeline->process("line");

    EXPECT_EQ(pipeline->getRejectedCount(), 1);
    EXPECT_EQ(pipeline->getValidCount(), 0);
    EXPECT_EQ(mockDisplay_->getErrorCount(), 1);
}

TEST_F(PipelineTest, Process_WithPassingFilter_AcceptsPoint) {
    pipeline->addFilter(std::make_unique<MockFilter>("Passer", FilterResult::PASS));

    mockParser_->enqueueResult(createValidPoint());
    pipeline->process("line");

    EXPECT_EQ(pipeline->getValidCount(), 1);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
    EXPECT_EQ(mockDisplay_->getPointCount(), 1);
}

TEST_F(PipelineTest, Process_WithStopFilter_AcceptsPoint) {
    pipeline->addFilter(std::make_unique<MockFilter>("Stopper", FilterResult::STOP));

    mockParser_->enqueueResult(createValidPoint());
    pipeline->process("line");

    EXPECT_EQ(pipeline->getValidCount(), 1);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
}

TEST_F(PipelineTest, Process_WithDisabledFilter_DoesNotReject) {
    auto filter = std::make_unique<MockFilter>("Disabled", FilterResult::REJECT);
    filter->setEnabled(false);
    pipeline->addFilter(std::move(filter));

    mockParser_->enqueueResult(createValidPoint());
    pipeline->process("line");

    EXPECT_EQ(pipeline->getValidCount(), 1);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
}

TEST_F(PipelineTest, Process_MultipleFilters_RespectsPriority) {
    auto first = std::make_unique<MockFilter>("First", FilterResult::REJECT);
    auto firstPtr = first.get();

    auto second = std::make_unique<MockFilter>("Second", FilterResult::PASS);
    auto secondPtr = second.get();

    pipeline->addFilter(std::move(second), 2);
    pipeline->addFilter(std::move(first), 1);

    mockParser_->enqueueResult(createValidPoint());
    pipeline->process("line");

    EXPECT_EQ(firstPtr->getProcessCalls(), 1);
    EXPECT_EQ(secondPtr->getProcessCalls(), 0);
    EXPECT_EQ(pipeline->getRejectedCount(), 1);
}

TEST_F(PipelineTest, Process_StopFilter_PreventsSubsequentFilters) {
    auto stopper = std::make_unique<MockFilter>("Stopper", FilterResult::STOP);
    auto after = std::make_unique<MockFilter>("After", FilterResult::REJECT);
    auto afterPtr = after.get();

    pipeline->addFilter(std::move(stopper), 1);
    pipeline->addFilter(std::move(after), 2);

    mockParser_->enqueueResult(createValidPoint());
    pipeline->process("line");

    EXPECT_EQ(afterPtr->getProcessCalls(), 0);
    EXPECT_EQ(pipeline->getValidCount(), 1);
}

// --- History integration ---

TEST_F(PipelineTest, Process_ValidPoint_AddsToHistory) {
    mockParser_->enqueueResult(createValidPoint());

    pipeline->process("line");

    EXPECT_EQ(mockHistory_->size(), 1);
    auto last = mockHistory_->getLastValid();
    ASSERT_TRUE(last.has_value());
    EXPECT_NEAR(last->latitude, 48.1173, 0.0001);
}

TEST_F(PipelineTest, Process_RejectedPoint_DoesNotAddToHistory) {
    pipeline->addFilter(std::make_unique<MockFilter>("Rejector", FilterResult::REJECT));

    mockParser_->enqueueResult(createValidPoint());
    pipeline->process("line");

    EXPECT_EQ(mockHistory_->size(), 0);
}

TEST_F(PipelineTest, GetHistory_ReturnsHistoryReference) {
    mockParser_->enqueueResult(createValidPoint());
    pipeline->process("line");

    const IHistory& history = pipeline->getHistory();
    EXPECT_EQ(history.size(), 1);
}

// --- Statistics ---

TEST_F(PipelineTest, GetStatistics_InitiallyZero) {
    EXPECT_EQ(pipeline->getProcessedCount(), 0);
    EXPECT_EQ(pipeline->getValidCount(), 0);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 0);
}

TEST_F(PipelineTest, Process_MultipleLines_CorrectStatistics) {
    mockParser_->enqueueResult(createValidPoint());
    mockParser_->enqueueResult(createValidPoint());
    mockParser_->enqueueResult(std::nullopt);
    mockParser_->enqueueResult(createInvalidPoint());

    pipeline->process("valid1");
    pipeline->process("valid2");
    pipeline->process("error");
    pipeline->process("invalid");

    EXPECT_EQ(pipeline->getProcessedCount(), 4);
    EXPECT_EQ(pipeline->getValidCount(), 2);
    EXPECT_EQ(pipeline->getErrorCount(), 1);
    EXPECT_EQ(mockDisplay_->getPointCount(), 2);
    EXPECT_EQ(mockDisplay_->getInvalidFixCount(), 1);
    EXPECT_EQ(mockDisplay_->getErrorCount(), 1);
}

// --- Parser receives lines ---

TEST_F(PipelineTest, Process_PassesLineToParser) {
    mockParser_->enqueueResult(std::nullopt);

    pipeline->process("$GPRMC,test");

    auto calls = mockParser_->getCalls();
    ASSERT_EQ(calls.size(), 1);
    EXPECT_EQ(calls[0], "$GPRMC,test");
}

// --- SetHistorySize ---

TEST_F(PipelineTest, SetHistorySize_DelegatesToHistory) {
    pipeline->setHistorySize(20);
    EXPECT_EQ(mockHistory_->getMaxSize(), 20);
}
