// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceTexture2D.cpp

#include "SubstanceTexture2D.h"
#include "SubstanceCorePrivatePCH.h"
#include "SubstanceInstanceFactory.h"
#include "SubstanceCoreHelpers.h"
#include "SubstanceSettings.h"
#include "substance/framework/output.h"

#include "DeviceProfiles/DeviceProfileManager.h"
#include "DeviceProfiles/DeviceProfile.h"
#include "Engine/TextureLODSettings.h"
#include "Engine/Texture2D.h"
#include "RenderUtils.h"

#if WITH_EDITOR
#include "Interfaces/ITargetPlatform.h"
#include "ObjectTools.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "IContentBrowserSingleton.h"
#endif //WITH_EDITOR

#if PLATFORM_PS4 || (SUBSTANCE_HAS_PS4_SDK && WITH_EDITOR)
#include "SubstanceCorePS4Utils.h"
#endif

USubstanceTexture2D::USubstanceTexture2D(class FObjectInitializer const& PCIP) : Super(PCIP)
{
	bLinkLegacy = false;
}

FString USubstanceTexture2D::GetDesc()
{
	return FString::Printf(TEXT("%dx%d[%s]"), SizeX, SizeY, GPixelFormats[Format].Name);
}

void USubstanceTexture2D::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	//If we aren't loading, always use most up to date serialization method
	if (!Ar.IsLoading())
	{
		Ar.UsingCustomVersion(FSubstanceCoreCustomVersion::GUID);
		SerializeCurrent(Ar);
		return;
	}

	//Check the version to see if we need to serialize legacy
	bool ShouldSerializeLegacy = false;
	if (Ar.CustomVer(FSubstanceCoreCustomVersion::GUID) < FSubstanceCoreCustomVersion::FrameworkRefactor)
	{
		//Handle Legacy Loading here
		bLinkLegacy = true;
		ShouldSerializeLegacy = true;
	}

	//Register the new version of the engine.
	Ar.UsingCustomVersion(FSubstanceCoreCustomVersion::GUID);

	//Call serialize based on version
	(ShouldSerializeLegacy == true) ? SerializeLegacy(Ar) : SerializeCurrent(Ar);
}

void USubstanceTexture2D::SerializeCurrent(FArchive& Ar)
{
	//TODO:: Remove filler before release
	int32 Filler;

	Ar << Format;
	Ar << SizeX;
	Ar << SizeY;
	Ar << NumMips;
	Ar << mUid;
	Ar << ParentInstance;
	Ar << Filler;
	Ar << mUserData.CacheGuid;

	int32 FirstMipToSerialize = 0;

	bCooked = Ar.IsCooking();
	Ar << bCooked;

	if (bCooked && !this->IsDefaultSubobject())
	{

		if (Ar.IsSaving() || Ar.IsCooking())
		{
			//The number of mipmap levels which will not be saved / regenerated during loading
			int32 MipsToRegen = 0;

			FirstMipToSerialize += FMath::Max(0, MipsToRegen);
		}

		Ar << FirstMipToSerialize;
	}

	//Special case for the ps4
	if (Ar.IsCooking() && !this->IsDefaultSubobject())
	{
#if WITH_EDITOR
		if (Ar.CookingTarget()->PlatformName() == FString(TEXT("PS4")))
			CookTextureForPS4(Ar, FirstMipToSerialize);
		else
			CookTexture2D(Ar, FirstMipToSerialize);
#endif
	}
	else if (Ar.IsSaving() && !Ar.IsTransacting() && !this->IsDefaultSubobject())
	{
		CookTexture2D(Ar, FirstMipToSerialize);
	}

	if (Ar.IsLoading())
	{
		NumMips -= FirstMipToSerialize;
		Mips.Empty(NumMips);

		for (int32 MipIndex = 0; MipIndex < NumMips; ++MipIndex)
		{
			Mips.Add(new FTexture2DMipMap);
			SerializeTexture2DMip(Mips[MipIndex], Ar, this, MipIndex);
		}
	}

	//Associate this asset with the current plugin version
	Ar.UsingCustomVersion(FSubstanceCoreCustomVersion::GUID);
	Ar.SetCustomVersion(FSubstanceCoreCustomVersion::GUID, FSubstanceCoreCustomVersion::LatestVersion, FName("LegacyUpdated"));
}

