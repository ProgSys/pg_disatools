#pragma once

#include <Files/PG_ExtractorBase.h>

namespace PG {
	namespace FILE {

		class ARC : public ExtractorBase {
		public:
			using ExtractorBase::ExtractorBase;

			ARC(const PG::UTIL::File& file) {
				open(file);
			}
			ARC(const std::string& file) : ARC(PG::UTIL::File(file)) {}

			/*!
			 * @brief Opens the given ARC file.
			 * @return true on error
			 */
			EXPORT bool open(const PG::UTIL::File& file, PercentIndicator* percent = nullptr);
			/*!
			 * @brief Saves the changes done with insert or remove.
			 * @return true on error
			 */
			EXPORT bool save(const PG::UTIL::File& targetfile, PercentIndicator* percent = nullptr);

			EXPORT char const* getType() const;
		private:
			PG::UTIL::File m_file_buffer;
		};

	} /* namespace FILE */
} /* namespace PG */