#pragma once

#include <memory>
#include <Util/PG_File.h>
#include <Util/PG_Dir.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>

class zip;
using zipUPtr = std::unique_ptr<zip, void(*)(zip*)>;
using zipSPtr = std::shared_ptr<zip>;

class PG_ZIP : public PG_DisaFileContainer {
public:
	using PG_DisaFileContainer::PG_DisaFileContainer;
	using PG_DisaFileInterface::open;
	bool open(const PG_File& file) override;
	bool open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) override;
	bool save(std::shared_ptr<PG::STREAM::Out> writer) const override;
	bool refresh(std::shared_ptr<PG::STREAM::In> reader) override;
	bool save() override;
	

	std::shared_ptr<PG::STREAM::In> createEntryReaderOriginal(const PG_DisaFileEntry& entry) const override;
	std::shared_ptr<PG::STREAM::In> createReaderFromZip(zip* zip, const PG_DisaFileEntry& entry) const;

	static bool unizipToFolder(const PG_File& zipFile, const PG_Dir& dir);

protected:
	virtual void afterOpen(zipUPtr& zip) { /*empty*/ };

private:
	mutable zip* m_originalZip = nullptr;
};