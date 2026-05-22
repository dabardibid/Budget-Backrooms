// Copyright 2022 Mickael Daniel. All Rights Reserved.


#include "XeluIconsDeveloperSettings.h"

UXeluIconsDeveloperSettings::UXeluIconsDeveloperSettings()
{
	IconsDataTable = FSoftObjectPath(TEXT("/XeluIcons/DT_Xelu_Icons.DT_Xelu_Icons"));
}

FName UXeluIconsDeveloperSettings::GetCategoryName() const
{
	return FName("Plugins");
}
