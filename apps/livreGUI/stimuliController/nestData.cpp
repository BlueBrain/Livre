#include <livreGUI/stimuliController/nestData.h>

#include <QVariantList>

#include <boost/foreach.hpp>

namespace livre
{

typedef std::map< std::string, PropertyList  > GeneratorPropertyMap;

const Strings& getGenerators()
{
    static const Strings generators = { "ac_generator",
                                        "dc_generator",
                                        "gamma_sup_generator",
                                        "mip_generator",
                                        "noise_generator",
                                        "poisson_generator",
                                        "poisson_generator_ps",
                                        "ppd_sup_generator",
                                        "sinusoidal_gamma_generator",
                                        "sinusoidal_poisson_generator",
                                        "spike_generator",
                                        "step_current_generator" };
    return generators;
}

void addProperty( PropertyList& properties,
                  const std::string& name,
                  const QVariant& val )
{
    properties.push_back( std::make_pair( name, val ));
}

const PropertyList& getGeneratorProperties( const std::string& generator )
{
    static GeneratorPropertyMap generatorProperties;
    static PropertyList emptyMap;
    if( generatorProperties.empty( ))
    {
        BOOST_FOREACH( const std::string& gen, getGenerators( ))
        {
            generatorProperties[ gen ] = PropertyList();

            PropertyList& map = generatorProperties[ gen ];
            if( gen == "ac_generator" )
            {
                addProperty( map, "amplitude", 0.0 );
                addProperty( map, "frequency", 0.0 );
                addProperty( map, "phase", 0.0 );
                continue;
            }
            else if( gen == "dc_generator" )
            {
                addProperty( map, "amplitude", 0.0 );
                continue;
            }
            else if( gen ==  "gamma_sup_generator" )
            {
                addProperty( map, "gamma_shape", false );
                addProperty( map, "rate", 0.0 );
                continue;
            }
            else if( gen ==  "mip_generator" )
            {
                addProperty( map, "mother_seed", 0 );
                continue;
            }
            else if( gen ==  "noise_generator" )
            {
                addProperty( map, "dt", 1.0 );
                addProperty( map, "frequency", 0.0 );
                addProperty( map, "mean", 0.0 );
                addProperty( map, "phase", 0.0 );
                addProperty( map, "y_0", 0.0 );
                addProperty( map, "y_1", 0.0 );
                continue;
            }
            else if( gen ==  "poisson_generator" )
            {
                addProperty( map, "rate", 0.0 );
                continue;
            }
            else if( gen ==  "poisson_generator_ps" )
            {
                addProperty( map, "rate", 0.0 );
                continue;
            }
            else if( gen ==  "ppd_sup_generator" )
            {
                addProperty( map, "amplitude", 0.0 );
                addProperty( map, "dead_time", 0.0 );
                addProperty( map, "frequency", 0.0 );
                addProperty( map, "rate", 1.0 );
                continue;
            }
            else if( gen ==  "sinusoidal_gamma_generator" )
            {
                addProperty( map, "ac", 0.0 );
                addProperty( map, "dc", 0.0 );
                addProperty( map, "individual_spike_trains", true );
                addProperty( map, "freq", 0.0 );
                addProperty( map, "phi", 0.0 );
                continue;
            }
            else if( gen ==  "sinusoidal_poisson_generator" )
            {
                addProperty( map, "ac", 0.0 );
                addProperty( map,"dc", 0.0 );
                addProperty( map,"individual_spike_trains", true );
                addProperty( map,"freq", 0.0 );
                addProperty( map,"phi", 0.0 );
                continue;
            }
            else if( gen ==  "spike_generator" )
            {
                addProperty( map, "allow_offgrid_spikes", false );
                addProperty( map, "precise_times", false );
                addProperty( map, "shift_now_spikes", true );
                addProperty( map, "spike_times", QList<QVariant>() );
                addProperty( map, "spike_weights", QList<QVariant>() );
                continue;
            }
            else if( gen ==  "step_current_generator" )
            {
                addProperty( map, "amplitude_times", QList<QVariant>() );
                addProperty( map, "amplitude_values", QList<QVariant>() );
                continue;
            }
        }
    }

    GeneratorPropertyMap::const_iterator it =
            generatorProperties.find( generator );
    return it == generatorProperties.cend() ? emptyMap : it->second;
}



std::string getJSON( const PropertyList& list )
{
    std::stringstream jsonStr;

    if( list.empty( ))
        return jsonStr.str();

    jsonStr << "{";

    BOOST_FOREACH( const Property& property, list )
    {
        const std::string name = property.first;
        const QVariant& var = property.second;

        jsonStr << '"' << name << '"' << " : ";
        switch( var.type( ))
        {
            case QVariant::Double:
            {
                jsonStr << var.toDouble();
                break;
            }
            case QVariant::Bool:
            {
                jsonStr << var.toBool();
                break;
            }
            case QVariant::Int:
            {
                jsonStr << var.toInt();
                break;
            }
            case QVariant::List:
            {
                const QList<QVariant>& varList = var.toList();
                if( varList.empty( ))
                    break;

                jsonStr << " [ ";
                BOOST_FOREACH( const QVariant& v, varList )
                {
                    if( v.type() == QVariant::Double )
                    {
                        jsonStr << v.toDouble();
                    }
                    else if( v.type() == QVariant::String )
                    {
                        jsonStr << '"' << v.toString().toStdString() << '"';
                    }
                    if( v != varList.back( ))
                        jsonStr << " , ";
                }
                jsonStr << " ] ";
                break;
            }
            default:
                jsonStr << '"' << var.toString().toStdString() << '"';
                break;
        }

        if( property != list.back( ))
           jsonStr << " , ";
    }

    jsonStr << "}";
    return jsonStr.str();
}

}
