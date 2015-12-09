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

#include <boost/test/unit_test.hpp>

namespace ut = boost::unit_test;

const uint32_t defaultMeaningOfLife = 42;
const uint32_t defaultThanksForAllTheFish = 51;
const uint32_t addMoreFish = 10;

struct InputData
{
    InputData( uint32_t value = defaultMeaningOfLife )
        : meaningOfLife( value )
    {}

    uint32_t meaningOfLife;
};

struct OutputData
{
    OutputData( uint32_t value = defaultThanksForAllTheFish )
        : thanksForAllTheFish( value )
    {}

    uint32_t thanksForAllTheFish;
};

class TestFilter : public livre::Filter
{
    void execute( livre::PipeFilter& pipeFilter ) const final
    {
        const size_t inputSize = pipeFilter.getInputSize( "InputData" );

        OutputData outputData;
        for( size_t i = 0; i < inputSize; ++i )
        {
            const InputData& inputData =
                    pipeFilter.getInputValue< InputData >( "InputData", i );

             outputData.thanksForAllTheFish += inputData.meaningOfLife + addMoreFish;
        }

        pipeFilter.setOutput( "OutputData", outputData );

    }

    void getInputPorts( livre::PortInfos& inputPorts ) const final
    {
        livre::addPortInfo( inputPorts, "InputData", InputData());
    }

    void getOutputPorts( livre::PortInfos& outputPorts ) const final
    {
        livre::addPortInfo( outputPorts, "OutputData", OutputData());
    }
};


class ConvertFilter : public livre::Filter
{
    void execute( livre::PipeFilter& pipeFilter ) const final
    {
        const OutputData& outputData =
                pipeFilter.getInputValue< OutputData >( "OutputData" );

        InputData inputData;
        inputData.meaningOfLife = outputData.thanksForAllTheFish + addMoreFish;
        pipeFilter.setOutput( "InputData", inputData );

    }

    void getInputPorts( livre::PortInfos& inputPorts ) const final
    {
        livre::addPortInfo( inputPorts, "OutputData", OutputData( ));
    }

    void getOutputPorts( livre::PortInfos& outputPorts ) const final
    {
        livre::addPortInfo( outputPorts, "InputData", InputData( ));
    }
};

bool check_error( const std::runtime_error& ) { return true; }

BOOST_AUTO_TEST_CASE( testFilterNoInput )
{
    livre::FilterPtr filter( new TestFilter( ));
    livre::PipeFilterPtr pipeFilter( new livre::PipeFilter( filter ));
    pipeFilter->execute();
    const OutputData& outputData = pipeFilter->getOutputValue<OutputData>( "OutputData" );
    BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, defaultThanksForAllTheFish );
}

BOOST_AUTO_TEST_CASE( testFilterWithInput )
{
    livre::FilterPtr filter( new TestFilter( ));
    livre::PipeFilterPtr pipeFilter( new livre::PipeFilter( filter ));
    const uint32_t inputValue = 90;
    pipeFilter->setInput( "InputData", InputData( inputValue ));
    pipeFilter->execute();
    const OutputData& outputData = pipeFilter->getOutputValue<OutputData>( "OutputData" );
    BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, 151 );
}

BOOST_AUTO_TEST_CASE( testSetAndGetWrongParameters )
{
    livre::FilterPtr filter( new TestFilter( ));
    livre::PipeFilterPtr pipeFilter( new livre::PipeFilter( filter ));
    BOOST_CHECK_EXCEPTION( pipeFilter->setInput( "InputData", OutputData( 0 )),
                           std::runtime_error, check_error );
    pipeFilter->execute();
    BOOST_CHECK_EXCEPTION( pipeFilter->getOutputValue<InputData>( "OutputData" ),
                           std::runtime_error, check_error );
}

BOOST_AUTO_TEST_CASE( testInvalidConnection )
{
    livre::FilterPtr filter( new TestFilter( ));
    livre::PipeFilterPtr pipeFilter1( new livre::PipeFilter( filter ));
    livre::PipeFilterPtr pipeFilter2( new livre::PipeFilter( filter ));

    BOOST_CHECK_EXCEPTION( livre::connectFilters( pipeFilter1,
                                                  pipeFilter2,
                                                  "Helloworld" ),
                           std::runtime_error, check_error );

    BOOST_CHECK_EXCEPTION( livre::connectFilters( pipeFilter1,
                                                  pipeFilter2,
                                                  "OutputData" ),
                           std::runtime_error, check_error );
}

