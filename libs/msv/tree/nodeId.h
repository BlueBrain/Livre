
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__NODE_ID_H
#define MASS_VOL__NODE_ID_H

#include <stdint.h>
#include <vector>
#include <msv/types/types.h>

typedef uint32_t NodeId;


struct NodeIdPos
{
    NodeIdPos() : id(0), treePos(0), rank(0) {}

    NodeIdPos( NodeId id_, uint32_t treePos_, byte rank_ )
        : id(  id_  )
        , treePos( treePos_ )
        , rank(    rank_    ){}

    NodeId   id;
    uint32_t treePos;
    byte     rank;

    bool operator == ( const NodeIdPos& other ) const
    {
        return id      == other.id      &&
               treePos == other.treePos;
    }
};

typedef std::vector<NodeId>     NodeIdVec;
typedef std::vector<NodeIdPos>  NodeIdPosVec;

#endif // MASS_VOL__NODE_ID_H
