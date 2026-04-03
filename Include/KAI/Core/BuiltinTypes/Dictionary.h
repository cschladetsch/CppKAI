#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Object/Label.h>

#include <map>

KAI_BEGIN

	// TODO: use unordered map
typedef std::map<Label, Object> Dictionary;

KAI_END
