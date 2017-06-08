/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *
 * This file is part of Livre <https://github.com/BlueBrain/Livre>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define BOOST_TEST_MODULE Pipeline

#include <livre/core/pipeline/Filter.h>
#include <livre/core/pipeline/FutureMap.h>
#include <livre/core/pipeline/FuturePromise.h>
#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/Pipeline.h>
#include <livre/core/pipeline/PromiseMap.h>
#include <livre/core/pipeline/SimpleExecutor.h>
#include <livre/core/pipeline/Workers.h>

#include <boost/test/unit_test.hpp>

namespace ut = boost::unit_test;

const uint32_t defaultMeaningOfLife = 42;
const uint32_t defaultThanksForAllTheFish = 51;
const uint32_t addMoreFish = 10;

struct InputData
{
    explicit InputData(uint32_t value = defaultMeaningOfLife)
        : meaningOfLife(value)
    {
    }

    uint32_t meaningOfLife;
};

struct OutputData
{
    explicit OutputData(uint32_t value = defaultThanksForAllTheFish)
        : thanksForAllTheFish(value)
    {
    }

    uint32_t thanksForAllTheFish;
};

class TestFilter : public livre::Filter
{
    void execute(const livre::FutureMap& input,
                 livre::PromiseMap& output) const final
    {
        const std::vector<InputData>& results =
            input.get<InputData>("TestInputData");
        OutputData outputData;

        for (const auto& data : results)
        {
            const InputData& inputData = data;
            outputData.thanksForAllTheFish +=
                inputData.meaningOfLife + addMoreFish;
        }

        output.set("TestOutputData", outputData);
    }

    livre::DataInfos getInputDataInfos() const final
    {
        return {{"TestInputData", livre::getType<InputData>()}};
    }

    livre::DataInfos getOutputDataInfos() const final
    {
        return {{"TestOutputData", livre::getType<OutputData>()}};
    }
};

class ConvertFilter : public livre::Filter
{
    void execute(const livre::FutureMap& input,
                 livre::PromiseMap& output) const final
    {
        const std::vector<OutputData>& results =
            input.get<OutputData>("ConvertInputData");

        InputData inputData;
        for (const auto& data : results)
        {
            inputData.meaningOfLife = data.thanksForAllTheFish + addMoreFish;
        }

        output.set("ConvertOutputData", inputData);
    }

    livre::DataInfos getInputDataInfos() const final
    {
        return {{"ConvertInputData", livre::getType<OutputData>()}};
    }

    livre::DataInfos getOutputDataInfos() const final
    {
        return {{"ConvertOutputData", livre::getType<InputData>()}};
    }
};

bool check_error(const std::runtime_error&)
{
    return true;
}