BOOST_AUTO_TEST_CASE( testConnection )
{
    livre::FilterPtr filter( new TestFilter( ));
    livre::PipeFilterPtr pipeInput( new livre::PipeFilter( filter ) );
    livre::PipeFilterPtr pipeOutput( new livre::PipeFilter( filter ) );

    livre::FilterPtr convertFilter( new ConvertFilter( ));
    livre::PipeFilterPtr convertPipeFilter( new livre::PipeFilter( convertFilter ) );

    livre::connectFilters( pipeInput, convertPipeFilter, "OutputData" );
    livre::connectFilters( convertPipeFilter, pipeOutput, "InputData" );

    const uint32_t inputValue = 90;
    pipeInput->setInput( "InputData", InputData( inputValue ));
    pipeInput->execute();
    convertPipeFilter->execute();
    pipeOutput->execute();

    const OutputData& outputData = pipeOutput->getOutputValue<OutputData>( "OutputData" );
    BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, 222 );
}

livre::PipelinePtr createPipeline( livre::PipeFilterPtr& pipeOutput,
                                   uint32_t inputValue,
                                   size_t nConvertFilter = 1 )
{
    livre::FilterPtr filter( new TestFilter( ));
    livre::PipelinePtr pipeline( new livre::Pipeline( ));
    livre::PipeFilterPtr pipeInput = pipeline->add( filter );
    pipeOutput = pipeline->add( filter );

    livre::FilterPtr convertFilter( new ConvertFilter( ));
    for( size_t i = 0; i < nConvertFilter; ++i )
    {
        livre::PipeFilterPtr convertPipeFilter = pipeline->add( convertFilter );
        livre::connectFilters( pipeInput, convertPipeFilter, "OutputData" );
        livre::connectFilters( convertPipeFilter, pipeOutput, "InputData" );
    }

    pipeInput->setInput( "InputData", InputData( inputValue ));
    return pipeline;
}

livre::ExecutorPtr createExecutor()
{
    livre::WorkersPtr workers( new livre::Workers( 2 ));
    livre::ExecutorPtr executor( new livre::SimpleExecutor( workers ));
    return executor;
}

BOOST_AUTO_TEST_CASE( testSynchronousPipeline )
{
    livre::PipeFilterPtr pipeOutput;
    const uint32_t inputValue = 90;
    livre::PipelinePtr pipeline = createPipeline( pipeOutput, inputValue );
    pipeline->execute();

    const OutputData& outputData = pipeOutput->getOutputValue<OutputData>( "OutputData" );
    BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, 222 );
}

BOOST_AUTO_TEST_CASE( testWaitPipeline )
{
    livre::PipeFilterPtr pipeOutput;
    const uint32_t inputValue = 90;
    livre::PipelinePtr pipeline = createPipeline( pipeOutput, inputValue );
    livre::ExecutorPtr executor = createExecutor();

    executor->execute( pipeline );
    pipeline->waitForAll();

    const OutputData& outputData = pipeOutput->getOutputValue<OutputData>( "OutputData" );
    BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, 222 );
}

BOOST_AUTO_TEST_CASE( testAsynchronousPipeline )
{
    livre::PipeFilterPtr pipeOutput;
    const uint32_t inputValue = 90;
    livre::PipelinePtr pipeline = createPipeline( pipeOutput, inputValue );
    livre::ExecutorPtr executor = createExecutor();

    executor->execute( pipeline );

    const OutputData& outputData = pipeOutput->getOutputValue<OutputData>( "OutputData" );
    BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, 222 );
}

BOOST_AUTO_TEST_CASE( testOneToManyManyToOnePipeline )
{
    const size_t convertFilterCount = 10;
    const uint32_t inputValue = 90;
    livre::PipeFilterPtr pipeOutput;
    livre::PipelinePtr pipeline = createPipeline( pipeOutput,
                                                  inputValue,
                                                  convertFilterCount );
    livre::ExecutorPtr executor = createExecutor();
    executor->execute( pipeline );

    const OutputData& outputData = pipeOutput->getOutputValue<OutputData>( "OutputData" );
    BOOST_CHECK_EQUAL( outputData.thanksForAllTheFish, 1761 );
}

