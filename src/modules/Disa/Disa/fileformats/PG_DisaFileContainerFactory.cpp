#include <Disa/fileformats/PG_DisaFileContainerFactory.h>

#include <Disa/fileformats/PG_DisaFileContainer.h>
#include <Disa/fileformats/PG_OLA.h>
#include <Disa/fileformats/PG_SOLA.h>
#include <Disa/fileformats/PG_KM3.h>
#include <Disa/fileformats/PG_PSPFS.h>
#include <Disa/fileformats/PG_Script.h>
#include <Disa/fileformats/PG_TalkScript.h>

namespace PG_Disa {

	std::shared_ptr<PG_DisaFileContainer> createFileContainer(PG_DisaFileInfo::Type type) {
		switch (type) {
		case PG_DisaFileInfo::PSPFS_V1: return std::shared_ptr<PG_PSPFS>(PG_new<PG_PSPFS>("PG_PSPFS"));
		case PG_DisaFileInfo::OLA: return std::shared_ptr<PG_OLA>(PG_new<PG_OLA>("PG_OLA"));
		case PG_DisaFileInfo::SOLA: return std::shared_ptr<PG_SOLA>(PG_new<PG_SOLA>("PG_SOLA"));
		case PG_DisaFileInfo::KM3: return std::shared_ptr<PG_KM3>(PG_new<PG_KM3>("PG_KM3"));
		case PG_DisaFileInfo::SCRIPT: return std::shared_ptr<PG_Script>(PG_new<PG_Script>("PG_Script"));
		case PG_DisaFileInfo::TALK_SCRIPT: return std::shared_ptr<PG_TalkScript>(PG_new<PG_TalkScript>("PG_TalkScript"));
		default: break;
		}
		return {};
	}

	std::shared_ptr<PG_DisaFileContainer> createFileContainer(PG::STREAM::In* reader) {
		if (!reader) return {};
		return createFileContainer(PG_DisaFileInfo::typeFromFileFormat(reader));
	}

} //namespace PG_Disa
