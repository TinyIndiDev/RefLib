#pragma once

#include <stdint.h>
#include "reflib_type_def.h"

namespace RefLib
{

class CompositId
{
public:
    CompositId(uint32 id, uint32 salt=0)
        : _id(id)
        , _salt(salt)
    {
    }

    uint32 GetSlotId() const { return _id; }
    uint64 GetIndex() const { return ((_id & UINT64_MAX) << 32 | _salt); }

    // call when NetConnection is reused.
    void IncSalt()
    {
        _salt = (_salt + 1) % UINT32_MAX;
    }

    bool operator==(const CompositId& rhs) { return (_id == rhs._id && _salt == rhs._salt); }

private:
    uint32 _id;
    uint32 _salt;
};

} // namespace RefLib
