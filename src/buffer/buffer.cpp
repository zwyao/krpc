#include "buffer.h"

namespace knet { namespace util {

FreeDeallocator Buffer::_free_deallocator;
DeleteDeallocator Buffer::_delete_deallocator;
DonothingDeallocator Buffer::_donothing_deallocator;

}}

