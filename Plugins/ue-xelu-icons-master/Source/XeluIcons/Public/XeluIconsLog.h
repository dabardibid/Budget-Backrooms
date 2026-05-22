// Copyright 2022 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

XELUICONS_API DECLARE_LOG_CATEGORY_EXTERN(LogXeluIcons, Display, All);

#define XI_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogXeluIcons, Verbosity, Format, ##__VA_ARGS__); \
}
