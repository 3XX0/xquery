#pragma once

#include <libxml++/libxml++.h>

namespace xmlpp
{
typedef std::vector<const Element*>  Elements;
}

namespace xquery
{
namespace xml = xmlpp;
}
