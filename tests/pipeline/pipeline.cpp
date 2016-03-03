/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
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

#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/Filter.h>
#include <livre/core/pipeline/Pipeline.h>
#include <livre/core/pipeline/SimpleExecutor.h>
#include <livre/core/pipeline/Workers.h>
#include <livre/core/pipeline/FutureMap.h>
#include <livre/core/pipeline/PromiseMap.h>
#include <livre/core/pipeline/Future.h>
#include <livre/core/pipeline/Promise.h>

#include <boost/test/unit_test.hpp>

namespace ut = boost::unit_test;

const uint32_t defaultMeaningOfLife = 42;
const uint32_t defaultThanksForAllTheFish = 51;
const uint32_t addMoreFish = 10;

struct InputData
{
    explicit InputData( uint32_t value = defaultMeaningOfLife )
        : meaningOfLife( value )
    {}

    uint32_t meaningOfLife;
};

struct OutputData
{
    explicit OutputData( uint32_t value = defaultThanksForAllTheFish )
        : thanksForAllTheFish( value )
    {}

    uint32_t thanksForAllTheFish;
};

class TestFilter : public livre::Filter
{
    void execute( const livre::InFutureMap& input, livre::PromiseMap& output ) const final
    {

        const livre::ResultsT< InputData >& results = input.get< InputData >( "TestInputData" );
        OutputData outputData;

        for( const auto& data: results )
        {
             const InputData& inputData = data;
             outputData.thanksForAllTheFish += inputData.meaningOfLife + addMoreFish;
        }

        output.set( "TestOutputData", outputData );

    }

    livre::PortInfos getInputPorts() const final
    {
        livre::PortInfos inputPorts;
        livre::addPortInfo< InputData >( inputPorts, "TestInputData" );
        return inputPorts;
    }

    livre::PortInfos getOutputPorts() const final
    {
        livre::PortInfos outputPorts;
        livre::addPortInfo< OutputData >( outputPorts, "TestOutputData");
        return outputPorts;
    }
};

class ConvertFilter : public livre::Filter
{
    void execute( const livre::InFutureMap& input, livre::PromiseMap& output ) const final
    {
        const livre::ResultsT< OutputData >& results = input.get< OutputData >( "ConvertInputData" );

        InputData inputData;
        for( const auto& data: results )
        {
             inputData.meaningOfLife = data.thanksForAllTheFish + addMoreFish;
        }

        output.set( "ConvertOutputData", inputData );
    }

    livre::PortInfos getInputPorts() const final
    {
        livre::PortInfos outputPorts;
        livre::addPortInfo< OutputData >( outputPorts, "ConvertInputData");
        return outputPorts;
    }

    livre::PortInfos getOutputPorts() const final
    {
        livre::PortInfos inputPorts;
        livre::addPortInfo< InputData >( inputPorts, "ConvertOutputData" );
        return inputPorts;
    }
};

bool check_error( const std::runtime_error& ) { return true; }

BOOST_AUTO_TEST_CASE( testFilterNoInput )
{
    livre::FilterPtr filter( new TestFilter( ));
    livre::PipeFilterPtr pipeFilter( new livre::PipeFilter( "Producer", filter ));

    // Execute will fail because there are no inputs where data is retrieved
    BOOST_CHECK_EXCEPTION( pipeFilter->execute(), std::runtime_error, check_error );
    const livre::OutFutureMap portFutures( pipeFilter->getPostconditions( ));

    // Results of the filter will be empty.
    BOOST_CHECK_EXCEPTION( portFutures.get< OutputData >( "TestOutputData" ),
                           std::runtime_error, check_error );
}

BOOST_AUTO_TEST_CASE( testFilterWithInput )
{
    livre::FilterPtr filter( new TestFilter( ));
    livre::PipeFilterPtr pipeFilter( new livre::PipeFilter( "Producer", filter ));
    const uint32_t inputValue = 90;

    pipeFilter->getPromise( "TestInputData" )->set( InputData( inputValue ));
    pipeFilter->execute();
    const livre::OutFutureMap portFutures( pipeFilter->getPostconditions( ));
    const OutputData& outputData =
            portFutures.get< OutputData >( "TestOutputData" );
    BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, 151 );
}

BOOST_AUTO_TEST_CASE( testSetAndGetWrongParameters )
{
    livre::FilterPtr filter( new TestFilter( ));
    livre::PipeFilterPtr pipeFilter( new livre::PipeFilter( "Producer", filter ));
    pipeFilter->getPromise( "TestInputData" )->set( InputData());

    BOOST_CHECK_EXCEPTION( pipeFilter->getPromise( "InputData" )->set( OutputData( 0 )),
                           std::runtime_error, check_error );
    pipeFilter->execute();
    const livre::OutFutureMap portFutures( pipeFilter->getPostconditions());
    BOOST_CHECK_EXCEPTION( portFutures.get<InputData>( "TestOutputData" ),
                           std::runtime_error, check_error );
}

BOOST_AUTO_TEST_CASE( testInvalidConnection )
{
    livre::FilterPtr filter( new TestFilter( ));
    livre::PipeFilterPtr pipeFilter1( new livre::PipeFilter( "Producer", filter ));
    livre::PipeFilterPtr pipeFilter2( new livre::PipeFilter( "Consumer", filter ));

    BOOST_CHECK_EXCEPTION( pipeFilter1->connect( "TestOutputData",
                                                 pipeFilter2,
                                                 "Helloworld" ),
                           std::runtime_error, check_error );
}