BOOST_AUTO_TEST_CASE(testFilterNoInput)
{
    livre::PipeFilterT<TestFilter> pipeFilter("Producer");

    // Execute will fail because there are no inputs where data is retrieved
    BOOST_CHECK_THROW(pipeFilter.execute(), std::logic_error);
    const livre::UniqueFutureMap portFutures(pipeFilter.getPostconditions());

    // Results of the filter will be empty.
    BOOST_CHECK_THROW(portFutures.get<OutputData>("TestOutputData"),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(testFilterWithInput)
{
    livre::PipeFilterT<TestFilter> pipeFilter("Producer");
    const uint32_t inputValue = 90;

    pipeFilter.getPromise("TestInputData").set(InputData(inputValue));
    pipeFilter.execute();

    const livre::UniqueFutureMap portFutures(pipeFilter.getPostconditions());
    const OutputData& outputData =
        portFutures.get<OutputData>("TestOutputData");
    BOOST_CHECK_EQUAL(outputData.thanksForAllTheFish, 151);
}

BOOST_AUTO_TEST_CASE(testSetAndGetWrongParameters)
{
    TestFilter filter;
    livre::PipeFilterT<TestFilter> pipeFilter("Producer");
    pipeFilter.getPromise("TestInputData").set(InputData());

    BOOST_CHECK_THROW(pipeFilter.getPromise("InputData").set(OutputData(0)),
                      std::runtime_error);
    pipeFilter.execute();
    const livre::UniqueFutureMap portFutures(pipeFilter.getPostconditions());
    BOOST_CHECK_THROW(portFutures.get<InputData>("TestOutputData"),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(testInvalidConnection)
{
    livre::PipeFilterT<TestFilter> pipeFilter1("Producer");
    livre::PipeFilterT<TestFilter> pipeFilter2("Consumer");

    BOOST_CHECK_THROW(pipeFilter1.connect("TestOutputData", pipeFilter2,
                                          "NotExistingConnection"),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(testConnection)
{
    livre::PipeFilterT<TestFilter> pipeInput("Producer");
    livre::PipeFilterT<TestFilter> pipeOutput("Consumer");

    livre::PipeFilterT<ConvertFilter> convertPipeFilter("Converter");

    pipeInput.connect("TestOutputData", convertPipeFilter, "ConvertInputData");
    convertPipeFilter.connect("ConvertOutputData", pipeOutput, "TestInputData");

    const uint32_t inputValue = 90;
    pipeInput.getPromise("TestInputData").set(InputData(inputValue));
    pipeInput.execute();
    convertPipeFilter.execute();
    pipeOutput.execute();

    const livre::UniqueFutureMap portFutures(pipeOutput.getPostconditions());
    const OutputData& outputData =
        portFutures.get<OutputData>("TestOutputData");
    BOOST_CHECK_EQUAL(outputData.thanksForAllTheFish, 222);
}

livre::Pipeline createPipeline(const uint32_t inputValue,
                               size_t nConvertFilter = 1)
{
    livre::Pipeline pipeline;
    livre::PipeFilter pipeInput = pipeline.add<TestFilter>("Producer");
    livre::PipeFilter pipeOutput = pipeline.add<TestFilter>("Consumer");

    for (size_t i = 0; i < nConvertFilter; ++i)
    {
        std::stringstream name;
        name << "Converter" << i;
        livre::PipeFilter convertPipeFilter =
            pipeline.add<ConvertFilter>(name.str());
        pipeInput.connect("TestOutputData", convertPipeFilter,
                          "ConvertInputData");
        convertPipeFilter.connect("ConvertOutputData", pipeOutput,
                                  "TestInputData");
    }

    pipeInput.getPromise("TestInputData").set(InputData(inputValue));
    return pipeline;
}

BOOST_AUTO_TEST_CASE(testSynchronousPipeline)
{
    const uint32_t inputValue = 90;

    livre::Pipeline pipeline = createPipeline(inputValue, 1);
    pipeline.execute();

    const livre::Executable& pipeOutput = pipeline.getExecutable("Consumer");
    const livre::UniqueFutureMap portFutures(pipeOutput.getPostconditions());
    const OutputData& outputData =
        portFutures.get<OutputData>("TestOutputData");
    BOOST_CHECK_EQUAL(outputData.thanksForAllTheFish, 222);
}

BOOST_AUTO_TEST_CASE(testWaitPipeline)
{
    const uint32_t inputValue = 90;
    livre::Pipeline pipeline = createPipeline(inputValue, 1);

    livre::SimpleExecutor executor("test", 2);
    const livre::FutureMap pipelineFutures(pipeline.schedule(executor));
    pipelineFutures.wait();

    const livre::Executable& pipeOutput = pipeline.getExecutable("Consumer");
    const livre::UniqueFutureMap portFutures(pipeOutput.getPostconditions());
    const OutputData& outputData =
        portFutures.get<OutputData>("TestOutputData");
    BOOST_CHECK_EQUAL(outputData.thanksForAllTheFish, 222);
}

BOOST_AUTO_TEST_CASE(testAsynchronousPipeline)
{
    const uint32_t inputValue = 90;

    livre::Pipeline pipeline = createPipeline(inputValue, 1);
    livre::SimpleExecutor executor("test", 2);

    pipeline.schedule(executor);
    const livre::Executable& pipeOutput = pipeline.getExecutable("Consumer");
    const livre::UniqueFutureMap portFutures(pipeOutput.getPostconditions());
    const OutputData& outputData =
        portFutures.get<OutputData>("TestOutputData");
    BOOST_CHECK_EQUAL(outputData.thanksForAllTheFish, 222);
}

BOOST_AUTO_TEST_CASE(testPromiseFuture)
{
    livre::Promise promise(
        livre::DataInfo("Helloworld", livre::getType<uint32_t>()));
    livre::Future future1 = promise.getFuture();
    livre::Future future2 = promise.getFuture();
    BOOST_CHECK(future1 == future2);

    // Promise only be set with the right type
    BOOST_CHECK_THROW(promise.set(12.0f), std::runtime_error);

    promise.set(42u);
    BOOST_CHECK_EQUAL(future1.get<uint32_t>(), 42u);
    BOOST_CHECK_EQUAL(future2.get<uint32_t>(), 42u);

    // Promise only can be set once
    BOOST_CHECK_THROW(promise.set(42u), std::runtime_error);

    promise.reset();
    livre::Future future3 = promise.getFuture();
    BOOST_CHECK(future1 != future3);

    // Promise is set with explicit conversion
    promise.set<uint32_t>(43.0f);
    BOOST_CHECK_EQUAL(future1.get<uint32_t>(), 42u);
    BOOST_CHECK_EQUAL(future3.get<uint32_t>(), 43u);
}

BOOST_AUTO_TEST_CASE(testFutureMaps)
{
    livre::PipeFilterT<TestFilter> pipeFilter("Producer");
    const livre::Futures& uniqueFutures = pipeFilter.getPostconditions();

    // The output futures are unique
    const livre::UniqueFutureMap portFuturesUnique(uniqueFutures);
    const livre::FutureMap portFutures1(uniqueFutures);

    livre::Futures nonUniqueFutures = {uniqueFutures.front(),
                                       uniqueFutures.front()};

    BOOST_CHECK_THROW(const livre::UniqueFutureMap portFuturesNonUnique(
                          nonUniqueFutures),
                      std::logic_error);
    const livre::FutureMap portFutures2(nonUniqueFutures);
}
