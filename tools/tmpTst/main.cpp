
#include <msv/IO/volumeFileInfo.h>

#include <msv/util/fileIO.h>

using namespace massVolVis;

int main( )//int argc, char **argv )
{
    // isDir / isFile test
    std::cout << "1:  0 == " << util::isDir( "" ) << std::endl;
    std::cout << "2:  1 == " << util::isDir( "/" ) << std::endl;
    std::cout << "3:  1 == " << util::isDir( "./" ) << std::endl;
    std::cout << "4:  1 == " << util::isDir( "/home" ) << std::endl;
    std::cout << "5:  1 == " << util::isDir( "/home/" ) << std::endl;
    std::cout << "6:  1 == " << util::isDir( "/home/maxus" ) << std::endl;
    std::cout << "7:  0 == " << util::isDir( "/home/maxu" ) << std::endl;
    std::cout << "8:  0 == " << util::isDir( "/home/maxus/fix_keys.sh" ) << std::endl;
    std::cout << "9:  1 == " << util::isFile( "/home/maxus/fix_keys.sh" ) << std::endl;
    std::cout << "10: 0 == " << util::isFile( "/home/maxus" ) << std::endl;
    std::cout << "11: 0 == " << util::isFile( "/home/maxus/fix_keyssh" ) << std::endl;
    std::cout << "12: 0 == " << util::isFile( "/home/ma" ) << std::endl;
    std::cout << "13: 1 == " << util::fileOrDirExists( "/home/" ) << std::endl;
    std::cout << "14: 1 == " << util::fileOrDirExists( "/home/maxus/fix_keys.sh" ) << std::endl;
    std::cout << "15: 0 == " << util::fileOrDirExists( "/home/maxus/fix_kys.sh" ) << std::endl;

    std::string fdNames[]  = { "/home/maxus/fix_kys.sh", "/home/maxus", "/home/maxus/", "/home/maxu", "./" };
    std::string fdNamesR[] = { "/home/maxus/",           "/home/maxus", "/home/maxus/", "/home/",     "./" };
    for( int i = 0; i < 4; ++i )
        std::cout << "src: " << fdNames[i] << " res: " << util::getDirName( fdNames[i] ).c_str() << " exp: " << fdNamesR[i] << std::endl;
    return 0;

    VolumeFileInfo fi;
    std::cout << fi;

    fi.setDefaults();
    fi.setVersion(       3 );
    fi.setSourceDims(    Vec3_ui16( 10, 20, 555 ));
    fi.setBlockDim(     64 );
    fi.setBytesNum(      2 );
    fi.setDataFileName( "tmp.raw" );
    std::cout << fi;

//    fi.save( "/Volumes/Home/Users/maxus/tmp/test.dat" );
    fi.save( "/Users/maxus/tmp/test.dat" );

    VolumeFileInfo fi2;
//    fi2.load( "/Volumes/Home/Users/maxus/tmp/test.dat" );
    fi2.load( "/Users/maxus/tmp/test.dat" );
    std::cout << "file2: " << std::endl << fi2;

    return 0;
}