BOOST_AUTO_TEST_CASE( testConnection )
{
    livre::FilterPtr filter( new TestFilter( ));
    livre::PipeFilterPtr pipeInput( new livre::PipeFilter( "Producer", filter ) );
    livre::PipeFilterPtr pipeOutput( new livre::PipeFilter( "Consumer", filter ) );

    livre::FilterPtr convertFilter( new ConvertFilter( ));
    livre::PipeFilterPtr convertPipeFilter( new livre::PipeFilter( "Converter", convertFilter ) );

    pipeInput->connect( "TestOutputData", convertPipeFilter, "ConvertInputData" );
    convertPipeFilter->connect( "ConvertOutputData", pipeOutput, "TestInputData" );

    const uint32_t inputValue = 90;
    pipeInput->getPromise( "TestInputData" )->set( InputData( inputValue ));
    pipeInput->execute();
    convertPipeFilter->execute();
    pipeOutput->execute();

    const livre::OutFutureMap portFutures( pipeOutput->getPostconditions( ));
    const OutputData& outputData =
            portFutures.get< OutputData >( "TestOutputData" );
    BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, 222 );
}

livre::PipelinePtr createPipeline( livre::PipeFilterPtr& pipeOutput,
                                   uint32_t inputValue,
                                   size_t nConvertFilter = 1 )
{
    livre::FilterPtr filter( new TestFilter( ));
    livre::PipelinePtr pipeline( new livre::Pipeline( ));
    livre::PipeFilterPtr pipeInput = pipeline->add( "Producer", filter );
    pipeOutput = pipeline->add( "Consumer", filter );

    livre::FilterPtr convertFilter( new ConvertFilter( ));
    for( size_t i = 0; i < nConvertFilter; ++i )
    {
        std::stringstream name;
        name << "Converter" << i;
        livre::PipeFilterPtr convertPipeFilter = pipeline->add( name.str(), convertFilter );
        pipeInput->connect( "TestOutputData", convertPipeFilter, "ConvertInputData" );
        convertPipeFilter->connect( "ConvertOutputData", pipeOutput, "TestInputData" );
    }

    pipeInput->getPromise( "TestInputData" )->set( InputData( inputValue ));
    return pipeline;
}

livre::ExecutorPtr createExecutor(const size_t nbOfWorkerThreads )
{
    livre::ExecutorPtr executor( new livre::SimpleExecutor( nbOfWorkerThreads ));
    return executor;
}

BOOST_AUTO_TEST_CASE( testSynchronousPipeline )
{
    livre::PipeFilterPtr pipeOutput;
    const uint32_t inputValue = 90;
    livre::PipelinePtr pipeline = createPipeline( pipeOutput, inputValue );
    pipeline->execute();

    const livre::OutFutureMap portFutures( pipeOutput->getPostconditions( ));
    const OutputData& outputData = portFutures.get< OutputData >( "TestOutputData" );
    BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, 222 );
}

BOOST_AUTO_TEST_CASE( testWaitPipeline )
{
    livre::PipeFilterPtr pipeOutput;
    const uint32_t inputValue = 90;
    livre::PipelinePtr pipeline = createPipeline( pipeOutput, inputValue );
    livre::ExecutorPtr executor = createExecutor( 2 );

    const livre::FutureMap pipelineFutures( executor->execute( pipeline->getExecutables( )));
    pipelineFutures.wait();

    const livre::OutFutureMap portFutures( pipeOutput->getPostconditions( ));
    const OutputData& outputData = portFutures.get< OutputData >( "TestOutputData" );
    BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, 222 );
}

BOOST_AUTO_TEST_CASE( testAsynchronousPipeline )
{
    livre::PipeFilterPtr pipeOutput;
    const uint32_t inputValue = 90;
    livre::PipelinePtr pipeline = createPipeline( pipeOutput, inputValue );
    livre::ExecutorPtr executor = createExecutor( 2 );

    executor->execute( pipeline->getExecutables( ));

    const livre::OutFutureMap portFutures( pipeOutput->getPostconditions( ));
    const OutputData& outputData = portFutures.get< OutputData >( "TestOutputData" );
    BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, 222 );
}

BOOST_AUTO_TEST_CASE( testOneToManyManyToOnePipeline )
{
    // Try using 1 execution thread, output result should not change
    {
        const size_t convertFilterCount = 10;
        const uint32_t inputValue = 90;
        livre::PipeFilterPtr pipeOutput;

        livre::PipelinePtr pipeline = createPipeline( pipeOutput,
                                                      inputValue,
                                                      convertFilterCount );
        livre::ExecutorPtr executor = createExecutor( 1 );
        executor->execute( pipeline->getExecutables( ));

        const livre::OutFutureMap portFutures( pipeOutput->getPostconditions( ));
        const OutputData& outputData = portFutures.get< OutputData >( "TestOutputData" );
        BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, 1761 );
    }

    // Try using 8 execution thread, output result should not change
    {
        const size_t convertFilterCount = 10;
        const uint32_t inputValue = 90;
        livre::PipeFilterPtr pipeOutput;

        livre::PipelinePtr pipeline = createPipeline( pipeOutput,
                                                      inputValue,
                                                      convertFilterCount );
        livre::ExecutorPtr executor = createExecutor( 8 );
        executor->execute( pipeline->getExecutables( ));

        const livre::OutFutureMap portFutures( pipeOutput->getPostconditions( ));
        const OutputData& outputData = portFutures.get< OutputData >( "TestOutputData" );
        BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, 1761 );
    }


}

