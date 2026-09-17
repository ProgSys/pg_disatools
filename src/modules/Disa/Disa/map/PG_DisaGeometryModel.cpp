#include "PG_DisaGeometryModel.h"

#include <Scene/Model/PG_ListModelBase.h>

namespace {

void notifyParent(PG_DisaGeometryModel* self, const PG_IdentitySet& identities) {
	if (auto* base = self->getParent<PG_ListModelBase>(); base && self->getLocation() >= 0) {
		emit base->itemsChanged(self->getLocation(), self->getLocation(), identities);
	}
}

} //namespace

void PG_DisaGeometryModel::setResource(const std::string& value) {
	if (m_resource == value) return;
	m_resource = value;
	emit resourceChanged();
	notifyParent(this, { Identity_Resource });
}

void PG_DisaGeometryModel::setScale(float value) {
	if (m_scale == value) return;
	m_scale = value;
	emit scaleChanged();
	notifyParent(this, { Identity_Scale });
}

PG_Variety PG_DisaGeometryModel::getData(Identity identity) const {
	switch (identity) {
	case Identity_Resource: return m_resource;
	case Identity_Scale: return m_scale;
	}
	return PG_DisaActorModel::getData(identity);
}

bool PG_DisaGeometryModel::setData(const PG_Variety& value, Identity identity) {
	switch (identity) {
	case Identity_Resource: {
		if (auto v = value.convert<std::string>(); v != m_resource) {
			setResource(v);
			return true;
		}
	}break;
	case Identity_Scale: {
		if (auto v = value.convert<float>(); v != m_scale) {
			setScale(v);
			return true;
		}
	}break;
	}
	return PG_DisaActorModel::setData(value, identity);
}
