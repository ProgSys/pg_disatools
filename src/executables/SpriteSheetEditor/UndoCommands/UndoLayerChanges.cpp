/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2020  ProgSys
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, version 3 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public License
 *	along with this program.  If not, see http://doc.qt.io/qt-5/lgpl.html
 *	or http://www.gnu.org/licenses/
 */
#include "UndoLayerChanges.h"

#include <SpriteData.h>
#include <glm/ext.hpp>

UndoLayerChanges::UndoLayerChanges(SpriteData* spriteData, Layer* layer, QUndoCommand* parent) :
	QUndoCommand("UndoLayerChanges", parent), m_spriteData(spriteData), 
	m_layer(layer)
{
	m_created = std::chrono::system_clock::now();
	QObject::connect(layer, &QObject::destroyed, &m_binder, [this]() { m_layer = nullptr; });

	for (const Keyframe* key : layer->getKeyframes()) {
		m_keyframes.push_back({
			key->getStart(),
			key->getDuration(),
			key->getCutoutID(),
			key->getColortableID(),
			key->getAnchorX(),
			key->getAnchorY(),
			key->getScaleX(),
			key->getScaleY(),
			key->getOffsetX(),
			key->getOffsetY(),
			key->getRotation(),
			key->getTransparency(),
			key->getMic(),
			});
	}

}

int UndoLayerChanges::id() const {
	return 2;
}

bool UndoLayerChanges::mergeWith(const QUndoCommand* other) {
	if (other->id() != id()) // make sure other is also an AppendText command
		return false;
	auto command = dynamic_cast<const UndoLayerChanges*>(other);
	if (!command || command->m_layer != m_layer) return false;

	const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(command->m_created - m_created).count();
	if (diff < 1000) {
		m_created = command->m_created;
		return true;
	}
	else {
		return false;
	}
}

void UndoLayerChanges::redo() {}
void UndoLayerChanges::undo() {
	if (!m_layer) return;

	//make sure numbers are the same
	m_layer->blockSignals(true);
	if (m_layer->getKeyframes().size() < m_keyframes.size()) {
		for(int i = m_keyframes.size() - m_layer->getKeyframes().size(); i >= 0; --i)
			m_layer->push_backKeyframe(new Keyframe(m_layer));

	}
	else if (m_layer->getKeyframes().size() > m_keyframes.size()) {
		for (int i = m_layer->getKeyframes().size() - m_keyframes.size() ; i >= 0; --i)
			m_layer->removeKeyframe(m_layer->getKeyframes().front());
	}

	//copy data
	auto bakedKeyIt = m_keyframes.begin();

	for (Keyframe* key : m_layer->getKeyframes()) {
		key->setStart(bakedKeyIt->start);

		key->setStart(bakedKeyIt->start);
		key->setDuration(bakedKeyIt->duration);
		key->setCutoutID(bakedKeyIt->cutoutID);
		key->setColortableID(bakedKeyIt->colortableID);
		key->setAnchorX(bakedKeyIt->anchorx);
		key->setAnchorY(bakedKeyIt->anchory);
		key->setScaleX(bakedKeyIt->scalex);
		key->setScaleY(bakedKeyIt->scaley);
		key->setOffsetX(bakedKeyIt->offsetx);
		key->setOffsetY(bakedKeyIt->offsety);
		key->setRotation(bakedKeyIt->rotation);
		key->setTransparency(bakedKeyIt->transparency);
		key->setMic(bakedKeyIt->mic);

		++bakedKeyIt;
	}
	m_layer->blockSignals(false);
	emit m_layer->onDurationChanged();
	emit m_layer->onNumberOfKeyframesChanged();
	emit m_spriteData->refresh();
};