void USubstanceTexture2D::SerializeLegacy(FArchive& Ar)
{
	Ar << Format;
	Ar << SizeX;
	Ar << SizeY;
	Ar << NumMips;
	Ar << mUserData.CacheGuid;
	Ar << ParentInstance;

	int32 FirstMipToSerialize = 0;

	bCooked = Ar.IsCooking();
	Ar << bCooked;

	if (bCooked && !this->IsDefaultSubobject())
	{
		Ar << FirstMipToSerialize;
	}

	NumMips -= FirstMipToSerialize;
	Mips.Empty(NumMips);

	for (int32 MipIndex = 0; MipIndex < NumMips; ++MipIndex)
	{
		Mips.Add(new FTexture2DMipMap);
		SerializeTexture2DMip(Mips[MipIndex], Ar, this, MipIndex);
	}

	//Clear the archive to be updated with the new serialization on save.
	Ar.FlushCache();

	//Forcing package dirty
	UPackage* Package = GetOutermost();
	Package->SetDirtyFlag(true);
}

void USubstanceTexture2D::CookTextureForPS4(FArchive& Ar, int32 FirstMip)
{

}

void USubstanceTexture2D::CookTexture2D(FArchive& Ar, int32 FirstMip)
{

}

void USubstanceTexture2D::SerializeTexture2DMip(FTexture2DMipMap& mip, FArchive& Ar, UObject* Owner, int32 MipIdx)
{
	bool bMipCooked = Ar.IsCooking();
	Ar << bMipCooked;

	mip.BulkData.Serialize(Ar, Owner, MipIdx);
	Ar << mip.SizeX;
	Ar << mip.SizeY;

#if WITH_EDITORONLY_DATA
	if (!bMipCooked)
	{
		Ar << mip.DerivedDataKey;
	}
#endif // #if WITH_EDITORONLY_DATA
}

void USubstanceTexture2D::BeginDestroy()
{
	//Route BeginDestroy.
	Super::BeginDestroy();

	if (OutputCopy)
	{
		//Reset output user data
		OutputCopy->mUserData = 0;

		//Nullify the pointer to this texture's address so that others see it has been destroyed
		mUserData.Texture.Reset();

		//Disable the output in the parent instance
		if (ParentInstance && ParentInstance->Instance)
		{
			Substance::Helpers::ClearFromRender(ParentInstance);

			auto ItOut = ParentInstance->Instance->getOutputs().begin();
			for (; ItOut != ParentInstance->Instance->getOutputs().end(); ++ItOut)
			{
				if ((*ItOut)->mDesc.mUid == mUid)
				{
					(*ItOut)->mEnabled = false;
					break;
				}
			}
		}
	}
}

#if WITH_EDITOR
bool USubstanceTexture2D::CanEditChange(const FProperty* InProperty) const
{
	bool bIsEditable = Super::CanEditChange(InProperty);

	return bIsEditable;
}
#endif

void USubstanceTexture2D::LinkOutputInstance()
{
	
}

void USubstanceTexture2D::LinkLegacyOutputInstance()
{
	
}

void USubstanceTexture2D::PostLoad()
{
	Super::PostLoad();
}

void USubstanceTexture2D::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
}

void USubstanceTexture2D::GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize)
{
	
}

FTextureResource* USubstanceTexture2D::CreateResource()
{
	return nullptr;
}

void USubstanceTexture2D::UpdateResource()
{
	Super::UpdateResource();
}
