#include "PG_DisaEditorModule.h"

void PG_DisaEditorModule::setMode(EMode mode) {
	if (m_mode == mode) return;
	m_mode = mode;
	emit modeChanged(m_mode);
}

void PG_DisaEditorModule::setMaterialInspectorMode(EMaterialInspectorMode mode) {
	if (m_materialInspectorMode == mode) return;
	m_materialInspectorMode = mode;
	emit materialInspectorModeChanged(m_materialInspectorMode);
}