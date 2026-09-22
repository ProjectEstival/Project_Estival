// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR

class UDataTable;
struct FCameraPresetData;

//Editor-only. Writes a camera preset into a Data Table row the same way the Data Table editor's own
//"Add Row" does: one undoable transaction, announced to every open Data Table editor so an open tab
//refreshes immediately, with the asset flagged as unsaved. Its only job is table persistence - it knows
//nothing about cameras or where the preset came from.
class FCameraPresetTableWriter
{
public:

	//Adds the row, or overwrites it if RowName already exists. Returns false (and writes nothing) if the table's row type is not FCameraPresetData
	static bool WriteRow(UDataTable& Table, FName RowName, const FCameraPresetData& Preset);
};

#endif
