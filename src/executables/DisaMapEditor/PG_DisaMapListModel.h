#pragma once

#include <Scene/Model/PG_ListModel.h>
#include <Util/PG_File.h>
#include <Disa/fileformats/PG_DisaFileInfo.h>
#include <Disa/fileformats/PG_PSPFS.h>
#include <Disa/fileformats/PG_DisaMPDPack.h>

class PG_DisaMapListModel: public PG_ListModel<PG_DisaMPDPack> {
	PG_OBJECT(PG_DisaMapListModel);
	PG_PARENT(PG_ListModel<PG_DisaMPDPack>);

	PG_PROPERTY(bool hasChanges GET hasChanges CHANGED hasChangesChanged)
	PG_PROPERTY(bool enabled GET isEnabled SET setEnabled CHANGED enabledChanged)
signals:
	void hasChangesChanged();
	void enabledChanged();
public:
	enum ECustomIdentity : Identity {
		Identity_CanGLTFExport = EIdentity::PG_Identity_Custom,
		Identity_CanJSONExport,
		Identity_HasChanges,
		Identity_File,
	};
	using PG_ListModel<PG_DisaMPDPack>::PG_ListModel;
	void open(const PG_File& file);

	bool hasChanges() const { return false; }
	void save();
	void save(const PG_File& file);

	bool isEnabled() const { return m_enabled; }
	void setEnabled(bool enabled) {
		if(m_enabled != enabled){
			m_enabled = enabled;
			emit enabledChanged();
		}
	}

	PG_Variety getData(PG_Index index, Identity identity) const override;

	void exportData(PG_ListModelLocation location);
	void importData(PG_ListModelLocation location);

	void exportJSON(PG_ListModelLocation location) const;
	void importJSON(PG_ListModelLocation location);


	inline const std::shared_ptr<PG_PSPFS>& getFileSystem() const { return m_fileSystem; };
private:
	void parseFileSystem();

	std::shared_ptr<PG_PSPFS> m_fileSystem;
	bool m_enabled = true;
};