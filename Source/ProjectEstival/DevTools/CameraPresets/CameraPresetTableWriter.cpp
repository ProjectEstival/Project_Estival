// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraPresetTableWriter.h"

#if WITH_EDITOR

#include "CameraPresetData.h"
#include "DataTableEditorUtils.h"
#include "Engine/DataTable.h"
#include "ScopedTransaction.h"

bool FCameraPresetTableWriter::WriteRow(UDataTable& Table, FName RowName, const FCameraPresetData& Preset)
{
	// UDataTable::AddRow copies using the table's own row layout, so any other row type would read past our struct
	if (Table.GetRowStruct() != FCameraPresetData::StaticStruct())
	{
		return false;
	}

	const bool bIsNewRow = !Table.GetRowMap().Contains(RowName);
	const FDataTableEditorUtils::EDataTableChangeInfo ChangeInfo = bIsNewRow
		? FDataTableEditorUtils::EDataTableChangeInfo::RowList
		: FDataTableEditorUtils::EDataTableChangeInfo::RowData;

	const FScopedTransaction Transaction(NSLOCTEXT("CameraPresetTableWriter", "WriteRow", "Save Camera Preset"));

	FDataTableEditorUtils::BroadcastPreChange(&Table, ChangeInfo);
	Table.Modify();
	Table.AddRow(RowName, Preset);
	FDataTableEditorUtils::BroadcastPostChange(&Table, ChangeInfo);

	return true;
}

#endif
