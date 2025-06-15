#pragma once
#include <string>
#include <vector>

class PG_ListModelBase;

class PG_UiStateBaseModel {
public:
	enum UiState {
		Empty,
		FileArchive,
	};

	PG_UiStateBaseModel() = default;

	virtual UiState getUiState() const = 0;
	virtual PG_ListModelBase* getListModel() const { return nullptr; }
	virtual void openUi() {};
	virtual void closeUi() {};
};