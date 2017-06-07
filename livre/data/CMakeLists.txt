
# Copyright (c) 2011-2017, EPFL/Blue Brain Project
#                          Daniel.Nachbaur@epfl.ch
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

set(LIVREDATA_PUBLIC_HEADERS
  DataSource.h
  DataSourcePlugin.h
  DataSourceVisitor.h
  DFSTraversal.h
  Frustum.h
  LODNode.h
  MemoryDataSource.h
  MemoryUnit.h
  NodeId.h
  NodeVisitor.h
  RawDataSource.h
  SelectVisibles.h
  types.h
  VolumeInformation.h
)

set(LIVREDATA_SOURCES
  DataSource.cpp
  DataSourcePlugin.cpp
  DataSourceVisitor.cpp
  DFSTraversal.cpp
  Frustum.cpp
  LODNode.cpp
  MemoryDataSource.cpp
  MemoryUnit.cpp
  NodeId.cpp
  RawDataSource.cpp
  SelectVisibles.cpp
  VolumeInformation.cpp
)

set(LIVREDATA_LINK_LIBRARIES PUBLIC ${Boost_LIBRARIES} Lexis Lunchbox Servus vmmlib)

set(LIVREDATA_INCLUDE_NAME livre/data)
set(LIVREDATA_NAMESPACE livredata)
common_library(LivreData)
