#include "PG_DisaContext.h"

bool PG_DisaContext::getGameFlag(unsigned int flagIndex) const {
	const std::size_t byteIndex = flagIndex >> 3;
	if (byteIndex >= m_gameState.size()) return false;
	return (m_gameState[byteIndex] & (1 << (flagIndex & 7))) != 0;
}

void PG_DisaContext::setGameFlag(unsigned int flagIndex, bool value) {
	const std::size_t byteIndex = flagIndex >> 3;
	if (byteIndex >= m_gameState.size()) return;
	const unsigned char mask = 1 << (flagIndex & 7);
	if (value) m_gameState[byteIndex] |= mask;
	else m_gameState[byteIndex] &= ~mask;
	emit gameFlagChanged(flagIndex, value);
}