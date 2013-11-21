
# Copyright (c) 2011 Maxim Makhinya <maxmah@gmail.com>

set(MSV_IO_PUBLIC_HEADERS
  ./IO/dataHDDIO.h
  ./IO/dataHDDIORaw.h
  ./IO/dataHDDIOOctree.h
  ./IO/dataHDDIOTensor.h
  ./IO/volumeFileInfo.h
  )

set(MSV_IO_SOURCES
  ./IO/dataHDDIO.cpp
  ./IO/dataHDDIORaw.cpp
  ./IO/dataHDDIOOctree.cpp
  ./IO/dataHDDIOTensor.cpp
  ./IO/volumeFileInfo.cpp
  )

set(MSV_TREE_PUBLIC_HEADERS
  ./tree/nodeId.h
  ./tree/rankErrors.h
  ./tree/volumeTreeBase.h
  ./tree/volumeTreeTensor.h
  )

set(MSV_TREE_SOURCES
  ./tree/rankErrors.cpp
  ./tree/volumeTreeBase.cpp
  ./tree/volumeTreeTensor.cpp
  )

set(MSV_TYPES_PUBLIC_HEADERS
  ./types/bits.h
  ./types/box.h
  ./types/limits.h
  ./types/nonCopyable.h
  ./types/plane.h
  ./types/types.h
  ./types/vmmlTypes.h
  ./types/vec2.h
  ./types/vec3.h
  ./types/rectangle.h
  )

set(MSV_TYPES_SOURCES
  ./types/box.cpp
  ./types/box.ipp
  ./types/plane.cpp
  ./types/plane.ipp
  ./types/types.cpp
  ./types/vec2.cpp
  ./types/vec2.ipp
  ./types/vec3.cpp
  ./types/vec3.ipp
  ./types/rectangle.cpp
  )

set(MSV_UTIL_PUBLIC_HEADERS
  ./util/debug.h
  ./util/fileIO.h
  ./util/hlp.h
  ./util/md5.h
#  ./util/pngwriter.h
  ./util/statLogger.h
  ./util/str.h
  ./util/testing.h
  )

set(MSV_UTIL_SOURCES
  ./util/debug.cpp
  ./util/fileIO.cpp
  ./util/md5.cpp
#  ./util/pngwriter.cpp
  ./util/statLogger.cpp
  ./util/str.cpp
  ./util/testing.cpp
  )

